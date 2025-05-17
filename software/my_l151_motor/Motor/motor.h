#ifndef MOTOR_H
#define MOTOR_H

#include "main.h"

enum{
	CONTROL_MOTOR_IDLE,
	CONTROL_MOTOR_DIRECTION,			//����
	CONTROL_MOTOR_TURN,					//ת��
	CONTROL_MOTOR_STOP					//ֹͣ
};

enum{
	MY_MOTOR_IDLE,
	MY_MOTOR_1,							//��ǰ��
	MY_MOTOR_2,							//��ǰ��
	MY_MOTOR_3,							//�����
	MY_MOTOR_4							//�Һ���
};

enum{
	MY_TURN_IDLE,
	MY_TURN_LEFT,						//��ת
	MY_TURN_RIGHT						//��ת
};
enum{
	MY_CAR_DIRECTION_IDLE,
	MY_CAR_DIRECTION_FORWARD,			//ǰ��
	MY_CAR_DIRECTION_BACKWARD			//����
};

enum{
	MY_CAR_SPEED_IDLE,					//ֹ̬ͣ
	MY_CAR_SPEED_LEVEL1,				//�ٶȵȼ�1 	100%��ȫ�٣�
	MY_CAR_SPEED_LEVEL2,				//�ٶȵȼ�2  	75%
	MY_CAR_SPEED_LEVEL3,				//�ٶȵȼ�3		50%
	MY_CAR_SPEED_LEVEL4,				//�ٶȵȼ�4		30%
	MY_CAR_SPEED_LEVEL5					//�ٶȵȼ�5		10%������٣�
};

enum{
	MY_TURN_LEVEL_IDLE,					//ֹ̬ͣ
	MY_TURN_LEVEL1,						//ת��ȼ���΢
	MY_TURN_LEVEL2,						//ת��ȼ��е�
	MY_TURN_LEVEL3						//ת��ȼ���ת
};

extern uint8_t g_MotorDirection;		//������� 0��ֹͣ 1��ǰ����2������
extern uint8_t g_MotorSpeedLevel;		//����ٶȵȼ� 0��ֹ̬ͣ  1 = 100%��ȫ�٣�2 = 75%3 = 50%4 = 30% 5 = 10%������٣�
extern uint8_t g_MotorTurnDirection;	//���ת�䷽�� 0����ת��  1����ת�䣬2����ת��
extern uint8_t g_MotorTurnLevel;		//���ת��ȼ� 0��ֹ̬ͣ  ת��ǿ�ȵȼ���1 = ��΢��2 = �еȣ�3 = ��ת

extern void motor_set_speed(uint8_t motor_channel, uint8_t speed_level);
extern void motor_stop(uint8_t motor_channel);
extern void my_motor_control_task(void);
#endif


