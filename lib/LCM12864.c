/************************************************************************************
**                            YM12864驱动模块
**                     (c) Copyright 2007-2013, Dodge
**                          All Rights Reserved
**					https://github.com/dodgepudding/LCM2864
**
**                              V1.00.14
*************************************************************************************/
#include "LCM12864.h"

#ifndef __LCD12864_MODEL__
#define __LCD12864_MODEL__
	
#define LCD12864_CMD_CLEAR 		0x01 //清除显示，将DDRAM填满“20H”，并且设定DDRAM的地址计数器（AC）到“00H”,4.6ms
#define LCD12864_CMD_RSTDDRAM 	0x02 //地址归位，设定DDRAM的地址计数器（AC）到“00H”，并且将游标移到开头原点位置；这个指令并不改变DDRAM的内容，4.6ms
#define LCD12864_CMD_SETCUR(id,s)  (0x04|id<<1|s) //进入点设定，指定在资料的读取与写入时，设定游标移动方向及指定显示的移位，ID=1：游标向右移，ID=0：游标向左移；S=1：画面整体右/左移动。72us			  
#define LCD12864_CMD_DISP(d,c,b)  (0x08|d<<2|c<<1|b) //显示状态开/关，D=1：整体显示ON，C=1：游标“─“ON，B=1：游标位置“■”ON，72us
#define LCD12864_CMD_CURXY(sc,rl) (0x10|sc<<3|rl<<2) //游标或显示移位控制，设定游标的移动与显示的移位控制位元；这个指令并不改变DDRAM的内容
#define LCD12864_CMD_MODE(re)	(0x30|re<<2) //模式设定，RE=1：扩充指令集动作，RE=0：基本指令集动作
#define LCD12864_CMD_MODE_NORMAL  0
#define LCD12864_CMD_NORMAL 0x30
#define LCD12864_CMD_MODE_EXTEND  1
#define LCD12864_CMD_EXTEND 0x34
#define LCD12864_CMD_SETCGRAM(ac) (0x40|(ac&0x3f))   //设定CGRAM地址，设定CGRAM地址到地址计数器（AC）
#define LCD12864_CMD_SETDDRAM(ac) (0x80|(ac&0x7f))	 //设定DDRAM 地址，设定DDRAM地址到地址计数器（AC）
#define LCD12864_CMD_BUSY(in,bf)    bf=(in&0x80)>>7  //读取忙碌标志（BF），in为读出的数据输入
#define LCD12864_CMD_READAC(in,ac)     ac=(in&0x7f)  //读出地址计数器（AC）的值，in为读出的数据输入
#define LCD12864_DATA_WRITE(r)		r               //写入资料到内部的RAM（DDRAM/CGRAM/IRAM/GDRAM）
#define LCD12864_DATA_READ(r)		r				//从内部RAM读取资料（DDRAM/CGRAM/IRAM/GDRAM）
#define LCD12864_EXTCMD_READY   0x01 				//待命模式,将DDRAM填满“20H”，并且设定DDRAM的地址计数器（AC）到“00H”
#define LCD12864_EXTCMD_SCROLL(sr)  (0x02|sr)		//卷动地址或IRAM地址选择，SR=1：允许输入垂直卷动地址，SR=0：允许输入IRAM地址
#define LCD12864_EXTCMD_INVERSE(r) (0x04|(r&0x03))  //反白选择，选择4行中的任一行作反白显示，并可决定反白与否72us
#define LCD12864_EXTCMD_SLEEP(sl)	(0x08|sl<<2)	//睡眠模式，SL=1：脱离睡眠模式，SL=0：进入睡眠模式
#define LCD12864_EXTCMD_MODE(re,g)   (0x30|re<<2|g<<1) //扩充功能设定，RE=1：扩充指令集动作，RE=0：基本指令集动作，G=1 ：绘图显示ON，G=0 ：绘图显示OFF	
#define LCD12864_EXTCMD_SCRARRD(ac)	 (0x40|ac&0x3f) 	//设定IRAM地址或卷动地址，SR=1：AC5—AC0为垂直卷动地址，SR=0：AC3—AC0为ICON IRAM地址
#define LCD12864_EXTCMD_CGADDR(ac)	 (0x80|ac&0x7f)		//SR=1：AC5—AC0为垂直卷动地址SR=0：AC3—AC0为ICON IRAM地址


	
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
{0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87},      //第一行汉字位置
{0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97},      //第二行汉字位置
{0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f},      //第三行汉字位置
{0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f},      //第四行汉字位置
};

/*****************************串行模式基本读写函数*************************************/
#ifdef __USE_SPI_MODE__
void SPI_Write(uchar dat) //写SPI时序
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

