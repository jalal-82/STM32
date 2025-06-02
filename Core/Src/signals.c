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
#include "led_control.h"
#include "report_generator.h" // Include for real-time logging

// Simple fixed midpoint for voltage (1.65V = half of 3.3V)
#define FIXED_MIDPOINT_VOLTAGE 1.65f

// Add debug flag
// #define DEBUG_SIGNALS

// Private variables
static TIM_HandleTypeDef *potTimer;
static TIM_HandleTypeDef *ultraTimer;
static ADC_HandleTypeDef *adc;
static uint8_t potSampleCounter = 0;
static uint32_t sampleNumber = 0; // For generating clean timestamps

// Shared sensor data for cross-sensor statistics
static float currentVoltage = 0.0f;       // Current potentiometer voltage
static float currentDistance = 0.0f;      // Current ultrasonic distance (mm)
static uint8_t isHighSignal = 0;          // Flag for voltage > 50% of range
static uint8_t isCloseDistance = 0;       // Flag for distance < 100mm

static uint32_t lastUltrasonicSampleTime = 0;
static const uint32_t ULTRASONIC_SAMPLE_INTERVAL = 100; // 10Hz sampling
static const float CLOSE_DISTANCE_THRESHOLD = 100.0f;   // 100mm threshold for "close" detection

// Static variables that need to be reset at recording start
static uint32_t lastDirChangeCount = 0;
// Temporary flags for logging the current sample's peak/dir_change status
static uint8_t current_sample_is_peak = 0;
static uint8_t current_sample_is_dir_change = 0;
// Variables for midpoint crossing detection
static float prev_voltage_for_crossing = 0;
static uint8_t was_above_mid = 0;

// Reset all static tracking variables - accessible from outside
void resetStaticTrackers() {
    lastDirChangeCount = 0;
    isHighSignal = 0;
    isCloseDistance = 0;
    sampleNumber = 0; // Reset sample counter
    current_sample_is_peak = 0;
    current_sample_is_dir_change = 0;
    prev_voltage_for_crossing = 0; // Reset midpoint crossing detection
    was_above_mid = 0;
    
    // Reset midpoint tracking in potentioRecording.c
    extern uint8_t was_above_midpoint;
    was_above_midpoint = 0;
}

// Initialize all signal handling
void Signals_Init(TIM_HandleTypeDef *htim_pot, TIM_HandleTypeDef *htim_ultra, ADC_HandleTypeDef *hadc) {
    // Store handlers
    potTimer = htim_pot;
    ultraTimer = htim_ultra;
    adc = hadc;

    // Initialize sensors
    Potentiometer_Init(adc);
    Ultrasonic_Init(ultraTimer);

    // Initialize recording functionality through the wrapper
    Recording_Init();

    // Start the potentiometer timer for consistent sampling
    HAL_TIM_Base_Start_IT(potTimer);

    LED_Init();
    LED_SetProximityThreshold(800.0f); // 80cm default threshold
}

// Add a function to periodically output voltage statistics
static void debugPrintVoltageStats() {
    #ifdef DEBUG_SIGNALS
    static uint32_t lastStatsTime = 0;
    uint32_t currentTime = HAL_GetTick();
    
    // Print stats every 2 seconds
    if (currentTime - lastStatsTime >= 2000) {
        extern UART_HandleTypeDef huart2;
        char debug[150];
        
        // Get the actual recording data
        PotRecordingData* potData = Recording_GetPotData();
        
        // Only if recording
        if (potData->isRecording) {
            float range = potData->maxVoltage - potData->minVoltage;
            float midpoint = potData->minVoltage + (range * 0.5f);
            
            sprintf(debug, "VOLTAGE STATS: Current=%.3f, Min=%.3f, Max=%.3f, Range=%.3f, Mid=%.3f, isHigh=%d\r\n",
                    currentVoltage, potData->minVoltage, potData->maxVoltage, 
                    range, midpoint, isHighSignal);
            HAL_UART_Transmit(&huart2, (uint8_t*)debug, strlen(debug), 10);
        }
        
        lastStatsTime = currentTime;
    }
    #endif
}

