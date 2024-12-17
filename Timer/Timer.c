#include "Timer.h"
#include "usart.h"

int timeout=0;

void TIM3_Init() 
{

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // ʹ��TIM3
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    // ��ʼ��TIM3��ʱ��
    TIM_TimeBaseStructure.TIM_Period = 999;
    TIM_TimeBaseStructure.TIM_Prescaler = 71;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    // ��������жϱ�־λ
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

    // ʹ�ܸ����ж�
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	
		/****************************************
		�˶�ʱ�����ж����ȼ�Ҫ���ڴ�����ʵ��׼ȷ��ʱ
		****************************************/
		
    // NVIC����
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
						USART1_RX_STA = 0; // �������״̬
						timeout=0;
						enterflag=0;
						TIM_Cmd(TIM3, DISABLE); // ֹͣ��ʱ��
				}
		}
}