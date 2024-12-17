#include "oled.h"
#include "iic.h"
#include "oledfont.h" 


//OLED���Դ�
//��Ÿ�ʽ����.
//[0]0 1 2 3 ... 127	
//[1]0 1 2 3 ... 127	
//[2]0 1 2 3 ... 127	
//[3]0 1 2 3 ... 127	
//[4]0 1 2 3 ... 127	
//[5]0 1 2 3 ... 127	
//[6]0 1 2 3 ... 127	
//[7]0 1 2 3 ... 127 		   
u8 OLED_GRAM[128][8];


//IICд����
void write_iic_cmd(u8 cmd)
{
	IIC_Start();
	IIC_Send_Byte(0x78);//Slave address,SA0=0
	IIC_Wait_Ack();	
	IIC_Send_Byte(0x00);//write command
	IIC_Wait_Ack();	
	IIC_Send_Byte(cmd); 
	IIC_Wait_Ack();	
	IIC_Stop();
}

//IICд����
void write_iic_data(u8 dat)
{
	IIC_Start();
	IIC_Send_Byte(0x78);//Slave address,SA0=0
	IIC_Wait_Ack();	
	IIC_Send_Byte(0x40);//write data
	IIC_Wait_Ack();	
	IIC_Send_Byte(dat); 
	IIC_Wait_Ack();	
	IIC_Stop();
}

//��SSD1306д��һ���ֽڡ�
//dat:Ҫд�������/����
//cmd:����/�����־ 0,��ʾ����;1,��ʾ����;
void OLED_WR_Byte(u8 dat,u8 cmd)
{
	if(cmd)
		write_iic_data(dat);
	else
		write_iic_cmd(dat);
}

//�����Դ浽LCD		 
void OLED_Refresh_Gram(void)
{
	int i,n;		    
	for(i=7;i>=0;i--) //�޸�ˢ�·��� 
	{  
		OLED_WR_Byte (0xb0+i,OLED_CMD);    //����ҳ��ַ��0~7��
		OLED_WR_Byte (0x00,OLED_CMD);      //������ʾλ�á��е͵�ַ
		OLED_WR_Byte (0x10,OLED_CMD);      //������ʾλ�á��иߵ�ַ   
		for(n=0;n<128;n++)OLED_WR_Byte(OLED_GRAM[n][i],OLED_DATA); 
	}   
}

void OLED_Set_Pos(unsigned char x, unsigned char y) 
{ 
	OLED_WR_Byte(0xb0+y,OLED_CMD);
	OLED_WR_Byte(((x&0xf0)>>4)|0x10,OLED_CMD);
	OLED_WR_Byte((x&0x0f)|0x01,OLED_CMD); 
}   	  
//����OLED��ʾ    
void OLED_Display_On(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC����
	OLED_WR_Byte(0X14,OLED_CMD);  //DCDC ON
	OLED_WR_Byte(0XAF,OLED_CMD);  //DISPLAY ON
}
//�ر�OLED��ʾ     
void OLED_Display_Off(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC����
	OLED_WR_Byte(0X10,OLED_CMD);  //DCDC OFF
	OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
}
		   			 
//��������,������,������Ļ�Ǻ�ɫ��!��û����һ��!!!	  
void OLED_Clear(void)  
{  
	u8 i,n;		    
	for(i=0;i<8;i++)  
	{    
		for(n=0;n<128;n++)
		{
			OLED_GRAM[n][i]=0;
		}
	}
	OLED_Refresh_Gram();//������ʾ 
}

//m^n����
u32 oled_pow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}

//���� 
//x:0~127
//y:0~63
//t:1 ��� 0,���				   
void OLED_DrawPoint(u8 x,u8 y,u8 t)
{
	u8 pos,bx,temp=0;
	if(x>127||y>63)return;//������Χ��.
	pos=7-y/8;
	bx=y%8;
	temp=1<<(7-bx);
	if(t)OLED_GRAM[x][pos]|=temp;
	else OLED_GRAM[x][pos]&=~temp;		    
}

