/*
 * potentiometer.h
 *
 *  Created on: May 18, 2025
 *      Author: jalal
 */

#ifndef INC_POTENTIOMETER_H_
#define INC_POTENTIOMETER_H_

#include "main.h"

void Potentiometer_Init(ADC_HandleTypeDef *hadc);
uint16_t Potentiometer_Read(void);
float Potentiometer_GetVoltage(void);


#endif /* INC_POTENTIOMETER_H_ */