uchar SPI_Read() //读SPI时序
{
uchar dat;
uchar t = 8;
LCD_DATA = 1; //读取数据之前DIO 引脚要置1 以切换到输入状态
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

void SPIWR(uchar wdata,uchar rs)   //写SPI移入数据, RS=0输入指令, RS=1输入数据
{
	SPI_Write(0xf8+(rs<<1));		//数据格式：11111 | RW | RS | 0 |
	SPI_Write(wdata&0xf0);			//写入数据字高4位，数据格式： D7-D4| 0000 |
	SPI_Write((wdata<<4)&0xf0);		//写入数据字低4位，数据格式:  D3-D0| 0000 |
}

uchar SPIRD(uchar rs)   //读SPI移入数据, RS=1输入指令, RS=1输入数据
{
uchar rdata;
	SPI_Write(0xf8+(1<<2)+(rs<<1));		//数据格式：11111 | RW | RS | 0 |
	rdata=SPI_Read()&0xf0;			//读取数据字高4位，数据格式： D7-D4| 0000 |
	rdata=rdata|((SPI_Read()&0xf0)>>4);		//读取数据字低4位，数据格式:  D3-D0| 0000 |
return rdata;
}

void SendCMD(uchar CMD)
{
SPIWR(CMD,0);
LongDelay(DELAY8US);//命令后延时，延时不够可能会出现乱码，适当调节此值	
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
	
void SendCMD(uchar cmd)//写指令
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
void SendData(uchar dat)//写数据
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

uchar ReadData()//写数据
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

     SendCMD(LCD12864_CMD_NORMAL);//功能设置，基本指令集
	 LongDelay(DELAY4MS);
#ifdef __USE_PARALLEL_MODE__ 
LongDelay(DELAY4MS);
#endif
	 SendCMD(LCD12864_CMD_DISP(1,0,0));//0000,1100  整体显示，游标off，游标位置off
     SendCMD(LCD12864_CMD_RSTDDRAM);//0000,0010 DDRAM地址归位
     LongDelay(DELAY4MS);
#ifdef __USE_PARALLEL_MODE__ 
LongDelay(DELAY4MS);
#endif
	 SendCMD(LCD12864_CMD_CLEAR);//0000,0001 清DDRAM
     LongDelay(DELAY4MS);
	 SendCMD(LCD12864_CMD_SETCUR(1,0));//点设定，显示字符/光标从左到右移位，DDRAM地址AC加1//   
#ifdef __USE_PARALLEL_MODE__ 
LongDelay(DELAY4MS);
LongDelay(DELAY4MS);
#endif
}
/*****************************
发送字符串
*****************************/
void ClearLCD()  //在当前模式下清屏幕
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
void Scroll_left() //整体向左移动，AC=AC
{
	SendCMD(LCD12864_CMD_CURXY(1,0));
}
void Scroll_right()//整体向右移动，AC=AC
{
	SendCMD(LCD12864_CMD_CURXY(1,1));
}
void Cursor_left() //游标向左移动，AC=AC
{
	SendCMD(LCD12864_CMD_CURXY(0,0));
}
void Cursor_right()//游标向右移动，AC=AC
{
	SendCMD(LCD12864_CMD_CURXY(0,1));
}
void Line_Inverse(uchar line) //第line行显示反白
{
	SendCMD(LCD12864_CMD_EXTEND);
	SendCMD(LCD12864_EXTCMD_INVERSE(line));
}

void Sleep() //进入睡眠模式
{
	SendCMD(LCD12864_CMD_EXTEND);
	SendCMD(LCD12864_EXTCMD_SLEEP(0));
}

void Wakeup() //从睡眠模式唤醒
{
	SendCMD(LCD12864_CMD_EXTEND);
	SendCMD(LCD12864_EXTCMD_SLEEP(1));
}

void PutChar(uchar row,uchar col,uchar cc)  //X表示第几个字,Y表示第几行,cc表示字符;
{
   SendCMD(LCD_ADDR[row][col]);
   SendData(cc);
}

void PutStr(uchar row,uchar col,uchar *puts)
{
     SendCMD(LCD12864_CMD_NORMAL);      //8BitMCU,基本指令集合
     SendCMD(LCD_ADDR[row][col]);      //起始位置
  while(*puts != '\0')      //判断字符串是否显示完毕
     {
           if(col==8)            //判断换行
           {            //若不判断,则自动从第一行到第三行
                 col=0;
                 row++;
           }
           if(row==4) row=0;      //一屏显示完,回到屏左上角
           SendCMD(LCD_ADDR[row][col]);
           SendData(*puts);      //一个汉字要写两次
           puts++;
           if (*puts == '\0') break; else SendData(*puts);
           puts++;
           col++;
     }
	
}

void Draw_on()
{
SendCMD(LCD12864_CMD_EXTEND);	   //8Bit扩充指令集,即使是36H也要写两次
SendCMD(LCD12864_EXTCMD_MODE(1,1));	   //绘图ON,基本指令集里面36H不能开绘图
}

void Draw_off()
{
SendCMD(LCD12864_CMD_EXTEND);	   //8Bit扩充指令集,即使是36H也要写两次
SendCMD(LCD12864_EXTCMD_MODE(1,0));	   //绘图OFF,基本指令集里面36H不能开绘图
}

void DrawBMP(unsigned char *dc)
{
     unsigned int x=0;
     unsigned char xx,yy;
 	 Draw_off();
     for(yy=0;yy<32;yy++)
		for(xx=0;xx<8;xx++)
		{
			SendCMD(0x80+yy);   //y 按位
			SendCMD(0x80+xx);//x 按16位
			SendData(*dc);
			dc++;
			SendData(*dc);
			dc++;
		}
	for(yy=0;yy<32;yy++)
		for(xx=8;xx<16;xx++)
		{
			SendCMD(0x80+yy);   //y 按位
			SendCMD(0x80+xx);//x 按16位
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

