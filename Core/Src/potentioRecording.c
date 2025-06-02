/*
 * potentioRecording.c
 *
 *  Created on: May 31, 2025
 *      Author: jalal
 */

#include "potentioRecording.h"
#include "i2c-lcd.h"
#include <stdio.h>
#include <string.h>
#include "led_control.h"

// Global variable for midpoint crossing detection
// Tracks which side of the fixed 1.65V midpoint the voltage is on
uint8_t was_above_midpoint = 0;

/**
 * Initialize potentiometer recording data structure
 * Sets all values to their initial state for a new recording session
 */
void PotRecording_Init(PotRecordingData *data) {
    data->startTime = 0;
    data->elapsedTime = 0;
    data->minVoltage = 3.3f;    // Initialize to max possible so first reading becomes minimum
    data->maxVoltage = 0.0f;    // Initialize to min possible so first reading becomes maximum
    data->peakCount = 0;
    data->isRecording = 0;
    data->lastVoltage = 0.0f;   // Used for peak detection
    data->risingEdge = 0;       // Flag to track if voltage is rising (for peak detection)
    data->peakThreshold = 0.1f; // 10% of range as threshold for peaks
    data->timeAbove95Pct = 0;   // Time spent above 95% of the voltage range
    data->timeBelow5Pct = 0;    // Time spent below 5% of the voltage range
    data->crossings50Pct = 0;   // Number of times the voltage crosses the 50% level
    data->lastPeakTime = 0;     // Timestamp of the last detected peak
    data->totalPeakInterval = 0;// Sum of time intervals between consecutive peaks
    data->peaksWhenCloseCount = 0; // Peaks detected when object is close to ultrasonic sensor
    // Initialize alias members
    data->midCrossingCount = 0;
    data->timeAboveHighThreshold = 0;
    data->timeBelowLowThreshold = 0;
    data->peakCountWhenClose = 0;
}

/**
 * Start recording potentiometer data
 * Resets all statistics and begins a new recording session
 */
void PotRecording_Start(PotRecordingData *data) {
    data->startTime = HAL_GetTick();
    data->isRecording = 1;
    data->peakCount = 0;
    data->minVoltage = 3.3f;
    data->maxVoltage = 0.0f;
    data->lastVoltage = -1.0f;  // Set to invalid value to force initialization on first reading
    data->risingEdge = 0;
    data->elapsedTime = 0;
    data->timeAbove95Pct = 0;
    data->timeBelow5Pct = 0;
    data->crossings50Pct = 0;
    data->lastPeakTime = 0;
    data->totalPeakInterval = 0;
    data->peaksWhenCloseCount = 0;
    // Initialize alias members
    data->midCrossingCount = 0;
    data->timeAboveHighThreshold = 0;
    data->timeBelowLowThreshold = 0;
    data->peakCountWhenClose = 0;
    
    // Reset the midpoint tracking
    was_above_midpoint = 0;
}

/**
 * Stop recording potentiometer data
 * Marks the recording as stopped but preserves all collected statistics
 */
void PotRecording_Stop(PotRecordingData *data) {
    data->isRecording = 0;
}

/**
 * Process a new potentiometer voltage reading
 * Updates statistics, detects peaks, and manages time-related metrics
 * Uses rising and falling edge detection with a threshold for peak detection
 */
