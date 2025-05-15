#include "motor.h"
#include "tim.h"
#include <stdlib.h>

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
			if(direction == MY_CAR_DIRECTION_FORWARD)
			{
				HAL_GPIO_WritePin(MOTOR_CW_1_GPIO_Port, MOTOR_CW_1_Pin,  GPIO_PIN_RESET);
			}else if(direction == MY_CAR_DIRECTION_BACKWARD)
			{
				HAL_GPIO_WritePin(MOTOR_CW_1_GPIO_Port, MOTOR_CW_1_Pin,  GPIO_PIN_SET);
			}else
			{
				HAL_GPIO_WritePin(MOTOR_CW_1_GPIO_Port, MOTOR_CW_1_Pin,  GPIO_PIN_RESET);
			}
            break;
        case 2:
			if(direction == MY_CAR_DIRECTION_FORWARD)
			{
				HAL_GPIO_WritePin(MOTOR_CW_2_GPIO_Port, MOTOR_CW_2_Pin,  GPIO_PIN_RESET);
			}else if(direction == MY_CAR_DIRECTION_BACKWARD)
			{
				HAL_GPIO_WritePin(MOTOR_CW_2_GPIO_Port, MOTOR_CW_2_Pin,  GPIO_PIN_SET);
			}else
			{
				HAL_GPIO_WritePin(MOTOR_CW_2_GPIO_Port, MOTOR_CW_2_Pin,  GPIO_PIN_RESET);
			}
            break;
        case 3:
			if(direction == MY_CAR_DIRECTION_FORWARD)
			{
				HAL_GPIO_WritePin(MOTOR_CW_3_GPIO_Port, MOTOR_CW_3_Pin,  GPIO_PIN_RESET);
			}else if(direction == MY_CAR_DIRECTION_BACKWARD)
			{
				HAL_GPIO_WritePin(MOTOR_CW_3_GPIO_Port, MOTOR_CW_3_Pin,  GPIO_PIN_SET);
			}else
			{
				HAL_GPIO_WritePin(MOTOR_CW_3_GPIO_Port, MOTOR_CW_3_Pin,  GPIO_PIN_RESET);
			}
            break;
//        case 4:
//            HAL_GPIO_WritePin(MOTOR_CW_4_GPIO_Port, MOTOR_CW_4_Pin,  direction == 1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
//            break;
        default: break;
    }
}

/**
 * @brief     同时控制4个电机的软启动速度控制(统一速度和方向版)
 * @param     target_speed_level   所有电机的速度等级：
 *                           1 = 100%（全速）
 *                           2 = 75%
 *                           3 = 50%
 *                           4 = 30%
 *                           5 = 10%（最低速）
 * @param     direction      所有电机的方向：
 *                           0 = 前进（正转）
 *                           1 = 后退（反转）
 * @param     accel_time_ms  加速时间(ms)，0=使用默认值200ms
 */
