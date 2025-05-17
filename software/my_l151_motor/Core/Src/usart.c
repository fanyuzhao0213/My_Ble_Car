/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
#include "motor.h"
#include "string.h"
/* USER CODE END 0 */

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 DMA Init */
    /* USART1_RX Init */
    hdma_usart1_rx.Instance = DMA1_Channel5;
    hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode = DMA_NORMAL;
    hdma_usart1_rx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart1_rx);

    /* USART1_TX Init */
    hdma_usart1_tx.Instance = DMA1_Channel4;
    hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_tx.Init.Mode = DMA_NORMAL;
    hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_usart1_tx);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
/*У���У���װ����*/
uint8_t my_check_code_calculate(uint8_t * data, uint16_t len)
{
    uint8_t sum = 0;
    for(int i=0; i<len; i++)
        sum = (uint8_t)(sum + data[i]);
    sum = 0xff - sum;
    return sum;
}

uint8_t my_check_code_analysis(uint8_t * data, uint16_t len)
{
    uint8_t sum = 0;
    for(int i=0; i<len; i++)
        sum = (uint8_t)(sum + data[i]);
	print_log("check sum result:%02x\r\n",sum);
    if(sum == 0xff)
    {
        return 0;
    }
    else
    {
        return 2;
    }
}

/*UART3 TX��װ����*/
void my_uart3_tx_to_BLE(uint8_t ID,uint8_t ret)
{
	static uint8_t temp_txbuf[20] = {0};
	static uint8_t len = 0;
	uint8_t temp_data;
	
	len = 0;
	memset(temp_txbuf,0,20);
	
	temp_txbuf[len++] = 0xAA;
	temp_txbuf[len++] = 0xAA;
	switch(ID)
	{
		case 0xA1:
			temp_txbuf[len++] = 0x06;
			temp_txbuf[len++] = ID;
			temp_txbuf[len++] = ret;
			break;
		case 0xA2:
			temp_txbuf[len++] = 0x06;
			temp_txbuf[len++] = ID;
			temp_txbuf[len++] = ret;
			break;
		case 0xA3:
			temp_txbuf[len++] = 0x06;
			temp_txbuf[len++] = ID;
			temp_txbuf[len++] = ret;
			break;
		case 0xA4:
			temp_txbuf[len++] = 0x09;
			temp_txbuf[len++] = ID;
			temp_txbuf[len++] = g_MotorDirection;
			temp_txbuf[len++] = g_MotorSpeedLevel;
			temp_txbuf[len++] = g_MotorTurnDirection;
			temp_txbuf[len++] = g_MotorTurnLevel;
			break;
		case 0xA5:
			temp_txbuf[len++] = 0x06;
			temp_txbuf[len++] = ID;
			temp_txbuf[len++] = ret;
			break;
		case 0xA6:
			temp_txbuf[len++] = 0x05;
			temp_txbuf[len++] = ID;
			break;
		default:
		break;
	}
	temp_data = my_check_code_calculate(&temp_txbuf[3], len + 1 - 4);
	temp_txbuf[len++] = temp_data;
	
	HAL_UART_Transmit_DMA(&huart1, temp_txbuf,len);//������õ����ݷ��ͳ�ȥ
}

/*
	0xA1 ָ��	���õ�ǰ����ķ�����ٶ�
		AA AA 07 A1 01 01 5C			ǰ��1��
		AA AA 07 A1 01 02 5B			ǰ��2��
		AA AA 07 A1 01 03 5A			ǰ��3��
		AA AA 07 A1 01 04 59			ǰ��4��
		AA AA 07 A1 01 05 58			ǰ��5��
		AA AA 07 A1 02 01 5B			����1��
		AA AA 07 A1 02 02 5A			����2��
		AA AA 07 A1 02 03 59			����3��
		AA AA 07 A1 02 04 58			����4��
		AA AA 07 A1 02 05 57			����5��

	0xA2 ָ��	���õ�ǰС����ת���Լ�ת�䵵λ
		AA AA 06 A2 01 5C				��΢��ת��
		AA AA 06 A2 02 5B				�е���ת��
		AA AA 06 A2 03 5A				������ת��

	0xA3 ָ��	���õ�ǰС����ת���Լ�ת�䵵λ
		AA AA 06 A3 01 5B				��΢��ת��
		AA AA 06 A3 02 5A				�е���ת��
		AA AA 06 A3 03 59				������ת��
		
	0xA4 ָ��   ��ȡ��ǰ����ķ�����ٶ�
		AA AA 05 A4 5B
		���ӻظ�0xA4ָ��ʵ��
		AA AA 09 A4 01 02 01 03 54   
		// 01: ����ǰ��  02 :�ٶ��и��� 01:������ת�� 03��������ת��
	0xA5 ָ��   ֹͣ���ָ��
		AA AA 06 A5 00 5A				ָֹͣ��

*/

