#include "KEY.h"

int key_status_1()
{
    return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15);// ��������ʱ��GPIO_ReadInputDataBit ����0
}

int key_status_2()
{
    return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_14); // ��������ʱ��GPIO_ReadInputDataBit ����0
}
int key_status_3()
{
	  return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_13); // ��������ʱ��GPIO_ReadInputDataBit ����0
}
int key_status_4()
{
		return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12); // ��������ʱ��GPIO_ReadInputDataBit ����0
}