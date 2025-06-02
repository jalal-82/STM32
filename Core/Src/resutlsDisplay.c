/*
 * resutlsDisplay.c
 *
 *  Created on: May 31, 2025
 *      Author: jalal
 */

#include "resutlsDisplay.h"
#include "i2c-lcd.h"
#include <stdio.h>
#include <string.h>

/**
 * Display detailed statistics window 1 for potentiometer data
 * Shows recording duration, min/max voltage, and peak count
 * 
 * @param data Pointer to the potentiometer recording data structure
 */
void resutlsDisplay_PotentiometerWindow1(PotRecordingData *data) {
    char buffer[20];
    
    // Clear the LCD
    lcd_clear();
    
    // First line: Recording duration
    lcd_send_string("Pot 1/4 ");
    
    // Format and display elapsed time in seconds
    if (data->elapsedTime >= 60000) {
        // Show minutes:seconds if recording is 1 minute or longer
        sprintf(buffer, "%lum%lus", data->elapsedTime / 60000, (data->elapsedTime % 60000) / 1000);
    } else {
        // Show seconds.tenths for shorter recordings
        sprintf(buffer, "%lu.%lus", data->elapsedTime / 1000, (data->elapsedTime % 1000) / 100);
    }
    lcd_send_string(buffer);
    
    // Second line: Min/Max voltage and peak count
    lcd_send_cmd(LCD_LINE2, 4);
    
    // Format and display min/max voltage
    sprintf(buffer, "%.2f-%.2fV", data->minVoltage, data->maxVoltage);
    lcd_send_string(buffer);
    
    // Display peak count
    lcd_send_string(" P:");
    sprintf(buffer, "%lu", data->peakCount);
    lcd_send_string(buffer);
}

/**
 * Display detailed statistics window 2 for potentiometer data
 * Shows calculated frequency, time above 95%, time below 5%, 
 * and midpoint crossings
 * 
 * @param data Pointer to the potentiometer recording data structure
 */
void resutlsDisplay_PotentiometerWindow2(PotRecordingData *data) {
    char buffer[20];
    float frequencyHz = 0.0f;
    
    // Clear the LCD
    lcd_clear();
    
    // First line: Window title and calculated frequency
    lcd_send_string("Pot 2/4 ");
    
    // Calculate and display frequency if we have peaks and reasonable recording time
    if (data->peakCount >= 2 && data->elapsedTime > 1000) {
        // Calculate frequency: peaks per second
        frequencyHz = (float)data->peakCount / ((float)data->elapsedTime / 1000.0f);
        sprintf(buffer, "%.1fHz", frequencyHz);
    } else {
        sprintf(buffer, "---Hz");
    }
    lcd_send_string(buffer);
    
    // Second line: Time above 95%, below 5%, and midpoint crossings
    lcd_send_cmd(LCD_LINE2, 4);
    
    // Calculate percentage of time above high threshold (95%)
    float percentAbove = (float)data->timeAbove95Pct * 100.0f / (float)data->elapsedTime;
    sprintf(buffer, "H:%.0f%%", percentAbove);
    lcd_send_string(buffer);
    
    // Calculate percentage of time below low threshold (5%)
    float percentBelow = (float)data->timeBelow5Pct * 100.0f / (float)data->elapsedTime;
    sprintf(buffer, " L:%.0f%%", percentBelow);
    lcd_send_string(buffer);
    
    // Display midpoint crossing count
    sprintf(buffer, " X:%lu", data->crossings50Pct);
    lcd_send_string(buffer);
}

/**
 * Display detailed statistics window for ultrasonic data
 * Shows time in range, min/max/avg distance
 * 
 * @param data Pointer to the ultrasonic recording data structure
 */
void resutlsDisplay_UltrasonicWindow(UltraRecordingData *data) {
    char buffer[20];
    
    // Clear the LCD
    lcd_clear();
    
    // First line: Window title and min/max distance
    lcd_send_string("Ultra 3/4");
    
    // Second line: Min, max, and average distance
    lcd_send_cmd(LCD_LINE2, 4);
    
    // Check if we have valid data
    if (data->validDistanceCount > 0 && data->minDistance <= data->maxDistance) {
        // Calculate average distance
        float avgDistance = data->totalDistanceSum / data->validDistanceCount;
        
        // Display min/max/avg distances in millimeters
        sprintf(buffer, "%.0f/%.0f/%.0f", data->minDistance, avgDistance, data->maxDistance);
        lcd_send_string(buffer);
    } else {
        lcd_send_string("No valid data");
    }
}

/**
 * Display combined statistics window
 * Shows combined information from both sensors, focusing on
 * direction changes during high signals and peaks detected when
 * objects are close
 * 
 * @param potData Pointer to the potentiometer recording data structure
 * @param ultraData Pointer to the ultrasonic recording data structure
 */
void resutlsDisplay_CombinedWindow(PotRecordingData *potData, UltraRecordingData *ultraData) {
    char buffer[20];
    
    // Clear the LCD
    lcd_clear();
    
    // First line: Window title and direction changes during high signal
    lcd_send_string("Comb 4/4 ");
    
    // Display direction changes that occurred during high signal
    sprintf(buffer, "DC:%lu", ultraData->dirChangeWhenHighSignalCount);
    lcd_send_string(buffer);
    
    // Second line: Peak count when object was close
    lcd_send_cmd(LCD_LINE2, 4);
    
    // Display peaks that occurred when object was close
    sprintf(buffer, "PeaksClose:%lu", potData->peaksWhenCloseCount);
    lcd_send_string(buffer);
} 