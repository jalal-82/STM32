/* USER CODE BEGIN Header */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "i2c-lcd.h"
#include "led.h"
#include "ultrasonic.h"
#include "button.h"
#include "potentiometer.h"
#include "recording.h"
#include "signals.h"
#include "led_control.h"
#include "report_generator.h"
// #define DEBUG_MODE

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
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint8_t buffer[50];


uint8_t buttonState = 0;
uint8_t prevButtonState = 0;
uint8_t displayToggle = 0;  // 0 = Pot view, 1 = Ultra view
uint32_t lastToggleTime = 0;
const uint32_t DISPLAY_TOGGLE_INTERVAL = 3000;  // Toggle every 3 seconds
uint8_t buttonPressCount = 0;  // Counter to track button presses

/* TIM2 interrupt callback */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    Signals_HandleTimerInterrupt(htim);
}

/* Helper function to wait for a time period or detect button press */
uint8_t waitForTimeOrButtonPress(uint32_t waitTimeMs) {
    uint32_t startTime = HAL_GetTick();
    uint8_t localButtonState = buttonState; // Start with current state
    uint8_t buttonPressed = 0;
    
    while (HAL_GetTick() - startTime < waitTimeMs) {
        // Check for button press (with debounce)
        uint8_t currentButtonState = !HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);
        
        // Button pressed (rising edge)
        if (currentButtonState && !localButtonState) {
            HAL_Delay(50); // Simple debounce
            // Recheck to confirm it's still pressed
            if (!HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin)) {
                // Update global button state
                buttonState = currentButtonState;
                prevButtonState = localButtonState;
                buttonPressed = 1;
                break;
            }
        }
        
        localButtonState = currentButtonState;
        HAL_Delay(10);
    }
    
    return buttonPressed;
}

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
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
  MX_TIM1_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  lcd_init(0x27);

  LED_Init();


  Signals_Init(&htim2, &htim1, &hadc1);
  
  // Initialize the report generator for UART reporting only
  Report_Init(&huart2);

  // Welcome message
  lcd_clear();
  lcd_send_string("Dual Sensor");
  lcd_send_cmd(LCD_LINE2, 4);
  lcd_send_string("Recorder v1.0");

  HAL_Delay(2000);
  lcd_clear();
  lcd_send_string("Press B1 to");
  lcd_send_cmd(LCD_LINE2, 4);
  lcd_send_string("start recording");

  // Init toggle timing
  lastToggleTime = HAL_GetTick();
  uint32_t lastDisplayUpdateTime = 0;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */


  while (1)
    {
    /* USER CODE END WHILE */
	  uint32_t currentTime = HAL_GetTick();

	  /* Process LED state */
	  LED_Process();

	  /* Read button state (B1 is active low) */
	  prevButtonState = buttonState;
	  buttonState = !HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);  // Invert because button is active low

	  /* Button press detection (rising edge) */
	  if (buttonState && !prevButtonState) {
		  buttonPressCount++;  // Increment the button press counter

		  if (buttonPressCount == 1) {
			  // First button press
			  if (!Signals_IsRecording()) {
				  Signals_StartRecording();
				  lcd_clear();
				  lcd_send_string("REC: Potentiomtr");
			  } else {
				  Signals_StopRecording();
				  // Generate the analysis report AFTER stopping and before resetting data
				  Report_GenerateAnalysisReport(Recording_GetPotData(), Recording_GetUltraData());

				  // Display a message that recording stopped and report was generated
				  lcd_clear();
				  lcd_send_string("REC Stopped.");
				  lcd_send_cmd(LCD_LINE2, 4);
				  lcd_send_string("Report Sent (UART)");
				  HAL_Delay(2000); // Show message for a bit

				  // Prepare for next session or stats view
				  lcd_clear();
				  lcd_send_string("Press B1 for");
				  lcd_send_cmd(LCD_LINE2, 4);
				  lcd_send_string("Stats / New Rec");
			  }
		  } else if (buttonPressCount == 2) {
			  // Second button press: display detailed statistics on LCD
			  // Data stream log was already sent live during recording.
			  // Analysis report was sent when recording stopped (on first press if it was stopping).

			  // If recording was active, stop it and generate final analysis report
			  if (Signals_IsRecording()) {
				  Signals_StopRecording();
				  // Generate final report using accessor functions
				  Report_GenerateAnalysisReport(Recording_GetPotData(), Recording_GetUltraData()); // Ensure final report is sent
				  HAL_Delay(100); // Small delay
			  }

			  // Cycle through statistics windows on LCD
			  uint8_t exitStatsView = 0;
			  
			  // Show each detailed stats window for 5 seconds or until button is pressed
			  // Window 1 - Potentiometer Basic Stats
			  Recording_ToggleDetailedStats(); // Toggle to window 1
			  Recording_UpdateDisplay();
			  if (waitForTimeOrButtonPress(5000)) { exitStatsView = 1; }
			  
			  if (!exitStatsView) {
				  // Window 2 - Potentiometer Advanced Stats
				  Recording_ToggleDetailedStats(); // Toggle to window 2
				  Recording_UpdateDisplay();
				  if (waitForTimeOrButtonPress(5000)) { exitStatsView = 1; }
			  }
			  
			  if (!exitStatsView) {
				  // Window 3 - Ultrasonic Stats
				  Recording_ToggleDetailedStats(); // Toggle to window 3
				  Recording_UpdateDisplay();
				  if (waitForTimeOrButtonPress(5000)) { exitStatsView = 1; }
			  }
			  
			  if (!exitStatsView) {
				  // Window 4 - Combined Stats
				  Recording_ToggleDetailedStats(); // Toggle to window 4
				  Recording_UpdateDisplay();
				  if (waitForTimeOrButtonPress(5000)) { exitStatsView = 1; }
			  }
			  
			  // Reset to normal view (Window 0) for next time
			  Recording_ToggleDetailedStats(); // Toggle back to 0

			  // Reset for new recording session
			  Recording_Init();
			  resetStaticTrackers(); // Reset tracking variables in signals.c

			  lcd_clear();
			  lcd_send_string("Press B1 to");
			  lcd_send_cmd(LCD_LINE2, 4);
			  lcd_send_string("start recording");

			  buttonPressCount = 0; // Reset for next cycle
			  buttonState = 0;
			  prevButtonState = 0;
			  HAL_Delay(100);
		  }
	  }

	  /* Toggle display if needed */
	  if (currentTime - lastToggleTime > DISPLAY_TOGGLE_INTERVAL) {
		  displayToggle = !displayToggle;
		  lastToggleTime = currentTime;
	  }

	  /* Update display at reasonable rate (5Hz is plenty for LCD) */
	  if (currentTime - lastDisplayUpdateTime >= 200) {
		  /* Display statistics on LCD based on current view */
		  if (displayToggle == 0) {
			  Signals_DisplayPotView();
		  } else {
			  Signals_DisplayUltraView();
		  }
		  lastDisplayUpdateTime = currentTime;
	  }

	  /* Small delay for system performance */
	  HAL_Delay(1);

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
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_56CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

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
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 83;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7199;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

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
  HAL_GPIO_WritePin(GPIOA, LD2_Pin|TRIG_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ECHO_Pin */
  GPIO_InitStruct.Pin = ECHO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ECHO_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD2_Pin TRIG_Pin */
  GPIO_InitStruct.Pin = LD2_Pin|TRIG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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
