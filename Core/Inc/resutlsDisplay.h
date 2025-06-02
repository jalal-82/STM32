/*
 * resutlsDisplay.h
 *
 *  Created on: May 31, 2025
 *      Author: jalal
 */

#ifndef INC_RESUTLSDISPLAY_H_
#define INC_RESUTLSDISPLAY_H_

#include "potentioRecording.h"
#include "ultrasonicRecording.h"

// Detailed statistics display functions
void resutlsDisplay_PotentiometerWindow1(PotRecordingData *data);
void resutlsDisplay_PotentiometerWindow2(PotRecordingData *data);
void resutlsDisplay_UltrasonicWindow(UltraRecordingData *data);
void resutlsDisplay_CombinedWindow(PotRecordingData *potData, UltraRecordingData *ultraData);

#endif /* INC_RESUTLSDISPLAY_H_ */
