/*******************************************
����Ŀ��ѭGNU General Public License v3.0
            COPYLEFT 2024

��Դ��ַ��https://github.com/xhytqw/PWM_out

������ϵͳ�汾���֣�

���ߣ�Qiwei Tian(Qiuye)

���ܣ�����STM32�Ĵ�GUI�Ŀ�ƵPWM�����

v1.0��12.8����
ʵ���˻�������
���⣺����Ƶ������ʱδԼ��SET�Ĵ�������bug��
���ڴ���bug����������ʱ����ʾ��
��Ҫ����atoi�������·���ֵһֱΪ0


v1.1��12.10��
�޸��˰����봮�ڵ�PWM SET��ε�bug
���⣺�����д�������ʾ�����ã�����ǿ³����


v1.2��12.12��:
����bugȫ���޸���ϣ����ۺ��ִ��ڴ������������ʾ
����Timer6��ʱ���жϴ����ڳ�ʱ����ǿ³����					
���⣺du���ַ��ո�����,whileѭ��ֻ�ܴ���ǰ�ո�


v1.3��12.13��
����˴���duty������ �����ֿ�ɾ�������пո��Ҳ�������
���봮���������Լ�ϵͳ������˵���˵�
���⣺��Ļ�������ˢ�ºķ�ϵͳ��Դ


v1.4 (12.15)
�����Ļˢ�����⣬�����¼�����
ֻ����Ҫʱˢ��һ�Σ���OLED_Refresh�������ع�
(����һ���ǳ��õĸĶ�ֱ�Ӱ�ϵͳ����������˺ü���)


v1.4.1(12.17)
��ʾfre du�����ݽṹ��Ϊ���飬ûʲô�õ������Ϲ淶


v1.4.2(12.17)
�ع���װ��һЩ����������ά��

***�µĸ��ļ���������������ڴ˴�˵��������ѭGPLЭ��***

����������ʹ�á��޸ĺͷַ�������������뱣����������Э���ı���
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

uint8_t duscr[3]={0,5,0};     //��ʾ�ĸ�λռ�ձ�

uint8_t du=50;				//����ģʽ��ռ�ձ�
//int ducache;  //����ģʽ�µ�ռ�ձȻ�������ȷ��ռ�ձ��Ƿ񱻸ı�

uint8_t sdu;		//����ģʽ�µ�ռ�ձ�
//int sducache; //����ģʽ�µ�ռ�ձȻ�������ȷ��ռ�ձ��Ƿ񱻸ı�

uint8_t freqscr[7]={0,0,0,1,0,0,0};   //��ʾ�ĸ�λƵ��


int32_t freq=1000;			 //����ģʽ�µ�Ƶ��
//int32_t freqcache; //����ģʽ�µ�Ƶ�ʻ�������ȷ��Ƶ���Ƿ񱻸ı�

int32_t sfreq;	   //����ģʽ�µ�Ƶ�� 

uint8_t enterflag=1;   //�жϴ��������Ƿ���Ϲ淶�����Ƿ�����س��ı�ʶ�� ȫ�ֱ�ʶ����Ҫ���������
uint8_t scrupdateflag = 0;    //��Ļ��Ҫˢ�µı�־λ

int main()
{
		uint8_t mainflag=1;       //���˵�ѡ����flag
		uint8_t secflag=0;        //�����˵���ʶ��
		uint8_t secselflag=1;     //�����˵�ѡ����flag
		uint8_t serialflag=0;     //���ڲ˵���ʶ��
		
		uint8_t insflag=1;          //����˵�����ѡ���ʶ��
		uint8_t insenflag=0;        //˵��ȷ����ʶ��
	
		uint8_t keypage=1;					//����˵����ҳ���ʶ
		uint8_t serpage=1;					//����˵����ҳ���ʶ
	
		uint8_t backrefresh=0;      //����һ��רΪ����ʱ���¼�����ˢ����Ļ�ı�ʶ�� ����ɿ�700�����ҵ�ע��
	
		uint8_t frechangeflag=0;   //Fre�ı�ı�ʶ�� 
		uint8_t duchangeflag=0;		 //du�ı�ı�ʶ��
	
		char finput_buffer[16] = {0};  // Fr���ջ���
		//uint8_t finput_index = 0;   //buffer����
		uint8_t finput_done = 0;      //Ƶ�ʽ�����ɱ�־
		
		char dinput_buffer[16] = {0};  // Du���ջ���
		//uint8_t dinput_index = 0;
		uint8_t dinput_done = 0;			//ռ�ձȽ�����ɱ�־
		
		uint8_t validflag = 1;      //����У���ñ�ʶ��

		
		SysTick_Init(72);//systick��ʼ��
		OLED_Init();     //��Ļ��ʼ��
		LED_Init();      //LED��ʼ��
		key_init();      //������ʼ��
		
		//������ʾ
		OLED_ShowString(30,20,"Welcome!",16);  
		OLED_Refresh_Gram();
		delay_ms(1000);
		OLED_Clear();
		scrupdateflag=1;
		
		while(1)
		{		
				//���˵��İ����߼�:
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
				//���˵��ĸ�ѡ��ķ�����ʾ�Լ�ѡ���߼�
				if(mainflag==1&&secflag==0)
				{
					  //ѡ��һ����ȷ�����߼�:		
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
								OLED_ShowString(120,50,"1",12);  //ҳ��
								update_screen();
						}
				}
				if(mainflag==2&&secflag==0)
				{
						//ѡ��һ����ȷ�����߼�:
				
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
				//ѡ��1ѡ�����߼�
				if(secflag==1)
				{		
						if(!key_status_1()) //����1ѡ������
						{
								//delay_ms(50);
								secselflag++;
								if(secselflag>10)
								{
									 secselflag=1;
								}
								scrupdateflag=1;
						}
						//Ƶ�� ռ�ձȸ�λ������ʾ
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
						//��������ʾ�Լ�ѡ���߼�
						//������ʾ��ע��Ϊ�˽�ʡ��Դ
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
										frechangeflag=1; //���������ı伴�øı�flagΪ1
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
										if(duscr[2]==1) //ռ�ձ�100�Ĵ���
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

						//�ı��ֻsetһ��PWM��ʵ��
						if(frechangeflag==1||duchangeflag==1)
						{
								//�˴�����freq duty�ɽ�ʡϵͳ��ѯ��Դ��ȱ���ǳ�ʼ�Լ�Ƶ��Ϊ0�Ĵ���Ҳ��Ҫ��������PWM
							
								freq=freCal(freqscr,7);   //��װ������math��ʵ��
								du=duCal(duscr,3);
							
							  /*�·�Ϊ��ʵ��*/
								//freq=freq0+(freq1*10)+(freq2*100)+(freq3*1000)+(freq4*10000)+(freq5*100000)+(freq6*1000000);  //ռ�ձ���Ƶ�ʵ�ת���߼�							
								//du=du0+(du1*10)+(du2*100);
								
								PWM2_CH1_set(freq,du);
							
								//��0 flag
								frechangeflag=0;
								duchangeflag=0;
						}
						//����Ƶ��Ϊ0�Ĵ���
						if(freq==0)
						{
								freqscr[0]=1;
								//�������Ҫ���¼���freq duty������PWM
								freq=freCal(freqscr,7);   
								du=duCal(duscr,3);
							
								PWM2_CH1_set(freq,du);
								
						}

						if(!key_status_4()) //�˻���һ���˵�
						{
								OLED_Clear();
								delay_ms(50);
								secflag=0;
								scrupdateflag=1;
						}
				}
				
				//ѡ��2ѡ�����߼������ڲ˵�0����ʼ�������Լ���������ϵͳ�����ʼ������
				//�������˺ÿ�ûʲô�ã�bushi��
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
					
						//���п��޵��˳� ������
						if(!key_status_4())
							{
									OLED_Clear();
									delay_ms(50);
									secflag=0;
									scrupdateflag=1;
							}
				}
				//���ڲ˵�1 ����Ƶ��
				if(secflag==2&&serialflag==1)
				{	
						OLED_ShowString(18,0,"Serial Reg",16);
						OLED_ShowString(0,20,"Enter Fr(1-36M):",16);
						update_screen();
						if(backrefresh==1) //Ϊ�¼���������Ļˢ�·���
						{
								scrupdateflag=1;//����Ƿ����¼������� ��ô�ٸ�ֵһ��scrupdate
								backrefresh=0;
						}
						while (!finput_done)
						{	
								//�˻���һ���˵�
								if(!key_status_4())
								{
											memset(finput_buffer, 0, sizeof(finput_buffer));  //ɾ����
											OLED_Clear();
											delay_ms(50);
											secflag=0;
											scrupdateflag=1;
											break;
								}
								//�س����жϲ���
							
								/**************************************
								md�ײ�Ĵ��ڷ��������0x0a��0x0d�жϵ� 
								��˵��ô��������ܲ���Ч �Ŀ���md
								���üӸ���ʱ�������Ƿ���
								***************************************/
							
								/*������д�Ŀɶ������д��*/
								if (!enterflag) 
								{
										// ��ʾ���벻����
										OLED_ShowString(0, 36, "Incomplete Input", 16);
										OLED_Refresh_Gram();
										memset(dinput_buffer,0,sizeof(finput_buffer));  // ɾ���� ����ʡ�Ե������������
										//USART1_RX_STA = 0;                            // ������ձ�־��Timer���Ѿ�ʵ��
										enterflag=1;
										delay_ms(1000);
										OLED_ShowString(0, 36, "                ", 16);
										OLED_Refresh_Gram();
										continue;  // �������δ�ѭ��
								}
								
								if (USART1_RX_STA&0x8000)  // �������
								{
										uint8_t len = USART1_RX_STA&0x3FFF;                    // �������ݳ���
										//char *p;
										//char *ptr;
									
										if (len<sizeof(finput_buffer)-1) 
										{
												// �����յ������ݸ��Ƶ�������
												strncpy(finput_buffer,(char *)USART1_RX_BUF,len);  //ǿתchar ���붯��
												finput_buffer[len] = '\0';                         // ����ַ���������
											
												/*�ɵ�ʵ�� ��Ҫ����
											
												for(p=finput_buffer;*p!='\0';p++)
												{
														if(*p =='\r'||*p =='\n')
														{
																enterflag=1;
																*p ='\0';
																USART1_RX_STA=0;  // ������ձ�־
															  break;
														}
												}
											
												*/
												
												/****************************************
												��dutyʵ�ַ�ʽ��ͬ��ԭ���� fre���Ͳ�����Ϊ0��
												���·���atoi�쳣�˳�����0ʱ�ᱻ�Ϸ��Լ����ֹ
												����ʵ�ָ���
												*****************************************/
											
												// �������벢���Ϸ���
												sfreq=atoi(finput_buffer);         // ת��Ϊ����
												if (sfreq>=1&&sfreq<=36000000) //36MΪ�������Ƶ�� �ܲ��ܵõ���Ҳ��֪��
												{
														finput_done=1;  //�������
														scrupdateflag=1;   //����İ����¼��Ǵ�����������
												}
												else 
												{
														// ��ʾ����Ƿ�
														OLED_ShowString(0, 36, "Invalid Input", 16);
														OLED_Refresh_Gram();
													
														//finput_index = 0;  // �������� ��������
														
														memset(finput_buffer, 0, sizeof(finput_buffer)); //memset ���������ķ���
														USART1_RX_STA=0;                                 //��0���ձ�ʶ���ٴν���
													
														delay_ms(1000);
														OLED_ShowString(0,36,"               ",16);
														OLED_Refresh_Gram();
												}                                 
										}
										else 
										{
												// ���ݳ�����Ч
												OLED_ShowString(0, 36, "Invalid Length", 16);
												OLED_Refresh_Gram();
												memset(dinput_buffer, 0, sizeof(finput_buffer));
												USART1_RX_STA = 0;  // ������ձ�־
												delay_ms(1000);
												OLED_ShowString(0, 36, "               ", 16);
												OLED_Refresh_Gram();
										}
										memset(finput_buffer, 0, sizeof(finput_buffer));//�˳�ɾ����
								}
						}
						//������ɺ������һ������˵��Ĳ���
						if(finput_done==1)
						{
									OLED_ShowNum0(20,42,sfreq,8,16);
									OLED_ShowString(84,42,"Hz",16);
									//OLED_Refresh_Gram();
									update_screen();
							
									/*������������� �Ѿ�����һ������ʵ�ֹ�����*/							
									/****************************************
									���ﲻʹ��update��ԭ�����ڷ���ʱ���ж���¼�
									����ʹ���¼����� �ᱻ539�е�update����
									���ʹ�ý��޷����˻�ʱ��ʾsfreq �󷽴���ͬ��
									����ʵ���Ե���дһ�������жϵģ�
									****************************************/

									if(!key_status_3())
									{	
											serialflag=2;
											OLED_ShowString(0,42,"               ",16);
											OLED_Refresh_Gram();
											scrupdateflag=1;
									}
						}
						//�˻�δ����״̬
						if(!key_status_4())
							{	
									finput_done=0;
									USART1_RX_STA=0;  
									memset(finput_buffer, 0, sizeof(finput_buffer));  //�˳�ɾ���� 
									OLED_ShowString(0,42,"               ",16);
									OLED_Refresh_Gram();
									scrupdateflag=1;
							}
				}
				//���ڲ˵�2������duty����
				if(secflag==2&&serialflag==2)
				{
						OLED_ShowString(18,0,"Serial Reg",16);
						OLED_ShowString(0,20,"Enter Du(0-100):",16);
						update_screen();
						if(backrefresh==1) //Ϊ�¼���������Ļˢ�·���
						{
								scrupdateflag=1;//����Ƿ����¼������� ��ô�ٸ�ֵһ��scrupdate
								backrefresh=0;
						}
						while (!dinput_done)
						{
								if (!key_status_4())  // ��������
								{
										OLED_Clear();
										delay_ms(50);
										memset(dinput_buffer, 0, sizeof(dinput_buffer));  //����ʱ��ջ���
										serialflag=1;
										scrupdateflag=1;
										backrefresh=1; //Ϊ�¼���������Ļˢ�·���
										break;
								}
								if (!enterflag) 
								{
										// ��ʾ���벻����
										OLED_ShowString(0, 36, "Incomplete Input", 16);
										OLED_Refresh_Gram();
										memset(dinput_buffer,0,sizeof(dinput_buffer));  // ɾ����
										//USART1_RX_STA = 0;  //ͬ�� ��Timer����ʵ������
										enterflag=1;
										delay_ms(1000);
										OLED_ShowString(0, 36, "                ", 16);
										OLED_Refresh_Gram();
										continue;  // �������δ�ѭ��
								}
								if (USART1_RX_STA&0x8000)  // �������
								{									
										char *p;         //forѭ����ֵ
										char *ptr;       //ָ�򻺴��ָ��,����Ԥ��������
									  char *allptr;    //ɾ��ǰ����ַ��Ĵ�����ָ�򻺴��ָ�� 
									
										int ptrsum=0;		 //��λ��ʶ������ʵ����ʡ��						
										uint8_t len = USART1_RX_STA&0x3FFF;  // �������ݳ���
										validflag=1;  //
										
										if (len>0&&len<sizeof(dinput_buffer)-1) 
										{
												// �����յ������ݸ��Ƶ�������
												strncpy(dinput_buffer,(char *)USART1_RX_BUF,len);
												dinput_buffer[len]='\0';  // ����ַ���������
											
												/*������fre������ͬ*/
											
												
											
												//��ʵ�� ͬ�� ע�͵�
											  /*
												for(p=dinput_buffer;*p!='\0';p++)
												{
														if(*p =='\r'||*p =='\n')
														{
																enterflag=1;
																*p ='\0';
																//USART1_RX_STA = 0;  // ������ձ�־																
															  break;
														}
												}
												*/
												/*******************************************************
												�˳�����ӵĲ��֣�duty����Ϊ0���atoi���쳣���ز��ᱻ����
												ʹ��ָ��Ԥ����������
												********************************************************/
												
												/***********************************************************
												ɾ���ո�Ĵ��� ��Ҫ��Ϊ�˴����������пո�ͻس�ʱ��Ҳ��������ʹ��
												������ӣ����ڱ��봫�봿���� ����ᱻ�·��������ж��߼�ʶ��Ϊ��Ч
												���ʻ�����Ϊduty����Ϊ0����atoi��һЩ�쳣���ܱ�����
												����atoi�Դ��������ַ��Ĳ���
												**********************************************************/
												ptr=dinput_buffer;    //δɾ���ո��ָ���ʼ��ַ
												allptr=dinput_buffer; //ɾ���ո���ָ���ʼ��ַ��ͬ
												
												while (*ptr!='\0') 
												{
														if (*ptr!='\r'&&*ptr!='\n'&&*ptr!=' ') 
														{
																*allptr=*ptr;  // ֻ������Ч�ַ�
																allptr++;			 //������һλ
															  ptrsum++;      //��λ��ʶ��
														}
														ptr++;             //������һλ
												}
												*allptr='\0';          //���ϴ�����ɺ��ָ��ָ�����ݵ���ֹ��
												allptr=allptr-ptrsum;  //������ɺ�ָ���λ��Ҳ��ֱ�Ӹ���ʼ��ַ
												
												/*********************************************************
												�·���Ϊ���ڼ����Ϸ�ɾ���ո�Ĵ����
												��ֹ����Ϊ���ַ��������������ּ�⣬atoiʶ�𲻵��ַ�
												ֱ�ӷ���0���´���ʹ�õ�
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
												allptr=dinput_buffer;    //ָ�븴λ
												/*********************************************************
												���˵��߼�Ϊ��Ϊ�˱��⴮�ڴ������� ����ɾ���ո�
												Ȼ���ж��Ƿ����֣����Ŵ��ݸ�atoi�������Ǵ�����atoi����0�����
												**********************************************************/
												
												if (validflag) 
												{
														sdu=atoi(allptr);  // ת��Ϊ����
														if (sdu>=0&&sdu<=100) 
														{
																dinput_done=1;  // �������
																scrupdateflag=1; //ͬ�ϣ�������������
														}
														else 
														{
																validflag=0;
														}
												}

												if (!validflag) 
												{
														// ��ʾ����Ƿ�
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
												// ���ݳ�����Ч
												OLED_ShowString(0, 36, "Invalid Length", 16);
												OLED_Refresh_Gram();
												memset(dinput_buffer, 0, sizeof(dinput_buffer));
												USART1_RX_STA=0;  // ������ձ�־
												delay_ms(1000);
												OLED_ShowString(0, 36, "               ", 16);
												OLED_Refresh_Gram();
										}
										memset(USART1_RX_BUF, 0, sizeof(USART1_RX_BUF));  //�������ɾ����
								}
						}
						//ͬ�� ������ɽ�����һ���˵�
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
										  PWM2_CH1_set(sfreq,sdu);//���ϵ�������setһ��
									}
						}
						//�˻�����״̬
						if(!key_status_4())
							{	
									dinput_done=0;
									USART1_RX_STA = 0;
									memset(finput_buffer, 0, sizeof(dinput_buffer));  //�˳�ɾ���� 
									OLED_ShowString(0,42,"               ",16);
									OLED_Refresh_Gram();
									scrupdateflag=1;
							}
				}
				//���ڵ���������˵�
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
								backrefresh=1;    //Ϊ�¼���������Ļˢ�·���
				
						}
						
				}
				//ϵͳ�ļ�Ҫ˵���˵�
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
								if(!key_status_4()) //�˻���һ���˵�
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
								if(!key_status_4()) //�˻���һ���˵�
								{
										OLED_Clear();
										delay_ms(50);
										insenflag=0;
										scrupdateflag=1;
								}
						}
				}
				//��������ϵͳ�汾��
				//GPL��ʾ˵��
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
