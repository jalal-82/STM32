/*
 * ultrasonicRecording.c
 *
 *  Created on: May 31, 2025
 *      Author: jalal
 */

#include "ultrasonicRecording.h"
#include "i2c-lcd.h"
#include <stdio.h>
#include <string.h>
#include "led_control.h"
#include "ultrasonic.h"

/**
 * Initialize ultrasonic recording data structure
 * Sets all values to their initial state for a new recording session
 */
void UltraRecording_Init(UltraRecordingData *data) {
    data->startTime = 0;
    data->elapsedTime = 0;
    data->minDistance = 5000.0f;  // Initialize to a high value so first reading becomes minimum
    data->maxDistance = 0.0f;     // Initialize to a low value so first reading becomes maximum
    data->dirChangeCount = 0;
    data->isRecording = 0;
    data->lastDistance = 0.0f;
    data->lastDirection = 0;      // 0 = no direction, 1 = increasing distance, -1 = decreasing distance
    data->totalDistanceSum = 0.0f;
    data->validDistanceCount = 0;
    data->dirChangeWhenHighSignalCount = 0;
}

/**
 * Start recording ultrasonic data
 * Resets all statistics and begins a new recording session
 */
void UltraRecording_Start(UltraRecordingData *data) {
    data->startTime = HAL_GetTick();
    data->isRecording = 1;
    data->dirChangeCount = 0;
    data->minDistance = 5000.0f;
    data->maxDistance = 0.0f;
    data->lastDistance = 0.0f;
    data->lastDirection = 0;
    data->totalDistanceSum = 0.0f;
    data->validDistanceCount = 0;
    data->dirChangeWhenHighSignalCount = 0;
}

/**
 * Stop recording ultrasonic data
 * Marks the recording as stopped but preserves all collected statistics
 */
void UltraRecording_Stop(UltraRecordingData *data) {
    data->isRecording = 0;
}

/**
 * Process a new ultrasonic distance reading
 * Updates statistics, detects direction changes, and manages LED based on proximity
 * 
 * @param data Pointer to the ultrasonic recording data structure
 * @param distance Current distance reading in millimeters
 */
void UltraRecording_Process(UltraRecordingData *data, float distance) {
    // Skip processing if recording is inactive or distance reading is invalid
    if (!data->isRecording || distance < 0) return;
    
    // Update elapsed time
    data->elapsedTime = HAL_GetTick() - data->startTime;

    // Skip the first reading as we need at least two readings to detect direction changes
    if (data->lastDistance == 0.0f) {
        data->lastDistance = distance;
        return;
    }

    // Update statistics for valid distance readings only
    if (distance >= ULTRASONIC_MIN_RANGE_MM && distance <= ULTRASONIC_MAX_RANGE_MM) {
        data->totalDistanceSum += distance;
        data->validDistanceCount++;
        
        // Update min/max distance
        if (distance < data->minDistance) {
            data->minDistance = distance;
        }
        if (distance > data->maxDistance) {
            data->maxDistance = distance;
        }
    }

    // Direction change detection logic
    int8_t currentDirection = 0;  // 0 = no significant change, 1 = increasing, -1 = decreasing
    float deadband = 10.0f;       // Deadband of 10mm to filter out noise and minor fluctuations
    
    // Determine current direction of movement based on change since last reading
    if (distance > data->lastDistance + deadband) {
        // Object is moving away (distance increasing beyond deadband)
        currentDirection = 1;
    } else if (distance < data->lastDistance - deadband) {
        // Object is moving closer (distance decreasing beyond deadband)
        currentDirection = -1;
    }
    // If change is within deadband, currentDirection remains 0 (no significant change)
    
    // Detect a direction change when:
    // 1. We have a previous direction (lastDirection != 0)
    // 2. Current reading shows significant movement (currentDirection != 0)
    // 3. Direction is different from last time (currentDirection != lastDirection)
    if (data->lastDirection != 0 && currentDirection != 0 && currentDirection != data->lastDirection) {
        data->dirChangeCount++;  // Increment direction change counter
    }

    // Update last distance for next comparison
    data->lastDistance = distance;
    
    // Update lastDirection only if we have a significant movement
    // This prevents noise from affecting direction change detection
    if (currentDirection != 0) {
        data->lastDirection = currentDirection;
    }

    // LED control based on proximity
    if (distance < LED_GetProximityThreshold()) {
        // Object is within proximity threshold - blink LED if not already flashing for a peak
        if (LED_GetMode() != LED_PEAK_FLASH) {
            LED_SetMode(LED_PROXIMITY_BLINK);
        }
    } else if (LED_GetMode() == LED_PROXIMITY_BLINK) {
        // Object is outside proximity threshold - turn off LED if it was blinking for proximity
        LED_SetMode(LED_OFF);
    }
}

/**
 * Display basic ultrasonic statistics on LCD
 * Shows recording time, direction changes count, and min/max distance
 */
void UltraRecording_DisplayStats(UltraRecordingData *data) {
    char buffer[20];

    // Clear LCD
    lcd_clear();

    if (!data->isRecording) {
        lcd_send_string("Press B1");
        lcd_send_cmd(LCD_LINE2, 4);
        lcd_send_string("to start");
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


