/*
 * recording.h
 *
 *  Created on: May 18, 2025
 *      Author: jalal
 *  
 *  This header serves as a wrapper for the modularized recording functionality.
 *  It maintains the original API while delegating to specialized modules.
 */

#ifndef INC_RECORDING_H_
#define INC_RECORDING_H_

#include "main.h"
#include <stdint.h>

// Include the specialized headers
#include "potentioRecording.h"
#include "ultrasonicRecording.h"
#include "resutlsDisplay.h"

// Main recording API function prototypes
void Recording_Init(void);
void Recording_Start(void);
void Recording_Stop(void);
void Recording_ProcessPotReading(float voltage, float distanceMm);
void Recording_ProcessUltraReading(float distanceMm, uint8_t isHighSignal);
void Recording_ToggleDetailedStats(void);
void Recording_UpdateDisplay(void);
uint8_t Recording_IsShowingDetailedStats(void);

// Accessor functions for data structures (for report generation)
PotRecordingData* Recording_GetPotData(void);
UltraRecordingData* Recording_GetUltraData(void);

#endif /* INC_RECORDING_H_ */
