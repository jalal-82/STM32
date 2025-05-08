/* USER CODE BEGIN Header */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "i2c-lcd.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint8_t buffer[50];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

//  -------------------------- Lab 8 Code ----------------------------- //
//  This works but do not remove the code, this is for reference for us
//  lcd_init(0x27);
//  char message[] = "Please help, UnivID";
//
//  // Display message on LCD
//  lcd_clear();
//  lcd_send_string(message);
//
//  // Transmit message via UART
//  HAL_UART_Transmit(&huart2, (uint8_t *)message, strlen(message), 100);
//
//  // For two-line display (after a 2-second delay)
//  HAL_Delay(5000);
//  lcd_clear();
//  lcd_send_string("Please help");
//
//  // Move to second line
//  lcd_send_cmd(LCD_LINE2, 4);
//  lcd_send_string("u7480662");
//  --------------------------- Lab 8 Code End ------------------------ //

// ---------------------------- Assignment Code (Both Members) ----------------------- //

// SYSTEM CONFIGURATION
// --- Clock configuration: 84MHz system clock
// --- Shared timing base: Timer for synchronizing both sensor readings
// --- Base interrupt priority structure
// --- Common data structures for combined statistics


// STATE MACHINE DEFINITION
typedef enum {
  STATE_IDLE,           // System ready, waiting for B1 press to start
  STATE_CAPTURING,      // Actively sampling both sensors
  STATE_POST_CAPTURE    // Displaying statistics after capture
} SystemState;

// GLobal state variables
volatile SystemState currentState = STATE_IDLE;
volatile uint32_t captureTimeMs = 0;  // Elapsed time in milliseconds
volatile bool autoToggleLCDView = true;  // Controls automatic view switching

// Shared Utility Functions
void SystemClock_Config(void);
void Error_Handler(void);
void InitializeAllPeripherals(void);
void ResetSystem(void);

// -----------------------------------------------------------------------------------------
// POTENTIOMETER INTERFACE (Jalal)
// -----------------------------------------------------------------------------------------

// ADC CONFIGURATION
// - Setup ADC1 for potentiometer reading (PA0, 12-bit resolution, continuous conversion)
// - Configure DMA for ADC readings
// - Setup sampling timer to ensure ≤10ms sampling interval

// Data structures for analog signal processing
typedef struct {
    float currentVoltage;     // Latest voltage reading
    float minVoltage;         // Minimum voltage observed
    float maxVoltage;         // Maximum voltage observed
    uint32_t peakCount;       // Number of peaks detected
    uint32_t midLevelCrossings; // Number of crossings of 50% level
    uint32_t timeAbove95Pct;  // Time spent above 95% of observed range (ms)
    uint32_t timeBelow5Pct;   // Time spent below 5% of observed range (ms)
    float avgFrequency;       // Average signal frequency (Hz)
    // Additional fields for capturing raw data points if memory allows
} AnalogSignalData;

volatile AnalogSignalData analogData = {0};

// Function prototypes for analog processing
void ADC_Init(void);
float ADC_ReadVoltage(void);
bool DetectPeak(float currentVoltage, float previousVoltage);
void ProcessAnalogReading(float voltage);
void CalculateAnalogStatistics(void);
void DisplayAnalogLiveView(void);
void DisplayAnalogStatsView(uint8_t viewNumber);

// -----------------------------------------------------------------------------------------
// ULTRASONIC SENSOR INTERFACE (ZARIF)
// -----------------------------------------------------------------------------------------

// ULTRASONIC SENSOR CONFIGURATION
// - Configure trigger pin as output (e.g., PB10)
// - Configure echo pin as input (e.g., PB11)
// - Setup input capture timer for precise echo timing
// - May use lower sampling rate than ADC (with justification)

