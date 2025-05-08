/*
 * button.c
 *
 *  Created on: May 8, 2025
 *      Author: jalal
 */



#include "button.h"
#include "main.h"

/* Global variables */
static uint8_t button_state = BUTTON_RELEASED;
static uint8_t button_pressed_flag = 0;

/**
  * @brief  Initialize button
  * @param  None
  * @retval None
  */
void button_init(void)
{
    /* Initialize button state */
    button_state = BUTTON_RELEASED;
    button_pressed_flag = 0;
}

/**
  * @brief  Check button state and update flags
  * @param  None
  * @retval None
  */
void button_check(void)
{
    /* Read current button state (assuming active low button) */
    GPIO_PinState current_state = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);

    /* Check if button is pressed (active low, so GPIO_PIN_RESET means pressed) */
    if (current_state == GPIO_PIN_RESET && button_state == BUTTON_RELEASED)
    {
        /* Button was just pressed */
        button_state = BUTTON_PRESSED;
        button_pressed_flag = 1;
    }
    /* Check if button is released */
    else if (current_state == GPIO_PIN_SET && button_state == BUTTON_PRESSED)
    {
        /* Button was just released */
        button_state = BUTTON_RELEASED;
    }

    /* Small delay for debouncing */
    HAL_Delay(10);
}

/**
  * @brief  Check if button was pressed since last check
  * @param  None
  * @retval 1 if button was pressed, 0 otherwise
  */
uint8_t button_is_pressed(void)
{
    return button_pressed_flag;
}


/**
  * @brief  Reset button pressed flag
  * @param  None
  * @retval None
  */
void button_reset_flag(void)
{
    button_pressed_flag = 0;
}

/**
  * @brief  EXTI line detection callback
  * @param  GPIO_Pin: Specifies the pin connected to EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == B1_Pin) {
        /* Set flag to indicate button press */
        button_state = BUTTON_PRESSED;
        button_pressed_flag = 1;
    }
}
