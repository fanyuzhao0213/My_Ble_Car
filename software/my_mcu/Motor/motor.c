#include "motor.h"
#include "tim.h"

uint8_t g_MotorDirection = MY_CAR_DIRECTION_IDLE;				//电机方向 0：停止 1：前进，2：后退
uint8_t g_MotorSpeedLevel = MY_CAR_SPEED_IDLE;					//电机速度等级 0：停止态  1 = 100%（全速）2 = 75% 3 = 50%4 = 30% 5 = 10%（最低速）
uint8_t g_MotorTurnDirection = MY_TURN_IDLE;					//电机转弯方向 0：无转弯  1：左转弯，2：右转弯
uint8_t g_MotorTurnLevel = MY_TURN_LEVEL_IDLE;					//电机转弯等级 0：停止态  转弯强度等级：1 = 轻微，2 = 中等，3 = 急转
/**
 * @brief 设置电机方向（前进 or 后退）
 * @param motor_channel 电机通道编号 1~4
 * @param direction     方向，0 = 前进，1 = 后退
 */
void motor_gpio_set_direction(uint8_t motor_channel, uint8_t direction)
{
    switch (motor_channel)
    {
        case 1:
            HAL_GPIO_WritePin(MOTOR_CW_1_GPIO_Port, MOTOR_CW_1_Pin,  direction == 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
            HAL_GPIO_WritePin(MOTOR_CW_1_GPIO_Port, MOTOR_CW_1_Pin, direction == 1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
            break;
        case 2:
            HAL_GPIO_WritePin(MOTOR_CW_2_GPIO_Port, MOTOR_CW_2_Pin,  direction == 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
            HAL_GPIO_WritePin(MOTOR_CW_2_GPIO_Port, MOTOR_CW_2_Pin, direction == 1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
            break;
        case 3:
            HAL_GPIO_WritePin(MOTOR_CW_3_GPIO_Port, MOTOR_CW_3_Pin,  direction == 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
            HAL_GPIO_WritePin(MOTOR_CW_3_GPIO_Port, MOTOR_CW_3_Pin, direction == 1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
            break;
        case 4:
            HAL_GPIO_WritePin(MOTOR_CW_4_GPIO_Port, MOTOR_CW_4_Pin,  direction == 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
            HAL_GPIO_WritePin(MOTOR_CW_4_GPIO_Port, MOTOR_CW_4_Pin, direction == 1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
            break;
        default: break;
    }
}


/**
 * @brief     控制某一路电机的速度和方向
 * @param     motor_channel 电机通道编号，范围为 1~4，分别对应 TIM2 的 CH1~CH4
 * @param     speed_level   电机速度等级：
 *                           1 = 100%（全速）
 *                           2 = 75%
 *                           3 = 50%
 *                           4 = 30%
 *                           5 = 10%（最低速）
 * @param     direction      电机方向：
 *                           0 = 前进（正转）
 *                           1 = 后退（反转）
 * @note      需要在 motor_gpio_set_direction() 中定义每个通道控制方向的 GPIO。
 */
void motor_set_speed_dir(uint8_t motor_channel, uint8_t speed_level, uint8_t direction)
{
    const uint16_t duty_table[] = {1000, 750, 500, 300, 100};
    TIM_HandleTypeDef *htim = &htim2;

    if (motor_channel < 1 || motor_channel > 4 ||  speed_level > 5 || speed_level < 1)
        return;

    // 映射 PWM 通道
    uint32_t tim_channel;
    switch (motor_channel)
    {
        case 1: tim_channel = TIM_CHANNEL_1; break;
        case 2: tim_channel = TIM_CHANNEL_2; break;
        case 3: tim_channel = TIM_CHANNEL_3; break;
        case 4: tim_channel = TIM_CHANNEL_4; break;
        default: return;
    }

    // 设置 PWM 占空比
    __HAL_TIM_SetCompare(htim, tim_channel, duty_table[speed_level-1]);

    // 设置方向
    motor_gpio_set_direction(motor_channel, direction);
}

//设置小车运行以特定的速度等级和方向
void my_motor_car_run(uint8_t speed_level, uint8_t direction)
{
	for(uint8_t i =0; i<=3; i++)
	{
		motor_set_speed_dir(i+1,speed_level,direction);
	}
}
/**
 * @brief     设置某一路电机停止
 * @param     motor_channel 电机通道编号，范围为 1~4，分别对应 TIM 的 CH1~CH4
 * @note      本函数使用 TIM2，需确保 TIM2 的 PWM 已经初始化并启动。
 */
void motor_stop(uint8_t motor_channel)
{
    TIM_HandleTypeDef *htim = &htim2;

    // 防止越界
    if (motor_channel < 1 || motor_channel > 4)
        return;

    // 映射通道号
    uint32_t tim_channel;
    switch (motor_channel)
    {
        case 1: tim_channel = TIM_CHANNEL_1; break;
        case 2: tim_channel = TIM_CHANNEL_2; break;
        case 3: tim_channel = TIM_CHANNEL_3; break;
        case 4: tim_channel = TIM_CHANNEL_4; break;
        default: return;
    }

    __HAL_TIM_SetCompare(htim, tim_channel, 0);
}

//设置小车停止
void my_motor_car_stop(void)
{
	motor_stop(1);
	motor_stop(2);
	motor_stop(3);
	motor_stop(4);
}

/**
 * @brief 控制小车左/右转弯，支持 3 个转弯级别
 * @param turn_dir   转弯方向：0 = 左转，1 = 右转
 * @param level      转弯强度等级：0 = 轻微，1 = 中等，2 = 急转
 */
void car_turn(uint8_t turn_dir, uint8_t level)
{
    // 检查参数合法性
    if (turn_dir > 1 || level > 2) return;

    // 占空比等级（强度）：每种级别下，外侧轮子全速，内侧轮子不同程度减速
    const uint16_t speed_outer = 1000; // 外侧轮子：全速
    const uint16_t speed_inner_table[3] = {700, 400, 200}; // 内侧轮子速度表

    if (turn_dir == MY_TURN_LEFT) // 左转
    {
        // 内侧：左轮（1,3）减速，右轮（2,4）全速
        __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, speed_inner_table[level]); // 左前
        __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_3, speed_inner_table[level]); // 左后
        __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_2, speed_outer);              // 右前
        __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_4, speed_outer);              // 右后
    }
    else // 右转
    {
        // 内侧：右轮（2,4）减速，左轮（1,3）全速
        __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, speed_outer);              // 左前
        __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_3, speed_outer);              // 左后
        __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_2, speed_inner_table[level]); // 右前
        __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_4, speed_inner_table[level]); // 右后
    }
}



void my_motor_control_task(void)
{
	if(g_ControlMotorFlag == CONTROL_MOTOR_IDLE)
		return;
	switch(g_ControlMotorFlag)
	{
		case CONTROL_MOTOR_DIRECTION:
			printf("[MAIN] set motor run and speedlevel!\r\n");
			my_motor_car_stop();									//先让4个电机电机停止
			HAL_Delay(1000);										//延时1S
			my_motor_car_run(g_MotorSpeedLevel,g_MotorDirection);	//运行电机
			break;
		case CONTROL_MOTOR_TURN:
			printf("[MAIN] set motor turn!\r\n");
			my_motor_car_stop();									//先让4个电机电机停止
			HAL_Delay(1000);										//延时1S
			car_turn(g_MotorTurnDirection,g_MotorTurnLevel);		//小车转向
			break;
		case CONTROL_MOTOR_STOP:
			printf("[MAIN] set motor stop!\r\n");
			my_motor_car_stop();									//4个电机电机停止
			break;
		default:
		break;			
	}
	g_ControlMotorFlag = CONTROL_MOTOR_IDLE;
}	




