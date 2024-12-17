#include "gpio.h"

// ���� GPIO ʱ��
static void GPIO_EnableClock(GPIO_TypeDef* GPIOx)
{
    if (GPIOx == GPIOA)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    }
    else if (GPIOx == GPIOB)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    }
    else if (GPIOx == GPIOC)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    }
    else if (GPIOx == GPIOD)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    }
    else if (GPIOx == GPIOE)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
    }
}

// ��ʼ�� GPIO ����
void GPIO_MyInit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIOMode_TypeDef Mode)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // ���� GPIO ʱ��
    GPIO_EnableClock(GPIOx);

    // ���� GPIO ����
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin;
    GPIO_InitStructure.GPIO_Mode = Mode;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;


    // ��ʼ�� GPIO
    GPIO_Init(GPIOx, &GPIO_InitStructure);
}
