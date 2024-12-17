#include "Math.h"
//自定义的幂函数
uint32_t mypow(uint8_t base,uint8_t exp) //自定义的幂函数
{
		uint32_t ans=1;
		for(int i=0;i<exp;i++)
		{
				ans*=base;
		}
		return ans;
}
//频率计算函数
uint32_t freCal(uint8_t *fre,uint8_t dig) //频率计算 传入数组的指针与位数
{
		uint32_t finalfre=0; 
		for(uint8_t i=0;i<=(dig-1);i++)
		{
				finalfre+=(*(fre+i))*mypow(10,i);  //通过指针移位计算
				//finalfre=*fre+(*(fre+1))*10+(*(fre+2))*100+(*(fre+3))*1000+(*(fre+4))*10000+(*(fre+5))*100000+(*(fre+6))*1000000;
		}
		return finalfre;
}
//占空比计算函数
uint8_t duCal(uint8_t *du,uint8_t dig) //占空比计算 同理频率
{
		uint8_t finaldu=0; 
		for(uint8_t i=0;i<=(dig-1);i++)
		{
				finaldu+=(*(du+i))*mypow(10,i);  //通过指针移位计算
		}
		return finaldu;
}