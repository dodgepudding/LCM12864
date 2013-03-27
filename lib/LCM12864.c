/************************************************************************************
**                            YM12864����ģ��
**                     (c) Copyright 2007-2013, Dodge
**                          All Rights Reserved
**					https://github.com/dodgepudding/LCM2864
**
**                              V1.00.14
*************************************************************************************/
#include "LCM12864.h"

#ifndef __LCD12864_MODEL__
#define __LCD12864_MODEL__
	
#define LCD12864_CMD_CLEAR 		0x01 //�����ʾ����DDRAM������20H���������趨DDRAM�ĵ�ַ��������AC������00H��,4.6ms
#define LCD12864_CMD_RSTDDRAM 	0x02 //��ַ��λ���趨DDRAM�ĵ�ַ��������AC������00H�������ҽ��α��Ƶ���ͷԭ��λ�ã����ָ����ı�DDRAM�����ݣ�4.6ms
#define LCD12864_CMD_SETCUR(id,s)  (0x04|id<<1|s) //������趨��ָ�������ϵĶ�ȡ��д��ʱ���趨�α��ƶ�����ָ����ʾ����λ��ID=1���α������ƣ�ID=0���α������ƣ�S=1������������/���ƶ���72us			  
#define LCD12864_CMD_DISP(d,c,b)  (0x08|d<<2|c<<1|b) //��ʾ״̬��/�أ�D=1��������ʾON��C=1���αꡰ����ON��B=1���α�λ�á�����ON��72us
#define LCD12864_CMD_CURXY(sc,rl) (0x10|sc<<3|rl<<2) //�α����ʾ��λ���ƣ��趨�α���ƶ�����ʾ����λ����λԪ�����ָ����ı�DDRAM������
#define LCD12864_CMD_MODE(re)	(0x30|re<<2) //ģʽ�趨��RE=1������ָ�������RE=0������ָ�����
#define LCD12864_CMD_MODE_NORMAL  0
#define LCD12864_CMD_NORMAL 0x30
#define LCD12864_CMD_MODE_EXTEND  1
#define LCD12864_CMD_EXTEND 0x34
#define LCD12864_CMD_SETCGRAM(ac) (0x40|(ac&0x3f))   //�趨CGRAM��ַ���趨CGRAM��ַ����ַ��������AC��
#define LCD12864_CMD_SETDDRAM(ac) (0x80|(ac&0x7f))	 //�趨DDRAM ��ַ���趨DDRAM��ַ����ַ��������AC��
#define LCD12864_CMD_BUSY(in,bf)    bf=(in&0x80)>>7  //��ȡæµ��־��BF����inΪ��������������
#define LCD12864_CMD_READAC(in,ac)     ac=(in&0x7f)  //������ַ��������AC����ֵ��inΪ��������������
#define LCD12864_DATA_WRITE(r)		r               //д�����ϵ��ڲ���RAM��DDRAM/CGRAM/IRAM/GDRAM��
#define LCD12864_DATA_READ(r)		r				//���ڲ�RAM��ȡ���ϣ�DDRAM/CGRAM/IRAM/GDRAM��
#define LCD12864_EXTCMD_READY   0x01 				//����ģʽ,��DDRAM������20H���������趨DDRAM�ĵ�ַ��������AC������00H��
#define LCD12864_EXTCMD_SCROLL(sr)  (0x02|sr)		//����ַ��IRAM��ַѡ��SR=1���������봹ֱ����ַ��SR=0����������IRAM��ַ
#define LCD12864_EXTCMD_INVERSE(r) (0x04|(r&0x03))  //����ѡ��ѡ��4���е���һ����������ʾ�����ɾ����������72us
#define LCD12864_EXTCMD_SLEEP(sl)	(0x08|sl<<2)	//˯��ģʽ��SL=1������˯��ģʽ��SL=0������˯��ģʽ
#define LCD12864_EXTCMD_MODE(re,g)   (0x30|re<<2|g<<1) //���书���趨��RE=1������ָ�������RE=0������ָ�������G=1 ����ͼ��ʾON��G=0 ����ͼ��ʾOFF	
#define LCD12864_EXTCMD_SCRARRD(ac)	 (0x40|ac&0x3f) 	//�趨IRAM��ַ�����ַ��SR=1��AC5��AC0Ϊ��ֱ����ַ��SR=0��AC3��AC0ΪICON IRAM��ַ
#define LCD12864_EXTCMD_CGADDR(ac)	 (0x80|ac&0x7f)		//SR=1��AC5��AC0Ϊ��ֱ����ַSR=0��AC3��AC0ΪICON IRAM��ַ


	
#define ShortDelay(x)\
{\
unsigned char t =x;\
while ( --t != 0 );\
}

