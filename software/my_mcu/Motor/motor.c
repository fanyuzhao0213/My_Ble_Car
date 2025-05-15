#include "motor.h"
#include "tim.h"
#include <stdlib.h>

uint8_t g_MotorDirection = MY_CAR_DIRECTION_IDLE;				//������� 0��ֹͣ 1��ǰ����2������
uint8_t g_MotorSpeedLevel = MY_CAR_SPEED_IDLE;					//����ٶȵȼ� 0��ֹ̬ͣ  1 = 100%��ȫ�٣�2 = 75% 3 = 50%4 = 30% 5 = 10%������٣�
uint8_t g_MotorTurnDirection = MY_TURN_IDLE;					//���ת�䷽�� 0����ת��  1����ת�䣬2����ת��
uint8_t g_MotorTurnLevel = MY_TURN_LEVEL_IDLE;					//���ת��ȼ� 0��ֹ̬ͣ  ת��ǿ�ȵȼ���1 = ��΢��2 = �еȣ�3 = ��ת
/**
 * @brief ���õ������ǰ�� or ���ˣ�
 * @param motor_channel ���ͨ����� 1~4
 * @param direction     ����0 = ǰ����1 = ����
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
 * @brief     ͬʱ����4��������������ٶȿ���(ͳһ�ٶȺͷ����)
 * @param     target_speed_level   ���е�����ٶȵȼ���
 *                           1 = 100%��ȫ�٣�
 *                           2 = 75%
 *                           3 = 50%
 *                           4 = 30%
 *                           5 = 10%������٣�
 * @param     direction      ���е���ķ���
 *                           0 = ǰ������ת��
 *                           1 = ���ˣ���ת��
 * @param     accel_time_ms  ����ʱ��(ms)��0=ʹ��Ĭ��ֵ200ms
 */
