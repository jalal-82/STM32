#include "recording.h"
#include "i2c-lcd.h"
#include <stdio.h>
#include <string.h>

/* Potentiometer recording functions */
void PotRecording_Init(PotRecordingData *data) {
    data->startTime = 0;
    data->elapsedTime = 0;
    data->minVoltage = 3.3f;  // Initialize to max possible value
    data->maxVoltage = 0.0f;  // Initialize to min possible value
    data->peakCount = 0;
    data->isRecording = 0;
    data->lastVoltage = 0.0f;
    data->risingEdge = 0;
    data->peakThreshold = 0.1f;  // 10% of range as threshold for peaks
}

void PotRecording_Start(PotRecordingData *data) {
    data->startTime = HAL_GetTick();
    data->isRecording = 1;
    data->peakCount = 0;
    // Start with more reasonable initial values
    data->minVoltage = 3.3f;  // Start high so any reading will become the new minimum
    data->maxVoltage = 0.0f;  // Start low so any reading will become the new maximum
    data->lastVoltage = -1.0f; // Invalid initial value to force first reading comparison
    data->risingEdge = 0;
    data->elapsedTime = 0;
}
void PotRecording_Stop(PotRecordingData *data) {
    data->isRecording = 0;
}

void PotRecording_Process(PotRecordingData *data, float voltage) {
    if (!data->isRecording) {
        return;
    }

    // Debug output
    // For testing only - remember to remove this
     char buffer[50];
//     sprintf(buffer, "V: %.2f, Min: %.2f, Max: %.2f\r\n", voltage, data->minVoltage, data->maxVoltage);
//     HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 10);

    // Update elapsed time
    data->elapsedTime = HAL_GetTick() - data->startTime;

    // Validate input (safety check)
    if (voltage < 0.0f || voltage > 3.4f) {
        return; // Skip invalid readings
    }

    // Update min/max voltage
    if (voltage < data->minVoltage) {
        data->minVoltage = voltage;
    }
    if (voltage > data->maxVoltage) {
        data->maxVoltage = voltage;
    }

    // Revised peak detection based on professor's clarification:
    float noiseThreshold = 0.1f;  // 100mV, adjust based on your testing
    float minPeakHeight = 0.2f;   // Minimum height for a peak to be counted

    // Special case for first reading
    if (data->lastVoltage < 0.0f) {
        data->lastVoltage = voltage;
        return;
    }

    // Only look for peaks when we have enough signal amplitude
    if (data->maxVoltage - data->minVoltage >= minPeakHeight) {
        // Track rising and falling edges with hysteresis (noise reduction)
        if (!data->risingEdge) {
            // Look for rising edge
            if (voltage > data->lastVoltage + noiseThreshold) {
                data->risingEdge = 1;
            }
        } else {
            // Already on rising edge, look for falling edge (peak)
            if (voltage < data->lastVoltage - noiseThreshold) {
                // Verify the peak is significant enough
                if (data->lastVoltage >= data->minVoltage + minPeakHeight) {
                    // We have a peak! (rise then fall)
                    data->peakCount++;
                }
                data->risingEdge = 0;
            }
        }
    }

    // Store current voltage for next comparison
    data->lastVoltage = voltage;
}

void PotRecording_DisplayStats(PotRecordingData *data) {
    char buffer[20];

    // Clear LCD
    lcd_clear();

    // Title for view identification
    lcd_send_string("ADC View");

    if (!data->isRecording) {
        lcd_send_cmd(LCD_LINE2, 4);
        lcd_send_string("Press B1 to start");
        return;
    }

    // First line: Time & Peaks
    lcd_send_cmd(LCD_LINE1, 4);  // Return to beginning of first line
    lcd_send_string("T:");
    sprintf(buffer, "%lu.%lus", data->elapsedTime / 1000, (data->elapsedTime % 1000) / 100);
    lcd_send_string(buffer);

    lcd_send_string(" P:");
    sprintf(buffer, "%lu", data->peakCount);
    lcd_send_string(buffer);

    // Second line: Min/Max voltage
    lcd_send_cmd(LCD_LINE2, 4);
    sprintf(buffer, "%.1f", data->minVoltage);
    lcd_send_string(buffer);
    lcd_send_string("V-");
    sprintf(buffer, "%.1fV", data->maxVoltage);
    lcd_send_string(buffer);
}