#define LongDelay(x)\
{\
unsigned int t =x;\
while ( --t != 0 );\
}

unsigned char code LCD_ADDR[4][8]={
{0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87},      //��һ�к���λ��
{0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97},      //�ڶ��к���λ��
{0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f},      //�����к���λ��
{0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f},      //�����к���λ��
};

/*****************************����ģʽ������д����*************************************/
#ifdef __USE_SPI_MODE__
void SPI_Write(uchar dat) //дSPIʱ��
{
uchar t = 8;
do
{
LCD_DATA = (bit)(dat & 0x80);
dat <<= 1;
LCD_CLK = 1;
//ShortDelay(2);
LCD_CLK = 0;
//ShortDelay(2);
} while ( --t != 0 );
}

uchar SPI_Read() //��SPIʱ��
{
uchar dat;
uchar t = 8;
LCD_DATA = 1; //��ȡ����֮ǰDIO ����Ҫ��1 ���л�������״̬
do
{
LCD_CLK = 1;
ShortDelay(2);
dat <<= 1;
if (LCD_DATA) dat++;
LCD_CLK = 0;
ShortDelay(2);
} while ( --t != 0 );
return dat;
}

void SPIWR(uchar wdata,uchar rs)   //дSPI��������, RS=0����ָ��, RS=1��������
{
	SPI_Write(0xf8+(rs<<1));		//���ݸ�ʽ��11111 | RW | RS | 0 |
	SPI_Write(wdata&0xf0);			//д�������ָ�4λ�����ݸ�ʽ�� D7-D4| 0000 |
	SPI_Write((wdata<<4)&0xf0);		//д�������ֵ�4λ�����ݸ�ʽ:  D3-D0| 0000 |
}

uchar SPIRD(uchar rs)   //��SPI��������, RS=1����ָ��, RS=1��������
{
uchar rdata;
	SPI_Write(0xf8+(1<<2)+(rs<<1));		//���ݸ�ʽ��11111 | RW | RS | 0 |
	rdata=SPI_Read()&0xf0;			//��ȡ�����ָ�4λ�����ݸ�ʽ�� D7-D4| 0000 |
	rdata=rdata|((SPI_Read()&0xf0)>>4);		//��ȡ�����ֵ�4λ�����ݸ�ʽ:  D3-D0| 0000 |
return rdata;
}

void SendCMD(uchar CMD)
{
SPIWR(CMD,0);
LongDelay(DELAY8US);//�������ʱ����ʱ�������ܻ�������룬�ʵ����ڴ�ֵ	
}

void SendData(uchar Data)
{
SPIWR(Data,1);
LongDelay(DELAY8US/2);
}

uchar ReadData()
{
return  SPIRD(1);
}

#else
	
void SendCMD(uchar cmd)//дָ��
{
    LCD_CS=0;
	LCD_RW=0;
    LCD_RS=0;
    ShortDelay(DELAY8US);
	LCD_CS=1;
    LCDData=cmd;
    ShortDelay(DELAY8US);
    LCD_CS=0;   
}
void SendData(uchar dat)//д����
{
    LCD_CS=0;
	LCD_RW=0;
	LCD_RS=1;
	ShortDelay(DELAY8US);
	LCD_CS=1;
	LCDData=dat;
    ShortDelay(DELAY8US);
    LCD_CS=0; 

}

