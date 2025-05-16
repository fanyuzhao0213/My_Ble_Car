/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "main.h"
#include "stm32f1xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "HC_SR04.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

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
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern DMA_HandleTypeDef hdma_usart3_rx;
extern DMA_HandleTypeDef hdma_usart3_tx;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M3 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles DMA1 channel2 global interrupt.
  */
void DMA1_Channel2_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel2_IRQn 0 */

  /* USER CODE END DMA1_Channel2_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart3_tx);
  /* USER CODE BEGIN DMA1_Channel2_IRQn 1 */

  /* USER CODE END DMA1_Channel2_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel3 global interrupt.
  */
void DMA1_Channel3_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel3_IRQn 0 */

  /* USER CODE END DMA1_Channel3_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart3_rx);
  /* USER CODE BEGIN DMA1_Channel3_IRQn 1 */

  /* USER CODE END DMA1_Channel3_IRQn 1 */
}

/**
  * @brief This function handles TIM1 update interrupt.
  */
void TIM1_UP_IRQHandler(void)
{
  /* USER CODE BEGIN TIM1_UP_IRQn 0 */

  /* USER CODE END TIM1_UP_IRQn 0 */
  HAL_TIM_IRQHandler(&htim1);
  /* USER CODE BEGIN TIM1_UP_IRQn 1 */

  /* USER CODE END TIM1_UP_IRQn 1 */
}

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */

  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */

  /* USER CODE END TIM2_IRQn 1 */
}

/**
  * @brief This function handles TIM3 global interrupt.
  */
void TIM3_IRQHandler(void)
{
  /* USER CODE BEGIN TIM3_IRQn 0 */

  /* USER CODE END TIM3_IRQn 0 */
  HAL_TIM_IRQHandler(&htim3);
  /* USER CODE BEGIN TIM3_IRQn 1 */

  /* USER CODE END TIM3_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief This function handles USART3 global interrupt.
  */
void USART3_IRQHandler(void)
{
  /* USER CODE BEGIN USART3_IRQn 0 */

  /* USER CODE END USART3_IRQn 0 */
  HAL_UART_IRQHandler(&huart3);
  /* USER CODE BEGIN USART3_IRQn 1 */
	uint8_t tmp_flag =__HAL_UART_GET_FLAG(&huart3,UART_FLAG_IDLE); //��ȡIDLE��־λ
	if((tmp_flag != RESET))//ͨ����־λ�жϽ����Ƿ����
	{ 
		recv_end_flag = 1; //��1�������ս���
		__HAL_UART_CLEAR_IDLEFLAG(&huart3);//�����־λ
		HAL_UART_DMAStop(&huart3); 
		uint8_t temp=__HAL_DMA_GET_COUNTER(&hdma_usart3_rx);
		rx_len = MAXSIZE-temp; //��������ݳ���
		printf("dma rx_len:%d\r\n",rx_len);
//		HAL_UART_Transmit_DMA(&huart3, rx_buffer,rx_len);//���ܵ������ݷ��ͳ�ȥ
		HAL_UART_Receive_DMA(&huart3,rx_buffer,MAXSIZE);//����DMA���գ�������һ�ν�������
	}
  /* USER CODE END USART3_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[15:10] interrupts.
  */
void EXTI15_10_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI15_10_IRQn 0 */

  /* USER CODE END EXTI15_10_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(Front_Echo_Pin);
  HAL_GPIO_EXTI_IRQHandler(Rear_Echo_Pin);
  /* USER CODE BEGIN EXTI15_10_IRQn 1 */

  /* USER CODE END EXTI15_10_IRQn 1 */
}

/* USER CODE BEGIN 1 */
/*�ⲿ�жϴ�������*/

// �������ʱ�䶨��
#define DEBOUNCE_TIME  10  // 10us����ʱ��

// �޸��жϻص�����
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) 
{ 
    static uint32_t last_interrupt_time = 0;
    uint32_t current_time;
    uint8_t sensor;
    
    // ��ȡ��ǰʱ��
    current_time = __HAL_TIM_GET_COUNTER(&htim3);
    
    // ��������
    if((current_time - last_interrupt_time) < DEBOUNCE_TIME)
    {
        return;  // ���Զ���
    }
    last_interrupt_time = current_time;
    
    // ȷ�����ĸ��������������ж� 
    if(GPIO_Pin == Front_Echo_Pin) 
    { 
        sensor = SENSOR_FRONT; 
    } 
    else if(GPIO_Pin == Rear_Echo_Pin) 
    { 
        sensor = SENSOR_BACK; 
    } 
    else 
        return; 
    
    // ���ݵ�ƽ״̬��¼ʱ�� 
    if(HAL_GPIO_ReadPin(sensor == SENSOR_FRONT ? Front_Echo_GPIO_Port : Rear_Echo_GPIO_Port, 
                        GPIO_Pin) == GPIO_PIN_SET) 
    { 
        // �����أ��������������ʼ��ʱ
        __HAL_TIM_SET_COUNTER(&htim1, 0);
		rising_count++;
        HC_SR04_DATA[sensor].echo_start = 0;
        HC_SR04_DATA[sensor].measure_complete = 0;  // �����ɱ�־
		
    } 
    else 
    { 
		falling_count++;
        // �½��أ���¼������ֵ
        HC_SR04_DATA[sensor].echo_end = __HAL_TIM_GET_COUNTER(&htim1);
        // ���ʱ���Ƿ��ں���Χ��(���25ms)
        if(HC_SR04_DATA[sensor].echo_end < 25000)
        {
            HC_SR04_DATA[sensor].measure_complete = 1;
        }
    } 
} 



