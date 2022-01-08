/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define OUT_LCD_LED_Pin GPIO_PIN_13
#define OUT_LCD_LED_GPIO_Port GPIOC
#define OUT_RUN_Pin GPIO_PIN_14
#define OUT_RUN_GPIO_Port GPIOC
#define OUT_GAS_Pin GPIO_PIN_15
#define OUT_GAS_GPIO_Port GPIOC
#define ADC1_Pin GPIO_PIN_0
#define ADC1_GPIO_Port GPIOA
#define ADC2_Pin GPIO_PIN_1
#define ADC2_GPIO_Port GPIOA
#define ADC_U_Pin GPIO_PIN_2
#define ADC_U_GPIO_Port GPIOA
#define ADC_I_Pin GPIO_PIN_3
#define ADC_I_GPIO_Port GPIOA
#define OUT_Wire_Run_Pin GPIO_PIN_4
#define OUT_Wire_Run_GPIO_Port GPIOA
#define OUT_Wire_Dir_Pin GPIO_PIN_5
#define OUT_Wire_Dir_GPIO_Port GPIOA
#define DAC_I_Pin GPIO_PIN_6
#define DAC_I_GPIO_Port GPIOA
#define DAC_V_Pin GPIO_PIN_7
#define DAC_V_GPIO_Port GPIOA
#define DAC_Reserve_Pin GPIO_PIN_0
#define DAC_Reserve_GPIO_Port GPIOB
#define DAC_U_Pin GPIO_PIN_1
#define DAC_U_GPIO_Port GPIOB
#define LCD_D7_Pin GPIO_PIN_12
#define LCD_D7_GPIO_Port GPIOB
#define LCD_D6_Pin GPIO_PIN_13
#define LCD_D6_GPIO_Port GPIOB
#define LCD_D5_Pin GPIO_PIN_14
#define LCD_D5_GPIO_Port GPIOB
#define LCD_D4_Pin GPIO_PIN_15
#define LCD_D4_GPIO_Port GPIOB
#define LCD_E_Pin GPIO_PIN_8
#define LCD_E_GPIO_Port GPIOA
#define LCD_RS_Pin GPIO_PIN_10
#define LCD_RS_GPIO_Port GPIOA
#define Enc_A_Pin GPIO_PIN_15
#define Enc_A_GPIO_Port GPIOA
#define Enc_B_Pin GPIO_PIN_3
#define Enc_B_GPIO_Port GPIOB
#define IN_Ok_Btn_Pin GPIO_PIN_4
#define IN_Ok_Btn_GPIO_Port GPIOB
#define IN_Start_Btn_Pin GPIO_PIN_5
#define IN_Start_Btn_GPIO_Port GPIOB
#define IN_Menu_Btn_Pin GPIO_PIN_6
#define IN_Menu_Btn_GPIO_Port GPIOB
#define IN_GasTest_Btn_Pin GPIO_PIN_7
#define IN_GasTest_Btn_GPIO_Port GPIOB
#define IN_WireDown_Btn_Pin GPIO_PIN_8
#define IN_WireDown_Btn_GPIO_Port GPIOB
#define IN_WireUp_Btn_Pin GPIO_PIN_9
#define IN_WireUp_Btn_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