/* Ultrasonic recording functions */
void UltraRecording_Init(UltraRecordingData *data) {
    data->startTime = 0;
    data->elapsedTime = 0;
    data->minDistance = 5000.0f;  // Initialize to a large value (5m)
    data->maxDistance = 0.0f;     // Initialize to a small value
    data->dirChangeCount = 0;
    data->isRecording = 0;
    data->lastDistance = 0.0f;
    data->lastDirection = 0;      // Start with no direction
}

void UltraRecording_Start(UltraRecordingData *data) {
    data->startTime = HAL_GetTick();
    data->isRecording = 1;
    data->dirChangeCount = 0;
    data->minDistance = 5000.0f;  // 5m as initial max
    data->maxDistance = 0.0f;
    data->lastDistance = 0.0f;
    data->lastDirection = 0;
}

void UltraRecording_Stop(UltraRecordingData *data) {
    data->isRecording = 0;
}

void UltraRecording_Process(UltraRecordingData *data, float distance) {
    if (!data->isRecording || distance < 0) {  // Skip if not recording or error
        return;
    }

    // Update elapsed time
    data->elapsedTime = HAL_GetTick() - data->startTime;

    // Skip the first reading to initialize last distance
    if (data->lastDistance == 0.0f) {
        data->lastDistance = distance;
        return;
    }

    // Update min/max distance
    if (distance < data->minDistance) {
        data->minDistance = distance;
    }
    if (distance > data->maxDistance) {
        data->maxDistance = distance;
    }

    // Determine current direction (with a small deadband to reduce noise)
    int8_t currentDirection = 0;
    float deadband = 15.0f;  // 5mm deadband

    if (distance > data->lastDistance + deadband) {
        currentDirection = 1;  // Moving away
    } else if (distance < data->lastDistance - deadband) {
        currentDirection = -1; // Moving closer
    }

    // Detect direction change
    if (data->lastDirection != 0 && currentDirection != 0 &&
        currentDirection != data->lastDirection) {
        data->dirChangeCount++;
    }

    // Update last values for next comparison
    data->lastDistance = distance;
    if (currentDirection != 0) {  // Only update if not in deadband
        data->lastDirection = currentDirection;
    }

    extern UART_HandleTypeDef huart2;
    char debug[100];
    sprintf(debug, "Dist: %.1f, Last: %.1f, CurDir: %d, LastDir: %d, Changes: %lu\r\n",
            distance, data->lastDistance, currentDirection, data->lastDirection, data->dirChangeCount);
    HAL_UART_Transmit(&huart2, (uint8_t*)debug, strlen(debug), 10);
}

void UltraRecording_DisplayStats(UltraRecordingData *data) {
    char buffer[20];

    // Clear LCD
    lcd_clear();

    // Title for view identification
    lcd_send_string("Ultra View");

    if (!data->isRecording) {
        lcd_send_cmd(LCD_LINE2, 4);
        lcd_send_string("Press B1 to start");
        return;
    }

    // First line: Time & Direction Changes
    lcd_send_cmd(LCD_LINE1, 4);  // Return to beginning of first line
    lcd_send_string("T:");
    sprintf(buffer, "%lu.%lus", data->elapsedTime / 1000, (data->elapsedTime % 1000) / 100);
    lcd_send_string(buffer);

    lcd_send_string(" C:");
    sprintf(buffer, "%lu", data->dirChangeCount);
    lcd_send_string(buffer);

    // Second line: Min/Max distance in mm
    lcd_send_cmd(LCD_LINE2, 4);

    // Check if we have valid data
    if (data->minDistance > data->maxDistance) {
        lcd_send_string("No valid data");
    } else {
        sprintf(buffer, "%.0f", data->minDistance);
        lcd_send_string(buffer);
        lcd_send_string("-");
        sprintf(buffer, "%.0fmm", data->maxDistance);
        lcd_send_string(buffer);
    }
}


void PotRecording_GetDebugData(PotRecordingData *data, PotDebugData *debugData) {
    if (debugData != NULL) {
        debugData->currentVoltage = data->lastVoltage;
        debugData->minVoltage = data->minVoltage;
        debugData->maxVoltage = data->maxVoltage;
        debugData->peakCount = data->peakCount;
    }
}