void PotRecording_Process(PotRecordingData *data, float voltage) {
    // Skip processing if recording is not active
    if (!data->isRecording) {
        return;
    }
    
    // Update elapsed time
    uint32_t currentTime = HAL_GetTick();
    data->elapsedTime = currentTime - data->startTime;

    // Update min/max voltage
    if (voltage < data->minVoltage) data->minVoltage = voltage;
    if (voltage > data->maxVoltage) data->maxVoltage = voltage;

    // Calculate voltage range thresholds for statistics
    float range = data->maxVoltage - data->minVoltage;
    if (range > 0.01f) {  // Only calculate thresholds if we have a meaningful voltage range
        float threshold5pct = data->minVoltage + (range * 0.05f);   // 5% level
        float threshold95pct = data->minVoltage + (range * 0.95f);  // 95% level
        
        static uint32_t lastSampleTime = 0;
        
        // Track time spent above 95% of voltage range
        if (voltage > threshold95pct) {
            if (lastSampleTime > 0) {
                uint32_t timeSpentHere = currentTime - lastSampleTime;
                data->timeAbove95Pct += timeSpentHere;
                data->timeAboveHighThreshold = data->timeAbove95Pct; // Keep alias in sync
            }
        }
        
        // Track time spent below 5% of voltage range
        if (voltage < threshold5pct) {
            if (lastSampleTime > 0) {
                uint32_t timeSpentHere = currentTime - lastSampleTime;
                data->timeBelow5Pct += timeSpentHere;
                data->timeBelowLowThreshold = data->timeBelow5Pct; // Keep alias in sync
            }
        }
        
        // Simple midpoint crossing detection using fixed 1.65V threshold
        uint8_t is_above_midpoint = (voltage > FIXED_MIDPOINT_VOLTAGE) ? 1 : 0;
        
        // Detect a crossing (change in which side of midpoint we're on)
        if (is_above_midpoint != was_above_midpoint) {
            // We crossed the midpoint - increment the counter
            data->crossings50Pct++;
            data->midCrossingCount = data->crossings50Pct; // Keep alias in sync
        }
        
        // Update for next comparison
        was_above_midpoint = is_above_midpoint;
        
        lastSampleTime = currentTime;
    }

    // Skip first reading since we need two readings for peak detection
    if (data->lastVoltage < 0.0f) {
        data->lastVoltage = voltage;
        return;
    }

    // Peak detection logic
    float threshold = 0.03f;  // Minimum change to consider as significant (to filter noise)
    
    // Rising edge detection (voltage increasing)
    if (voltage > data->lastVoltage + threshold) {
        if (data->risingEdge == 0) data->risingEdge = 1;  // Mark as rising edge
    } 
    // Falling edge detection (voltage decreasing)
    else if (voltage < data->lastVoltage - threshold) {
        // If we were on a rising edge and now voltage is falling, we've detected a peak
        if (data->risingEdge == 1) {
            data->peakCount++;  // Increment peak counter
            data->risingEdge = 0;  // Reset rising edge flag
            
            // Flash LED to indicate peak detection
            LED_SetMode(LED_PEAK_FLASH);
            
            // Calculate time between peaks for frequency calculation
            uint32_t currentPeakTime = currentTime;
            if (data->lastPeakTime > 0 && data->peakCount > 1) {
                data->totalPeakInterval += (currentPeakTime - data->lastPeakTime);
            }
            data->lastPeakTime = currentPeakTime;
        }
    }
    
    // Update lastVoltage for next comparison
    data->lastVoltage = voltage;
}

/**
 * Display basic potentiometer statistics on LCD
 * Shows recording time, peak count, and min/max voltage
 */
void PotRecording_DisplayStats(PotRecordingData *data) {
    char buffer[20];

    // Clear LCD
    lcd_clear();

    if (!data->isRecording) {
        lcd_send_string("Press B1");
        lcd_send_cmd(LCD_LINE2, 4);
        lcd_send_string("to start");
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

/**
 * Get debug data from potentiometer recording
 * Copies key information to a separate structure for diagnostic purposes
 */
void PotRecording_GetDebugData(PotRecordingData *data, PotDebugData *debugData) {
    if (debugData != NULL && data != NULL) {
        debugData->currentVoltage = data->lastVoltage;
        debugData->minVoltage = data->minVoltage;
        debugData->maxVoltage = data->maxVoltage;
        debugData->peakCount = data->peakCount;
        debugData->midCrossingCount = data->midCrossingCount;
        debugData->timeAboveHighThreshold = data->timeAboveHighThreshold;
        debugData->timeBelowLowThreshold = data->timeBelowLowThreshold;
    }
}