//x1,y1,x2,y2 �������ĶԽ�����
//ȷ��x1<=x2;y1<=y2 0<=x1<=127 0<=y1<=63	 	 
//dot:0,���;1,���	  
void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot)  
{  
	u8 x,y;  
	for(x=x1;x<=x2;x++)
	{
		for(y=y1;y<=y2;y++)
		{ 	
			OLED_DrawPoint(x,y,dot);	
		}
	}													    
}

//������
//x0,y0:���ε����Ͻ�����
//width,height:���εĳߴ�
//color:��ɫ
void OLED_Fill_rectangle(u8 x0,u8 y0,u8 width,u8 height,u8 color)
{	  							   
	if(width==0||height==0)return;//�Ƿ�.	 
	OLED_Fill(x0,y0,x0+width-1,y0+height-1,color);	   	   
}

//����
//x1,y1:�������
//x2,y2:�յ�����  
void OLED_DrawLine(u8 x1, u8 y1, u8 x2, u8 y2,u8 color)
{
	u8 t; 
	char xerr=0,yerr=0,delta_x,delta_y,distance; 
	char incx,incy,uRow,uCol; 
	delta_x=x2-x1; //������������ 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //���õ������� 
	else if(delta_x==0)incx=0;//��ֱ�� 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//ˮƽ�� 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//������� 
	{  
		OLED_DrawPoint(uRow,uCol,color);//���� 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}

//����㺯��
//x0,y0:����
//color:��ɫ
//��(x0,y0)Ϊ����,��һ��9����Ĵ��
void OLED_Draw_Bigpoint(u8 x0,u8 y0,u8 color)
{
	u8 i,j;
	u8 x,y;				    
	if(x0>=1)x=x0-1;
	else x=x0;
	if(y0>=1)y=y0-1;
	else y=y0;
	for(i=y;i<y0+2;i++)
	{
		for(j=x;j<x0+2;j++)OLED_DrawPoint(j,i,color);
	}  						   
}

//����ֱ��
//x0,y0:����
//len:�߳���
//color:��ɫ
void OLED_Draw_vline(u8 x0,u8 y0,u8 len,u8 color)
{
	if((len==0)||(x0>Max_Column-1)||(y0>Max_Row-1))return;
	OLED_Fill(x0,y0,x0,y0+len-1,color);	
}

//��ˮƽ��
//x0,y0:����
//len:�߳���
//color:��ɫ
void OLED_Draw_hline(u8 x0,u8 y0,u8 len,u8 color)
{
	if((len==0)||(x0>Max_Column-1)||(y0>Max_Row-1))return;
	OLED_Fill(x0,y0,x0+len-1,y0,color);	
}

//��ʵ��Բ
//x0,y0:����
//r�뾶
//color:��ɫ
void OLED_Fill_circle(u8 x0,u8 y0,u8 r,u8 color)
{											  
	u16 i;
	u16 imax = ((u32)r*707)/1000+1;
	u16 sqmax = (u32)r*(u32)r+(u32)r/2;
	u16 x=r;
	OLED_Draw_hline(x0-r,y0,2*r,color);
	for (i=1;i<=imax;i++) 
	{
		if ((i*i+x*x)>sqmax) 
		{
			// draw lines from outside  
			if (x>imax) 
			{
				OLED_Draw_hline (x0-i+1,y0+x,2*(i-1),color);
				OLED_Draw_hline (x0-i+1,y0-x,2*(i-1),color);
			}
			x--;
		}
		// draw lines from inside (center)  
		OLED_Draw_hline(x0-x,y0+i,2*x,color);
		OLED_Draw_hline(x0-x,y0-i,2*x,color);
	}
}

//����Բ
//x0,y0:����
//rx:x����뾶
//ry:y����뾶
void OLED_Draw_ellipse(u8 x0, u8 y0, u8 rx, u8 ry,u8 color) 
{
	u16 OutConst, Sum, SumY;
	u8 x,y;
	u8 xOld;
	u8 _rx = rx;
	u8 _ry = ry;
	if(rx>x0||ry>y0)return;//�Ƿ�.
	OutConst = _rx*_rx*_ry*_ry+(_rx*_rx*_ry>>1);  // Constant as explaint above 
	// To compensate for rounding  
	xOld = x = rx;
	for (y=0; y<=ry; y++) 
	{
		if (y==ry)x=0; 
		else 
		{
			SumY =((u16)(rx*rx))*((u32)(y*y)); // Does not change in loop  
			while(Sum = SumY + ((u16)(ry*ry))*((u32)(x*x)),(x>0) && (Sum>OutConst)) x--;
		}
		// Since we draw lines, we can not draw on the first iteration		    
		if (y) 
		{
			OLED_DrawLine(x0-xOld,y0-y+1,x0-x,y0-y,color);
			OLED_DrawLine(x0-xOld,y0+y-1,x0-x,y0+y,color);
			OLED_DrawLine(x0+xOld,y0-y+1,x0+x,y0-y,color);
			OLED_DrawLine(x0+xOld,y0+y-1,x0+x,y0+y,color);
		}
		xOld = x;
	}
}

//������	  
//(x1,y1),(x2,y2):���εĶԽ�����
void OLED_DrawRectangle(u8 x1, u8 y1, u8 x2, u8 y2,u8 color)
{
	OLED_DrawLine(x1,y1,x2,y1,color);
	OLED_DrawLine(x1,y1,x1,y2,color);
	OLED_DrawLine(x1,y2,x2,y2,color);
	OLED_DrawLine(x2,y1,x2,y2,color);
}

//��ָ��λ�û�һ��ָ����С��Բ
//(x,y):���ĵ�
//r    :�뾶
void OLED_Draw_Circle(u8 x0,u8 y0,u8 r,u8 color)
{
	char a,b;
	char di;
	a=0;b=r;	  
	di=3-(r<<1);             //�ж��¸���λ�õı�־
	while(a<=b)
	{
		OLED_DrawPoint(x0+a,y0-b,color);             //5
 		OLED_DrawPoint(x0+b,y0-a,color);             //0           
		OLED_DrawPoint(x0+b,y0+a,color);             //4               
		OLED_DrawPoint(x0+a,y0+b,color);             //6 
		OLED_DrawPoint(x0-a,y0+b,color);             //1       
 		OLED_DrawPoint(x0-b,y0+a,color);             
		OLED_DrawPoint(x0-a,y0-b,color);             //2             
  		OLED_DrawPoint(x0-b,y0-a,color);             //7     	         
		a++;
		//ʹ��Bresenham�㷨��Բ     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 						    
	}
}  

//��ָ��λ����ʾһ���ַ�,���������ַ�
//x:0~127
//y:0~63
//mode:0,������ʾ;1,������ʾ				 
//size:ѡ������ 12/16/24
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size,u8 mode)
{      			    
	u8 temp,t,t1;
	u8 y0=y;
	u8 csize=(size/8+((size%8)?1:0))*(size/2);		//�õ�����һ���ַ���Ӧ������ռ���ֽ���
	chr=chr-' ';//�õ�ƫ�ƺ��ֵ		 
    for(t=0;t<csize;t++)
    {   
		if(size==12)temp=ascii_1206[chr][t]; 	 	//����1206����
		else if(size==16)temp=ascii_1608[chr][t];	//����1608����
		else if(size==24)temp=ascii_2412[chr][t];	//����2412����
		else return;								//û�е��ֿ�
        for(t1=0;t1<8;t1++)
		{
			if(temp&0x80)OLED_DrawPoint(x,y,mode);
			else OLED_DrawPoint(x,y,!mode);
			temp<<=1;
			y++;
			if((y-y0)==size)
			{
				y=y0;
				x++;
				break;
			}
		}  	 
    }          
}

//��ʾ2������
//x,y :�������	 
//len :���ֵ�λ��
//size:�����С
//num:��ֵ(0~4294967295);	 		  
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size)
{         	
	u8 t,temp;
	u8 enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/oled_pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				OLED_ShowChar(x+(size/2)*t,y,' ',size,1);
				continue;
			}else enshow=1; 
		 	 
		}
	 	OLED_ShowChar(x+(size/2)*t,y,temp+'0',size,1); 
	}
} 
//������ʾ2������
//x,y :�������	 
//len :���ֵ�λ��
//size:�����С
//num:��ֵ(0~4294967295);	 		  
void OLED_ShowNum0(u8 x,u8 y,u32 num,u8 len,u8 size)
{         	
	u8 t,temp;
	u8 enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/oled_pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				OLED_ShowChar(x+(size/2)*t,y,' ',size,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	OLED_ShowChar(x+(size/2)*t,y,temp+'0',size,0); 
	}
} 
// xfl,yfl,ffl,j,i//j����λ��  iС��λ��
void OLED_ShowFloat(u16 xf1,u16 yf1,double ff1,u8 j,u8 i)  
{
	u16 nf=0;
	 if(ff1<0){
			
			ff1=-ff1;
		 OLED_ShowChar(xf1,yf1,'-',16,1);
		 xf1+=8;
		}
		nf=ff1;
		OLED_ShowNum(xf1,yf1,nf,j,16);
		xf1+=(j*8);
		OLED_ShowChar(xf1,yf1,'.',16,1);
		for (;i>0;i--)
		{
			xf1+=8;
			ff1=(ff1-nf)*10;
			nf=ff1;
		  OLED_ShowNum(xf1,yf1,nf,1,16);
		}
			
}