// Handle timer interrupts
void Signals_HandleTimerInterrupt(TIM_HandleTypeDef *htim) {
    uint32_t currentTime = HAL_GetTick();

    if (htim->Instance == potTimer->Instance) {
        // This triggers at 1kHz
        potSampleCounter++;

        // We want 100Hz sampling rate (every 10 interrupts)
        if (potSampleCounter >= 10) {
            potSampleCounter = 0;
            float voltage = Potentiometer_GetVoltage();

            if (voltage >= 0.0f && voltage <= 3.3f) {
                currentVoltage = voltage;
                debugPrintVoltageStats(); // Enabled debug output

                if (Signals_IsRecording()) {
                    // Get the actual recording data
                    PotRecordingData* potData = Recording_GetPotData();
                    
                    // Update min/max before processing for accurate range calculations
                    if (voltage < potData->minVoltage) potData->minVoltage = voltage;
                    if (voltage > potData->maxVoltage) potData->maxVoltage = voltage;

                    // Determine if signal is high based on FIXED threshold (1.65V)
                    isHighSignal = (voltage > FIXED_MIDPOINT_VOLTAGE) ? 1 : 0;
                    
                    // Store previous peak count to detect if PotRecording_Process finds a new one
                    uint32_t prevPeakCount = potData->peakCount;
                    
                    // Process the reading through the recording module
                    Recording_ProcessPotReading(voltage, currentDistance);
                    
                    // Track if a peak was detected in this sample
                    current_sample_is_peak = (potData->peakCount > prevPeakCount) ? 1 : 0;
                }
            }

            // Check if it's time to sample ultrasonic AND log data
            if (currentTime - lastUltrasonicSampleTime >= ULTRASONIC_SAMPLE_INTERVAL) {
                lastUltrasonicSampleTime = currentTime;

                if (Signals_IsRecording()) {
                    float distance_mm = Ultrasonic_GetDistance();
                    
                    if (distance_mm >= 0) {
                        currentDistance = distance_mm;
                        isCloseDistance = (distance_mm < CLOSE_DISTANCE_THRESHOLD) ? 1 : 0;
                    }
                    
                    // Get the actual recording data
                    UltraRecordingData* ultraData = Recording_GetUltraData();
                    PotRecordingData* potData = Recording_GetPotData();
                    
                    // Store previous direction change count
                    uint32_t prevDirChangeCount = ultraData->dirChangeCount;
                    
                    // Process the reading through the recording module
                    Recording_ProcessUltraReading(distance_mm, isHighSignal);
                    
                    // Track if a direction change was detected in this sample
                    current_sample_is_dir_change = (ultraData->dirChangeCount > prevDirChangeCount) ? 1 : 0;
                    
                    // Combined cross-sensor statistics update
                    if (isCloseDistance && current_sample_is_peak) {
                        potData->peaksWhenCloseCount++; 
                        potData->peakCountWhenClose = potData->peaksWhenCloseCount; // Keep alias in sync
                    }
                    
                    // Log data at ultrasonic rate (10Hz) with clean timestamps
                    if (Signals_IsRecording()) {
                        // Generate cleaner timestamps that match the assignment example (0, 10, 20, 30...)
                        // Incrementing by 10 for each ultrasonic sample (every ~100ms)
                        uint32_t clean_timestamp = sampleNumber * 10; 
                        
                        // Cap the distance at 2000mm (2 meters) for logging
                        float capped_distance = currentDistance;
                        if (capped_distance > 2000.0f) {
                            capped_distance = 2000.0f;
                        }
                        
                        // Check if we're above or below the fixed midpoint
                        uint8_t now_above_mid = (currentVoltage > FIXED_MIDPOINT_VOLTAGE) ? 1 : 0;
                        
                        // Detect if we've crossed the midpoint since last sample
                        uint8_t mid_crossing = 0;
                        if (prev_voltage_for_crossing != 0 && was_above_mid != now_above_mid) {
                            mid_crossing = 1;
                        }
                        
                        // Update tracking variables
                        prev_voltage_for_crossing = currentVoltage;
                        was_above_mid = now_above_mid;
                        
                        // Live data logging with enhanced information
                        Report_LogLiveDataLine(
                            clean_timestamp,          // Time in ms
                            currentVoltage,           // Current voltage
                            current_sample_is_peak,   // Peak detected
                            capped_distance,          // Distance (capped at 2000mm)
                            current_sample_is_dir_change, // Direction change
                            potData->minVoltage,       // Min voltage for range calculation
                            potData->maxVoltage,       // Max voltage for range calculation
                            isHighSignal,             // Voltage > 50% of range
                            0,                        // Is low signal (unused parameter - calculated in function)
                            mid_crossing,             // Midpoint crossing detected
                            isCloseDistance           // Object is close (<100mm)
                        );
                        
                        sampleNumber++; // Increment for next sample
                    }
                }
            }
        }
    }
}

