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

// 初始化TIM2的PWM功能
void TIM2_PWM_CH1_Init(u16 arr, u16 psc) 
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    // 使能TIM2和GPIOA时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // 配置GPIOA的0引脚为复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 初始化TIM2定时器
    TIM_TimeBaseStructure.TIM_Period = arr;
    TIM_TimeBaseStructure.TIM_Prescaler = psc;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    // 配置TIM2的输出比较模式为PWM模式1
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    // 初始化TIM2的通道1
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
/*
    // 初始化TIM2的通道2
    TIM_OC2Init(TIM2, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);

    // 初始化TIM2的通道3
    TIM_OC3Init(TIM2, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);

    // 初始化TIM2的通道4
    TIM_OC4Init(TIM2, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);
*/
    // 使能TIM2的自动重装寄存器
    TIM_ARRPreloadConfig(TIM2, ENABLE);
		TIM2->EGR = TIM_PSCReloadMode_Immediate;
    TIM_GenerateEvent(TIM2, TIM_EventSource_Update);
		TIM_Cmd(TIM2, ENABLE);

}
/*****************************************
动态调整arr以实现极宽的频率调整范围
频率理论最高值36M
******************************************/
void PWM2_CH1_set(int32_t fre, int duty)  //fre为32位int
{
    int arr, ccr;
    int psc=0;       // 初始值为1
		
		//static int prev_arr=-1, prev_psc=-1;
	
    if(fre<=0||duty< 0||duty>100) 
		{
				return;      // 避免非法参数
		}
		
		arr = (int)((72000000.0/((psc+1) * fre))+0.5)-1;
		
		if (arr<1)
		{
    // 避免 ARR = 0 的情况
			arr = 1;
		}
		while(arr>65535&&psc< 65536)   //arr的动态参数调整
		{
				psc++;
				arr = 72000000/(psc*fre)-1;
		}
		
		arr = (int)((72000000.0/((psc+1) * fre))+0.5)-1;		//float强转int提高精度 四舍五入
		
		if (psc>=65536||arr<=0)
		{
				return; // 超出范围
		}
    // 计算占空比的比较值 CCR
    ccr = ((arr + 1) * duty) / 100; // 占空比计算公式
		
		/*duty = 100时，公式可能计算出 ccr 超出 arr + 1 的范围*/
		
		if(ccr>arr)   //加限制
		{
				ccr=arr;
		}
		/*
    if (arr!=prev_arr||psc!=prev_psc) 
		{
        
        prev_arr=arr;
        prev_psc=psc;
    }
		*/
		TIM2_PWM_CH1_Init(arr, psc);
    PWM_SetDutyCycle(TIM2, 1, ccr);
}
