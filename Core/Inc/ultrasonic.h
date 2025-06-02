/*
 * ultrasonic.h
 *
 *  Created on: May 8, 2025
 *      Author: jalal
 */

#ifndef INC_ULTRASONIC_H_
#define INC_ULTRASONIC_H_

#include "main.h"

// Range constants in mm
#define ULTRASONIC_MIN_RANGE_MM 20.0f    // Minimum reliable range (2cm)
#define ULTRASONIC_MAX_RANGE_MM 2000.0f   // Maximum practical range (200cm)

void Ultrasonic_Init(TIM_HandleTypeDef *htim);
float Ultrasonic_Read(void);
float Ultrasonic_GetDistance(void);

/**
 * @brief Debug function for direction change detection
 * @param currentDistance Current filtered distance in mm
 * @param lastDistance Previous distance in mm used for comparison
 * @param lastDirection Previous direction value (-1, 0, 1)
 * @param currentDirection Current direction value (-1, 0, 1) 
 * @param detectedChange Whether a direction change was detected (0 or 1)
 */
void Ultrasonic_DebugDirectionChange(float currentDistance, float lastDistance, 
                                    int8_t lastDirection, int8_t currentDirection, 
                                    uint8_t detectedChange);

extern TIM_HandleTypeDef *ultrasonicTimer;


#endif /* INC_ULTRASONIC_H_ */
