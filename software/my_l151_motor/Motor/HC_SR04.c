#include "HC_SR04.h"
#include "tim.h"
#include "motor.h"


// ��ӵ��Ա���
uint32_t debug_rising_count = 0;
uint32_t debug_falling_count = 0;

HC_SR04_DATA_tydef HC_SR04_DATA[2] = {0};

// �洢�������ݵĽṹ��
typedef struct {
    float front_distance;    // ǰ�����룬��λcm
    float back_distance;     // �󷽾��룬��λcm
    uint8_t front_valid;     // ǰ��������Ч��־
    uint8_t back_valid;      // ��������Ч��־
} Ultrasonic_Data_t;

static Ultrasonic_Data_t ultrasonic_data = {0};

/**
 * @brief ΢�뼶��ʱ����
 * @param us: Ҫ��ʱ��΢����
 * @note  �����ʱʱ��Լ65ms
 */
void Delay_us(uint32_t us)
{
    // ���������
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    
    // �ȴ��������ﵽָ��ֵ
    while(__HAL_TIM_GET_COUNTER(&htim3) < us);
}


/**
 * @brief ��ʼһ�β���
 * @param sensor ��������ţ�SENSOR_FRONT��SENSOR_BACK
 */
void HC_SR04_StartMeasure(uint8_t sensor)
{
    GPIO_TypeDef* trig_port;
    uint16_t trig_pin;
    
    // ѡ���Ӧ��TRIG����
    if(sensor == SENSOR_FRONT) {
        trig_port = Front_Trig_GPIO_Port;
        trig_pin = Front_Trig_Pin;
    } else {
        trig_port = Rear_Trig_GPIO_Port;
        trig_pin = Rear_Trig_Pin;
    }
    
    // �����ɱ�־
    HC_SR04_DATA[sensor].measure_complete = 0;
    
    // ����10us�Ĵ�������
    HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_SET);
    Delay_us(10);  // ΢����ʱ
    HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_RESET);
}


/**
 * @brief ��ȡǰ������ֵ
 * @return ���ز������룬��λ���ף�-1��ʾ����ʧ��
 */
float HC_SR04_GetFrontDistance(void)
{
    return HC_SR04_GetDistance(SENSOR_FRONT);
}

/**
 * @brief ��ȡ�󷽾���ֵ
 * @return ���ز������룬��λ���ף�-1��ʾ����ʧ��
 */
float HC_SR04_GetBackDistance(void)
{
    return HC_SR04_GetDistance(SENSOR_BACK);
}


/**
 * @brief ��ȡָ���������ľ���ֵ
 * @param sensor ���������
 * @return ���ز������룬��λ���ף�-1��ʾ����ʧ��
 */
static float HC_SR04_GetDistance(uint8_t sensor)
{
    float distance = 0.0f;
    uint32_t timeout = 0;
    
    // �ȴ�������ɻ�ʱ
    while(!HC_SR04_DATA[sensor].measure_complete)
    {
        timeout++;
        if(timeout > 1000)
            return -1;
        HAL_Delay(1);
    }
    
	// ������루��λ�����ף�
	// ���㹫ʽ������ = (��������ʱ�� * ����) / 2
	// ���У�
	// - ���� = 340 m/s = 34000 cm/s = 34 cm/ms
	// - echo_end - echo_start �õ����Ǻ���(ms)��λ��ʱ��
	//��Ϊʹ�õ���us ��˻���Ҫ����1000
	// - ����2����Ϊ����������·�̣�������Ҫ���̾���
	distance = (float)(HC_SR04_DATA[sensor].echo_end - 
					  HC_SR04_DATA[sensor].echo_start) * 34.0f / 2.0f / 1000;
    
    return distance;
}

/**
 * @brief ���ǰ���Ƿ�ȫ
 * @return 1=��ȫ��0=����ȫ
 */
uint8_t Is_Front_Safe(void)
{
    if(!ultrasonic_data.front_valid) return 1;  // ������ЧʱĬ�ϰ�ȫ
    return (ultrasonic_data.front_distance > SAFE_DISTANCE_FRONT);
}

/**
 * @brief �����Ƿ�ȫ
 * @return 1=��ȫ��0=����ȫ
 */
uint8_t Is_Back_Safe(void)
{
    if(!ultrasonic_data.back_valid) return 1;  // ������ЧʱĬ�ϰ�ȫ
    return (ultrasonic_data.back_distance > SAFE_DISTANCE_BACK);
}


/**
 * @brief �����������������ڶ�ʱ���ж��е���
 */
void Ultrasonic_Task_Handler(void)
{
//	uint32_t start_time, end_time;       // ����ִ��ʱ���¼
//	// ��¼������ʼִ��ʱ��
//    start_time = HAL_GetTick();
	// ����ǰ������
	HC_SR04_StartMeasure(SENSOR_FRONT);
	ultrasonic_data.front_distance = HC_SR04_GetFrontDistance();
	ultrasonic_data.front_valid = (ultrasonic_data.front_distance > 0);
	// ��ӡ�������
	if(ultrasonic_data.front_valid) 
	{
		print_log("ǰ������: %.2f cm\r\n", ultrasonic_data.front_distance);
	}
	// ���ǰ����ȫ����
	if(!Is_Front_Safe() && g_MotorDirection == MY_CAR_DIRECTION_FORWARD)
	{
		print_log("ǰ���ϰ������ͣ����\r\n");
	}
	HAL_Delay(100);
	
//	// �����󷽾���
//	HC_SR04_StartMeasure(SENSOR_BACK);
//	ultrasonic_data.back_distance = HC_SR04_GetBackDistance();
//	ultrasonic_data.back_valid = (ultrasonic_data.back_distance > 0);
//	if(ultrasonic_data.back_valid) 
//	{
//		print_log("�󷽾���: %.2f cm\r\n", ultrasonic_data.back_distance);
//	}
//	// ���󷽰�ȫ����
//	if(!Is_Back_Safe() && g_MotorDirection == MY_CAR_DIRECTION_BACKWARD)
//	{
//		print_log("���ϰ������ͣ����\r\n");
//	}
//	HAL_Delay(100);
	
	
//	// ��¼��������ʱ�䲢����ִ��ʱ��
//    end_time = HAL_GetTick();
}


