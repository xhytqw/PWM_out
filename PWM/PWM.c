#include "pwm.h"

void PWM_SetDutyCycle(TIM_TypeDef* TIMx, uint8_t channel, uint16_t dutyCycle)
{
    switch (channel) 
		{
        case 1:
            TIM_SetCompare1(TIMx, dutyCycle);
            break;
        case 2:
            TIM_SetCompare2(TIMx, dutyCycle);
            break;
        case 3:
            TIM_SetCompare3(TIMx, dutyCycle);
            break;
        case 4:
            TIM_SetCompare4(TIMx, dutyCycle);
            break;
        default:
            break;
    }
}

// ��ʼ��TIM2��PWM����
void TIM2_PWM_CH1_Init(u16 arr, u16 psc) 
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    // ʹ��TIM2��GPIOAʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // ����GPIOA��0����Ϊ�����������
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // ��ʼ��TIM2��ʱ��
    TIM_TimeBaseStructure.TIM_Period = arr;
    TIM_TimeBaseStructure.TIM_Prescaler = psc;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    // ����TIM2������Ƚ�ģʽΪPWMģʽ1
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    // ��ʼ��TIM2��ͨ��1
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
/*
    // ��ʼ��TIM2��ͨ��2
    TIM_OC2Init(TIM2, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);

    // ��ʼ��TIM2��ͨ��3
    TIM_OC3Init(TIM2, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);

    // ��ʼ��TIM2��ͨ��4
    TIM_OC4Init(TIM2, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);
*/
    // ʹ��TIM2���Զ���װ�Ĵ���
    TIM_ARRPreloadConfig(TIM2, ENABLE);
		TIM_Cmd(TIM2, ENABLE);

}
/*****************************************
��̬����arr��ʵ�ּ����Ƶ�ʵ�����Χ
Ƶ���������ֵ36M
******************************************/
void PWM2_CH1_set(int32_t fre, int duty)  //freΪ32λint
{
    int arr, ccr;
    int psc=1; // ��ʼֵΪ1
		
		static int prev_arr=-1, prev_psc=-1;
	
    if (fre <= 0 || duty < 0 || duty > 100) 
		return; // ����Ƿ�����

		arr = (int)((72000000.0/(psc * fre))+0.5)-1;
	
		while (arr > 65535 && psc < 65536)   //arr�Ķ�̬��������
		{
				psc++;
				arr = 72000000 / (psc * fre) - 1;
		}
		arr = (int)((72000000.0/(psc * fre))+0.5)-1;		//floatǿתint��߾��� ��������
		
		if (psc >= 65536 || arr <= 0) return; // ������Χ

    // ����ռ�ձȵıȽ�ֵ CCR
    ccr = ((arr + 1) * duty) / 100; // ռ�ձȼ��㹫ʽ
		
		/*duty = 100ʱ����ʽ���ܼ���� ccr ���� arr + 1 �ķ�Χ*/
		if(ccr>arr)   //������
			ccr=arr;

    if (arr != prev_arr || psc != prev_psc) 
		{
        TIM2_PWM_CH1_Init(arr, psc);
        prev_arr = arr;
        prev_psc = psc;
    }

    PWM_SetDutyCycle(TIM2, 1, ccr);
}
/*
void PWM2_CH2_set(int fre, int duty)
{
    int arr, ccr;
    int psc = 71;
    // ���Ƶ�ʺ�ռ�ձȲ����Ƿ�Ϸ�
    if (fre <= 0 || duty < 0 || duty > 100) return; // ����Ƿ�����

    // Ԥ��Ƶ������Ϊ�̶�ֵ
    

    // �����Զ���װֵ ARR
    arr = (72000000 / psc / fre) - 1;

    // ��� ARR �Ƿ񳬳���Χ
    if (arr < 0 || arr > 65535) return; // ������ʱ����Χ

    // ����ռ�ձȵıȽ�ֵ CCR
    ccr = (arr + 1) * duty / 100; // ռ�ձȼ��㹫ʽ

    // ��ʼ����ʱ�� PWM
    TIM2_PWM_CH2_Init(arr, psc);

    // ���� PWM ռ�ձ�
    PWM_SetDutyCycle(TIM2, 1, ccr);
}
void PWM2_CH3_set(int fre, int duty)
{
    int arr, ccr;
    int psc = 71;
    // ���Ƶ�ʺ�ռ�ձȲ����Ƿ�Ϸ�
    if (fre <= 0 || duty < 0 || duty > 100) return; // ����Ƿ�����

    // Ԥ��Ƶ������Ϊ�̶�ֵ
    

    // �����Զ���װֵ ARR
    arr = (72000000 / psc / fre) - 1;

    // ��� ARR �Ƿ񳬳���Χ
    if (arr < 0 || arr > 65535) return; // ������ʱ����Χ

    // ����ռ�ձȵıȽ�ֵ CCR
    ccr = (arr + 1) * duty / 100; // ռ�ձȼ��㹫ʽ

    // ��ʼ����ʱ�� PWM
    TIM2_PWM_CH3_Init(arr, psc);

    // ���� PWM ռ�ձ�
    PWM_SetDutyCycle(TIM2, 1, ccr);
}
void PWM2_CH4_set(int fre, int duty)
{
    int arr, ccr;
    int psc = 71;
    // ���Ƶ�ʺ�ռ�ձȲ����Ƿ�Ϸ�
    if (fre <= 0 || duty < 0 || duty > 100) return; // ����Ƿ�����

    // Ԥ��Ƶ������Ϊ�̶�ֵ
    

    // �����Զ���װֵ ARR
    arr = (72000000 / psc / fre) - 1;

    // ��� ARR �Ƿ񳬳���Χ
    if (arr < 0 || arr > 65535) return; // ������ʱ����Χ

    // ����ռ�ձȵıȽ�ֵ CCR
    ccr = (arr + 1) * duty / 100; // ռ�ձȼ��㹫ʽ

    // ��ʼ����ʱ�� PWM
    TIM2_PWM_CH4_Init(arr, psc);

    // ���� PWM ռ�ձ�
    PWM_SetDutyCycle(TIM2, 1, ccr);
}
*/
