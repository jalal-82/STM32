/*
 * report_generator.c
 *
 *  Created on: May 25, 2025
 *      Author: jalal
 */

#include "report_generator.h"
#include <stdio.h>
#include <string.h>

// Static variables
static UART_HandleTypeDef *reportUart;
static char reportBuffer[384]; // Increased buffer size for more data

// Initialize the report generator module
void Report_Init(UART_HandleTypeDef *huart) {
    reportUart = huart;
}

// Helper function to send a line of text via UART (for analysis report)
static void SendUartLine(const char *text) {
    if (reportUart) {
        HAL_UART_Transmit(reportUart, (uint8_t*)text, strlen(text), 100);
        const char *newline = "\r\n";
        HAL_UART_Transmit(reportUart, (uint8_t*)newline, 2, 10);
    }
}

// Log a single line of real-time sensor data via UART in CSV format
int Report_LogLiveDataLine(uint32_t timestamp, float voltage, uint8_t is_peak, 
                          float distance_mm, uint8_t is_dir_change, 
                          float min_voltage, float max_voltage,
                          uint8_t is_high_signal, uint8_t is_low_signal,
                          uint8_t is_mid_crossing, uint8_t is_close_distance) {
    if (!reportUart) {
        return -1; // UART not initialized
    }
    
    // Calculate signal levels based on min/max
    float voltage_range = max_voltage - min_voltage;
    float volt_5pct = min_voltage + (voltage_range * 0.05f);
    float volt_95pct = min_voltage + (voltage_range * 0.95f);
    
    // Check if voltage is above 95% or below 5% of range
    uint8_t above_95pct = (voltage > volt_95pct) ? 1 : 0;
    uint8_t below_5pct = (voltage < volt_5pct) ? 1 : 0;
    
    // Format data into CSV string with additional columns
    sprintf(reportBuffer, "%lu, %.2f, %d, %.0f, %d, %d, %d, %d, %d, %d\r\n", 
            timestamp,           // Time in ms
            voltage,             // Voltage reading
            is_peak,             // 1 if a peak was detected
            distance_mm,         // Distance in mm
            is_dir_change,       // 1 if direction change detected
            above_95pct,         // 1 if voltage > 95% of range
            below_5pct,          // 1 if voltage < 5% of range
            is_mid_crossing,     // 1 if voltage crossed midpoint
            is_close_distance,   // 1 if distance < 100mm (close)
            is_high_signal       // 1 if voltage > 50% of range
           );
    
    // Transmit via UART
    HAL_StatusTypeDef status = HAL_UART_Transmit(reportUart, (uint8_t*)reportBuffer, strlen(reportBuffer), 100);
    
    return (status == HAL_OK) ? 0 : -1;
}

// Write CSV header for live data logging
void Report_WriteLiveDataHeader(void) {
    if (reportUart) {
        const char *header = "# Time(ms), Voltage(V), Peak, Distance(mm), ReverseDir, Above95%, Below5%, MidCross, CloseObj, HighSignal\r\n";
        HAL_UART_Transmit(reportUart, (uint8_t*)header, strlen(header), 100);
    }
}

// Generate a comprehensive analysis report via UART
void Report_GenerateAnalysisReport(PotRecordingData *potData, UltraRecordingData *ultraData) {
    // Report header
    SendUartLine("---------------------------------------------------");
    SendUartLine("            SIGNAL ANALYSIS REPORT                 ");
    SendUartLine("---------------------------------------------------");
    
    // General information
    float durationSeconds = potData->elapsedTime / 1000.0f;
    sprintf(reportBuffer, "Duration: %.1f s", durationSeconds);
    SendUartLine(reportBuffer);
    
    // Potentiometer data
    SendUartLine("\n--- POTENTIOMETER DATA ---");
    
    sprintf(reportBuffer, "Min Voltage: %.2f V", potData->minVoltage);
    SendUartLine(reportBuffer);
    
    sprintf(reportBuffer, "Max Voltage: %.2f V", potData->maxVoltage);
    SendUartLine(reportBuffer);
    
    sprintf(reportBuffer, "Peak Count: %lu", potData->peakCount);
    SendUartLine(reportBuffer);
    
    // Calculate average frequency (peaks per second)
    float avgFrequency = 0.0f;
    if (potData->peakCount > 1 && potData->totalPeakInterval > 0) {
        // Avg interval in ms = totalInterval / (peakCount - 1)
        float avgIntervalMs = (float)potData->totalPeakInterval / (float)(potData->peakCount - 1);
        // Frequency = 1000 / avgIntervalMs (to get Hz)
        avgFrequency = 1000.0f / avgIntervalMs;
        
        sprintf(reportBuffer, "Avg Frequency: %.2f Hz", avgFrequency);
        SendUartLine(reportBuffer);
    } else {
        SendUartLine("Avg Frequency: N/A (insufficient peaks)");
    }
    
    // Time statistics
    float timeAbove95Sec = potData->timeAbove95Pct / 1000.0f;
    sprintf(reportBuffer, "Time >95%%: %.1f s", timeAbove95Sec);
    SendUartLine(reportBuffer);
    
    float timeBelow5Sec = potData->timeBelow5Pct / 1000.0f;
    sprintf(reportBuffer, "Time <5%%: %.1f s", timeBelow5Sec);
    SendUartLine(reportBuffer);
    
    sprintf(reportBuffer, "Mid-Level Crossings: %lu", potData->crossings50Pct);
    SendUartLine(reportBuffer);
    
    // Ultrasonic data
    SendUartLine("\n--- ULTRASONIC DATA ---");
    
    if (ultraData->minDistance <= ultraData->maxDistance) {
        sprintf(reportBuffer, "Min Distance: %.0f mm", ultraData->minDistance);
        SendUartLine(reportBuffer);
        
        sprintf(reportBuffer, "Max Distance: %.0f mm", ultraData->maxDistance);
        SendUartLine(reportBuffer);
        
        // Calculate average distance
        float avgDistance = 0.0f;
        if (ultraData->validDistanceCount > 0) {
            avgDistance = ultraData->totalDistanceSum / (float)ultraData->validDistanceCount;
            sprintf(reportBuffer, "Avg Distance: %.1f mm", avgDistance);
            SendUartLine(reportBuffer);
        } else {
            SendUartLine("Avg Distance: N/A (no valid readings)");
        }
    } else {
        SendUartLine("No valid ultrasonic data recorded");
    }
    
    sprintf(reportBuffer, "Direction Changes: %lu", ultraData->dirChangeCount);
    SendUartLine(reportBuffer);
    
    // Calculate time in ultrasonic range
    float validTimeSeconds = 0.0f;
    if (ultraData->validDistanceCount > 0) {
        // Approximate time based on sampling rate (10Hz = 100ms per sample)
        validTimeSeconds = (float)ultraData->validDistanceCount * 0.1f;
        sprintf(reportBuffer, "Time in Range: %.1f s", validTimeSeconds);
        SendUartLine(reportBuffer);
    } else {
        SendUartLine("Time in Range: 0.0 s");
    }
    
    // Combined statistics
    SendUartLine("\n--- COMBINED STATISTICS ---");
    
    sprintf(reportBuffer, "Dir Changes when Signal>50%%: %lu", ultraData->dirChangeWhenHighSignalCount);
    SendUartLine(reportBuffer);
    
    sprintf(reportBuffer, "Peaks when Distance<100mm: %lu", potData->peaksWhenCloseCount);
    SendUartLine(reportBuffer);
    
    // Report footer
    SendUartLine("---------------------------------------------------");
}
