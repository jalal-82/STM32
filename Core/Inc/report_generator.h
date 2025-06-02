/*
 * report_generator.h
 *
 *  Created on: May 25, 2025
 *      Author: jalal
 */

#ifndef INC_REPORT_GENERATOR_H_
#define INC_REPORT_GENERATOR_H_

#include "main.h"
#include "potentioRecording.h"
#include "ultrasonicRecording.h"

// Initialize the report generator module (UART for analysis)
void Report_Init(UART_HandleTypeDef *huart);

// Generate a comprehensive analysis report via UART (statistics summary)
void Report_GenerateAnalysisReport(PotRecordingData *potData, UltraRecordingData *ultraData);

// Log a single line of real-time sensor data via UART in CSV format
// Returns 0 on success, -1 on failure
int Report_LogLiveDataLine(uint32_t timestamp, float voltage, uint8_t is_peak, 
                          float distance_mm, uint8_t is_dir_change,
                          float min_voltage, float max_voltage,
                          uint8_t is_high_signal, uint8_t is_low_signal,
                          uint8_t is_mid_crossing, uint8_t is_close_distance);

// Write CSV header for live data logging
void Report_WriteLiveDataHeader(void);

#endif /* INC_REPORT_GENERATOR_H_ */