void motors_set_speed_soft(uint8_t target_speed_level,
                          uint8_t direction,
                          uint16_t accel_time_ms)
{
    const uint16_t duty_table[] = {1000, 750, 500, 300, 100};	//�ٶȵȼ�ֵ
    TIM_HandleTypeDef *htim = &htim2;
    uint32_t tim_channels[] = {
        TIM_CHANNEL_1, TIM_CHANNEL_2,TIM_CHANNEL_3
    };
    
    // �������
    if(target_speed_level < 1 || target_speed_level > 5) {
        return;
    }
    
    // Ŀ��PWMֵ
    uint16_t target_pwm = duty_table[target_speed_level-1];
    
    // ��ȡ���е����ǰPWMֵ
    uint16_t current_pwms[4];
    int32_t max_delta = 0;
    
    for(int i = 0; i < 3; i++) {
        // ���õ������
        motor_gpio_set_direction(i+1, direction);
        
        // ��ȡ��ǰPWMֵ
        current_pwms[i] = __HAL_TIM_GetCompare(htim, tim_channels[i]);
        
        // �������仯��(����ȷ�����ٲ���)//labs - ���룺һ����������������������������- ���������������ľ���ֵ������������
		// ȷ�������Ǽ��ٻ��Ǽ��� ���ֵ��������
		int32_t delta = labs((int32_t)target_pwm - (int32_t)current_pwms[i]);
        if(delta > max_delta) {
            max_delta = delta;
        }
    }
    
    // ������е�����Ѿ���Ŀ���ٶȣ�ֱ�ӷ���
    if(max_delta == 0) {
        return;
    }
    
    // ������ٲ���
    uint16_t step_time_ms = 10; 								// ÿ��10ms
    accel_time_ms = (accel_time_ms == 0) ? 200 : accel_time_ms;	//Ĭ��200ms������ɣ������0��200ms�ڼ��ټ������
	//- ���д���ʹ������������ļ��㷽��- step_time_ms/2 ��Ϊ��ʵ���������루���ϰ��������
	uint16_t steps = (accel_time_ms + step_time_ms/2) / step_time_ms;
	//- ȷ����������Ϊ1
	//- ��ֹ�����㲽�����������ܵ��µ���޷�����
    steps = (steps == 0) ? 1 : steps;
    
    // �����������ֵ������
    int32_t base_step = max_delta / steps;			//����200ms  һ��20��       �������1000ms  ÿ������ 1000/20 = 50ms
    int32_t remainder = max_delta % steps;			//����Ϊ0
	//����ܱ仯����545����Ҫ10�����
	//base_step = 545 �� 10 = 54
	//remainder = 545 % 10 = 5
	//��ôǰ5����incrementΪ55����5��Ϊ54
    // ִ��ƽ������
    for(uint16_t step = 0; step < steps; step++) {
        // ���㵱ǰ����ֵ�������������䣩
        int32_t increment = base_step;
        if(remainder > 0) {
            increment++;
            remainder--;
        }
        
        // �������е��PWMֵ
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
    
    // ȷ�����е������ֵ��ȷ
    for(int i = 0; i < 3; i++) {
        __HAL_TIM_SetCompare(htim, tim_channels[i], target_pwm);
    }
}


//����С���������ض����ٶȵȼ��ͷ���
void my_motor_car_run(uint8_t speed_level, uint8_t direction)
{
	motors_set_speed_soft(speed_level,direction,0);		//Ĭ��200ms�Ӽ���
}

/**
 * @brief     ƽ��ֹͣĳһ·���
 * @param     motor_channel ���ͨ����ţ���ΧΪ 1~4���ֱ��Ӧ TIM �� CH1~CH4
 * @param     decel_time_ms ����ʱ��(����)��Ĭ��100ms(�������0)
 * @note      ������ʹ�� TIM2����ȷ�� TIM2 �� PWM �Ѿ���ʼ��������
 * @note      �˺�����������ǰ�߳�ֱ���������
 */
void motor_soft_stop(uint8_t motor_channel, uint16_t decel_time_ms)
{
    TIM_HandleTypeDef *htim = &htim2;
    const uint16_t DEFAULT_DECEL_TIME = 100; // Ĭ�ϼ���ʱ��100ms
    const uint16_t STEP_TIME = 10;           // ÿ��10ms
    
    // �������
    if (motor_channel < 1 || motor_channel > 4) return;
    if (decel_time_ms == 0) decel_time_ms = DEFAULT_DECEL_TIME;
    
    // ӳ��ͨ����
    uint32_t tim_channel;
    switch (motor_channel)
    {
        case 1: tim_channel = TIM_CHANNEL_1; break;
        case 2: tim_channel = TIM_CHANNEL_2; break;
        case 3: tim_channel = TIM_CHANNEL_3; break;
        case 4: tim_channel = TIM_CHANNEL_4; break;
        default: return;
    }
    
    // ��ȡ��ǰPWMֵ
    uint32_t current_pwm = __HAL_TIM_GetCompare(htim, tim_channel);
    if (current_pwm == 0) return;  // �Ѿ�ֹͣ
    
    // ������ٲ���
    uint16_t steps = decel_time_ms / STEP_TIME;
    if (steps == 0) steps = 1;
    uint32_t step_size = current_pwm / steps;
    
    // �𲽽���PWM
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
 * @brief     ͬ��ƽ��ֹͣ����4·���
 * @param     decel_time_ms ����ʱ��(����)�������Ҫ200ms��ֹͣ���������200������Ĭ����100msֹͣ���
�ܲ���(steps) = ����ʱ�� / ����ʱ�� = 100ms / 10ms = 10��
ÿ��������(step_size) = max_pwm / steps = 800 / 10 = 80

 */
void stop_all_motors_sync(uint16_t decel_time_ms)
{
    TIM_HandleTypeDef *htim = &htim2;
    const uint16_t DEFAULT_DECEL_TIME = 100;
    const uint16_t STEP_TIME = 10;
    
    if(decel_time_ms == 0) decel_time_ms = DEFAULT_DECEL_TIME;
    
    // ��ȡ����ͨ����ǰPWMֵ
    uint32_t pwm_values[3] = {
        __HAL_TIM_GetCompare(htim, TIM_CHANNEL_1),
        __HAL_TIM_GetCompare(htim, TIM_CHANNEL_2),
        __HAL_TIM_GetCompare(htim, TIM_CHANNEL_3),
    };
    
    // �ҳ�����PWMֵ��Ϊ��׼
    uint32_t max_pwm = 0;
    for(int i = 0; i < 3; i++) {
        if(pwm_values[i] > max_pwm) max_pwm = pwm_values[i];
    }
    if(max_pwm == 0) return;  // ���е���Ѿ�ֹͣ
    
    // ������ٲ���
    uint16_t steps = decel_time_ms / STEP_TIME;
    if(steps == 0) steps = 1;
    uint32_t step_size = max_pwm / steps;
    
    // ͬ������ѭ��
    while(max_pwm > 0)
    {
        // ������PWMֵ������С��0��
        if(max_pwm > step_size) {
            max_pwm -= step_size;
        } else {
            max_pwm = 0;
        }
        
		// ��������ͨ�������������٣�
		for(int i = 0; i < 3; i++) {
			// ֻ����PWMֵ����0��ͨ��
			if(pwm_values[i] > 0) {
				// �����µ�PWMֵ
				uint32_t new_pwm = (pwm_values[i] * max_pwm) / (max_pwm + step_size);
				
				// ��������ѡ��ʱ��ͨ��
				uint32_t channel;
				if(i == 0) {
					channel = TIM_CHANNEL_1;
				} else if(i == 1) {
					channel = TIM_CHANNEL_2;
				} else {
					channel = TIM_CHANNEL_3;
				}
				
				// �����µ�PWMֵ
				__HAL_TIM_SetCompare(htim, channel, new_pwm);
				
				// ����PWMֵ����
				pwm_values[i] = new_pwm;
			}
		}
        HAL_Delay(STEP_TIME);
    }
}

/**
 * @brief     ����ĳһ·���ֹͣ
 * @param     motor_channel ���ͨ����ţ���ΧΪ 1~4���ֱ��Ӧ TIM �� CH1~CH4
 * @note      ������ʹ�� TIM2����ȷ�� TIM2 �� PWM �Ѿ���ʼ����������
 */
void motor_stop(uint8_t motor_channel)
{
    TIM_HandleTypeDef *htim = &htim2;

    // ��ֹԽ��
    if (motor_channel < 1 || motor_channel > 4)
        return;

    // ӳ��ͨ����
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

//����С��ֹͣ
void my_motor_car_stop(void)
{
	stop_all_motors_sync(200);		//����С��200ms���е��ֹͣ
}

/**
 * @brief ����С����/��ת�䣬֧�� 3 ��ת�伶��
 * @param turn_dir   ת�䷽��0 = ��ת��1 = ��ת
 * @param level      ת��ǿ�ȵȼ���0 = ��΢��1 = �еȣ�2 = ��ת
 */
void car_turn(uint8_t turn_dir, uint8_t level)
{
    // �������Ϸ���
    if (turn_dir > 1 || level > 2) return;

    // ռ�ձȵȼ���ǿ�ȣ���ÿ�ּ����£��������ȫ�٣��ڲ����Ӳ�ͬ�̶ȼ���
    const uint16_t speed_outer = 1000; // ������ӣ�ȫ��
    const uint16_t speed_inner_table[3] = {700, 400, 200}; // �ڲ������ٶȱ�

    if (turn_dir == MY_TURN_LEFT) // ��ת
    {
        // �ڲࣺ���֣�1,3�����٣����֣�2,4��ȫ��
        __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, speed_inner_table[level]); // ��ǰ
        __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_3, speed_inner_table[level]); // ���
        __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_2, speed_outer);              // ��ǰ
        __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_4, speed_outer);              // �Һ�
    }
    else // ��ת
    {
        // �ڲࣺ���֣�2,4�����٣����֣�1,3��ȫ��
        __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, speed_outer);              // ��ǰ
        __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_3, speed_outer);              // ���
        __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_2, speed_inner_table[level]); // ��ǰ
        __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_4, speed_inner_table[level]); // �Һ�
    }
}