//��ʾ�ַ���
//x,y:�������  
//size:�����С 
//*p:�ַ�����ʼ��ַ 
void OLED_ShowString(u8 x,u8 y,const u8 *p,u8 size)
{	
    while((*p<='~')&&(*p>=' '))//�ж��ǲ��ǷǷ��ַ�!
    {       
        if(x>(X_WIDTH-(size/2))){x=0;y+=size;}
        if(y>(Y_WIDTH-size)){y=x=0;OLED_Clear();}
        OLED_ShowChar(x,y,*p,size,1);	 
        x+=size/2;
        p++;
    }  	
}

//��ʾ�ַ���(��ɫ)
//x,y:�������  
//size:�����С 
//*p:�ַ�����ʼ��ַ 
void OLED_ShowString0(u8 x,u8 y,const u8 *p,u8 size)
{	
    while((*p<='~')&&(*p>=' '))//�ж��ǲ��ǷǷ��ַ�!
    {       
        if(x>(X_WIDTH-(size/2))){x=0;y+=size;}
        if(y>(Y_WIDTH-size)){y=x=0;OLED_Clear();}
        OLED_ShowChar(x,y,*p,size,0);	 
        x+=size/2;
        p++;
    }  	
}
//��ʾ����
//x,y:�������  
//pos:����λ�ú�����ʾ
//size:�����С 
//mode:0,������ʾ;1,������ʾ
void OLED_ShowFontHZ(u8 x,u8 y,u8 pos,u8 size,u8 mode)
{
	u8 temp,t,t1;
	u8 y0=y;  
	u8 csize=(size/8+((size%8)?1:0))*(size);//�õ�����һ���ַ���Ӧ������ռ���ֽ���	 
	if(size!=12&&size!=16&&size!=24)return;	//��֧�ֵ�size
	 
	for(t=0;t<csize;t++)
	{   												   
		if(size==12)temp=FontHzk[pos][t]; 	 	//����1206����
		else if(size==16)temp=FontHzk[pos][t];	//����1608����
		else if(size==24)temp=FontHzk[pos][t];	//����2412����
		else return;								//û�е��ֿ�
        for(t1=0;t1<8;t1++)
		{
			if(temp&0x80)OLED_DrawPoint(x,y,mode);
			else OLED_DrawPoint(x,y,!mode);
			temp<<=1;
			y++;
			if((y-y0)==size)
			{
				y=y0;
				x++;
				break;
			}
		}  	 
	}  	
} 

