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
	uint8_t tmp_flag =__HAL_UART_GET_FLAG(&huart3,UART_FLAG_IDLE); //获取IDLE标志位
	if((tmp_flag != RESET))//通过标志位判断接收是否结束
	{ 
		recv_end_flag = 1; //置1表明接收结束
		__HAL_UART_CLEAR_IDLEFLAG(&huart3);//清除标志位
		HAL_UART_DMAStop(&huart3); 
		uint8_t temp=__HAL_DMA_GET_COUNTER(&hdma_usart3_rx);
		rx_len = MAXSIZE-temp; //计算出数据长度
		printf("dma rx_len:%d\r\n",rx_len);
//		HAL_UART_Transmit_DMA(&huart3, rx_buffer,rx_len);//将受到的数据发送出去
		HAL_UART_Receive_DMA(&huart3,rx_buffer,MAXSIZE);//开启DMA接收，方便下一次接收数据
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
/*外部中断触发函数*/

// 添加消抖时间定义
#define DEBOUNCE_TIME  10  // 10us消抖时间

// 修改中断回调函数
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) 
{ 
    static uint32_t last_interrupt_time = 0;
    uint32_t current_time;
    uint8_t sensor;
    
    // 获取当前时间
    current_time = __HAL_TIM_GET_COUNTER(&htim3);
    
    // 消抖处理
    if((current_time - last_interrupt_time) < DEBOUNCE_TIME)
    {
        return;  // 忽略抖动
    }
    last_interrupt_time = current_time;
    
    // 确定是哪个传感器触发的中断 
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
    
    // 根据电平状态记录时间 
    if(HAL_GPIO_ReadPin(sensor == SENSOR_FRONT ? Front_Echo_GPIO_Port : Rear_Echo_GPIO_Port, 
                        GPIO_Pin) == GPIO_PIN_SET) 
    { 
        // 上升沿，清零计数器并开始计时
        __HAL_TIM_SET_COUNTER(&htim1, 0);
		rising_count++;
        HC_SR04_DATA[sensor].echo_start = 0;
        HC_SR04_DATA[sensor].measure_complete = 0;  // 清除完成标志
		
    } 
    else 
    { 
		falling_count++;
        // 下降沿，记录计数器值
        HC_SR04_DATA[sensor].echo_end = __HAL_TIM_GET_COUNTER(&htim1);
        // 检查时间是否在合理范围内(最大25ms)
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
		if(ic_state[Motor_channel] == runing)	/* 判断是否为是 正在运行状态 */
		{
			
		}
	}
	if(htim == &htim1)							//定时器1中断 进去一次是65ms
    {
//		Ultrasonic_Task_Handler();				//超声波测量前后距离task
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
	if(htim == &htim3){							         /* 判断是否为定时器TIM2 */
		if (ic_state[Motor_channel] == idle) // 正确判断状态
		{
				ic_state[Motor_channel] = runing; // 设置状态为 running
				// 设置当前通道为下降沿
        switch(Motor_channel) {
            case 0: __HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING); break;
            case 1: __HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_FALLING); break;
            case 2: __HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_FALLING); break;
            case 3: __HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_4, TIM_INPUTCHANNELPOLARITY_FALLING); break;
        }
		 }else{								             						/* 此时不是空闲状态 */
				ic_state[Motor_channel] = idle;						     		/* 将状态设置为 空闲  继续捕获*/
				// 设置当前通道为上升沿
        switch(Motor_channel) {
            case 0: 
							Motor1_Number_Of_Pulses++;
//							//电机减速比为90，因此一圈有810个脉冲
//							if(Motor1_Number_Of_Pulses >= 300)
//							{
//								__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, 500);    //修改比较值，修改占空比
//							}
//							if(Motor1_Number_Of_Pulses >= 500)
//							{
//								__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, 200);    //修改比较值，修改占空比

//							}
//							if(Motor1_Number_Of_Pulses >= 700)
//							{
//								__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, 100);    //修改比较值，修改占空比比
//							}
//							if(Motor1_Number_Of_Pulses >= 810)
//							{
//								Motor1_Number_Of_Pulses = 0;
//								__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, 0);    //修改比较值，修改占空比
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
		 	 __HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_1,TIM_INPUTCHANNELPOLARITY_RISING);/* 将TIM2的通道1捕获极性设置为上升沿捕获 */
		 }
	}
}

/* USER CODE END 1 */

