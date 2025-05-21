/*
 * potentiometer.c
 *
 *  Created on: May 18, 2025
 *      Author: jalal
 */


#include "potentiometer.h"

ADC_HandleTypeDef *potADC;


/**
  * @brief Initialize potentiometer (ADC settings)
  * @param hadc Pointer to ADC handle
  * @retval None
  */
void Potentiometer_Init(ADC_HandleTypeDef *hadc) {
    potADC = hadc;
}

uint16_t Potentiometer_Read(void) {
    // Start conversion
    HAL_ADC_Start(potADC);

    // Wait for conversion to complete with timeout
    if (HAL_ADC_PollForConversion(potADC, 10) != HAL_OK) {
        return 0; // Return 0 if timeout or error
    }

    // Read ADC value
    uint16_t value = HAL_ADC_GetValue(potADC);

    // Stop conversion
    HAL_ADC_Stop(potADC);

    return value;
}
/**
  * @brief Get voltage from potentiometer (0-3.3V)
  * @retval Voltage in volts
  */
float Potentiometer_GetVoltage(void) {
    uint16_t rawValue = Potentiometer_Read();

    // Ensure valid reading
    if (rawValue == 0 && HAL_ADC_GetState(potADC) != HAL_ADC_STATE_READY) {
        return 0.0f; // Return 0 if ADC is not ready
    }

    // Convert to voltage (assuming 12-bit ADC and 3.3V reference)
    // Add a small epsilon to avoid negative zero (-0.00) in display
    float voltage = ((float)rawValue * 3.3f) / 4095.0f;
    return voltage < 0.001f ? 0.0f : voltage;  // Prevent negative zero
}
