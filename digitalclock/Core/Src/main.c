//------------------------------------------------------HEADER FILES---------------------------------------------------------------------------//
//00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000//

#include "main.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

//------------------------------------------------------HANDLEs---------------------------------------------------------------------------//
//00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000//

TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart2;


//------------------------------------------------------VARIABLES---------------------------------------------------------------------------//
//00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000//

uint32_t milliseconds = 0;
uint32_t seconds = 0;
uint32_t minutes = 0; // Change this to uint32_t
uint32_t hours = 0;   // Change this to uint32_t

uint32_t adjustmentStart = 0;
uint32_t buttonPressCount = 0;
uint32_t buttonPressStart = 0;
uint32_t clockValue = 0;



//------------------------------------------------------FUNCTION PROTOTYPES--------------------------------------------------------------------------//
//00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000//

//void EXTI0_IRQHandler(void);
//void EXTI1_IRQHandler(void);
//void EXTI2_IRQHandler(void);

void AdjustHour(uint32_t *hours, uint16_t GPIO_Pin);
void AdjustMinute(uint32_t *minutes, uint16_t GPIO_Pin);

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);



//------------------------------------------------------CLOCK---------------------------------------------------------------------------//
//00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000//

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

//------------------------------------------------------ADJUST HOUR & MINUTE---------------------------------------------------------------------------//
//00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000//

void AdjustHour(uint32_t *hours, uint16_t GPIO_Pin) {
	if (*hours >= 0 && GPIO_Pin == GPIO_PIN_7) {
		(*hours)++;
	} else if (*hours > 0 && GPIO_Pin == GPIO_PIN_8) {
		(*hours)--;
	} else if (*hours == 23 && GPIO_Pin == GPIO_PIN_7) {
		*hours = 0;
	} else if (*hours == 0 && GPIO_Pin == GPIO_PIN_8) {
		*hours = 23;
	}
}

void AdjustMinute(uint32_t *minutes, uint16_t GPIO_Pin) {
	if (*minutes >= 0 && GPIO_Pin == GPIO_PIN_7) {
		(*minutes)++;
	} else if (*minutes > 0 && GPIO_Pin == GPIO_PIN_8) {
		(*minutes)--;
	} else if (*minutes == 59 && GPIO_Pin == GPIO_PIN_7) {
		*minutes = 0;
	} else if (*minutes == 0 && GPIO_Pin == GPIO_PIN_8) {
		*minutes = 59;
	}
}


//------------------------------------------------------BUTTON PRESSED---------------------------------------------------------------------------//
//00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000//

bool adjustmentMode = false;
int buttonclick = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

	adjustmentStart = HAL_GetTick();

	//adjust hour
	if (  GPIO_Pin == GPIO_PIN_14) {
		buttonclick++;


	    adjustmentMode = true;

		while(buttonclick == 1) {
			AdjustHour(&hours, GPIO_Pin);
			if (  GPIO_Pin == GPIO_PIN_14) {
				buttonclick++;
			}
			buttonclick = 0;


		}





		if (adjustmentStart - milliseconds > 20000) {
			milliseconds = adjustmentStart;
			adjustmentMode = false;
			buttonclick = 0;


		}
	}




	if (buttonclick == 2 && GPIO_Pin == GPIO_PIN_14) {
		while(buttonclick == 2) {
			AdjustMinute(&minutes, GPIO_Pin);
			if (  GPIO_Pin == GPIO_PIN_14) {
				buttonclick++;
			}
			buttonclick = 0;


		}

		if (adjustmentStart - milliseconds > 20000) {
			milliseconds = adjustmentStart;
			adjustmentMode = false;
			buttonclick = 0;



		}
	}
	if (buttonclick == 3 && GPIO_Pin == GPIO_PIN_14) {
		adjustmentMode = false;
		buttonclick = 0;
	}

}

//------------------------------------------------------MAIN FUNCTION---------------------------------------------------------------------------//
//00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000//

int main(void)
{

	HAL_Init();
	SystemClock_Config();
	MX_USART2_UART_Init();
	MX_GPIO_Init();
	MX_TIM2_Init();





	while (1)
	{
		int currentValue = HAL_GetTick();

				if (currentValue - milliseconds >= 1000) {
					milliseconds = currentValue;
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



			char buffer[50];
			sprintf(buffer, "Clock Time: %02lu:%02lu:%02lu\r\n", hours, minutes,seconds);
			HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
		}
	}
}




static void MX_TIM2_Init(void)
{

	/* USER CODE BEGIN TIM2_Init 0 */

	/* USER CODE END TIM2_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	/* USER CODE BEGIN TIM2_Init 1 */

	/* USER CODE END TIM2_Init 1 */
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 42015;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 1000-1;
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
	huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
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
	HAL_GPIO_WritePin(GPIOA, SMPS_EN_Pin|SMPS_V1_Pin|SMPS_SW_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : B1_Pin */
	GPIO_InitStruct.Pin = B1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

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

	/*Configure GPIO pins : set_Pin decrease_Pin increase_Pin */
	GPIO_InitStruct.Pin = set_Pin|decrease_Pin|increase_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

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