uchar ReadData()//д����
{
uchar dat;
    LCD_CS=0;
	LCD_RW=1;
	LCD_RS=1;
	ShortDelay(DELAY8US);
	LCD_CS=1;
	dat=LCDData;
    ShortDelay(DELAY8US);
    LCD_CS=0; 
return dat;
}

uchar ReadBusy()
{
uchar dat;
    LCD_CS=0;
	LCD_RW=1;
	LCD_RS=0;
	ShortDelay(DELAY8US);
	LCD_CS=1;
	dat=LCDData;
    ShortDelay(DELAY8US);
    LCD_CS=0; 
return (dat>>7);
}
	
void SetAddr(uchar addr)
{
 SendCMD(LCD12864_CMD_SETDDRAM(addr));
}

uchar ReadChar(uchar addr)
{
uchar dat;
 SetAddr(addr);
 while (ReadBusy());
 dat=ReadData();
 dat=ReadData();
return dat;
}
#endif


void LCDInit()
{
#ifdef P_S
LCD_PS=P_S;
#endif
LCD_CS=0;
ShortDelay(10);
LCD_CS=1;
ShortDelay(10);

     SendCMD(LCD12864_CMD_NORMAL);//�������ã�����ָ�
	 LongDelay(DELAY4MS);
#ifdef __USE_PARALLEL_MODE__ 
LongDelay(DELAY4MS);
#endif
	 SendCMD(LCD12864_CMD_DISP(1,0,0));//0000,1100  ������ʾ���α�off���α�λ��off
     SendCMD(LCD12864_CMD_RSTDDRAM);//0000,0010 DDRAM��ַ��λ
     LongDelay(DELAY4MS);
#ifdef __USE_PARALLEL_MODE__ 
LongDelay(DELAY4MS);
#endif
	 SendCMD(LCD12864_CMD_CLEAR);//0000,0001 ��DDRAM
     LongDelay(DELAY4MS);
	 SendCMD(LCD12864_CMD_SETCUR(1,0));//���趨����ʾ�ַ�/����������λ��DDRAM��ַAC��1//   
#ifdef __USE_PARALLEL_MODE__ 
LongDelay(DELAY4MS);
LongDelay(DELAY4MS);
#endif
}
/*****************************
�����ַ���
*****************************/
void ClearLCD()  //�ڵ�ǰģʽ������Ļ
{
	SendCMD(LCD12864_CMD_CLEAR);
	LongDelay(100*DELAY4MS);
}
void ClearRAM()
{
	SendCMD(LCD12864_CMD_RSTDDRAM);
	LongDelay(100*DELAY4MS);
}
void Display_on()
{
	SendCMD(LCD12864_CMD_DISP(1,0,0));
}
void Display_off()
{
	SendCMD(LCD12864_CMD_DISP(0,0,0));	
}
void Direct_left()
{
	SendCMD(LCD12864_CMD_SETCUR(0,0));
}
void Direct_Right()
{
	SendCMD(LCD12864_CMD_SETCUR(1,0));
}
void Cursor_on()
{
	SendCMD(LCD12864_CMD_DISP(1,0,1));
}
void Cursor_off()
{
	SendCMD(LCD12864_CMD_DISP(1,0,0));
}
void Scroll_left() //���������ƶ���AC=AC
{
	SendCMD(LCD12864_CMD_CURXY(1,0));
}
void Scroll_right()//���������ƶ���AC=AC
{
	SendCMD(LCD12864_CMD_CURXY(1,1));
}
void Cursor_left() //�α������ƶ���AC=AC
{
	SendCMD(LCD12864_CMD_CURXY(0,0));
}
void Cursor_right()//�α������ƶ���AC=AC
{
	SendCMD(LCD12864_CMD_CURXY(0,1));
}
void Line_Inverse(uchar line) //��line����ʾ����
{
	SendCMD(LCD12864_CMD_EXTEND);
	SendCMD(LCD12864_EXTCMD_INVERSE(line));
}

void Sleep() //����˯��ģʽ
{
	SendCMD(LCD12864_CMD_EXTEND);
	SendCMD(LCD12864_EXTCMD_SLEEP(0));
}

