/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32l1xx_hal.h"

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
#define MAXSIZE  256

extern uint8_t rx_buffer[MAXSIZE];//接收数组
extern volatile uint8_t rx_len; //接收到的数据长度
extern volatile uint8_t recv_end_flag; //接收结束标志位

extern uint8_t g_ControlMotorFlag;		//控制电机标志位,0:默认,1:控制电机前进后退，2：控制电机转向，3：控制电机停止
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Front_Echo_Pin GPIO_PIN_0
#define Front_Echo_GPIO_Port GPIOB
#define Front_Echo_EXTI_IRQn EXTI0_IRQn
#define Rear_Trig_Pin GPIO_PIN_1
#define Rear_Trig_GPIO_Port GPIOB
#define Rear_Echo_Pin GPIO_PIN_2
#define Rear_Echo_GPIO_Port GPIOB
#define Rear_Echo_EXTI_IRQn EXTI2_IRQn
#define MOTOR_Direction1_Pin GPIO_PIN_12
#define MOTOR_Direction1_GPIO_Port GPIOB
#define MOTOR_Direction2_Pin GPIO_PIN_13
#define MOTOR_Direction2_GPIO_Port GPIOB
#define MOTOR_Direction3_Pin GPIO_PIN_14
#define MOTOR_Direction3_GPIO_Port GPIOB
#define Front_Trig_Pin GPIO_PIN_15
#define Front_Trig_GPIO_Port GPIOB
#define LED_Pin GPIO_PIN_9
#define LED_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
void print_log(const char * sFormat, ...);
void print_array_hex(const uint8_t *array, uint16_t length, const char *name);
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
