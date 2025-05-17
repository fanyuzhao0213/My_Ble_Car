#ifndef HC_SR04_H
#define HC_SR04_H

#include "main.h"


// 传感器编号定义
#define SENSOR_FRONT 0
#define SENSOR_BACK  1

// 安全距离定义
#define SAFE_DISTANCE_FRONT  20.0f   // 前方安全距离，单位cm
#define SAFE_DISTANCE_BACK   20.0f   // 后方安全距离，单位cm

// 存储两个传感器的测量数据
typedef struct {
    volatile uint32_t echo_start;    		// 回波信号开始时间(上升沿)，使用volatile防止编译器优化
    volatile uint32_t echo_end;     	 	// 回波信号结束时间(下降沿)，使用volatile防止编译器优化
    volatile uint8_t measure_complete;		// 测量完成标志，0=未完成，1=完成，volatile确保中断与主程序同步
} HC_SR04_DATA_tydef;                		// 超声波传感器数据结构体类型定义

extern HC_SR04_DATA_tydef HC_SR04_DATA[2];

// 添加调试变量
extern uint32_t debug_rising_count;
extern uint32_t debug_falling_count;


static float HC_SR04_GetDistance(uint8_t sensor);
extern float HC_SR04_GetBackDistance(void);
extern float HC_SR04_GetFrontDistance(void);
extern void HC_SR04_StartMeasure(uint8_t sensor);
extern void Ultrasonic_Task_Handler(void);
#endif


