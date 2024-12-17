#ifndef __PWM_H
#define __PWM_H
#include "stm32f10x.h"

// ����PWM��ռ�ձ�
// timer: ��ʱ�� (TIM2, TIM3��TIM4)
// channel: PWMͨ�� (1-4)
// dutyCycle: ռ�ձ� (0-999)
void PWM_SetDutyCycle(TIM_TypeDef* TIMx, uint8_t channel, uint16_t dutyCycle);
void TIM2_PWM_CH1_Init(u16 arr, u16 psc);
void PWM2_CH1_set(int fre, int duty);
#endif
