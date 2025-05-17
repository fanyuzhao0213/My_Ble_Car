#include "HC_SR04.h"
#include "tim.h"
#include "motor.h"


// 添加调试变量
uint32_t debug_rising_count = 0;
uint32_t debug_falling_count = 0;

HC_SR04_DATA_tydef HC_SR04_DATA[2] = {0};

// 存储距离数据的结构体
typedef struct {
    float front_distance;    // 前方距离，单位cm
    float back_distance;     // 后方距离，单位cm
    uint8_t front_valid;     // 前方数据有效标志
    uint8_t back_valid;      // 后方数据有效标志
} Ultrasonic_Data_t;

static Ultrasonic_Data_t ultrasonic_data = {0};

/**
 * @brief 微秒级延时函数
 * @param us: 要延时的微秒数
 * @note  最大延时时间约65ms
 */
void Delay_us(uint32_t us)
{
    // 清零计数器
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    
    // 等待计数器达到指定值
    while(__HAL_TIM_GET_COUNTER(&htim3) < us);
}


/**
 * @brief 开始一次测量
 * @param sensor 传感器编号：SENSOR_FRONT或SENSOR_BACK
 */
void HC_SR04_StartMeasure(uint8_t sensor)
{
    GPIO_TypeDef* trig_port;
    uint16_t trig_pin;
    
    // 选择对应的TRIG引脚
    if(sensor == SENSOR_FRONT) {
        trig_port = Front_Trig_GPIO_Port;
        trig_pin = Front_Trig_Pin;
    } else {
        trig_port = Rear_Trig_GPIO_Port;
        trig_pin = Rear_Trig_Pin;
    }
    
    // 清除完成标志
    HC_SR04_DATA[sensor].measure_complete = 0;
    
    // 发送10us的触发脉冲
    HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_SET);
    Delay_us(10);  // 微秒延时
    HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_RESET);
}


/**
 * @brief 获取前方距离值
 * @return 返回测量距离，单位厘米，-1表示测量失败
 */
float HC_SR04_GetFrontDistance(void)
{
    return HC_SR04_GetDistance(SENSOR_FRONT);
}

/**
 * @brief 获取后方距离值
 * @return 返回测量距离，单位厘米，-1表示测量失败
 */
float HC_SR04_GetBackDistance(void)
{
    return HC_SR04_GetDistance(SENSOR_BACK);
}


/**
 * @brief 获取指定传感器的距离值
 * @param sensor 传感器编号
 * @return 返回测量距离，单位厘米，-1表示测量失败
 */
static float HC_SR04_GetDistance(uint8_t sensor)
{
    float distance = 0.0f;
    uint32_t timeout = 0;
    
    // 等待测量完成或超时
    while(!HC_SR04_DATA[sensor].measure_complete)
    {
        timeout++;
        if(timeout > 1000)
            return -1;
        HAL_Delay(1);
    }
    
	// 计算距离（单位：厘米）
	// 计算公式：距离 = (声波往返时间 * 声速) / 2
	// 其中：
	// - 声速 = 340 m/s = 34000 cm/s = 34 cm/ms
	// - echo_end - echo_start 得到的是毫秒(ms)单位的时间
	//因为使用的是us 因此还需要除以1000
	// - 除以2是因为声波是往返路程，我们需要单程距离
	distance = (float)(HC_SR04_DATA[sensor].echo_end - 
					  HC_SR04_DATA[sensor].echo_start) * 34.0f / 2.0f / 1000;
    
    return distance;
}

/**
 * @brief 检查前方是否安全
 * @return 1=安全，0=不安全
 */
uint8_t Is_Front_Safe(void)
{
    if(!ultrasonic_data.front_valid) return 1;  // 数据无效时默认安全
    return (ultrasonic_data.front_distance > SAFE_DISTANCE_FRONT);
}

/**
 * @brief 检查后方是否安全
 * @return 1=安全，0=不安全
 */
uint8_t Is_Back_Safe(void)
{
    if(!ultrasonic_data.back_valid) return 1;  // 数据无效时默认安全
    return (ultrasonic_data.back_distance > SAFE_DISTANCE_BACK);
}


/**
 * @brief 超声波任务处理函数，在定时器中断中调用
 */
void Ultrasonic_Task_Handler(void)
{
//	uint32_t start_time, end_time;       // 函数执行时间记录
//	// 记录函数开始执行时间
//    start_time = HAL_GetTick();
	// 测量前方距离
	HC_SR04_StartMeasure(SENSOR_FRONT);
	ultrasonic_data.front_distance = HC_SR04_GetFrontDistance();
	ultrasonic_data.front_valid = (ultrasonic_data.front_distance > 0);
	// 打印测量结果
	if(ultrasonic_data.front_valid) 
	{
		print_log("前方距离: %.2f cm\r\n", ultrasonic_data.front_distance);
	}
	// 检查前方安全距离
	if(!Is_Front_Safe() && g_MotorDirection == MY_CAR_DIRECTION_FORWARD)
	{
		print_log("前方障碍物！紧急停车！\r\n");
	}
	HAL_Delay(100);
	
//	// 测量后方距离
//	HC_SR04_StartMeasure(SENSOR_BACK);
//	ultrasonic_data.back_distance = HC_SR04_GetBackDistance();
//	ultrasonic_data.back_valid = (ultrasonic_data.back_distance > 0);
//	if(ultrasonic_data.back_valid) 
//	{
//		print_log("后方距离: %.2f cm\r\n", ultrasonic_data.back_distance);
//	}
//	// 检查后方安全距离
//	if(!Is_Back_Safe() && g_MotorDirection == MY_CAR_DIRECTION_BACKWARD)
//	{
//		print_log("后方障碍物！紧急停车！\r\n");
//	}
//	HAL_Delay(100);
	
	
//	// 记录函数结束时间并计算执行时间
//    end_time = HAL_GetTick();
}