/**
 * @brief ���㵥��PWMֵ�����߽籣����
 */
static uint16_t calculate_step_pwm(uint16_t current, uint16_t target, float ratio)
{
    int32_t result = current + (int32_t)(ratio * (target - current));
    // �߽籣��
    if(target > current) {
        return (result > target) ? target : (uint16_t)result;
    } else {
        return (result < target) ? target : (uint16_t)result;
    }
}

/**
 * @brief ͨ����ת��������
 */
static uint8_t channel_to_index(uint32_t channel)
{
    // TIM_CHANNEL_x �� 0-3 ��ӳ��
    return (channel - TIM_CHANNEL_1); // ����ͨ����������ö��ֵ
}

/** 
 * @brief ����������С��ת����ƣ�֧��ǰ������ת�䣩
 * @param turn_dir ת�䷽��0=��ת��1=��ת 
 * @param level ת��ǿ�ȵȼ���0=��΢��1=�еȣ�2=��ת 
 * @param accel_time_ms ����ʱ��(ms)��0=ʹ��Ĭ��ֵ(200ms) 
 */ 
void car_turn_soft(uint8_t turn_dir, uint8_t level, uint16_t accel_time_ms) 
{ 
	//���3�������������ʽ  20250515
	uint8_t front = 0;
	uint8_t inner_rear = 0;
	uint8_t outer_rear = 0;
    // ������� 
    if( level > 3) 
		return; 
    
    // ���ò��� 
    const uint16_t DEFAULT_ACCEL_TIME = 200; 
    const uint16_t STEP_TIME = 10; 
    const uint16_t speed_outer = 1000; // �����ȫ�� 
    const uint16_t speed_inner_table[3] = {700, 400, 200}; 
   
	// ��ȡ��ǰ����
	uint8_t current_car_dir = (HAL_GPIO_ReadPin(MOTOR_CW_3_GPIO_Port, MOTOR_CW_3_Pin) == GPIO_PIN_SET) ? MY_CAR_DIRECTION_BACKWARD : MY_CAR_DIRECTION_FORWARD;
		
    // ȷ�����������ͨ�������ֵ�ǰ�˶�����
    if(turn_dir == MY_TURN_LEFT) // ��ת 
	{ 
		if(current_car_dir == MY_CAR_DIRECTION_FORWARD)
		{
			front = TIM_CHANNEL_3;			//ǰ��
			inner_rear = TIM_CHANNEL_2;		//�ڲ����
			outer_rear = TIM_CHANNEL_1;		//������
		}
		else
		{
			front = TIM_CHANNEL_3;			//ǰ��
			outer_rear = TIM_CHANNEL_1;		//�ڲ����
			inner_rear = TIM_CHANNEL_2;		//������
		}
    } 
	else 						// ��ת 
	{ 
		if(current_car_dir == MY_CAR_DIRECTION_FORWARD)
		{
			front = TIM_CHANNEL_3;			//ǰ��
			outer_rear = TIM_CHANNEL_2;		//�����
			inner_rear = TIM_CHANNEL_1;		//�Һ���
		}
		else
		{
			front = TIM_CHANNEL_3;			//ǰ��
			outer_rear = TIM_CHANNEL_1;		//�����
			inner_rear = TIM_CHANNEL_2;		//�Һ���
		}
    } 
    
    // ��ȡ��ǰPWMֵ
    uint16_t current_pwm[4]; 
    current_pwm[0] = __HAL_TIM_GetCompare(&htim2, TIM_CHANNEL_1); 
    current_pwm[1] = __HAL_TIM_GetCompare(&htim2, TIM_CHANNEL_2); 
    current_pwm[2] = __HAL_TIM_GetCompare(&htim2, TIM_CHANNEL_3); 

    // ����Ŀ��ֵ 
    uint16_t target_inner = speed_inner_table[level-1]; 
    
    // ��ʼ������������ 
    accel_time_ms = (accel_time_ms == 0) ? DEFAULT_ACCEL_TIME : accel_time_ms; 
    uint16_t steps = (accel_time_ms + STEP_TIME/2) / STEP_TIME; // �������� 
    steps = (steps == 0) ? 1 : steps; // ȷ������1�� 

    // ����������
    for(uint16_t step = 0; step <= steps; step++) { 
        float ratio = (float)step / steps; 
        
        // ǰ�ִ���				//����ٶ�
        uint16_t front_pwm = calculate_step_pwm(current_pwm[channel_to_index(front)], speed_outer, ratio); 
		
		
		// ������ִ���			//����ٶ�
        uint16_t outer_rear_pwm  = calculate_step_pwm(current_pwm[channel_to_index(outer_rear)], speed_outer, ratio); 
		// ���ڲ���				//���õ��ٶȼ���
        uint16_t inner_rear_pwm  = calculate_step_pwm(current_pwm[channel_to_index(inner_rear)], target_inner, ratio); 

        // ����PWM
        __HAL_TIM_SetCompare(&htim2, front, front_pwm); 
        __HAL_TIM_SetCompare(&htim2, outer_rear,  outer_rear_pwm); 
        __HAL_TIM_SetCompare(&htim2, inner_rear, inner_rear_pwm); 
        
        HAL_Delay(STEP_TIME); 
    } 

    // ����״̬ǿ��ͬ��
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
			//�����������ٶȿ��� �ļӼ���  ��˲���Ҫ��ֹͣ���
//			my_motor_car_stop();									//����4��������ֹͣ
//			HAL_Delay(1000);										//��ʱ1S
		    // ��ȡ��ǰ����
			current_car_dir = (HAL_GPIO_ReadPin(MOTOR_CW_1_GPIO_Port, MOTOR_CW_1_Pin) == GPIO_PIN_SET) ? MY_CAR_DIRECTION_BACKWARD : MY_CAR_DIRECTION_FORWARD;
			// �������һ�£���ͣ��
			if(current_car_dir != g_MotorDirection) 
			{
				printf("[MOTOR] set motor turn direction!\r\n");
				my_motor_car_stop();									//4����������ֹͣ
				// �ȴ������ȫֹͣ
				HAL_Delay(100);  // 100ms�ȴ�ʱ�䣬�ɸ���ʵ���������
			}
			my_motor_car_run(g_MotorSpeedLevel,g_MotorDirection);	//���е��
			break;
		case CONTROL_MOTOR_TURN:
			printf("[MOTOR] set motor turn!,g_MotorTurnDirection:%d!\r\n",g_MotorTurnDirection);
//			my_motor_car_stop();									//����4��������ֹͣ
//			HAL_Delay(1000);										//��ʱ1S
//			car_turn(g_MotorTurnDirection,g_MotorTurnLevel);		//С��ת��
			car_turn_soft(g_MotorTurnDirection,g_MotorTurnLevel,0);	//����ת����˲���Ҫֹͣ����ʹ����С������ǰ�����ߺ���״̬
			break;
		case CONTROL_MOTOR_STOP:
			printf("[MOTOR] set motor stop!\r\n");
			my_motor_car_stop();									//4��������ֹͣ
			break;
		default:
		break;			
	}
	g_ControlMotorFlag = CONTROL_MOTOR_IDLE;
}	




