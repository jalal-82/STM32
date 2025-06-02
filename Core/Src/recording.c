/*
 * recording.c
 *
 *  This file serves as a wrapper for the modularized recording functions.
 *  It maintains the original API while delegating functionality to the specialized modules.
 */

#include "recording.h"
#include "potentioRecording.h"
#include "ultrasonicRecording.h"
#include "resutlsDisplay.h"
#include <stdio.h>
#include <string.h>

// Global data structures
static PotRecordingData potData;
static UltraRecordingData ultraData;

// Internal variables to track window state
static uint8_t currentDetailedStatsWindow = 0; // 0: None, 1-4: Detailed windows
static uint32_t lastWindowChangeTime = 0;      // Time when last window change occurred
static const uint32_t windowSwitchDelay = 500; // 500ms delay to prevent rapid window switching

/**
 * Initialize recording functionality
 * Sets up data structures for both sensors
 */
void Recording_Init(void) {
    PotRecording_Init(&potData);
    UltraRecording_Init(&ultraData);
    currentDetailedStatsWindow = 0;
}

/**
 * Start recording from both sensors
 */
void Recording_Start(void) {
    PotRecording_Start(&potData);
    UltraRecording_Start(&ultraData);
}

/**
 * Stop recording from both sensors
 */
void Recording_Stop(void) {
    PotRecording_Stop(&potData);
    UltraRecording_Stop(&ultraData);
}

/**
 * Process a potentiometer voltage reading
 * Updates statistics and peak detection
 * 
 * @param voltage Current voltage reading from the potentiometer
 * @param distanceMm Current distance reading from the ultrasonic sensor
 */
void Recording_ProcessPotReading(float voltage, float distanceMm) {
    PotRecording_Process(&potData, voltage);
}

/**
 * Process an ultrasonic distance reading
 * Updates statistics and direction change detection
 * 
 * @param distanceMm Current distance reading from the ultrasonic sensor in millimeters
 * @param isHighSignal Flag indicating if the potentiometer signal is high
 */
void Recording_ProcessUltraReading(float distanceMm, uint8_t isHighSignal) {
    UltraRecording_Process(&ultraData, distanceMm);
    
    // Track direction changes during high signal separately
    // We need to maintain this here as it combines data from both sensors
    if (isHighSignal) {
        static int8_t lastDir = 0;
        static float lastDist = 0;
        
        if (lastDist == 0) {
            lastDist = distanceMm;
            return;
        }
        
        // Detect direction with a deadband to filter noise
        float deadband = 10.0f;
        int8_t currentDir = 0;
        
        if (distanceMm > lastDist + deadband) {
            currentDir = 1;  // Moving away
        } else if (distanceMm < lastDist - deadband) {
            currentDir = -1; // Moving closer
        }
        
        // Count direction change if we have a valid previous direction and current direction
        if (lastDir != 0 && currentDir != 0 && currentDir != lastDir) {
            ultraData.dirChangeWhenHighSignalCount++;
        }
        
        // Update tracking variables for next reading
        lastDist = distanceMm;
        if (currentDir != 0) {
            lastDir = currentDir;
        }
    }
}

/**
 * Toggle detailed statistics window display
 * Cycles through available windows (1-4) or returns to normal stats display (0)
 */
void Recording_ToggleDetailedStats(void) {
    uint32_t currentTime = HAL_GetTick();
    
    // Prevent rapid window switching by enforcing a delay
    if (currentTime - lastWindowChangeTime < windowSwitchDelay) {
        return;
    }
    
    lastWindowChangeTime = currentTime;
    
    // Cycle through windows: None -> 1 -> 2 -> 3 -> 4 -> None
    currentDetailedStatsWindow = (currentDetailedStatsWindow + 1) % 5;
}

/**
 * Update the LCD display with current statistics
 * Shows either basic stats or detailed stats depending on current window selection
 */
void Recording_UpdateDisplay(void) {
    // Display the appropriate window based on current selection
    switch (currentDetailedStatsWindow) {
        case 0:
            // Show basic stats from both sensors
            if (HAL_GetTick() % 4000 < 2000) {
                // First 2 seconds: Show potentiometer stats
                PotRecording_DisplayStats(&potData);
            } else {
                // Next 2 seconds: Show ultrasonic stats
                UltraRecording_DisplayStats(&ultraData);
            }
            break;
            
        case 1:
            // Detailed potentiometer stats window 1
            resutlsDisplay_PotentiometerWindow1(&potData);
            break;
            
        case 2:
            // Detailed potentiometer stats window 2
            resutlsDisplay_PotentiometerWindow2(&potData);
            break;
            
        case 3:
            // Detailed ultrasonic stats window
            resutlsDisplay_UltrasonicWindow(&ultraData);
            break;
            
        case 4:
            // Combined stats window
            resutlsDisplay_CombinedWindow(&potData, &ultraData);
            break;
    }
}

/**
 * Check if detailed statistics are currently being displayed
 * 
 * @return 1 if a detailed stats window is active, 0 otherwise
 */
uint8_t Recording_IsShowingDetailedStats(void) {
    return (currentDetailedStatsWindow > 0);
}

/**
 * Get pointer to potentiometer data structure
 * Used for report generation
 * 
 * @return Pointer to the potentiometer data structure
 */
PotRecordingData* Recording_GetPotData(void) {
    return &potData;
}

/**
 * Get pointer to ultrasonic data structure
 * Used for report generation
 * 
 * @return Pointer to the ultrasonic data structure
 */
UltraRecordingData* Recording_GetUltraData(void) {
    return &ultraData;
}
