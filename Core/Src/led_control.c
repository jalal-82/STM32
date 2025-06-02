#include "led_control.h"

// Private variables
static LedMode_t currentMode = LED_OFF;
static LedMode_t previousMode = LED_OFF; // To remember mode before peak flash
static uint32_t modeStartTime = 0;
static uint32_t lastBlinkTime = 0;
static float proximityThreshold = 200.0f; // Default 20cm threshold

// PWM variables for brightness control
static uint8_t pwmCounter = 0;
static const uint8_t PROXIMITY_DUTY_CYCLE = 30; // 30% duty cycle (0-100) for dimmer brightness
static const uint32_t PEAK_FLASH_DURATION = 500; // 0.5 second flash for peak detection
static const uint32_t BLINK_PERIOD_MS = 100; // 10Hz blink rate (100ms period)

// Initialize LED control
void LED_Init(void) {
    // LD2 should already be initialized as output in MX_GPIO_Init()
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET); // Ensure LED starts off
    currentMode = LED_OFF;
    previousMode = LED_OFF;
}

// Set the LED mode
void LED_SetMode(LedMode_t mode) {
    // Special case for peak flash: remember previous mode to return to it
    if (mode == LED_PEAK_FLASH) {
        if (currentMode != LED_PEAK_FLASH) {
            // Only save previous mode if we're not already in PEAK_FLASH mode
            previousMode = currentMode;
        }
        currentMode = LED_PEAK_FLASH;
        modeStartTime = HAL_GetTick();
        
        // Turn LED fully ON at the start of peak detection
        HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
        return;
    }
    
    // For other modes, only update if mode changes
    if (mode != currentMode) {
        currentMode = mode;
        modeStartTime = HAL_GetTick();
        pwmCounter = 0;

        // Immediate actions based on mode
        switch (mode) {
            case LED_OFF:
                HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
                break;

            case LED_ON:
                HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
                break;

            case LED_PROXIMITY_BLINK:
                // Initial state for blinking - start with ON state
                HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
                lastBlinkTime = modeStartTime;
                break;
                
            // LED_PEAK_FLASH handled separately above
            default:
                break;
        }
    }
}

// Process LED state based on current mode and timing
void LED_Process(void) {
    uint32_t currentTime = HAL_GetTick();

    switch (currentMode) {
        case LED_OFF:
        case LED_ON:
            // These modes maintain constant state, no processing needed
            break;

        case LED_PEAK_FLASH:
            // Flash for the defined duration, then return to previous mode
            if (currentTime - modeStartTime >= PEAK_FLASH_DURATION) {
                HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
                
                // Return to previous mode
                if (previousMode == LED_PROXIMITY_BLINK) {
                    // Special handling for returning to blink mode
                    currentMode = previousMode;
                    modeStartTime = currentTime;
                    lastBlinkTime = currentTime;
                    pwmCounter = 0;
                } else {
                    // For other modes, just restore them
                    currentMode = previousMode;
                }
            }
            break;

        case LED_PROXIMITY_BLINK:
            // Blink at defined rate with PWM for brightness
            if (currentTime - lastBlinkTime >= 5) { // Update every 5ms for smoother PWM
                lastBlinkTime = currentTime;

                // Increment PWM counter (0-99)
                pwmCounter = (pwmCounter + 1) % 100;

                // Calculate blink phase - create a 50% duty cycle blink pattern
                uint8_t blinkPhase = ((currentTime / (BLINK_PERIOD_MS / 2)) % 2) == 0;

                if (blinkPhase) {
                    // ON phase - apply PWM duty cycle for dimming
                    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 
                                    (pwmCounter < PROXIMITY_DUTY_CYCLE) ? GPIO_PIN_SET : GPIO_PIN_RESET);
                } else {
                    // OFF phase
                    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
                }
            }
            break;
    }
}

// Set proximity threshold
void LED_SetProximityThreshold(float threshold_mm) {
    proximityThreshold = threshold_mm;
}

// Get proximity threshold
float LED_GetProximityThreshold(void) {
    return proximityThreshold;
}

// Get current LED mode
LedMode_t LED_GetMode(void) {
    return currentMode;
}

// Optional: Allow adjusting the brightness
void LED_SetProximityBrightness(uint8_t dutyCycle) {
    // Constrain duty cycle to 0-100%
    if (dutyCycle > 100) dutyCycle = 100;
    // Update the duty cycle
    *((uint8_t*)&PROXIMITY_DUTY_CYCLE) = dutyCycle;
}