void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static uint8_t hc_sr04_count =0;
	if(htim == &htim3)
	{								    		
		if(ic_state[Motor_channel] == runing)	/* �ж��Ƿ�Ϊ�� ��������״̬ */
		{
			
		}
	}
	if(htim == &htim1)							//��ʱ��1�ж� ��ȥһ����65ms
    {
//		Ultrasonic_Task_Handler();				//����������ǰ�����task
//        HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_8);
    }
}
	
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) Motor_channel = 0;
	else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) Motor_channel = 1;
	else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) Motor_channel = 2;
	else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4) Motor_channel = 3;
	else return;
	if(htim == &htim3){							         /* �ж��Ƿ�Ϊ��ʱ��TIM2 */
		if (ic_state[Motor_channel] == idle) // ��ȷ�ж�״̬
		{
				ic_state[Motor_channel] = runing; // ����״̬Ϊ running
				// ���õ�ǰͨ��Ϊ�½���
        switch(Motor_channel) {
            case 0: __HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING); break;
            case 1: __HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_FALLING); break;
            case 2: __HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_FALLING); break;
            case 3: __HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_4, TIM_INPUTCHANNELPOLARITY_FALLING); break;
        }
		 }else{								             						/* ��ʱ���ǿ���״̬ */
				ic_state[Motor_channel] = idle;						     		/* ��״̬����Ϊ ����  ��������*/
				// ���õ�ǰͨ��Ϊ������
        switch(Motor_channel) {
            case 0: 
							Motor1_Number_Of_Pulses++;
//							//������ٱ�Ϊ90�����һȦ��810������
//							if(Motor1_Number_Of_Pulses >= 300)
//							{
//								__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, 500);    //�޸ıȽ�ֵ���޸�ռ�ձ�
//							}
//							if(Motor1_Number_Of_Pulses >= 500)
//							{
//								__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, 200);    //�޸ıȽ�ֵ���޸�ռ�ձ�

//							}
//							if(Motor1_Number_Of_Pulses >= 700)
//							{
//								__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, 100);    //�޸ıȽ�ֵ���޸�ռ�ձȱ�
//							}
//							if(Motor1_Number_Of_Pulses >= 810)
//							{
//								Motor1_Number_Of_Pulses = 0;
//								__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, 0);    //�޸ıȽ�ֵ���޸�ռ�ձ�
//							}
							__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING); 
						break;
            case 1:
							__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_RISING); 
						break;
            case 2: 
							__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_RISING); 
						break;
            case 3:
							__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_4, TIM_INPUTCHANNELPOLARITY_RISING); 
						break;
        }
		 	 __HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_1,TIM_INPUTCHANNELPOLARITY_RISING);/* ��TIM2��ͨ��1����������Ϊ�����ز��� */
		 }
	}
}

/* USER CODE END 1 */

