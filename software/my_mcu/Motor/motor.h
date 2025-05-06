#ifndef MOTOR_H
#define MOTOR_H

#include "main.h"

enum{
	CONTROL_MOTOR_IDLE,
	CONTROL_MOTOR_DIRECTION,			//方向
	CONTROL_MOTOR_TURN,					//转向
	CONTROL_MOTOR_STOP					//停止
};

enum{
	MY_MOTOR_IDLE,
	MY_MOTOR_1,							//左前轮
	MY_MOTOR_2,							//右前轮
	MY_MOTOR_3,							//左后轮
	MY_MOTOR_4							//右后轮
};

enum{
	MY_TURN_IDLE,
	MY_TURN_LEFT,						//左转
	MY_TURN_RIGHT						//右转
};
enum{
	MY_CAR_DIRECTION_IDLE,
	MY_CAR_DIRECTION_FORWARD,			//前进
	MY_CAR_DIRECTION_BACKWARD			//后退
};

enum{
	MY_CAR_SPEED_IDLE,					//停止态
	MY_CAR_SPEED_LEVEL1,				//速度等级1 	100%（全速）
	MY_CAR_SPEED_LEVEL2,				//速度等级2  	75%
	MY_CAR_SPEED_LEVEL3,				//速度等级3		50%
	MY_CAR_SPEED_LEVEL4,				//速度等级4		30%
	MY_CAR_SPEED_LEVEL5					//速度等级5		10%（最低速）
};

enum{
	MY_TURN_LEVEL_IDLE,					//停止态
	MY_TURN_LEVEL1,						//转弯等级轻微
	MY_TURN_LEVEL2,						//转弯等级中等
	MY_TURN_LEVEL3						//转弯等级急转
};

extern uint8_t g_MotorDirection;		//电机方向 0：停止 1：前进，2：后退
extern uint8_t g_MotorSpeedLevel;		//电机速度等级 0：停止态  1 = 100%（全速）2 = 75%3 = 50%4 = 30% 5 = 10%（最低速）
extern uint8_t g_MotorTurnDirection;	//电机转弯方向 0：无转弯  1：左转弯，2：右转弯
extern uint8_t g_MotorTurnLevel;		//电机转弯等级 0：停止态  转弯强度等级：1 = 轻微，2 = 中等，3 = 急转

extern void motor_set_speed(uint8_t motor_channel, uint8_t speed_level);
extern void motor_stop(uint8_t motor_channel);
extern void my_motor_control_task(void);
#endif


