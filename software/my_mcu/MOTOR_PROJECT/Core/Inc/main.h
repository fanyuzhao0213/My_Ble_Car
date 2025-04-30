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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
/* ����״̬ */
typedef enum{
		idle	= 0,  /* ���� */
		runing, 	  /* ����*/
		end			  /* ����*/
}IC_STATE;
extern IC_STATE ic_state[4];	  /* ����ṹ����� */
extern uint32_t Motor_channel;							//������벶��ͨ��
extern uint32_t Motor1_Number_Of_Pulses;		//���ͨ��1�������
extern uint32_t Motor2_Number_Of_Pulses;		//���ͨ��2�������
extern uint32_t Motor3_Number_Of_Pulses;		//���ͨ��3�������
extern uint32_t Motor4_Number_Of_Pulses;		//���ͨ��4�������
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
#define MOTOR_CW_1_Pin GPIO_PIN_3
#define MOTOR_CW_1_GPIO_Port GPIOB
#define MOTOR_CW_2_Pin GPIO_PIN_4
#define MOTOR_CW_2_GPIO_Port GPIOB
#define MOTOR_CW_3_Pin GPIO_PIN_5
#define MOTOR_CW_3_GPIO_Port GPIOB
#define MOTOR_CW_4_Pin GPIO_PIN_6
#define MOTOR_CW_4_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_7
#define LED1_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_8
#define LED2_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
