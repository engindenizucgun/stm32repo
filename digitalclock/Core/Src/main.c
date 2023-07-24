#include "main.h"
#include "stm32l4xx.h"
#include <stdio.h>

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

volatile uint32_t milliseconds = 0;
volatile uint32_t seconds = 0;
volatile uint32_t minutes = 0;
volatile uint32_t hours = 0;
volatile uint32_t adjustmentMode = 0;
uint32_t adjustmentStart = 0;


void SysTick_Handler(void);
void GPIO_Init(void);
void EXTI_Init(void);
void EnterAdjustmentMode(void);
void AdjustHour(void);
void AdjustMinute(void);
void PrintClockValue(void);



static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};


  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }


  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }


  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

int main(void)
{

		HAL_Init();
	    SystemClock_Config();
	    GPIO_Init();
	    EXTI_Init();
	    MX_GPIO_Init();
	    MX_USART2_UART_Init();
	    MX_TIM2_Init();
	    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	    // message

	    while (1) {
	        if (!adjustmentMode) {
	            if (milliseconds >= 1000) {
	                milliseconds = 0;
	                seconds++;
	                if (seconds >= 60) {
	                    seconds = 0;
	                    minutes++;
	                    if (minutes >= 60) {
	                        minutes = 0;
	                        hours++;
	                        if (hours >= 24) {
	                            hours = 0;
	                        }
	                    }
	                }
	                PrintClockValue(); // Call the PrintClockValue() function to print the clock value every second.
	            }
	        } else {
	            // If in adjustment mode, keep track of elapsed time
	            if (milliseconds - adjustmentStart >= 20000) {
	                adjustmentMode = 0;
	                PrintClockValue(); // Call the PrintClockValue() function when exiting adjustment mode.
	            }
	        }
	    }

  }




void GPIO_Init(void) {
  // Enable GPIOC clock
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;

  // Configure button pins as inputs with pull-up or pull-down resistors
  GPIOC->MODER &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1 | GPIO_MODER_MODE2);
  GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD0 | GPIO_PUPDR_PUPD1 | GPIO_PUPDR_PUPD2);
  GPIOC->PUPDR |= GPIO_PUPDR_PUPD0_1 | GPIO_PUPDR_PUPD1_1 | GPIO_PUPDR_PUPD2_1;
}

void EXTI_Init(void) {
  // Configure EXTI for the set button
  EXTI->IMR1 |= EXTI_IMR1_IM0;   // Enable interrupt
  EXTI->RTSR1 |= EXTI_RTSR1_RT0; // Enable rising edge trigger

  // Configure EXTI for the increase button
  EXTI->IMR1 |= EXTI_IMR1_IM1;   // Enable interrupt
  EXTI->RTSR1 |= EXTI_RTSR1_RT1; // Enable rising edge trigger

  // Configure EXTI for the decrease button
  EXTI->IMR1 |= EXTI_IMR1_IM2;   // Enable interrupt
  EXTI->RTSR1 |= EXTI_RTSR1_RT2; // Enable rising edge trigger

  // Enable EXTI interrupt in NVIC
  NVIC_EnableIRQ(EXTI0_IRQn);
  NVIC_EnableIRQ(EXTI1_IRQn);
  NVIC_EnableIRQ(EXTI2_IRQn);
}

void EXTI0_IRQHandler(void) {
  if (EXTI->PR1 & EXTI_PR1_PIF0) {
    EXTI->PR1 = EXTI_PR1_PIF0; // Clear the interrupt flag
    if (!adjustmentMode) {
      // Start adjustment mode if pressed for 2 seconds
      uint32_t pressStart = milliseconds;
      while ((GPIOC->IDR & GPIO_IDR_ID0) && ((milliseconds - pressStart) < 2000)) {
        // Wait for button release or timeout
      }
      if (!(GPIOC->IDR & GPIO_IDR_ID0) && ((milliseconds - pressStart) >= 2000)) {
        EnterAdjustmentMode();
      }
    } else {
      // Exit adjustment mode if pressed for 2 seconds
      uint32_t pressStart = milliseconds;
      while ((GPIOC->IDR & GPIO_IDR_ID0) && ((milliseconds - pressStart) < 2000)) {
        // Wait for button release or timeout
      }
      if (!(GPIOC->IDR & GPIO_IDR_ID0) && ((milliseconds - pressStart) >= 2000)) {
        adjustmentMode = 0;  // Exit adjustment mode
        PrintClockValue();
      }
    }
  }
}

void EXTI1_IRQHandler(void) {
  if (EXTI->PR1 & EXTI_PR1_PIF1) {
    EXTI->PR1 = EXTI_PR1_PIF1; // Clear the interrupt flag
    if (adjustmentMode && (GPIOC->IDR & GPIO_IDR_ID1)) {
      AdjustHour();
    }
  }
}

void EXTI2_IRQHandler(void) {
  if (EXTI->PR1 & EXTI_PR1_PIF2) {
    EXTI->PR1 = EXTI_PR1_PIF2; // Clear the interrupt flag
    if (adjustmentMode && (GPIOC->IDR & GPIO_IDR_ID2)) {
      AdjustMinute();
    }
  }
}

void EnterAdjustmentMode(void) {
  adjustmentMode = 1;
  adjustmentStart = milliseconds;
}

// Adjust the hour
void AdjustHour(void) {
  if (hours >= 0 && GPIOC->IDR & GPIO_IDR_ID1) {
    hours++;
  } else if (hours > 0 && GPIOC->IDR & GPIO_IDR_ID2) {
    hours--;
  }
    else if (hours > 23 && GPIOC->IDR & GPIO_IDR_ID1) {
      hours = 0;
    }
    else if (hours == 0 && GPIOC->IDR & GPIO_IDR_ID2) {
    	hours = 23;
    }
}

// Adjust the minute
void AdjustMinute(void) {
  if (minutes >= 0 && GPIOC->IDR & GPIO_IDR_ID1) {
    minutes++;
  } else if (minutes > 0 && GPIOC->IDR & GPIO_IDR_ID2) {
	  minutes--;
  }

    else if (minutes == 59 && GPIOC->IDR & GPIO_IDR_ID1) {
      minutes = 0;
    }
    else if (minutes == 0 && GPIOC->IDR & GPIO_IDR_ID2) {
      minutes = 59;
    }
  }


// Print the clock value
void PrintClockValue(void) {
	printf("%02lu:%02lu:%02lu\n", hours, minutes, seconds);

}




static void MX_TIM2_Init(void)
{


  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 16000;
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
}


static void MX_USART2_UART_Init(void)
{
  __HAL_RCC_USART2_CLK_ENABLE();

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_Delay(100);

}


static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};



  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, SMPS_EN_Pin|SMPS_V1_Pin|SMPS_SW_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SET_BUTTON_Pin INCREASE_Pin DECREASE_Pin */
  GPIO_InitStruct.Pin = SET_BUTTON_Pin|INCREASE_Pin|DECREASE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : SMPS_EN_Pin SMPS_V1_Pin SMPS_SW_Pin */
  GPIO_InitStruct.Pin = SMPS_EN_Pin|SMPS_V1_Pin|SMPS_SW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : SMPS_PG_Pin */
  GPIO_InitStruct.Pin = SMPS_PG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SMPS_PG_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD4_Pin */
  GPIO_InitStruct.Pin = LD4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD4_GPIO_Port, &GPIO_InitStruct);

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
