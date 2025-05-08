/*
 * ultrasonic.c
 *
 *  Created on: May 8, 2025
 *      Author: jalal
 */


#include "ultrasonic.h"
#include "main.h"

void Ultrasonic_Init(void) {
    // Initialize TRIG pin to low
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
}

float Ultrasonic_Read(void) {
    uint32_t pulseDuration = 0;
    float distance = 0;
    uint32_t startTime, endTime;
    uint32_t timeout = HAL_GetTick();

    /* Send 10us pulse on TRIG pin */
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);
    for(volatile uint16_t i = 0; i < 80; i++) { /* Approximately 10us delay at 84MHz */ }
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);

    /* Wait for ECHO pin to go high */
    while(!HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin)) {
        // Timeout after 100ms - sensor not responding
        if(HAL_GetTick() - timeout > 100) {
            return -1; // Return error code
        }
    }

    /* Record start time */
    startTime = HAL_GetTick();
    timeout = startTime; // Reset timeout

    /* Wait for ECHO pin to go low */
    while(HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin)) {
        // Timeout after 100ms - something is wrong
        if(HAL_GetTick() - timeout > 100) {
            return -2; // Return different error code
        }
    }

    /* Record end time */
    endTime = HAL_GetTick();

    /* Calculate pulse duration in milliseconds */
    pulseDuration = endTime - startTime;

    /* Calculate distance in centimeters
     * Distance = (Time * Speed of sound) / 2
     * Speed of sound = 343 m/s = 34300 cm/s = 0.0343 cm/µs
     * But since we measured in ms, not µs: 34.3 cm/ms
     */
    distance = (pulseDuration * 34.3) / 2.0;

    return distance;
}
