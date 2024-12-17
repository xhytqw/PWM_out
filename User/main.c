/*******************************************
此项目遵循GNU General Public License v3.0
            COPYLEFT 2024

开源地址：https://github.com/xhytqw/PWM_out

署名与系统版本部分：

作者：Qiwei Tian(Qiuye)

功能：基于STM32的带GUI的宽频PWM输出器

v1.0（12.8）：
实现了基本功能
问题：按键频率设置时未约束SET的次数导致bug；
串口存在bug，错误输入时无提示，
主要由于atoi函数导致返回值一直为0


v1.1（12.10）
修复了按键与串口的PWM SET多次的bug
问题：串口有错误无提示但可用，需增强鲁棒性


v1.2（12.12）:
串口bug全部修复完毕，无论何种串口错误输入均有提示
引入Timer6定时器中断处理串口超时，增强鲁棒性					
问题：du的字符空格处理不清,while循环只能处理前空格


v1.3（12.13）
解决了串口duty的问题 代码现可删除串口中空格且不出问题
加入串口数据流以及系统操作的说明菜单
问题：屏幕多次无用刷新耗费系统资源


v1.4 (12.15)
解决屏幕刷新问题，按键事件驱动
只在需要时刷新一次，即OLED_Refresh函数的重构
(这是一个非常好的改动直接把系统流畅度提高了好几倍)


v1.4.1(12.17)
显示fre du的数据结构改为数组，没什么用但更符合规范


v1.4.2(12.17)
重构封装了一些函数，便于维护

***新的更改及更改作者请务必在此处说明，以遵循GPL协议***

您可以自由使用、修改和分发本软件，但必须保留本声明和协议文本。
*********************************************************/

#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "usart.h"
#include "oled.h"
#include "pwm.h"
#include "KEY_Init.h"
#include "Key.h"
#include "stdint.h"
#include "usart.h"
#include "Timer.h"
#include "Math.h"

uint8_t duscr[3]={0,5,0};     //显示的各位占空比

uint8_t du=50;				//按键模式下占空比
//int ducache;  //按键模式下的占空比缓存用于确定占空比是否被改变

uint8_t sdu;		//串口模式下的占空比
//int sducache; //串口模式下的占空比缓存用于确定占空比是否被改变

uint8_t freqscr[7]={0,0,0,1,0,0,0};   //显示的各位频率


int32_t freq=1000;			 //按键模式下的频率
//int32_t freqcache; //按键模式下的频率缓存用于确定频率是否被改变

int32_t sfreq;	   //串口模式下的频率 

uint8_t enterflag=1;   //判断串口输入是否符合规范，即是否包含回车的标识符 全局标识符需要各个库访问
uint8_t scrupdateflag = 0;    //屏幕需要刷新的标志位

