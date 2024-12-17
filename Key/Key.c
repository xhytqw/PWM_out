#include "KEY.h"

int key_status_1()
{
    return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15);// 按键按下时，GPIO_ReadInputDataBit 返回0
}

int key_status_2()
{
    return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_14); // 按键按下时，GPIO_ReadInputDataBit 返回0
}
int key_status_3()
{
	  return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_13); // 按键按下时，GPIO_ReadInputDataBit 返回0
}
int key_status_4()
{
		return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12); // 按键按下时，GPIO_ReadInputDataBit 返回0
}