#include "Timer.h"
#include "usart.h"

int timeout=0;

void TIM3_Init() 
{

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 使能TIM3
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    // 初始化TIM3定时器
    TIM_TimeBaseStructure.TIM_Period = 999;
    TIM_TimeBaseStructure.TIM_Prescaler = 71;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    // 清除更新中断标志位
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

    // 使能更新中断
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	
		/****************************************
		此定时器的中断优先级要高于串口以实现准确计时
		****************************************/
		
    // NVIC配置
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


void TIM3_IRQHandler(void)
{

    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
				TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  
				timeout++;
				if(timeout>1000)
				{
						USART1_RX_STA = 0; // 清除接收状态
						timeout=0;
						enterflag=0;
						TIM_Cmd(TIM3, DISABLE); // 停止定时器
				}
		}
}