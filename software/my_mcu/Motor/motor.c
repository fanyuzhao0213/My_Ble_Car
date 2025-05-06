#include "motor.h"
#include "tim.h"

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
 * @brief     ����ĳһ·������ٶȺͷ���
 * @param     motor_channel ���ͨ����ţ���ΧΪ 1~4���ֱ��Ӧ TIM2 �� CH1~CH4
 * @param     speed_level   ����ٶȵȼ���
 *                           1 = 100%��ȫ�٣�
 *                           2 = 75%
 *                           3 = 50%
 *                           4 = 30%
 *                           5 = 10%������٣�
 * @param     direction      �������
 *                           0 = ǰ������ת��
 *                           1 = ���ˣ���ת��
 * @note      ��Ҫ�� motor_gpio_set_direction() �ж���ÿ��ͨ�����Ʒ���� GPIO��
 */
void motor_set_speed_dir(uint8_t motor_channel, uint8_t speed_level, uint8_t direction)
{
    const uint16_t duty_table[] = {1000, 750, 500, 300, 100};
    TIM_HandleTypeDef *htim = &htim2;

    if (motor_channel < 1 || motor_channel > 4 ||  speed_level > 5 || speed_level < 1)
        return;

    // ӳ�� PWM ͨ��
    uint32_t tim_channel;
    switch (motor_channel)
    {
        case 1: tim_channel = TIM_CHANNEL_1; break;
        case 2: tim_channel = TIM_CHANNEL_2; break;
        case 3: tim_channel = TIM_CHANNEL_3; break;
        case 4: tim_channel = TIM_CHANNEL_4; break;
        default: return;
    }

    // ���� PWM ռ�ձ�
    __HAL_TIM_SetCompare(htim, tim_channel, duty_table[speed_level-1]);

    // ���÷���
    motor_gpio_set_direction(motor_channel, direction);
}

//����С���������ض����ٶȵȼ��ͷ���
void my_motor_car_run(uint8_t speed_level, uint8_t direction)
{
	for(uint8_t i =0; i<=3; i++)
	{
		motor_set_speed_dir(i+1,speed_level,direction);
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
	motor_stop(1);
	motor_stop(2);
	motor_stop(3);
	motor_stop(4);
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



void my_motor_control_task(void)
{
	if(g_ControlMotorFlag == CONTROL_MOTOR_IDLE)
		return;
	switch(g_ControlMotorFlag)
	{
		case CONTROL_MOTOR_DIRECTION:
			printf("[MAIN] set motor run and speedlevel!\r\n");
			my_motor_car_stop();									//����4��������ֹͣ
			HAL_Delay(1000);										//��ʱ1S
			my_motor_car_run(g_MotorSpeedLevel,g_MotorDirection);	//���е��
			break;
		case CONTROL_MOTOR_TURN:
			printf("[MAIN] set motor turn!\r\n");
			my_motor_car_stop();									//����4��������ֹͣ
			HAL_Delay(1000);										//��ʱ1S
			car_turn(g_MotorTurnDirection,g_MotorTurnLevel);		//С��ת��
			break;
		case CONTROL_MOTOR_STOP:
			printf("[MAIN] set motor stop!\r\n");
			my_motor_car_stop();									//4��������ֹͣ
			break;
		default:
		break;			
	}
	g_ControlMotorFlag = CONTROL_MOTOR_IDLE;
}	




