/*
 * potentioRecording.h
 *
 *  Created on: May 31, 2025
 *      Author: jalal
 */

#ifndef INC_POTENTIORECORDING_H_
#define INC_POTENTIORECORDING_H_

#include "main.h"
#include <stdint.h>

// Simple fixed midpoint for voltage (1.65V = half of 3.3V)
#define FIXED_MIDPOINT_VOLTAGE 1.65f

// Data structure for potentiometer recording
typedef struct {
    uint32_t startTime;      // Start time in milliseconds
    uint32_t elapsedTime;    // Elapsed time in milliseconds
    float minVoltage;        // Minimum voltage recorded
    float maxVoltage;        // Maximum voltage recorded
    uint32_t peakCount;      // Number of detected peaks
    uint8_t isRecording;     // Recording status flag
    float lastVoltage;       // Previous voltage reading (for peak detection)
    uint8_t risingEdge;      // Flag to track rising edge (for peak detection)
    float peakThreshold;     // Threshold for peak detection
    uint32_t timeAbove95Pct; // Time spent above 95% of range (ms)
    uint32_t timeBelow5Pct;  // Time spent below 5% of range (ms)
    uint32_t crossings50Pct; // Number of crossings of the 50% threshold
    uint32_t lastPeakTime;   // Time of the last peak (for frequency calculation)
    uint32_t totalPeakInterval; // Sum of intervals between peaks (for average calculation)
    uint32_t peaksWhenCloseCount; // Peaks detected when distance < 100mm
    // Add missing members that are expected by the code
    uint32_t midCrossingCount; // Midpoint crossings count (alias for crossings50Pct)
    uint32_t timeAboveHighThreshold; // Time above high threshold (alias for timeAbove95Pct)
    uint32_t timeBelowLowThreshold; // Time below low threshold (alias for timeBelow5Pct)
    uint32_t peakCountWhenClose; // Peaks when close (alias for peaksWhenCloseCount)
} PotRecordingData;

// Debug data structure (only for internal use)
typedef struct {
    float currentVoltage;
    float minVoltage;
    float maxVoltage;
    uint32_t peakCount;
    // Add missing members that are expected by the code
    uint32_t midCrossingCount;
    uint32_t timeAboveHighThreshold;
    uint32_t timeBelowLowThreshold;
} PotDebugData;

// Potentiometer recording function prototypes
void PotRecording_Init(PotRecordingData *data);
void PotRecording_Start(PotRecordingData *data);
void PotRecording_Stop(PotRecordingData *data);
void PotRecording_Process(PotRecordingData *data, float voltage);
void PotRecording_DisplayStats(PotRecordingData *data);
void PotRecording_GetDebugData(PotRecordingData *data, PotDebugData *debugData);

// Global variables that need to be accessed from other files
extern uint8_t was_above_midpoint;

#endif /* INC_POTENTIORECORDING_H_ */