void motors_set_speed_soft(uint8_t target_speed_level,
                          uint8_t direction,
                          uint16_t accel_time_ms)
{
    const uint16_t duty_table[] = {1000, 750, 500, 300, 100};	//速度等级值
    TIM_HandleTypeDef *htim = &htim2;
    uint32_t tim_channels[] = {
        TIM_CHANNEL_1, TIM_CHANNEL_2,TIM_CHANNEL_3
    };
    
    // 参数检查
    if(target_speed_level < 1 || target_speed_level > 5) {
        return;
    }
    
    // 目标PWM值
    uint16_t target_pwm = duty_table[target_speed_level-1];
    
    // 获取所有电机当前PWM值
    uint16_t current_pwms[4];
    int32_t max_delta = 0;
    
    for(int i = 0; i < 3; i++) {
        // 设置电机方向
        motor_gpio_set_direction(i+1, direction);
        
        // 获取当前PWM值
        current_pwms[i] = __HAL_TIM_GetCompare(htim, tim_channels[i]);
        
        // 计算最大变化量(用于确定加速步数)//labs - 输入：一个长整型数（可以是正数或负数）- 输出：返回这个数的绝对值（总是正数）
		// 确保不管是加速还是减速 这个值都是正的
		int32_t delta = labs((int32_t)target_pwm - (int32_t)current_pwms[i]);
        if(delta > max_delta) {
            max_delta = delta;
        }
    }
    
    // 如果所有电机都已经是目标速度，直接返回
    if(max_delta == 0) {
        return;
    }
    
    // 计算加速参数
    uint16_t step_time_ms = 10; 								// 每步10ms
    accel_time_ms = (accel_time_ms == 0) ? 200 : accel_time_ms;	//默认200ms加速完成，如果传0则200ms内加速减速完成
	//- 这行代码使用了四舍五入的计算方法- step_time_ms/2 是为了实现四舍五入（加上半个步长）
	uint16_t steps = (accel_time_ms + step_time_ms/2) / step_time_ms;
	//- 确保步数至少为1
	//- 防止出现零步的情况，这可能导致电机无法启动
    steps = (steps == 0) ? 1 : steps;
    
    // 计算基础步进值和余数
    int32_t base_step = max_delta / steps;			//例如200ms  一共20步       如果增加1000ms  每步增加 1000/20 = 50ms
    int32_t remainder = max_delta % steps;			//余数为0
	//如果总变化量是545，需要10步完成
	//base_step = 545 ÷ 10 = 54
	//remainder = 545 % 10 = 5
	//那么前5步的increment为55，后5步为54
    // 执行平滑加速
    for(uint16_t step = 0; step < steps; step++) {
        // 计算当前步进值（包含余数分配）
        int32_t increment = base_step;
        if(remainder > 0) {
            increment++;
            remainder--;
        }
        
        // 更新所有电机PWM值
        for(int i = 0; i < 3; i++) {
            if(current_pwms[i] < target_pwm) {
                current_pwms[i] += increment;
                if(current_pwms[i] > target_pwm) {
                    current_pwms[i] = target_pwm;
                }
            } else if(current_pwms[i] > target_pwm) {
                current_pwms[i] -= increment;
                if(current_pwms[i] < target_pwm) {
                    current_pwms[i] = target_pwm;
                }
            }
            __HAL_TIM_SetCompare(htim, tim_channels[i], current_pwms[i]);
        }
        
        HAL_Delay(step_time_ms);
    }
    
    // 确保所有电机最终值精确
    for(int i = 0; i < 3; i++) {
        __HAL_TIM_SetCompare(htim, tim_channels[i], target_pwm);
    }
}


//设置小车运行以特定的速度等级和方向
void my_motor_car_run(uint8_t speed_level, uint8_t direction)
{
	motors_set_speed_soft(speed_level,direction,0);		//默认200ms加减速
}

/**
 * @brief     平滑停止某一路电机
 * @param     motor_channel 电机通道编号，范围为 1~4，分别对应 TIM 的 CH1~CH4
 * @param     decel_time_ms 减速时间(毫秒)，默认100ms(如果传入0)
 * @note      本函数使用 TIM2，需确保 TIM2 的 PWM 已经初始化并启动
 * @note      此函数会阻塞当前线程直到减速完成
 */
void motor_soft_stop(uint8_t motor_channel, uint16_t decel_time_ms)
{
    TIM_HandleTypeDef *htim = &htim2;
    const uint16_t DEFAULT_DECEL_TIME = 100; // 默认减速时间100ms
    const uint16_t STEP_TIME = 10;           // 每步10ms
    
    // 参数检查
    if (motor_channel < 1 || motor_channel > 4) return;
    if (decel_time_ms == 0) decel_time_ms = DEFAULT_DECEL_TIME;
    
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
    
    // 获取当前PWM值
    uint32_t current_pwm = __HAL_TIM_GetCompare(htim, tim_channel);
    if (current_pwm == 0) return;  // 已经停止
    
    // 计算减速步数
    uint16_t steps = decel_time_ms / STEP_TIME;
    if (steps == 0) steps = 1;
    uint32_t step_size = current_pwm / steps;
    
    // 逐步降低PWM
    while (current_pwm > 0)
    {
        if (current_pwm > step_size)
            current_pwm -= step_size;
        else
            current_pwm = 0;
            
        __HAL_TIM_SetCompare(htim, tim_channel, current_pwm);
        HAL_Delay(STEP_TIME);
    }
}

/**
 * @brief     同步平滑停止所有4路电机
 * @param     decel_time_ms 减速时间(毫秒)，如果需要200ms内停止电机，则传入200，否则默认是100ms停止电机
总步数(steps) = 减速时间 / 步进时间 = 100ms / 10ms = 10步
每步减少量(step_size) = max_pwm / steps = 800 / 10 = 80

 */
