#ifndef HC_SR04_H
#define HC_SR04_H

#include "main.h"


// ��������Ŷ���
#define SENSOR_FRONT 0
#define SENSOR_BACK  1

// ��ȫ���붨��
#define SAFE_DISTANCE_FRONT  20.0f   // ǰ����ȫ���룬��λcm
#define SAFE_DISTANCE_BACK   20.0f   // �󷽰�ȫ���룬��λcm

// �洢�����������Ĳ�������
typedef struct {
    volatile uint32_t echo_start;    		// �ز��źſ�ʼʱ��(������)��ʹ��volatile��ֹ�������Ż�
    volatile uint32_t echo_end;     	 	// �ز��źŽ���ʱ��(�½���)��ʹ��volatile��ֹ�������Ż�
    volatile uint8_t measure_complete;		// ������ɱ�־��0=δ��ɣ�1=��ɣ�volatileȷ���ж���������ͬ��
} HC_SR04_DATA_tydef;                		// ���������������ݽṹ�����Ͷ���

extern HC_SR04_DATA_tydef HC_SR04_DATA[2];

// ��ӵ��Ա���
extern uint32_t debug_rising_count;
extern uint32_t debug_falling_count;


static float HC_SR04_GetDistance(uint8_t sensor);
extern float HC_SR04_GetBackDistance(void);
extern float HC_SR04_GetFrontDistance(void);
extern void HC_SR04_StartMeasure(uint8_t sensor);
extern void Ultrasonic_Task_Handler(void);
#endif