//��ʾBMPͼƬ128��64
//��ʼ������(x,y),x�ķ�Χ0��127��yΪҳ�ķ�Χ0��7
void OLED_DrawBMP(u8 x0, u8 y0,u8 x1, u8 y1,u8 BMP[])
{ 	
 	u16 j=0;
 	u8 x,y;
  
  	if(y1%8==0)y=y1/8;      
  	else y=y1/8+1;
	for(y=y0;y<y1;y++)
	{
		OLED_Set_Pos(x0,y);
    	for(x=x0;x<x1;x++)
	    {      
	    	OLED_WR_Byte(BMP[j++],OLED_DATA);	    	
	    }
	}
}

//OLED GPIO��ʼ��
//OLED��ʼ��
void OLED_Init(void)
{	  	
	IIC_Init();
	
	OLED_WR_Byte(0xAE,OLED_CMD); //�ر���ʾ
	OLED_WR_Byte(0xD5,OLED_CMD); //����ʱ�ӷ�Ƶ����,��Ƶ��
	OLED_WR_Byte(80,OLED_CMD);   //[3:0],��Ƶ����;[7:4],��Ƶ��
	OLED_WR_Byte(0xA8,OLED_CMD); //��������·��
	OLED_WR_Byte(0X3F,OLED_CMD); //Ĭ��0X3F(1/64) 
	OLED_WR_Byte(0xD3,OLED_CMD); //������ʾƫ��
	OLED_WR_Byte(0X00,OLED_CMD); //Ĭ��Ϊ0

	OLED_WR_Byte(0x40,OLED_CMD); //������ʾ��ʼ�� [5:0],����.
													    
	OLED_WR_Byte(0x8D,OLED_CMD); //��ɱ�����
	OLED_WR_Byte(0x14,OLED_CMD); //bit2������/�ر�
	OLED_WR_Byte(0x20,OLED_CMD); //�����ڴ��ַģʽ
	OLED_WR_Byte(0x02,OLED_CMD); //[1:0],00���е�ַģʽ;01���е�ַģʽ;10,ҳ��ַģʽ;Ĭ��10;
	OLED_WR_Byte(0xA1,OLED_CMD); //���ض�������,bit0:0,0->0;1,0->127;
	OLED_WR_Byte(0xC0,OLED_CMD); //����COMɨ�跽��;bit3:0,��ͨģʽ;1,�ض���ģʽ COM[N-1]->COM0;N:����·��
	OLED_WR_Byte(0xDA,OLED_CMD); //����COMӲ����������
	OLED_WR_Byte(0x12,OLED_CMD); //[5:4]����
		 
	OLED_WR_Byte(0x81,OLED_CMD); //�Աȶ�����
	OLED_WR_Byte(0xEF,OLED_CMD); //1~255;Ĭ��0X7F (��������,Խ��Խ��)
	OLED_WR_Byte(0xD9,OLED_CMD); //����Ԥ�������
	OLED_WR_Byte(0xf1,OLED_CMD); //[3:0],PHASE 1;[7:4],PHASE 2;
	OLED_WR_Byte(0xDB,OLED_CMD); //����VCOMH ��ѹ����
	OLED_WR_Byte(0x30,OLED_CMD); //[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

	OLED_WR_Byte(0xA4,OLED_CMD); //ȫ����ʾ����;bit0:1,����;0,�ر�;(����/����)
	OLED_WR_Byte(0xA6,OLED_CMD); //������ʾ��ʽ;bit0:1,������ʾ;0,������ʾ	    						   
	OLED_WR_Byte(0xAF,OLED_CMD); //������ʾ	

	OLED_Clear();
}
void update_screen() 
{
    if (scrupdateflag) 
		{
        OLED_Refresh_Gram();
        scrupdateflag=0; // �����־λ
    }
}