int main()
{
		uint8_t mainflag=1;       //主菜单选择用flag
		uint8_t secflag=0;        //二级菜单标识符
		uint8_t secselflag=1;     //二级菜单选择用flag
		uint8_t serialflag=0;     //串口菜单标识符
		
		uint8_t insflag=1;          //进入说明后的选择标识符
		uint8_t insenflag=0;        //说明确定标识符
	
		uint8_t keypage=1;					//按键说明的页码标识
		uint8_t serpage=1;					//串口说明的页码标识
	
		uint8_t backrefresh=0;      //这是一个专为返回时多事件驱动刷新屏幕的标识符 具体可看700行左右的注释
	
		uint8_t frechangeflag=0;   //Fre改变的标识符 
		uint8_t duchangeflag=0;		 //du改变的标识符
	
		char finput_buffer[16] = {0};  // Fr接收缓存
		//uint8_t finput_index = 0;   //buffer索引
		uint8_t finput_done = 0;      //频率接收完成标志
		
		char dinput_buffer[16] = {0};  // Du接收缓存
		//uint8_t dinput_index = 0;
		uint8_t dinput_done = 0;			//占空比接收完成标志
		
		uint8_t validflag = 1;      //数据校验用标识符

		
		SysTick_Init(72);//systick初始化
		OLED_Init();     //屏幕初始化
		LED_Init();      //LED初始化
		key_init();      //按键初始化
		
		//开机显示
		OLED_ShowString(30,20,"Welcome!",16);  
		OLED_Refresh_Gram();
		delay_ms(1000);
		OLED_Clear();
		scrupdateflag=1;
		
		while(1)
		{		
				//主菜单的按键逻辑:
				if((!key_status_1())&&(secflag==0))
				{	
						delay_ms(50);
						if(mainflag>3)
						{
								mainflag=1;
						}
						else
								mainflag++;
						scrupdateflag=1;
				}
				//主菜单四个选项的反白显示以及选择逻辑
				if(mainflag==1&&secflag==0)
				{
					  //选项一按下确定的逻辑:		
						if(!key_status_3())
						{	
								OLED_Clear();
						
								delay_ms(50);
								PWM2_CH1_set(freq,du);
								secflag=1;
								scrupdateflag=1;
						}  
						else
						{
								OLED_ShowString(0,20,"              ",16);
								OLED_ShowString(0,40,"              ",16);
								//OLED_ShowString(48,60,"   ",12);
								OLED_ShowString(25,0,"Main Menu",16);
								OLED_ShowString0(0,20,"Key Regu",16);
								OLED_ShowString(0,40,"Serial Regu",16);
								OLED_ShowString(120,50,"1",12);  //页码
								update_screen();
						}
				}
				if(mainflag==2&&secflag==0)
				{
						//选项一按下确定的逻辑:
				
						if(!key_status_3())
						{	
								OLED_Clear();
								delay_ms(50);
								secflag=2;
								scrupdateflag=1;
						}					
						else
						{
								OLED_ShowString(0,20,"             ",16);
								OLED_ShowString(0,40,"             ",16);
								//OLED_ShowString(48,60,"   ",12);
								OLED_ShowString(25,0,"Main Menu",16);
								OLED_ShowString(0,20,"Key Regu",16);
								OLED_ShowString0(0,40,"Serial Regu",16);
								OLED_ShowString(120,50,"1",12);
								update_screen();
						}
				}
				if(mainflag==3&&secflag==0)
				{
						if(!key_status_3())
						{	
								OLED_Clear();
								delay_ms(50);
								secflag=3;
								scrupdateflag=1;
						}	
						else
						{
								OLED_ShowString(0,20,"             ",16);
								OLED_ShowString(0,40,"             ",16);
								//OLED_ShowString(48,60,"  ",12);
								OLED_ShowString(25,0,"Main Menu",16);
								OLED_ShowString0(0,20,"Instructions",16);
								OLED_ShowString(0,40,"More Info",16);
								OLED_ShowString(120,50,"2",12);
								update_screen();
						}
				}
				if(mainflag==4&&secflag==0)
				{
						if(!key_status_3())
						{	
								OLED_Clear();
								delay_ms(50);
								secflag=4;
								scrupdateflag=1;
						}	
						else
						{
								OLED_ShowString(0,20,"          ",16);
								OLED_ShowString(0,40,"            ",16);
								//OLED_ShowString(48,60,"  ",12);
								OLED_ShowString(25,0,"Main Menu",16);
								OLED_ShowString(0,20,"Instructions",16);
								OLED_ShowString0(0,40,"More Info",16);
								OLED_ShowString(120,50,"2",12);
								update_screen();
						}
				}
				//选项1选择后的逻辑
				if(secflag==1)
				{		
						if(!key_status_1()) //按键1选择数字
						{
								//delay_ms(50);
								secselflag++;
								if(secselflag>10)
								{
									 secselflag=1;
								}
								scrupdateflag=1;
						}
						//频率 占空比各位数的显示
						OLED_ShowString(25,0,"Key Reg",16);
						OLED_ShowString(0,20,"Fre:",16);
						OLED_ShowString(86,20,"Hz",16);
						OLED_ShowString(0,40,"Duty:",16);
						OLED_ShowString(62,40,"%",16);
						
						OLED_ShowNum(30,20,freqscr[6],1,16);
						OLED_ShowNum(38,20,freqscr[5],1,16);
						OLED_ShowNum(46,20,freqscr[4],1,16);
						OLED_ShowNum(54,20,freqscr[3],1,16);
						OLED_ShowNum(62,20,freqscr[2],1,16);
						OLED_ShowNum(70,20,freqscr[1],1,16);
						OLED_ShowNum(78,20,freqscr[0],1,16);
						
						OLED_ShowNum(38,40,duscr[2],1,16);
						OLED_ShowNum(46,40,duscr[1],1,16);
						OLED_ShowNum(54,40,duscr[0],1,16);
						//各数的显示以及选择逻辑
						//数字显示的注释为了节省资源
						if(secselflag==1)
						{
								OLED_ShowNum0(30,20,freqscr[6],1,16);
								OLED_ShowNum(38,20,freqscr[5],1,16);
								//OLED_ShowNum(46,20,freqscr[4],1,16);
								//OLED_ShowNum(54,20,freqscr[3],1,16);
								//OLED_ShowNum(62,20,freqscr[2],1,16);
								//OLED_ShowNum(70,20,freqscr[1],1,16);
								OLED_ShowNum(78,20,freqscr[0],1,16);
								update_screen();
								if(!key_status_3())
								{	
										freqscr[6]++;
										if(freqscr[6]>9)
										{
												freqscr[6]=0;
										}
										frechangeflag=1; //任意数被改变即置改变flag为1
										scrupdateflag=1;
								}
						}
						if(secselflag==2)
						{
								OLED_ShowNum(30,20,freqscr[6],1,16);
								OLED_ShowNum0(38,20,freqscr[5],1,16);
								OLED_ShowNum(46,20,freqscr[4],1,16);
								//OLED_ShowNum(54,20,freqscr[3],1,16);
								//OLED_ShowNum(62,20,freqscr[2]2,1,16);
								//OLED_ShowNum(70,20,freqscr[1],1,16);
								//OLED_ShowNum(78,20,freqscr[0],1,16);
								update_screen();
								if(!key_status_3())
								{	
										freqscr[5]++;
										if(freqscr[5]>9)
										{
												freqscr[5]=0;
										}
										frechangeflag=1;
										scrupdateflag=1;
								}
						}
						if(secselflag==3)
						{
								//OLED_ShowNum(30,20,freqscr[6],1,16);
								OLED_ShowNum(38,20,freqscr[5],1,16);
								OLED_ShowNum0(46,20,freqscr[4],1,16);
								OLED_ShowNum(54,20,freqscr[3],1,16);
								//OLED_ShowNum(62,20,freqscr[2],1,16);
								//OLED_ShowNum(70,20,freqscr[1],1,16);
								//OLED_ShowNum(78,20,freqscr[0],1,16);
								update_screen();
								if(!key_status_3())
								{	
										freqscr[4]++;
										if(freqscr[4]>9)
										{
												freqscr[4]=0;
										}
										frechangeflag=1;
										scrupdateflag=1;
								}
						}
						if(secselflag==4)
						{
								//OLED_ShowNum(30,20,freqscr[6],1,16);
								//OLED_ShowNum(38,20,freqscr[5],1,16);
								OLED_ShowNum(46,20,freqscr[4],1,16);
								OLED_ShowNum0(54,20,freqscr[3],1,16);
								OLED_ShowNum(62,20,freqscr[2],1,16);
								//OLED_ShowNum(70,20,freqscr[1],1,16);
								//OLED_ShowNum(78,20,freqscr[0],1,16);
								update_screen();
								if(!key_status_3())
								{	
										freqscr[3]++;
										if(freqscr[3]>9)
										{
												freqscr[3]=0;
										}
										frechangeflag=1;
										scrupdateflag=1;
								}
						}
						if(secselflag==5)
						{
								//OLED_ShowNum(30,20,freqscr[6],1,16);
								//OLED_ShowNum(38,20,freqscr[5],1,16);
								//OLED_ShowNum(46,20,freqscr[4],1,16);
								OLED_ShowNum(54,20,freqscr[3],1,16);
								OLED_ShowNum0(62,20,freqscr[2],1,16);
								OLED_ShowNum(70,20,freqscr[1],1,16);
								//OLED_ShowNum(78,20,freqscr[0],1,16);
								update_screen();
								if(!key_status_3())
								{	
										freqscr[2]++;
										if(freqscr[2]>9)
										{
												freqscr[2]=0;
										}
										frechangeflag=1;
										scrupdateflag=1;
								}
						}
						if(secselflag==6)
						{
								OLED_ShowNum(30,20,freqscr[6],1,16);
								//OLED_ShowNum(38,20,freqscr[5],1,16);
								//OLED_ShowNum(46,20,freqscr[4],1,16);
								//OLED_ShowNum(54,20,freqscr[3],1,16);
								OLED_ShowNum(62,20,freqscr[2],1,16);
								OLED_ShowNum0(70,20,freqscr[1],1,16);
								OLED_ShowNum(78,20,freqscr[0],1,16);
								update_screen();
								if(!key_status_3())
								{	
										freqscr[1]++;
										if(freqscr[1]>9)
										{
												freqscr[1]=0;
										}
										frechangeflag=1;
										scrupdateflag=1;
								}
						}
						if(secselflag==7)
						{
								OLED_ShowNum(30,20,freqscr[6],1,16);
								//OLED_ShowNum(38,20,freqscr[5],1,16);
								//OLED_ShowNum(46,20,freqscr[4],1,16);
								//OLED_ShowNum(54,20,freqscr[3],1,16);
								//OLED_ShowNum(62,20,freqscr[2],1,16);
								OLED_ShowNum(70,20,freqscr[1],1,16);
								OLED_ShowNum0(78,20,freqscr[0],1,16);
								update_screen();
								if(!key_status_3())
								{	
										freqscr[0]++;
										
										if(freqscr[0]>9)
										{
												freqscr[0]=0;
										}
										frechangeflag=1;
										scrupdateflag=1;
								}
						}
						if(secselflag==8)
						{
								OLED_ShowNum0(38,40,duscr[2],1,16);
								OLED_ShowNum(46,40,duscr[1],1,16);
								OLED_ShowNum(54,40,duscr[0],1,16);
								update_screen();
								if(!key_status_3())
								{	
										duscr[2]++;
										if(duscr[2]==1) //占空比100的处理
										{
												duscr[1]=0;
												duscr[0]=0;
										}
										if(duscr[2]>1)
										{
												duscr[2]=0;
										}
										duchangeflag=1;
										scrupdateflag=1;
								}
						}
						if(secselflag==9)
						{
								OLED_ShowNum(38,40,duscr[2],1,16);
								OLED_ShowNum0(46,40,duscr[1],1,16);
								OLED_ShowNum(54,40,duscr[0],1,16);
								update_screen();
								if(!key_status_3())
								{	
										if(duscr[2]!=1)
										{
												duscr[1]++;
										}
										if(duscr[1]>9)
										{
												duscr[1]=0;
										}
										duchangeflag=1;
										scrupdateflag=1;
								}
						}
						if(secselflag==10)
						{
								OLED_ShowNum(38,40,duscr[2],1,16);
								OLED_ShowNum(46,40,duscr[1],1,16);
								OLED_ShowNum0(54,40,duscr[0],1,16);
								update_screen();
								if(!key_status_3())
								{	
										if(duscr[2]!=1)
										{
												duscr[0]++;
										}
										if(duscr[0]>9)
										{
												duscr[0]=0;
										}
										duchangeflag=1;
										scrupdateflag=1;
								}
						}

						//改变后只set一次PWM的实现
						if(frechangeflag==1||duchangeflag==1)
						{
								//此处计算freq duty可节省系统轮询资源，缺点是初始以及频率为0的处理也需要重新设置PWM
							
								freq=freCal(freqscr,7);   //封装函数在math库实现
								du=duCal(duscr,3);
							
							  /*下方为旧实现*/
								//freq=freq0+(freq1*10)+(freq2*100)+(freq3*1000)+(freq4*10000)+(freq5*100000)+(freq6*1000000);  //占空比与频率的转换逻辑							
								//du=du0+(du1*10)+(du2*100);
								
								PWM2_CH1_set(freq,du);
							
								//置0 flag
								frechangeflag=0;
								duchangeflag=0;
						}
						//避免频率为0的处理
						if(freq==0)
						{
								freqscr[0]=1;
								//处理后需要重新计算freq duty并设置PWM
								freq=freCal(freqscr,7);   
								du=duCal(duscr,3);
							
								PWM2_CH1_set(freq,du);
								
						}

						if(!key_status_4()) //退回上一级菜单
						{
								OLED_Clear();
								delay_ms(50);
								secflag=0;
								scrupdateflag=1;
						}
				}
				
				//选项2选择后的逻辑，串口菜单0，初始化串口以及动画，与系统整体初始化解耦
				//动画除了好看没什么用（bushi）
				if(secflag==2&&serialflag==0)
				{		
						OLED_ShowString(18,0,"Serial Reg",16);
						TIM3_Init();
						USART1_Init(115200);	
						OLED_ShowString(0,28,"Pre for Serial",16);
						OLED_Refresh_Gram();
						delay_ms(300);
						OLED_ShowString(0,28,"Pre for Serial.",16);
						OLED_Refresh_Gram();
						delay_ms(300);
						OLED_ShowString(0,28,"Pre for Serial..",16);
						OLED_Refresh_Gram();
						delay_ms(300);
						OLED_Clear();
						OLED_ShowString(55,25,"OK",16);
						OLED_Refresh_Gram();
						delay_ms(700);
						OLED_Clear();
						serialflag=1;
						//OLED_ShowString(0,20,"Fre:",16);
						//OLED_ShowString(0,40,"Duty:",16);
						//OLED_ShowString(70,40,"%",16);
					
						//可有可无的退出 先留着
						if(!key_status_4())
							{
									OLED_Clear();
									delay_ms(50);
									secflag=0;
									scrupdateflag=1;
							}
				}
				//串口菜单1 设置频率
				if(secflag==2&&serialflag==1)
				{	
						OLED_ShowString(18,0,"Serial Reg",16);
						OLED_ShowString(0,20,"Enter Fr(1-36M):",16);
						update_screen();
						if(backrefresh==1) //为事件驱动的屏幕刷新服务
						{
								scrupdateflag=1;//如果是返回事件触发的 那么再赋值一次scrupdate
								backrefresh=0;
						}
						while (!finput_done)
						{	
								//退回上一级菜单
								if(!key_status_4())
								{
											memset(finput_buffer, 0, sizeof(finput_buffer));  //删缓存
											OLED_Clear();
											delay_ms(50);
											secflag=0;
											scrupdateflag=1;
											break;
								}
								//回车符判断部分
							
								/**************************************
								md底层的串口服务程序用0x0a和0x0d判断的 
								我说怎么这个代码总不生效 改库了md
								还得加个定时器我真是服了
								***************************************/
							
								/*垃圾库写的可读性依托答辩*/
								if (!enterflag) 
								{
										// 提示输入不完整
										OLED_ShowString(0, 36, "Incomplete Input", 16);
										OLED_Refresh_Gram();
										memset(dinput_buffer,0,sizeof(finput_buffer));  // 删缓存 可以省略但保险起见留下
										//USART1_RX_STA = 0;                            // 清除接收标志在Timer中已经实现
										enterflag=1;
										delay_ms(1000);
										OLED_ShowString(0, 36, "                ", 16);
										OLED_Refresh_Gram();
										continue;  // 跳过本次大循环
								}
								
								if (USART1_RX_STA&0x8000)  // 接收完成
								{
										uint8_t len = USART1_RX_STA&0x3FFF;                    // 接收数据长度
										//char *p;
										//char *ptr;
									
										if (len<sizeof(finput_buffer)-1) 
										{
												// 将接收到的数据复制到缓存中
												strncpy(finput_buffer,(char *)USART1_RX_BUF,len);  //强转char 不想动库
												finput_buffer[len] = '\0';                         // 添加字符串结束符
											
												/*旧的实现 不要看了
											
												for(p=finput_buffer;*p!='\0';p++)
												{
														if(*p =='\r'||*p =='\n')
														{
																enterflag=1;
																*p ='\0';
																USART1_RX_STA=0;  // 清除接收标志
															  break;
														}
												}
											
												*/
												
												/****************************************
												与duty实现方式不同的原因是 fre本就不允许为0，
												当下方的atoi异常退出返回0时会被合法性检测阻止
												固其实现更简单
												*****************************************/
											
												// 解析输入并检查合法性
												sfreq=atoi(finput_buffer);         // 转换为整数
												if (sfreq>=1&&sfreq<=36000000) //36M为理论最高频率 跑不跑得到我也不知道
												{
														finput_done=1;  //输入完成
														scrupdateflag=1;   //这里的按键事件是串口数据驱动
												}
												else 
												{
														// 提示输入非法
														OLED_ShowString(0, 36, "Invalid Input", 16);
														OLED_Refresh_Gram();
													
														//finput_index = 0;  // 重置输入 重置索引
														
														memset(finput_buffer, 0, sizeof(finput_buffer)); //memset 清数组最快的方法
														USART1_RX_STA=0;                                 //置0接收标识符再次接收
													
														delay_ms(1000);
														OLED_ShowString(0,36,"               ",16);
														OLED_Refresh_Gram();
												}                                 
										}
										else 
										{
												// 数据长度无效
												OLED_ShowString(0, 36, "Invalid Length", 16);
												OLED_Refresh_Gram();
												memset(dinput_buffer, 0, sizeof(finput_buffer));
												USART1_RX_STA = 0;  // 清除接收标志
												delay_ms(1000);
												OLED_ShowString(0, 36, "               ", 16);
												OLED_Refresh_Gram();
										}
										memset(finput_buffer, 0, sizeof(finput_buffer));//退出删缓存
								}
						}
						//输入完成后进入下一级数组菜单的操作
						if(finput_done==1)
						{
									OLED_ShowNum0(20,42,sfreq,8,16);
									OLED_ShowString(84,42,"Hz",16);
									//OLED_Refresh_Gram();
									update_screen();
							
									/*别看下面的内容了 已经加了一个变量实现功能了*/							
									/****************************************
									这里不使用update的原因是在返回时会有多个事件
									难以使用事件驱动 会被539行的update覆盖
									如果使用将无法在退回时显示sfreq 后方代码同理
									（其实可以单独写一个变量判断的）
									****************************************/

									if(!key_status_3())
									{	
											serialflag=2;
											OLED_ShowString(0,42,"               ",16);
											OLED_Refresh_Gram();
											scrupdateflag=1;
									}
						}
						//退回未输入状态
						if(!key_status_4())
							{	
									finput_done=0;
									USART1_RX_STA=0;  
									memset(finput_buffer, 0, sizeof(finput_buffer));  //退出删缓存 
									OLED_ShowString(0,42,"               ",16);
									OLED_Refresh_Gram();
									scrupdateflag=1;
							}
				}
				//串口菜单2，串口duty设置
				if(secflag==2&&serialflag==2)
				{
						OLED_ShowString(18,0,"Serial Reg",16);
						OLED_ShowString(0,20,"Enter Du(0-100):",16);
						update_screen();
						if(backrefresh==1) //为事件驱动的屏幕刷新服务
						{
								scrupdateflag=1;//如果是返回事件触发的 那么再赋值一次scrupdate
								backrefresh=0;
						}
						while (!dinput_done)
						{
								if (!key_status_4())  // 按键返回
								{
										OLED_Clear();
										delay_ms(50);
										memset(dinput_buffer, 0, sizeof(dinput_buffer));  //返回时清空缓存
										serialflag=1;
										scrupdateflag=1;
										backrefresh=1; //为事件驱动的屏幕刷新服务
										break;
								}
								if (!enterflag) 
								{
										// 提示输入不完整
										OLED_ShowString(0, 36, "Incomplete Input", 16);
										OLED_Refresh_Gram();
										memset(dinput_buffer,0,sizeof(dinput_buffer));  // 删缓存
										//USART1_RX_STA = 0;  //同上 在Timer中已实现清零
										enterflag=1;
										delay_ms(1000);
										OLED_ShowString(0, 36, "                ", 16);
										OLED_Refresh_Gram();
										continue;  // 跳过本次大循环
								}
								if (USART1_RX_STA&0x8000)  // 接收完成
								{									
										char *p;         //for循环初值
										char *ptr;       //指向缓存的指针,用于预处理数据
									  char *allptr;    //删除前后空字符的处理后的指向缓存的指针 
									
										int ptrsum=0;		 //移位标识符，其实可以省略						
										uint8_t len = USART1_RX_STA&0x3FFF;  // 接收数据长度
										validflag=1;  //
										
										if (len>0&&len<sizeof(dinput_buffer)-1) 
										{
												// 将接收到的数据复制到缓存中
												strncpy(dinput_buffer,(char *)USART1_RX_BUF,len);
												dinput_buffer[len]='\0';  // 添加字符串结束符
											
												/*到此与fre处理相同*/
											
												
											
												//旧实现 同上 注释掉
											  /*
												for(p=dinput_buffer;*p!='\0';p++)
												{
														if(*p =='\r'||*p =='\n')
														{
																enterflag=1;
																*p ='\0';
																//USART1_RX_STA = 0;  // 清除接收标志																
															  break;
														}
												}
												*/
												/*******************************************************
												此程序最复杂的部分，duty可以为0因此atoi的异常返回不会被发现
												使用指针预处理串口数据
												********************************************************/
												
												/***********************************************************
												删除空格的代码 主要是为了串口输入中有空格和回车时，也可以正常使用
												如果不加，串口必须传入纯数字 否则会被下方的数字判断逻辑识别为无效
												本质还是因为duty可以为0导致atoi的一些异常不能被发现
												否则atoi自带跳过空字符的操作
												**********************************************************/
												ptr=dinput_buffer;    //未删除空格的指针初始地址
												allptr=dinput_buffer; //删除空格后的指针初始地址相同
												
												while (*ptr!='\0') 
												{
														if (*ptr!='\r'&&*ptr!='\n'&&*ptr!=' ') 
														{
																*allptr=*ptr;  // 只保留有效字符
																allptr++;			 //处理下一位
															  ptrsum++;      //移位标识符
														}
														ptr++;             //处理下一位
												}
												*allptr='\0';          //加上处理完成后的指针指向内容的终止符
												allptr=allptr-ptrsum;  //处理完成后指针归位，也可直接赋初始地址
												
												/*********************************************************
												下方是为了在加上上方删除空格的代码后
												防止数据为空字符串导致跳过数字检测，atoi识别不到字符
												直接返回0导致错误使用的
												**********************************************************/
												if(*allptr=='\0')
												{	
														validflag=0;
												}
												for(p=allptr;*p!='\0';p++) 
												{
														if (*p<'0'||*p>'9') 
														{
																validflag=0;
																break;
														}
												}
												allptr=dinput_buffer;    //指针复位
												/*********************************************************
												到此的逻辑为：为了避免串口错误输入 首先删除空格，
												然后判断是否纯数字，接着传递给atoi，本质是处理了atoi返回0的情况
												**********************************************************/
												
												if (validflag) 
												{
														sdu=atoi(allptr);  // 转换为整数
														if (sdu>=0&&sdu<=100) 
														{
																dinput_done=1;  // 输入完成
																scrupdateflag=1; //同上，串口数据驱动
														}
														else 
														{
																validflag=0;
														}
												}

												if (!validflag) 
												{
														// 提示输入非法
														OLED_ShowString(0, 36, "Invalid Input", 16);
														OLED_Refresh_Gram();	
														memset(dinput_buffer, 0, sizeof(dinput_buffer));  
														delay_ms(1000);
														USART1_RX_STA=0;
														OLED_ShowString(0, 36, "             ", 16);
														OLED_Refresh_Gram();
												}
										}
										else 
										{
												// 数据长度无效
												OLED_ShowString(0, 36, "Invalid Length", 16);
												OLED_Refresh_Gram();
												memset(dinput_buffer, 0, sizeof(dinput_buffer));
												USART1_RX_STA=0;  // 清除接收标志
												delay_ms(1000);
												OLED_ShowString(0, 36, "               ", 16);
												OLED_Refresh_Gram();
										}
										memset(USART1_RX_BUF, 0, sizeof(USART1_RX_BUF));  //输完最后删缓存
								}
						}
						//同上 输入完成进入下一级菜单
						if(dinput_done==1)
						{
									OLED_ShowNum0(47,42,sdu,3,16);
									OLED_ShowString(71,42,"%",16);
									update_screen();
									if(!key_status_3())
									{	
											serialflag=3;
											OLED_ShowString(0,20,"                  ",16);
											OLED_ShowString(0,42,"               ",16);
											OLED_Refresh_Gram();
											scrupdateflag=1;
										  PWM2_CH1_set(sfreq,sdu);//整合到按键中set一次
									}
						}
						//退回输入状态
						if(!key_status_4())
							{	
									dinput_done=0;
									USART1_RX_STA = 0;
									memset(finput_buffer, 0, sizeof(dinput_buffer));  //退出删缓存 
									OLED_ShowString(0,42,"               ",16);
									OLED_Refresh_Gram();
									scrupdateflag=1;
							}
				}
				//串口的最终输出菜单
				if(secflag==2&&serialflag==3)
				{
						OLED_ShowString(18,0,"Serial Reg",16);
						OLED_ShowString(0,20,"Fre:",16);
						OLED_ShowNum(30,20,sfreq,8,16);
						OLED_ShowString(94,20,"Hz",16);
						OLED_ShowString(0,40,"Duty:",16);
						OLED_ShowNum(70,40,sdu,3,16);
						OLED_ShowString(94,40,"%",16);
						update_screen();
						if(!key_status_4())
						{	
								serialflag=2;
								OLED_ShowString(0,20,"                  ",16);
								OLED_ShowString(0,40,"                  ",16);
								OLED_Refresh_Gram();
								scrupdateflag=1;
								backrefresh=1;    //为事件驱动的屏幕刷新服务
				
						}
						
				}
				//系统的简要说明菜单
				if(secflag==3)
				{
						if(!key_status_1()&&!insenflag)
						{
								insflag++;
								if(insflag>2)
								{	
										insflag=1;
								}
								scrupdateflag=1;
						}
						if(insflag==1&&!insenflag)
						{
								OLED_ShowString(12,0,"Instructions",16);
								OLED_ShowString0(0,20,"Key Instruc",16);
								OLED_ShowString(0,40,"Serial Instruc",16);
								update_screen();
								if(!key_status_3())
								{
										OLED_Clear();
										delay_ms(50);
										insenflag=1;
										scrupdateflag=1;
								}
								if(!key_status_4())
								{
										OLED_Clear();
										delay_ms(50);
										secflag=0;
										scrupdateflag=1;
								}
						}
						if(insflag==1&&insenflag)
						{	
								if(!key_status_1())
								{	
										OLED_ShowString(0,20,"               ",16);
										OLED_ShowString(0,40,"               ",16);
										OLED_Refresh_Gram();
										keypage++;
										if(keypage>4)
										{	
												keypage=1;
										}
										scrupdateflag=1;
								}
								if(keypage==1)
								{
										OLED_ShowString(30,0,"Key Ins",16);
										OLED_ShowString(0,20,"K1:Sel Num Dig",16);
										OLED_ShowString(0,40,"K3:Change Num",16);
										update_screen();
								}
								if(keypage==2)
								{
										OLED_ShowString(30,0,"Key Ins",16);
										OLED_ShowString(0,20,"K4:Back Menu",16);
										OLED_ShowString(0,40,"Change Effect",16);
										update_screen();
								}
								if(keypage==3)
								{
										OLED_ShowString(30,0,"Key Ins",16);
										OLED_ShowString(0,20,"Immediately,No ",16);
										OLED_ShowString(0,40,"Need to Ensure.",16);
										update_screen();
								}
								if(keypage==4)
								{
										OLED_ShowString(30,0,"Key Ins",16);
										OLED_ShowString(0,20,"1=< Fre =<36M",16);
										OLED_ShowString(0,40,"0=< Duty =<100",16);
										update_screen();
								}
								if(!key_status_4()) //退回上一级菜单
								{
										OLED_Clear();
										delay_ms(50);
										insenflag=0;
										scrupdateflag=1;
								}
						}
						if(insflag==2&&!insenflag)
						{
								OLED_ShowString(12,0,"Instructions",16);
								OLED_ShowString(0,20,"Key Instruc",16);
								OLED_ShowString0(0,40,"Serial Instruc",16);
								update_screen();
								if(!key_status_3())
								{
										OLED_Clear();
										delay_ms(50);
										insenflag=1;
										scrupdateflag=1;
								}
								if(!key_status_4())
								{
										OLED_Clear();
										delay_ms(50);
										secflag=0;
										scrupdateflag=1;
								}
						}
						if(insflag==2&&insenflag)
						{	
								if(!key_status_1())
								{	
										OLED_ShowString(0,20,"               ",16);
										OLED_ShowString(0,40,"               ",16);
										OLED_Refresh_Gram();
										serpage++;
										if(serpage>5)
										{	
												serpage=1;
										}
										scrupdateflag=1;
								}
								if(serpage==1)
								{
										OLED_ShowString(15,0,"Serial Ins",16);
										OLED_ShowString(0,20,"Serial input",16);
										OLED_ShowString(0,40,"can contain",16);
										update_screen();
								}
								if(serpage==2)
								{
										OLED_ShowString(15,0,"Serial Ins",16);
										OLED_ShowString(0,20,"space,K3 to",16);
										OLED_ShowString(0,40,"confirm input.",16);
										update_screen();
								}
								if(serpage==3)
								{
										OLED_ShowString(15,0,"Serial Ins",16);
										OLED_ShowString(0,20,"The end of ",16);
										OLED_ShowString(0,40,"serial input",16);
										update_screen();
								}
								if(serpage==4)
								{
										OLED_ShowString(15,0,"Serial Ins",16);
										OLED_ShowString(0,20,"must have ",16);
										OLED_ShowString(0,40,"carriage and",16);
										update_screen();
								}
								if(serpage==5)
								{
										OLED_ShowString(15,0,"Serial Ins",16);
										OLED_ShowString(0,20,"line break.",16);
										update_screen();
								}
								if(!key_status_4()) //退回上一级菜单
								{
										OLED_Clear();
										delay_ms(50);
										insenflag=0;
										scrupdateflag=1;
								}
						}
				}
				//开发者与系统版本号
				//GPL提示说明
				if(secflag==4)
				{
						OLED_ShowString(25,0,"More Info",16);
					  OLED_ShowString(0,24,"OS By Qiuye(TQW)",16);
					  OLED_ShowString0(0,51,"GNU Proj",12);
						OLED_ShowString(56,51,"Version1.4.2",12);
						update_screen();
						if(!key_status_4())
						{
								OLED_Clear();
								delay_ms(50);
								secflag=0;
								scrupdateflag=1;
						}
				}
		}
}
