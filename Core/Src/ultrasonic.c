/* ultrasonic.c */
#include "ultrasonic.h"
#include "main.h"

/* Global variable to store timer handle */
TIM_HandleTypeDef *ultrasonicTimer;

void Ultrasonic_Init(TIM_HandleTypeDef *htim) {
    /* Store timer handle for later use */
    ultrasonicTimer = htim;

    /* Start the timer */
    HAL_TIM_Base_Start(ultrasonicTimer);

    /* Initialize TRIG pin to low */
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
    HAL_Delay(50);  /* Allow sensor to settle */
}

float Ultrasonic_Read(void) {
    uint32_t val1 = 0;
    uint32_t val2 = 0;
    uint32_t pMillis = 0;
    float distance = 0;

    /* 1. Send 10us pulse on TRIG pin */
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);
    __HAL_TIM_SET_COUNTER(ultrasonicTimer, 0);
    while (__HAL_TIM_GET_COUNTER(ultrasonicTimer) < 10); /* Wait for 10 us */
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);

    /* 2. Wait for ECHO pin to go HIGH (start of echo pulse) */
    pMillis = HAL_GetTick();
    while (!(HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin)) && pMillis + 10 > HAL_GetTick()) {
        /* If ECHO never goes high within timeout, return error */
        if (HAL_GetTick() > pMillis + 10) {
            return -1;  /* Error: Echo pin never went high */
        }
    }

    /* 3. Measure the time for which ECHO stays HIGH */
    val1 = __HAL_TIM_GET_COUNTER(ultrasonicTimer);  /* Capture start time */

    pMillis = HAL_GetTick();
    while ((HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin)) && pMillis + 50 > HAL_GetTick()) {
        /* If ECHO stays high too long, return error */
        if (HAL_GetTick() > pMillis + 50) {
            return -2;  /* Error: Echo pulse too long */
        }
    }

    val2 = __HAL_TIM_GET_COUNTER(ultrasonicTimer);  /* Capture end time */

    /* 4. Calculate distance based on time difference */
    distance = (val2-val1) * 0.034/2;

    return distance;
}

float Ultrasonic_GetDistance(void) {
    static float filtered_cm = 0;
    float alpha = 0.3;  // Filter strength (0-1), lower = more filtering

    // Get new sample in cm
    float distance_cm = Ultrasonic_Read();

    // Return error codes immediately
    if (distance_cm < 0) {
        return distance_cm;
    }

    // Apply low-pass filter
    if (filtered_cm == 0) {
        filtered_cm = distance_cm;  // Initialize on first reading
    } else {
        filtered_cm = alpha * distance_cm + (1 - alpha) * filtered_cm;
    }

    // Return filtered distance in mm
    return filtered_cm * 10.0f;  // Convert cm to mm
}

