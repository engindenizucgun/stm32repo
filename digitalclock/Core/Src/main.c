
//------------------------------------------------------HEADER FILES---------------------------------------------------------------------------//
//00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000//

#include "main.h"
#include <stdbool.h>
#include <stdio.h>

//------------------------------------------------------HANDLEs---------------------------------------------------------------------------//
//00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000//

TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart2;


//------------------------------------------------------VARIABLES---------------------------------------------------------------------------//
//00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000//

volatile uint32_t milliseconds = 0;
volatile uint32_t seconds = 0;
volatile uint32_t minutes = 0;
volatile uint32_t hours = 0;
bool adjustmentMode = false;
uint32_t adjustmentStart = 0;
uint32_t buttonPressCount = 0;
uint32_t buttonPressStart = 0;
uint32_t clockValue = 0;


//------------------------------------------------------FUNCTION PROTOTYPES--------------------------------------------------------------------------//
//00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000//

void EXTI0_IRQHandler(void);
void EXTI1_IRQHandler(void);
void EXTI2_IRQHandler(void);


void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);


//------------------------------------------------------EXTI PINS INIT---------------------------------------------------------------------------//
//00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000//

void GPIO_Init(void) {

    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;


    GPIOC->MODER &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1 | GPIO_MODER_MODE2);
    GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD0 | GPIO_PUPDR_PUPD1 | GPIO_PUPDR_PUPD2);
    GPIOC->PUPDR |= GPIO_PUPDR_PUPD0_1 | GPIO_PUPDR_PUPD1_1 | GPIO_PUPDR_PUPD2_1;

    //SET
    EXTI->IMR1 |= EXTI_IMR1_IM0;   // Enable interrupt
    EXTI->FTSR1 |= EXTI_FTSR1_FT0; // Enable falling edge trigger

    //INCREASE
    EXTI->IMR1 |= EXTI_IMR1_IM1;   // Enable interrupt
    EXTI->FTSR1 |= EXTI_FTSR1_FT1; // Enable falling edge trigger

    //DECREASE
    EXTI->IMR1 |= EXTI_IMR1_IM2;   // Enable interrupt
    EXTI->FTSR1 |= EXTI_FTSR1_FT2; // Enable falling edge trigger


    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI1_IRQn);
    NVIC_EnableIRQ(EXTI2_IRQn);
}
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
//------------------------------------------------------BUTTON PRESSED---------------------------------------------------------------------------//
//00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000//


bool isButtonPressed(uint32_t GPIO_Pin) {
    return (GPIOC->IDR & GPIO_Pin) == 0;
}

//------------------------------------------------------GET CURRENT TIME---------------------------------------------------------------------------//
//00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000//

uint32_t getCurrentTimeInSeconds(void) {
    uint32_t currentCounterValue = __HAL_TIM_GET_COUNTER(&htim2);
    uint32_t timeInSeconds = currentCounterValue / 1000;
    return timeInSeconds;
}

//------------------------------------------------------SECONDS TO MINUTES TO HOURS---------------------------------------------------------------------------//
//00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000//

void updateClock(uint32_t seconds, uint32_t *hours, uint32_t *minutes, uint32_t *secs) {
    *hours = seconds / 3600;
    seconds %= 3600;
    *minutes = seconds / 60;
    *secs = seconds % 60;
}
//------------------------------------------------------ADJUSTMENT MODE---------------------------------------------------------------------------//
//00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000//

void EnterAdjustmentMode(void) {
  adjustmentMode = true;
  adjustmentStart = milliseconds;
}

//------------------------------------------------------ADJUST HOUR & MINUTE---------------------------------------------------------------------------//
//00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000//

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

//------------------------------------------------------PRINT CLOCK---------------------------------------------------------------------------//
//00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000//

void printClockValue(void) {
    printf("%02lu:%02lu:%02lu\n", hours, minutes, seconds);
}

//------------------------------------------------------EXTI HANDLERS---------------------------------------------------------------------------//
//00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000//

void EXTI0_IRQHandler(void) {
    if (EXTI->PR1 & EXTI_PR1_PIF0) {
        EXTI->PR1 = EXTI_PR1_PIF0;
        // Handle the button press on EXTI line 0
    }
}

void EXTI1_IRQHandler(void) {
    if (EXTI->PR1 & EXTI_PR1_PIF1) {
        EXTI->PR1 = EXTI_PR1_PIF1;
        // Handle the button press on EXTI line 1
    }
}

void EXTI2_IRQHandler(void) {
    if (EXTI->PR1 & EXTI_PR1_PIF2) {
        EXTI->PR1 = EXTI_PR1_PIF2;
        // Handle the button press on EXTI line 2
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

  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
  GPIOC->MODER &= ~(GPIO_MODER_MODE0_Msk | GPIO_MODER_MODE1_Msk | GPIO_MODER_MODE2_Msk);

  while (1)
  {
	  if (GPIOC->IDR & GPIO_IDR_ID0) {
	              buttonPressCount++;
	              if (buttonPressCount == 1) {
	                  buttonPressStart = milliseconds;
	              }

	              // Button released, handle different cases based on press count and adjustment mode
	              if (buttonPressCount == 1 && !adjustmentMode) {
	                  // Pressed once, start adjustment mode for hour
	              	adjustmentMode = true;
	                  AdjustHour();
	                  buttonPressCount++;
	              }  else if (buttonPressCount == 2 && adjustmentMode) {
	                  // Pressed twice, start adjustment mode for minute
	                  AdjustMinute();
	                  buttonPressCount++;
	              }  else if (buttonPressCount == 3 && adjustmentMode) {
	                  // Pressed once, exit adjustment mode and print adjusted clock
	              	printClockValue();
	              	adjustmentMode = false;
	              	buttonPressStart = 0;
	                buttonPressCount = 0;

	              }
	          }
	}
}



//------------------------------------------------------COMPUTER GENERATED PART---------------------------------------------------------------------------//
//00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000//






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


}


static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();


  HAL_GPIO_WritePin(GPIOA, SMPS_EN_Pin|SMPS_V1_Pin|SMPS_SW_Pin, GPIO_PIN_RESET);


  HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_RESET);


  GPIO_InitStruct.Pin = B1_Pin|SET_Pin|INCREASE_Pin|DECREASE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);


  GPIO_InitStruct.Pin = SMPS_EN_Pin|SMPS_V1_Pin|SMPS_SW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


  GPIO_InitStruct.Pin = SMPS_PG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SMPS_PG_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LD4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD4_GPIO_Port, &GPIO_InitStruct);
 }


void Error_Handler(void)
{

  __disable_irq();


}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{

}
#endif