void Wakeup() //��˯��ģʽ����
{
	SendCMD(LCD12864_CMD_EXTEND);
	SendCMD(LCD12864_EXTCMD_SLEEP(1));
}

void PutChar(uchar row,uchar col,uchar cc)  //X��ʾ�ڼ�����,Y��ʾ�ڼ���,cc��ʾ�ַ�;
{
   SendCMD(LCD_ADDR[row][col]);
   SendData(cc);
}

void PutStr(uchar row,uchar col,uchar *puts)
{
     SendCMD(LCD12864_CMD_NORMAL);      //8BitMCU,����ָ���
     SendCMD(LCD_ADDR[row][col]);      //��ʼλ��
  while(*puts != '\0')      //�ж��ַ����Ƿ���ʾ���
     {
           if(col==8)            //�жϻ���
           {            //�����ж�,���Զ��ӵ�һ�е�������
                 col=0;
                 row++;
           }
           if(row==4) row=0;      //һ����ʾ��,�ص������Ͻ�
           SendCMD(LCD_ADDR[row][col]);
           SendData(*puts);      //һ������Ҫд����
           puts++;
           if (*puts == '\0') break; else SendData(*puts);
           puts++;
           col++;
     }
	
}

void Draw_on()
{
SendCMD(LCD12864_CMD_EXTEND);	   //8Bit����ָ�,��ʹ��36HҲҪд����
SendCMD(LCD12864_EXTCMD_MODE(1,1));	   //��ͼON,����ָ�����36H���ܿ���ͼ
}

void Draw_off()
{
SendCMD(LCD12864_CMD_EXTEND);	   //8Bit����ָ�,��ʹ��36HҲҪд����
SendCMD(LCD12864_EXTCMD_MODE(1,0));	   //��ͼOFF,����ָ�����36H���ܿ���ͼ
}

void DrawBMP(unsigned char *dc)
{
     unsigned int x=0;
     unsigned char xx,yy;
 	 Draw_off();
     for(yy=0;yy<32;yy++)
		for(xx=0;xx<8;xx++)
		{
			SendCMD(0x80+yy);   //y ��λ
			SendCMD(0x80+xx);//x ��16λ
			SendData(*dc);
			dc++;
			SendData(*dc);
			dc++;
		}
	for(yy=0;yy<32;yy++)
		for(xx=8;xx<16;xx++)
		{
			SendCMD(0x80+yy);   //y ��λ
			SendCMD(0x80+xx);//x ��16λ
			SendData(*dc);
			dc++;
			SendData(*dc);
			dc++;
		}

 
	 Draw_on();
}


void Point(uchar x,uchar y,uchar *buf)
{
uchar xx,yy;
uint16 dd=0;
x=x%128;
y=y%64;
xx=x/16;
yy=63-y;
dd=((uint16)buf[x/16*2+yy*16])<<8;
dd|=buf[x/16*2+1+yy*16];
if(yy>=32){xx=xx+8;yy-=32;}
dd|=0x8000>>(x%16);
buf[x/16*2+yy*16]=dd>>8;
buf[x/16*2+1+yy*16]=dd;
SendCMD(0x80+yy);
SendCMD(0x80+xx);
SendData(dd>>8);
SendData(dd);
}

void Point2(uchar x,uchar y)
{
uchar xx,yy;
uint16 dd=0;
x=x%128;
y=y%64;
xx=x/16;
yy=63-y;
if(yy>=32){xx=xx+8;yy-=32;}
dd|=0x8000>>(x%16);
SendCMD(0x80+yy);
SendCMD(0x80+xx);
SendData(dd>>8);
SendData(dd);
}

void Line(uchar sx,uchar sy,uchar ex,uchar ey,uchar *buf)
{
uchar tx;
int tmp;
	for (tx=sx;tx<ex;tx++)
	{
	tmp=((int)(ey-sy))*((int)(tx-sx))/((int)(ex-sx));
		Point(tx,(uchar)tmp,buf);	
	}
}
#endif