// Start recording on both sensors
void Signals_StartRecording(void) {
    resetStaticTrackers();
    Recording_Start();
    lastUltrasonicSampleTime = HAL_GetTick(); 
    LED_SetMode(LED_OFF);
    
    // Write the CSV header for live data logging via UART
    Report_WriteLiveDataHeader();
}

// Stop recording on both sensors
void Signals_StopRecording(void) {
    Recording_Stop();
    LED_SetMode(LED_OFF);
}

// Check if recording
uint8_t Signals_IsRecording(void) {
    PotRecordingData* potData = Recording_GetPotData();
    return potData->isRecording;
}

// Display potentiometer view on LCD
void Signals_DisplayPotView(void) {
    static uint32_t lastDisplayTime = 0;
    uint32_t currentTime = HAL_GetTick();
    
    // Update display at most every 200ms to avoid flickering
    if (currentTime - lastDisplayTime < 200) {
        return;
    }
    lastDisplayTime = currentTime;
    
    // If showing detailed stats, don't override it
    if (Recording_IsShowingDetailedStats()) {
        return;
    }
    
    // Update the display with current information
    Recording_UpdateDisplay();
}

// Display ultrasonic view on LCD
void Signals_DisplayUltraView(void) {
    static uint32_t lastDisplayTime = 0;
    uint32_t currentTime = HAL_GetTick();
    
    // Update display at most every 200ms to avoid flickering
    if (currentTime - lastDisplayTime < 200) {
        return;
    }
    lastDisplayTime = currentTime;
    
    // If showing detailed stats, don't override it
    if (Recording_IsShowingDetailedStats()) {
        return;
    }
    
    // Update the display with current information
    Recording_UpdateDisplay();
}

// Debug output function (can be called from main loop)
void Signals_DebugOutput(void) {
    static uint32_t lastDebugTime = 0;
    uint32_t currentTime = HAL_GetTick();
    
    // Limit debug output to once per second
    if (currentTime - lastDebugTime < 1000) {
        return;
    }
    lastDebugTime = currentTime;
    
    // Display debug information through our common data structure
    char buffer[100];
    extern UART_HandleTypeDef huart2;
    
    // Get the actual recording data
    PotRecordingData* potData = Recording_GetPotData();
    
    // Get debug data from potentiometer recording
    PotDebugData debugData;
    PotRecording_GetDebugData(potData, &debugData);
    
    // Format and send debug info
    sprintf(buffer, "DEBUG: V=%.2fV, Min=%.2fV, Max=%.2fV, Peaks=%lu\r\n",
            debugData.currentVoltage, debugData.minVoltage, debugData.maxVoltage, debugData.peakCount);
    HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 10);
}