void stop_all_motors_sync(uint16_t decel_time_ms)
{
    TIM_HandleTypeDef *htim = &htim2;
    const uint16_t DEFAULT_DECEL_TIME = 100;
    const uint16_t STEP_TIME = 10;
    
    if(decel_time_ms == 0) decel_time_ms = DEFAULT_DECEL_TIME;
    
    // 获取所有通道当前PWM值
    uint32_t pwm_values[3] = {
        __HAL_TIM_GetCompare(htim, TIM_CHANNEL_1),
        __HAL_TIM_GetCompare(htim, TIM_CHANNEL_2),
        __HAL_TIM_GetCompare(htim, TIM_CHANNEL_3),
    };
    
    // 找出最大的PWM值作为基准
    uint32_t max_pwm = 0;
    for(int i = 0; i < 3; i++) {
        if(pwm_values[i] > max_pwm) max_pwm = pwm_values[i];
    }
    if(max_pwm == 0) return;  // 所有电机已经停止
    
    // 计算减速参数
    uint16_t steps = decel_time_ms / STEP_TIME;
    if(steps == 0) steps = 1;
    uint32_t step_size = max_pwm / steps;
    
    // 同步减速循环
    while(max_pwm > 0)
    {
        // 计算新PWM值（不能小于0）
        if(max_pwm > step_size) {
            max_pwm -= step_size;
        } else {
            max_pwm = 0;
        }
        
		// 更新所有通道（按比例减速）
		for(int i = 0; i < 3; i++) {
			// 只处理PWM值大于0的通道
			if(pwm_values[i] > 0) {
				// 计算新的PWM值
				uint32_t new_pwm = (pwm_values[i] * max_pwm) / (max_pwm + step_size);
				
				// 根据索引选择定时器通道
				uint32_t channel;
				if(i == 0) {
					channel = TIM_CHANNEL_1;
				} else if(i == 1) {
					channel = TIM_CHANNEL_2;
				} else {
					channel = TIM_CHANNEL_3;
				}
				
				// 设置新的PWM值
				__HAL_TIM_SetCompare(htim, channel, new_pwm);
				
				// 更新PWM值数组
				pwm_values[i] = new_pwm;
			}
		}
        HAL_Delay(STEP_TIME);
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
	stop_all_motors_sync(200);		//设置小车200ms所有电机停止
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


/**
 * @brief 计算单步PWM值（带边界保护）
 */
static uint16_t calculate_step_pwm(uint16_t current, uint16_t target, float ratio)
{
    int32_t result = current + (int32_t)(ratio * (target - current));
    // 边界保护
    if(target > current) {
        return (result > target) ? target : (uint16_t)result;
    } else {
        return (result < target) ? target : (uint16_t)result;
    }
}

/**
 * @brief 通道号转数组索引
 */
static uint8_t channel_to_index(uint32_t channel)
{
    // TIM_CHANNEL_x 到 0-3 的映射
    return (channel - TIM_CHANNEL_1); // 假设通道号是连续枚举值
}

/** 
 * @brief 带软启动的小车转弯控制（支持前进后退转弯）
 * @param turn_dir 转弯方向：0=左转，1=右转 
 * @param level 转弯强度等级：0=轻微，1=中等，2=急转 
 * @param accel_time_ms 加速时间(ms)，0=使用默认值(200ms) 
 */ 
void car_turn_soft(uint8_t turn_dir, uint8_t level, uint16_t accel_time_ms) 
{ 
	//添加3个电机的驱动方式  20250515
	uint8_t front = 0;
	uint8_t inner_rear = 0;
	uint8_t outer_rear = 0;
    // 参数检查 
    if( level > 3) 
		return; 
    
    // 配置参数 
    const uint16_t DEFAULT_ACCEL_TIME = 200; 
    const uint16_t STEP_TIME = 10; 
    const uint16_t speed_outer = 1000; // 外侧轮全速 
    const uint16_t speed_inner_table[3] = {700, 400, 200}; 
   
	// 获取当前方向
	uint8_t current_car_dir = (HAL_GPIO_ReadPin(MOTOR_CW_3_GPIO_Port, MOTOR_CW_3_Pin) == GPIO_PIN_SET) ? MY_CAR_DIRECTION_BACKWARD : MY_CAR_DIRECTION_FORWARD;
		
    // 确定内外侧轮子通道（保持当前运动方向）
    if(turn_dir == MY_TURN_LEFT) // 左转 
	{ 
		if(current_car_dir == MY_CAR_DIRECTION_FORWARD)
		{
			front = TIM_CHANNEL_3;			//前轮
			inner_rear = TIM_CHANNEL_2;		//内侧后轮
			outer_rear = TIM_CHANNEL_1;		//外侧后轮
		}
		else
		{
			front = TIM_CHANNEL_3;			//前轮
			outer_rear = TIM_CHANNEL_1;		//内侧后轮
			inner_rear = TIM_CHANNEL_2;		//外侧后轮
		}
    } 
	else 						// 右转 
	{ 
		if(current_car_dir == MY_CAR_DIRECTION_FORWARD)
		{
			front = TIM_CHANNEL_3;			//前轮
			outer_rear = TIM_CHANNEL_2;		//左后轮
			inner_rear = TIM_CHANNEL_1;		//右后轮
		}
		else
		{
			front = TIM_CHANNEL_3;			//前轮
			outer_rear = TIM_CHANNEL_1;		//左后轮
			inner_rear = TIM_CHANNEL_2;		//右后轮
		}
    } 
    
    // 获取当前PWM值
    uint16_t current_pwm[4]; 
    current_pwm[0] = __HAL_TIM_GetCompare(&htim2, TIM_CHANNEL_1); 
    current_pwm[1] = __HAL_TIM_GetCompare(&htim2, TIM_CHANNEL_2); 
    current_pwm[2] = __HAL_TIM_GetCompare(&htim2, TIM_CHANNEL_3); 

    // 计算目标值 
    uint16_t target_inner = speed_inner_table[level-1]; 
    
    // 初始化软启动参数 
    accel_time_ms = (accel_time_ms == 0) ? DEFAULT_ACCEL_TIME : accel_time_ms; 
    uint16_t steps = (accel_time_ms + STEP_TIME/2) / STEP_TIME; // 四舍五入 
    steps = (steps == 0) ? 1 : steps; // 确保至少1步 

    // 软启动过程
    for(uint16_t step = 0; step <= steps; step++) { 
        float ratio = (float)step / steps; 
        
        // 前轮处理				//最大速度
        uint16_t front_pwm = calculate_step_pwm(current_pwm[channel_to_index(front)], speed_outer, ratio); 
		
		
		// 后外侧轮处理			//最大速度
        uint16_t outer_rear_pwm  = calculate_step_pwm(current_pwm[channel_to_index(outer_rear)], speed_outer, ratio); 
		// 后内测轮				//设置的速度级别
        uint16_t inner_rear_pwm  = calculate_step_pwm(current_pwm[channel_to_index(inner_rear)], target_inner, ratio); 

        // 设置PWM
        __HAL_TIM_SetCompare(&htim2, front, front_pwm); 
        __HAL_TIM_SetCompare(&htim2, outer_rear,  outer_rear_pwm); 
        __HAL_TIM_SetCompare(&htim2, inner_rear, inner_rear_pwm); 
        
        HAL_Delay(STEP_TIME); 
    } 

    // 最终状态强制同步
    __HAL_TIM_SetCompare(&htim2, front, speed_outer); 
    __HAL_TIM_SetCompare(&htim2, outer_rear, speed_outer); 
    __HAL_TIM_SetCompare(&htim2, inner_rear, target_inner); 
} 



void my_motor_control_task(void)
{
	uint8_t current_car_dir = 0;
	if(g_ControlMotorFlag == CONTROL_MOTOR_IDLE)
		return;
	switch(g_ControlMotorFlag)
	{
		case CONTROL_MOTOR_DIRECTION:
			printf("[MOTOR] set motor run and speedlevel!\r\n");
			//做了软启动速度控制 的加减速  因此不需要再停止电机
//			my_motor_car_stop();									//先让4个电机电机停止
//			HAL_Delay(1000);										//延时1S
		    // 获取当前方向
			current_car_dir = (HAL_GPIO_ReadPin(MOTOR_CW_1_GPIO_Port, MOTOR_CW_1_Pin) == GPIO_PIN_SET) ? MY_CAR_DIRECTION_BACKWARD : MY_CAR_DIRECTION_FORWARD;
			// 如果方向不一致，先停车
			if(current_car_dir != g_MotorDirection) 
			{
				printf("[MOTOR] set motor turn direction!\r\n");
				my_motor_car_stop();									//4个电机电机软停止
				// 等待电机完全停止
				HAL_Delay(100);  // 100ms等待时间，可根据实际情况调整
			}
			my_motor_car_run(g_MotorSpeedLevel,g_MotorDirection);	//运行电机
			break;
		case CONTROL_MOTOR_TURN:
			printf("[MOTOR] set motor turn!,g_MotorTurnDirection:%d!\r\n",g_MotorTurnDirection);
//			my_motor_car_stop();									//先让4个电机电机停止
//			HAL_Delay(1000);										//延时1S
//			car_turn(g_MotorTurnDirection,g_MotorTurnLevel);		//小车转向
			car_turn_soft(g_MotorTurnDirection,g_MotorTurnLevel,0);	//线性转向，因此不需要停止，即使现在小车处于前进或者后退状态
			break;
		case CONTROL_MOTOR_STOP:
			printf("[MOTOR] set motor stop!\r\n");
			my_motor_car_stop();									//4个电机电机停止
			break;
		default:
		break;			
	}
	g_ControlMotorFlag = CONTROL_MOTOR_IDLE;
}	




