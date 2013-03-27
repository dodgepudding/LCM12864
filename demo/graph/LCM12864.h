/************************************************************************************
**                            YM12864����ģ��
**                     (c) Copyright 2007-2008, Dodge
**                          All Rights Reserved
**
**                              V1.00.10
---------------------------------�ļ���Ϣ--------------------------------------------
**��  ��: LCM12864.c  ����ʵ��ģ��
**        LCM12864.h  �������ö��弰ͷ�ļ�����
**��д��: Guangcheng Pu
**��  ����YM12864��ʾ����ģ��
          �궨��˵����
          __USE_SPI_MODE__ 		��ͷ�ļ�����˺�������ʹ�ô��з�ʽͨ��
          __USE_PARALLEL_MODE__ ��ͷ�ļ�����˺�����ʹ�ò��з�ʽͨ��
          LCD12864_CMD_xxxx 	LCD����ָ���� 
          LCD12864_DATA_xxxx    LCD��RAM���ݲ��� 
          LCD12864_EXTCMD_xxxx  LCD��չָ���� 
          ����ͷ�ļ���������ţ�
          RST���͵�ƽ��λ���ţ�LCD_PS������ѡ�����ţ�1������ģʽ��0������ģʽ
          ����ģʽ��
          LCD_CLK(SCLK):ʱ������ţ�LCD_DATA(SID):������������ţ�LCD_CS(RS)��Ƭѡ��Ч
          ����ģʽ��
		  LCD_CS: Ƭѡ��Ч��LCD_RW����дѡ��
		  LCD_RS������ģʽָ��ģʽ�л���LCDData��8λ�������ݣ�
		  ʹ�÷�����
		  1���޸�LCM12864.Hͷ�ļ���Ӧ���� ��
		  2������Һ����ʼ������LCDInit()��������ģʽ��Ҫ����֮���һ����ʱ;
		  3�����и����ܺ������֣���ͼ�ȣ����磺
		  PutStr(0,0,"��ӭʹ��PDODGE��Һ������ģ��")��//��ʾ�ڵ�0��0������ַ���
		  DrawBMP(buf);//��ͼ�λ������ĵ������������LCD
*************************************************************************************/
#define __USE_SPI_MODE__   //�˴����޸ģ�ѡ��__USE_SPI_MODE__��__USE_PARALLEL_MODE__
#include <ADuC845.h>			//���޸�Ϊ��ĵ�Ƭ��ͷ�ļ�

//sbit LCD_PS=P2^4;		//����ѡ������ʹ�ô���ʱ��ͬʱ������һ��
//#define P_S 1			//�˴���ѡ���޸ģ�1Ϊ����ģʽ��0Ϊ����ģʽ
#ifdef P_S
   #if (P_S==1)
   	#define __USE_PARALLEL_MODE__
   #else
   	#define __USE_SPI_MODE__
   #endif
#endif

#ifdef __USE_SPI_MODE__
sbit LCD_CS=P2^3;
sbit LCD_DATA=P2^1;
sbit LCD_CLK=P2^0;
#endif

#ifdef __USE_PARALLEL_MODE__
#define	LCDData	P0
sbit LCD_CS=P2^0;
sbit LCD_RW=P2^1;
sbit LCD_RS=P2^3;
#endif

/**********************
����ģʽĬ��ֵ
#define DELAY4MS   150	   
#define DELAY8US   15
����ģʽĬ��ֵ
#define DELAY4MS   100	   
#define DELAY8US   1
����ַ���ʾ�������Ļ���
�ڳ�ʼ�����һ����ʱ��
***********************/
#define DELAY4MS   150	   //��������Ƶ���ʵ�������ʱֵ��ʹLCD��ʾ����
#define DELAY8US   15
/*********************��ʼ�����ܺ���*****************************/
void LCDInit();	    //��ʼ��Һ��
void ClearLCD();	//���LCD��ʾ
void ClearRAM();	//���RAM����
void Display_on();	//��������ʾ
void Display_off();	//�ر�������ʾ
void Draw_on();		//�򿪻�ͼģʽ
void Draw_off();	//�رջ�ͼģʽ
/*************************���ܺ���*******************************/
void Cursor_on();	//�α���˸��
void Cursor_off();	//�α���˸��
void Scroll_left(); //������������
void Scroll_right();//������������
void Line_Inverse(unsigned char line); //��line�з���
void Sleep(); 		//����˯��ģʽ
void Wakeup(); 		//��˯��ģʽ����
void PutChar(unsigned char row,unsigned char col,unsigned char cc);		//��row��,col����ʾһ���ַ�
void PutStr(unsigned char row,unsigned char col,unsigned char *puts);	//��row��,col�п�ʼ��ʾһ�ַ���
void DrawBMP(unsigned char *puts);				   						//��puts�����ʾ��ͼ��
void Point(unsigned char x,unsigned char y,unsigned char *buf);			//��(x,y)λ�û�һ�㣬(0,0)���������½�
void Point2(unsigned char x,unsigned char y);							//�����㣬����ԭͼ
void Line(unsigned char sx,unsigned char sy,unsigned char ex,unsigned char ey,unsigned char *buf); //�ӣ�sx,sy)��(ex,ey)�������ߣ�ex>sx
