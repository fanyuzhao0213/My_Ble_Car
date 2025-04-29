#ifndef CHECK_H  // 如果没有定义 MY_HEADER_H
#define CHECK_H  // 定义它，避免重复包含

#include "main.h"


uint8_t my_check_code_calculate(uint8_t * data, uint16_t len);
uint8_t my_check_code_analysis(uint8_t * data, uint16_t len);
#endif

