#include "check.h"

uint8_t my_check_code_calculate(uint8_t * data, uint16_t len)
{
    uint8_t sum = 0;
    for(int i=0; i<len; i++)
        sum = (uint8_t)(sum + data[i]);
    sum = 0xff - sum;
    return sum;
}

uint8_t my_check_code_analysis(uint8_t * data, uint16_t len)
{
    uint8_t sum = 0;
    for(int i=0; i<len; i++)
        sum = (uint8_t)(sum + data[i]);
    if(sum == 0xff)
    {
        return 0;
    }
    else
    {
        return 2;
    }
}

