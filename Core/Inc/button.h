/*
 * button.h
 *
 *  Created on: May 8, 2025
 *      Author: jalal
 */

#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#include "stm32f4xx_hal.h"

void button_init(void);
void button_check(void);
uint8_t button_is_pressed(void);
void button_led_blink(void);
void button_reset_flag(void);

/* Button states */
#define BUTTON_RELEASED 0
#define BUTTON_PRESSED  1


#endif /* INC_BUTTON_H_ */
