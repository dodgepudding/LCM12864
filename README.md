LCM2864
=======
YM12864驱动模块  

文  件
-------
LCM12864.c  功能实现模块  
LCM12864.h  引脚配置定义及头文件引用  

移植定义
-------
* 宏定义说明：  
  \_\_USE_SPI_MODE\_\_ 		在头文件定义此宏来声明使用串行方式通信  
  \_\_USE_PARALLEL_MODE\_\_ 在头文件定义此宏声明使用并行方式通信  
  LCD12864_CMD_xxxx 	LCD基本指令字  
  LCD12864_DATA_xxxx    LCD的RAM数据操作  
  LCD12864_EXTCMD_xxxx  LCD扩展指令字  
* 需在头文件定义的引脚：  
  RST：低电平复位引脚；LCD_PS：串并选择引脚，1：并行模式；0：串行模式
* 串行模式：  
  LCD_CLK(SCLK):时钟输入脚；LCD_DATA(SID):数据输入输出脚；LCD_CS(RS)：片选有效  
* 并行模式：  
  LCD_CS: 片选有效；LCD_RW：读写选择；  
  LCD_RS：命令模式指令模式切换；LCDData：8位并行数据；  

使用方法
-------
  1）修改LCM12864.H头文件相应配置；  
  2）运行液晶初始化函数LCDInit()，若并行模式，要在这之后加一定延时;  
  3）运行各功能函数打字，绘图等，例如：  
  PutStr(0,0,"欢迎使用PDODGE的液晶驱动模块")；//表示在第0行0列输出字符串  
  DrawBMP(buf);//将图形缓冲区的点阵数据输出到LCD  
  
功能函数
-------
/*********************初始化功能函数*****************************/    
void LCDInit();	    //初始化液晶  
void ClearLCD();	//清除LCD显示  
void ClearRAM();	//清楚RAM数据  
void Display_on();	//打开整体显示  
void Display_off();	//关闭整体显示  
void Draw_on();		//打开绘图模式  
void Draw_off();	//关闭绘图模式  
void SendData(unsigned char);  //发送指令数据
/*************************功能函数*******************************/  
void Cursor_on();	//游标闪烁开  
void Cursor_off();	//游标闪烁关  
void Direct_left();	//设定游标向左移动  
void Direct_right();//设定游标向右移动  
void Scroll_left(); //文字整体左移  
void Scroll_right();//文字整体右移  
void Cursor_left(); //游标向左移动  
void Cursor_right(); //游标向右移动  
void Line_Inverse(unsigned char line); //第line行反显  
void Sleep(); 		//进入睡眠模式  
void Wakeup(); 		//从睡眠模式唤醒  
void PutChar(unsigned char row,unsigned char col,unsigned char cc);		//在row行,col列显示一个字符  
void PutStr(unsigned char row,unsigned char col,unsigned char *puts);	//在row行,col列开始显示一字符串  
void DrawBMP(unsigned char *puts);				   						//画puts数组表示的图像  
void Point(unsigned char x,unsigned char y,unsigned char *buf);			//在(x,y)位置画一点，(0,0)坐标在左下角  
void Point2(unsigned char x,unsigned char y);							//纯画点，覆盖原图  
void Line(unsigned char sx,unsigned char sy,unsigned char ex,unsigned char ey,unsigned char *buf); //从（sx,sy)向(ex,ey)坐标连线，ex>sx  
  
历史版本
-------
V1.00.00 纯串行模式，基本字符和图形显示功能  
V1.00.10 加入并行模式、串行模式任意选择，完善图形显示功能  
V1.00.11 修复Scroll_left(),Scroll_right()函数指向的bug  
V1.00.12 调整高速核心模式下的延时系数  
V1.00.13 加入Direct_left(),Direct_right(),游标移动方向,Cursor_left(),Cursor_right(),游标单次移动功能  
