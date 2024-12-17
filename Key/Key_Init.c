#include "KEY_Init.h"

void key_init()
{
		GPIO_MyInit(GPIOA, GPIO_Pin_15, GPIO_Mode_IPU);
		GPIO_MyInit(GPIOA, GPIO_Pin_14, GPIO_Mode_IPU);
	  GPIO_MyInit(GPIOA, GPIO_Pin_13, GPIO_Mode_IPU);
	  GPIO_MyInit(GPIOA, GPIO_Pin_12, GPIO_Mode_IPU);
}
