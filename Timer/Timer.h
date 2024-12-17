#ifndef __TIMER_H
#define __TIMER_H

#include "stm32f10x.h"
#include "stm32f10x_tim.h"

extern uint8_t enterflag;

void TIM3_Init();
void TIM3_IRQHandler(void);

#endif
