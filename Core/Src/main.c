/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED_GFX.h"   
#include "Encoder.h"
#include "UISync.h"
#include <stdio.h>
#include "arm_math.h"
#include "PWM.h"
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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static UISync_DeviceHandleTypedef *gEncoderDev = NULL;
static UISync_DeviceHandleTypedef *gOledDev = NULL;

static volatile int32_t gEncoderTotalCount = 0;
static volatile int16_t gEncoderLastStep = 0;
static volatile uint8_t gOledNeedRefresh = 0;

static void UISyncTest_EncoderInit(void)
{
  Encoder_Init();
}

static void UISyncTest_EncoderDeInit(void)
{
  Encoder_DeInit();
}

static UISync_DeviceStatusTypedef UISyncTest_EncoderUpdate(void)
{
  int16_t step = Encoder_PopCount();
  // int16_t step = Encoder_GetCCR();

  if (step == 0)
  {
    return UISync_Waiting;
  }

  gEncoderLastStep = step;
  gEncoderTotalCount += step;
  gOledNeedRefresh = 1U;

  return UISync_Pending;
}

static void UISyncTest_EncoderProcess(void)
{
  
}

static void UISyncTest_OledInit(void)
{
  OLED_GFX_Init();
  OLED_GFX_Clear();
  OLED_ShowString(0, 0, "UISync Test");
  OLED_ShowString(0, 16, "Rotate encoder");
  OLED_GFX_Refresh();
}

static void UISyncTest_OledDeInit(void)
{
  OLED_GFX_Stop();
}

static UISync_DeviceStatusTypedef UISyncTest_OledUpdate(void)
{
  if (gOledNeedRefresh != 0U)
  {
    return UISync_Pending;
  }

  return UISync_Waiting;
}

static void UISyncTest_OledProcess(void)
{
  char line1[21];
  char line2[21];
  char line3[32];

  (void)snprintf(line1, sizeof(line1), "Step:%6d", (int)gEncoderLastStep);
  (void)snprintf(line2, sizeof(line2), "Total:%5ld", (long)gEncoderTotalCount);
  (void)snprintf(line3, sizeof(line3), "PI:%10.10f", PI);

  OLED_GFX_Clear();
  OLED_ShowString(0, 0, "UISync Test");
  OLED_ShowString(0, 16, line1);
  OLED_ShowString(0, 32, line2);
  OLED_ShowString(0, 47, line3);
  OLED_GFX_Refresh();

  gOledNeedRefresh = 0U;
}

static void UISyncTest_Init(void)
{
  UISync_DeviceInitTypedef encoderInit = {
      .Init = UISyncTest_EncoderInit,
      .DeInit = UISyncTest_EncoderDeInit,
      .Rank = 0U,
      .DataSheet = NULL,
      .Update = UISyncTest_EncoderUpdate,
      .Process = UISyncTest_EncoderProcess,
  };
  UISync_DeviceInitTypedef oledInit = {
      .Init = UISyncTest_OledInit,
      .DeInit = UISyncTest_OledDeInit,
      .Rank = 1U,
      .DataSheet = NULL,
      .Update = UISyncTest_OledUpdate,
      .Process = UISyncTest_OledProcess,
  };

  gEncoderDev = UISync_RegisterDevice(&encoderInit);
  gOledDev = UISync_RegisterDevice(&oledInit);

  if ((gEncoderDev == NULL) || (gOledDev == NULL))
  {
    OLED_GFX_Init();
    OLED_GFX_Clear();
    OLED_ShowString(0, 0, "UISync Reg Err");
    OLED_GFX_Refresh();
    return;
  }

  gOledNeedRefresh = 1U;
  HAL_TIM_Base_Start_IT(&htim4);
}

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
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  UISyncTest_Init();
  PWM_Init();
  PWM_SetDuty(PWM_BuckChannel, 0.5f);
  PWM_Start(PWM_BuckChannel);
  

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    UISync_Process();
	  
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM4)
  {
    UISync_Update();
  }
}



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
#ifdef USE_FULL_ASSERT
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
