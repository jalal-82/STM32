/*
 * ultrasonic.h
 *
 *  Created on: May 8, 2025
 *      Author: jalal
 */

#ifndef INC_ULTRASONIC_H_
#define INC_ULTRASONIC_H_

#include "main.h"

void Ultrasonic_Init(TIM_HandleTypeDef *htim);
float Ultrasonic_Read(void);
float Ultrasonic_GetDistance(void);

extern TIM_HandleTypeDef *ultrasonicTimer;


#endif /* INC_ULTRASONIC_H_ */
