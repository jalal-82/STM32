/*
 * recording.h
 *
 *  Created on: May 18, 2025
 *      Author: jalal
 */

#ifndef INC_RECORDING_H_
#define INC_RECORDING_H_

#include "main.h"
#include <stdint.h>

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
} PotRecordingData;

typedef struct {
    uint32_t startTime;      // Start time in milliseconds
    uint32_t elapsedTime;    // Elapsed time in milliseconds
    float minDistance;       // Minimum distance recorded (mm)
    float maxDistance;       // Maximum distance recorded (mm)
    uint32_t dirChangeCount; // Direction changes count
    uint8_t isRecording;     // Recording status flag
    float lastDistance;      // Previous distance reading
    int8_t lastDirection;    // Last direction (-1=closer, 0=static, 1=farther)
} UltraRecordingData;

// Function prototypes for potentiometer recording

void PotRecording_Init(PotRecordingData  *data);
void PotRecording_Start(PotRecordingData  *data);
void PotRecording_Stop(PotRecordingData  *data);
void PotRecording_Process(PotRecordingData  *data, float voltage);
void PotRecording_DisplayStats(PotRecordingData  *data);

// Function prototypes for ultrasonic recording
void UltraRecording_Init(UltraRecordingData *data);
void UltraRecording_Start(UltraRecordingData *data);
void UltraRecording_Stop(UltraRecordingData *data);
void UltraRecording_Process(UltraRecordingData *data, float distance);
void UltraRecording_DisplayStats(UltraRecordingData *data);

// debug only
typedef struct {
    float currentVoltage;
    float minVoltage;
    float maxVoltage;
    uint32_t peakCount;
} PotDebugData;
void PotRecording_GetDebugData(PotRecordingData *data, PotDebugData *debugData);



#endif /* INC_RECORDING_H_ */