// Data structures for ultrasonic sensor processing
typedef struct {
    uint32_t currentDistance;      // Latest distance reading (mm)
    uint32_t minDistance;          // Minimum distance observed (mm)
    uint32_t maxDistance;          // Maximum distance observed (mm)
    uint32_t avgDistance;          // Average distance observed (mm)
    uint32_t directionChanges;     // Number of direction changes
    uint32_t dirChangesWhileAnalogHigh; // Direction changes when analog > 50%
    uint32_t peaksWhileClose;      // Number of analog peaks when distance < 100mm
    uint32_t timeInRange;          // Total time object detected in range (ms)
    bool objectDetected;           // Whether object is currently in range
    // Additional fields for capturing raw data points if memory allows
} UltrasonicData;

volatile UltrasonicData sonicData = {0};

// Function prototypes for ultrasonic processing
void Ultrasonic_Init(void);
uint32_t Ultrasonic_MeasureDistance(void);
bool DetectDirectionChange(uint32_t current, uint32_t previous);
void ProcessUltrasonicReading(uint32_t distance);
void CalculateUltrasonicStatistics(void);
void DisplayUltrasonicLiveView(void);
void DisplayUltrasonicStatsView(uint8_t viewNumber);
void SetLED3State(bool objectDetected);

// -----------------------------------------------------------------------------------------
// LCD INTERFACE (Jalal)
// -----------------------------------------------------------------------------------------

// LCD CONFIGURATION
// - I2C setup for HD44780 with PCF8574 I2C backpack (address 0x27)
// - Display formatting and view management

// Function prototypes for LCD interface
void LCD_Init(void);
void LCD_Clear(void);
void LCD_SendString(char* str);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_ToggleView(void);
void LCD_UpdateDisplay(void);
void LCD_ShowReadyScreen(void);

// -----------------------------------------------------------------------------------------
// BUTTON INTERFACE (Jalal)
// -----------------------------------------------------------------------------------------

// BUTTON CONFIGURATION
// - Configure Button 1 (B1) and Button 2 (B2) with debouncing (not need debouncing if i remember)
// - Setup interrupt or polling mechanism

// Function prototypes for button interface
void Button_Init(void);
void ProcessB1Press(void);
void ProcessB2Press(void);

// -----------------------------------------------------------------------------------------
// LED INTERFACE (SHARED)
// -----------------------------------------------------------------------------------------

// LED CONFIGURATION
// - Configure LED2 (LD2) for analog peak indication
// - Configure LED3 (LD3) for ultrasonic object detection

// Function prototypes for LED control
void LED_Init(void);
void BlinkLED2(void);  // For analog peak detection (Student 2)

// -----------------------------------------------------------------------------------------
// UART COMMUNICATION (SHARED)
// -----------------------------------------------------------------------------------------

// UART CONFIGURATION
// - Setup UART for computer communication (115200 baud typical)
// - Format data according to project specifications

// Function prototypes for UART communication
void UART_Init(void);
void UART_SendChar(uint8_t c);
void UART_SendString(char* str);
void UART_SendAnalysisReport(void);
void UART_SendTimeIndexedLog(void);

// -----------------------------------------------------------------------------------------
// INTERRUPT HANDLERS
// -----------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------
// IMPLEMENTATION NOTES
// -----------------------------------------------------------------------------------------

// MEMORY CONSIDERATIONS
// - STM32F411 has 128KB RAM - sufficient for most signal processing
// - Consider using circular buffers for signal storage if needed
// - Be mindful of stack usage in interrupt handlers

// TIMING CONSIDERATIONS
// - ADC sampling at ≤10ms interval (100Hz minimum)
// - Ultrasonic sensor can use lower rate with justification
// - Ensure LCD updates don't interfere with sampling timing
// - Use DMA where appropriate to reduce CPU overhead

// INTEGRATION POINTS
// - Shared timing base ensures synchronized sampling
// - Coordinate access to shared resources (LCD, UART)
// - Combine statistics from both sensors in post-capture views
// - Ensure consistent event detection (peaks, direction changes)

// TESTING STRATEGY
// - Test individual components before integration
// - Verify signal detection with known input patterns
// - Use UART for debugging during development
// - Validate statistics calculations with test data

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