void control_protocol_analysis(uint8_t *data, uint8_t len)
{
	uint8_t command_id = 0;
	uint8_t param_valid = 0;
	command_id = data[3];
	switch(command_id)
	{
		case 0xA1:
			/*�ж�������Ч��*/
			if((1 <= data[4] && data[4] <= 2) && (data[5] <= 5))	//������Ч
			{
				g_MotorDirection = data[4];		//ǰ�����˷���ֵ
				g_MotorSpeedLevel = data[5];	//�ٶȵȼ���ֵ
				param_valid = 0;			
			}
			else	//������Ч
			{
				param_valid = 1;
			}
			my_uart3_tx_to_BLE(0xA1,param_valid);
			g_ControlMotorFlag = CONTROL_MOTOR_DIRECTION;	//���Ƶ����־λ���ڷ���
			break;
		case 0xA2:
			/*�ж�������Ч��*/
			if(data[4] <= 3)	//������Ч
			{
				g_MotorTurnDirection = MY_TURN_LEFT;		//��ת��
				g_MotorTurnLevel = data[4];					//ǰ�����˷���ֵ
				param_valid = 0;			
			}
			else	//������Ч
			{
				param_valid = 1;
			}
			my_uart3_tx_to_BLE(0xA2,param_valid);
			g_ControlMotorFlag = CONTROL_MOTOR_TURN;	//���Ƶ����־λ����ת��
			break;
		case 0xA3:
			/*�ж�������Ч��*/
			if(data[4] <= 3)	//������Ч
			{
				g_MotorTurnDirection = MY_TURN_RIGHT;		//��ת��
				g_MotorTurnLevel = data[4];					//ǰ�����˷���ֵ
				param_valid = 0;			
			}
			else	//������Ч
			{
				param_valid = 1;
			}
			my_uart3_tx_to_BLE(0xA2,param_valid);
			g_ControlMotorFlag = CONTROL_MOTOR_TURN;	//���Ƶ����־λ����ת��
			break;
		case 0xA4:
			my_uart3_tx_to_BLE(0xA4,0);
			break;
		case 0xA5:
			my_set_car_stop_func();
			my_uart3_tx_to_BLE(0xA5,0);
			g_ControlMotorFlag = CONTROL_MOTOR_STOP;	//���Ƶ����־λ����ֹͣ
		default:
			break;
	}
}

/*MOTOR ���� Э���������*/
void my_motor_control_protocol_analysis(void)
{
	uint8_t my_cal_len = 0;
	uint8_t ret;
	uint8_t temp_buf[256] = {0};
	if(recv_end_flag != SET)
		return;
	if(rx_buffer[0] != 0xAA || rx_buffer[1] != 0xAA)
	{
		rx_len = 0;
		memset(rx_buffer,0,rx_len);
		return;	
	}

	recv_end_flag = 0;
	my_cal_len = rx_len;
	rx_len = 0;
	memcpy(temp_buf,rx_buffer,my_cal_len);
	memset(rx_buffer,0,rx_len);
	print_array_hex(temp_buf,my_cal_len,"UART3 REV:");
	ret = my_check_code_analysis(&temp_buf[3],my_cal_len-3);
	if(ret != 0){
		print_log("check sum ERROR!\r\n");
		my_uart3_tx_to_BLE(0xA6,0);
		return;
	}
	else{
		print_log("check sum OK!\r\n");
		control_protocol_analysis(temp_buf,my_cal_len);
	}
	
}

//����С��ֹͣ
void my_set_car_stop_func(void)
{
	g_MotorDirection = MY_CAR_DIRECTION_IDLE;				//������� 0��ֹͣ 1��ǰ����2������
	g_MotorSpeedLevel = MY_CAR_SPEED_IDLE;					//����ٶȵȼ� 0��ֹ̬ͣ  1 = 100%��ȫ�٣�2 = 75% 3 = 50%4 = 30% 5 = 10%������٣�
	g_MotorTurnDirection = MY_TURN_IDLE;					//���ת�䷽�� 0����ת��  1����ת�䣬2����ת��
	g_MotorTurnLevel = MY_TURN_LEVEL_IDLE;					//���ת��ȼ� 0��ֹ̬ͣ  ת��ǿ�ȵȼ���1 = ��΢��2 = �еȣ�3 = ��ת
}

/* USER CODE END 1 */
