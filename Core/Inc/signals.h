
#ifndef INC_SIGNALS_H_
#define INC_SIGNALS_H_

#include "main.h"
#include "potentiometer.h"
#include "ultrasonic.h"
#include "recording.h"
extern UART_HandleTypeDef huart2;

// Initialization
void Signals_Init(TIM_HandleTypeDef *htim_pot, TIM_HandleTypeDef *htim_ultra, ADC_HandleTypeDef *hadc);

// Handler for timer interrupts
void Signals_HandleTimerInterrupt(TIM_HandleTypeDef *htim);

// Control functions
void Signals_StartRecording(void);
void Signals_StopRecording(void);
uint8_t Signals_IsRecording(void);

// Display functions
void Signals_DisplayPotView(void);
void Signals_DisplayUltraView(void);

void Signals_DebugOutput(void);

#endif /* INC_SIGNALS_H_ */
