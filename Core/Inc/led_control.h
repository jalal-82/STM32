/*
 * led_control.h
 *
 *  Created on: May 21, 2025
 *      Author: jalal
 */

#ifndef INC_LED_CONTROL_H_
#define INC_LED_CONTROL_H_

#include "main.h"

// LED modes
typedef enum {
    LED_OFF,            // LED is off
    LED_ON,             // LED is continuously on
    LED_PEAK_FLASH,     // Brief flash for peak detection
    LED_PROXIMITY_BLINK // Rapid blinking for proximity detection
} LedMode_t;

// Initialize LED control
void LED_Init(void);

// Set the LED mode
void LED_SetMode(LedMode_t mode);

// Process LED state based on current mode and timing
void LED_Process(void);

// Configuration
void LED_SetProximityThreshold(float threshold_mm);
float LED_GetProximityThreshold(void);

// Get current LED mode
LedMode_t LED_GetMode(void);

#endif /* INC_LED_CONTROL_H_ */
