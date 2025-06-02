/*
 * ultrasonicRecording.h
 *
 *  Created on: May 31, 2025
 *      Author: jalal
 */

#ifndef INC_ULTRASONICRECORDING_H_
#define INC_ULTRASONICRECORDING_H_

#include "main.h"
#include <stdint.h>

// Data structure for ultrasonic recording
typedef struct {
    uint32_t startTime;      // Start time in milliseconds
    uint32_t elapsedTime;    // Elapsed time in milliseconds
    float minDistance;       // Minimum distance recorded (mm)
    float maxDistance;       // Maximum distance recorded (mm)
    uint32_t dirChangeCount; // Direction changes count
    uint8_t isRecording;     // Recording status flag
    float lastDistance;      // Previous distance reading
    int8_t lastDirection;    // Last direction (-1=closer, 0=static, 1=farther)
    float totalDistanceSum;  // Sum of all valid distance readings for average calculation
    uint32_t validDistanceCount; // Number of valid distance readings
    uint32_t dirChangeWhenHighSignalCount; // Direction changes when analog signal > 50% of range
} UltraRecordingData;

// Ultrasonic recording function prototypes
void UltraRecording_Init(UltraRecordingData *data);
void UltraRecording_Start(UltraRecordingData *data);
void UltraRecording_Stop(UltraRecordingData *data);
void UltraRecording_Process(UltraRecordingData *data, float distance);
void UltraRecording_DisplayStats(UltraRecordingData *data);

#endif /* INC_ULTRASONICRECORDING_H_ */
