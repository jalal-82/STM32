/*
 * signals.c
 *
 *  Created on: May 18, 2025
 *      Author: jalal
 */


#include "signals.h"
#include "i2c-lcd.h"
#include <stdio.h>
#include <string.h>
#include "recording.h"

// Private variables
static PotRecordingData potData;
static UltraRecordingData ultraData;
static TIM_HandleTypeDef *potTimer;
static TIM_HandleTypeDef *ultraTimer;
static ADC_HandleTypeDef *adc;
static uint8_t potSampleCounter = 0;

// Initialize all signal handling
void Signals_Init(TIM_HandleTypeDef *htim_pot, TIM_HandleTypeDef *htim_ultra, ADC_HandleTypeDef *hadc) {
    // Store handlers
    potTimer = htim_pot;
    ultraTimer = htim_ultra;
    adc = hadc;

    // Initialize sensors
    Potentiometer_Init(adc);
    Ultrasonic_Init(ultraTimer);

    // Initialize recording structures
    PotRecording_Init(&potData);
    UltraRecording_Init(&ultraData);

    // Start the potentiometer timer for consistent sampling
    HAL_TIM_Base_Start_IT(potTimer);
}

// Handle timer interrupts
void Signals_HandleTimerInterrupt(TIM_HandleTypeDef *htim) {
    static float lastVoltage = 0.0f;

    if (htim->Instance == potTimer->Instance) {
        // This triggers at 1kHz
        potSampleCounter++;

        // We want 100Hz sampling rate (every 10 interrupts)
        if (potSampleCounter >= 10) {
            potSampleCounter = 0;

            // Sample potentiometer with safety checks
            float voltage = Potentiometer_GetVoltage();

            // Add more defensive checking
            if (voltage >= 0.0f && voltage <= 3.3f) {
                // Store for debugging
                lastVoltage = voltage;

                // Process if recording
                if (potData.isRecording) {
                    PotRecording_Process(&potData, voltage);
                }
            }
        }
    }
}


// Start recording on both sensors
void Signals_StartRecording(void) {
    PotRecording_Start(&potData);
    UltraRecording_Start(&ultraData);
}

// Stop recording on both sensors
void Signals_StopRecording(void) {
    PotRecording_Stop(&potData);
    UltraRecording_Stop(&ultraData);
}

// Check if recording
uint8_t Signals_IsRecording(void) {
    return potData.isRecording;
}

// Display potentiometer view
void Signals_DisplayPotView(void) {
    // Take a snapshot of the data for consistent display
    __disable_irq();
    PotRecordingData localData = potData;
    __enable_irq();

    char buffer[20];

    // Clear LCD
    lcd_clear();

    // Title for view identification
    lcd_send_string("ADC View");

    if (!localData.isRecording) {
        lcd_send_cmd(LCD_LINE2, 4);
        lcd_send_string("Press B1 to start");
        return;
    }

    // First line: Time & Peaks
    lcd_send_cmd(LCD_LINE1, 4);
    lcd_send_string("T:");
    sprintf(buffer, "%lu.%lus", localData.elapsedTime / 1000, (localData.elapsedTime % 1000) / 100);
    lcd_send_string(buffer);

    lcd_send_string(" P:");
    sprintf(buffer, "%lu", localData.peakCount);
    lcd_send_string(buffer);

    // Second line: Min/Max voltage
    lcd_send_cmd(LCD_LINE2, 4);

    // Check if we have valid min/max data
    if (localData.minVoltage > localData.maxVoltage) {
        lcd_send_string("No data yet");
    } else {
        sprintf(buffer, "%.2f", localData.minVoltage);
        lcd_send_string(buffer);
        lcd_send_string("V-");
        sprintf(buffer, "%.2fV", localData.maxVoltage);
        lcd_send_string(buffer);
    }
}
// Display ultrasonic view
void Signals_DisplayUltraView(void) {
    // Sample ultrasonic here (lower rate is fine for ultrasonic)
    if (ultraData.isRecording) {
        float distance_mm = Ultrasonic_GetDistance();
        UltraRecording_Process(&ultraData, distance_mm);
    }

    // Display the stats
    UltraRecording_DisplayStats(&ultraData);
}

// Debug access function - only compile in debug mode
void Signals_DebugOutput(void) {
    // Get the raw ADC value and voltage directly
    uint16_t rawValue = Potentiometer_Read();
    float direct_voltage = ((float)rawValue * 3.3f) / 4095.0f;

    PotDebugData debugData;
    PotRecording_GetDebugData(&potData, &debugData);

    char buffer[150];
    sprintf(buffer, "Raw: %u, Direct V: %.2f, Reported V: %.2f, Min: %.2f, Max: %.2f, Peaks: %lu\r\n",
           rawValue, direct_voltage, debugData.currentVoltage, debugData.minVoltage,
           debugData.maxVoltage, debugData.peakCount);

    extern UART_HandleTypeDef huart2;
    HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 10);
}

