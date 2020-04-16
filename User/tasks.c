#include <stdlib.h>
#include <stdio.h>

#include "delay.h"
#include "lcd.h"
#include "gui.h"
#include "tasks.h"
#include "bsp_key.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_adc.h"
#include "bsp_i2c_ee.h"
#include "bsp_i2c_gpio.h"
#include "bsp_led.h"

int delay_time=5000;
int sec;
char string[20];
uint8_t key;
uint8_t maxNum=100;
uint8_t minNum=0;
uint8_t setNum=80;
uint8_t curNum=0;


extern __IO uint16_t ADC_ConvertedValue;//模拟量口采集值

// 局部变量，用于保存转换计算后的电压值 	 
float ADC_ConvertedValueLocal;   

uint8_t ADC_task(void);

void NVIC_SysReset(void) //系统复位
{
  __DSB();

  SCB->AIRCR  = ((0x5FA << SCB_AIRCR_VECTKEY_Pos)      | 
                 (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) | 
                 SCB_AIRCR_SYSRESETREQ_Msk);     //置位 SYSRESETREQ
  __DSB();
  while(1);
}

uint8_t main_task1(uint8_t mode_num)
{
switch(mode_num)
      {
        case 0://皮带模式
						if( Key_Scan_High(KEY2_GPIO_PORT,KEY2_GPIO_PIN) == 1 ) //如果有输入
						{
							return 1;  //返回1
						}
						if( Key_Scan_High(KEY4_GPIO_PORT,KEY4_GPIO_PIN) == 1 )
						{
							return 1;
						}
						if( ADC_task() == 1 )
						{
							return 1;
						}
						return 0;
        
        case 1://大巷模式
						if( Key_Scan_High(KEY4_GPIO_PORT,KEY4_GPIO_PIN) == 1 )
						{
							return 1;
						}
						return 0;
				
				case 2://触碰模式
						if( Key_Scan_High(KEY4_GPIO_PORT,KEY4_GPIO_PIN) == 1 )
						{
							return 1;
						}
						return 0;
        
        
        default://放炮模式
						if( Key_Scan_High(KEY4_GPIO_PORT,KEY4_GPIO_PIN) == 1 )
						{
							return 1;
						}
						return 0;						
      }
};

uint8_t ADC_task(void)
{

	if(ADC_ConvertedValueLocal>((float) setNum/(maxNum-minNum)*3.3)) //转化值大于后面
	{
		return 1;
	}
	else return 0;
}

uint8_t i2c_saving(void) //存储设定值
{
	uint8_t write_buf[EEPROM_SIZE]; //创建数组

  
/*-----------------------------------------------------------------------------------*/  
  if (ee_CheckDevice(EEPROM_DEV_ADDR) == 1)
	{
		/* 没有检测到EEPROM */
		//printf("没有检测到串行EEPROM!\r\n");
				
		return 0;
	}
/*------------------------------------------------------------------------------------*/  
	write_buf[0] = delay_time/1000;
	write_buf[1] = maxNum;
	write_buf[2] = minNum;
	write_buf[3] = setNum;

/*------------------------------------------------------------------------------------*/  
  if (ee_WriteBytes(write_buf, 0, EEPROM_SIZE) == 0)
	{
		//printf("写eeprom出错！\r\n");
		return 0;
	}
	else
	{		
		//printf("写eeprom成功！\r\n");
		return 1;
	}
}  

uint8_t i2c_reading(void) //读取设定值
{
	  uint8_t read_buf[EEPROM_SIZE];
	if (ee_CheckDevice(EEPROM_DEV_ADDR) == 1)
	{
		/* 没有检测到EEPROM */
		//printf("没有检测到串行EEPROM!\r\n");
				
		return 0;
	}

  if (ee_ReadBytes(read_buf, 0, EEPROM_SIZE) == 0)
	{
		//printf("读eeprom出错！\r\n");
		return 0;
	}
	else
	{	
		delay_time = read_buf[0]*1000;
		maxNum = read_buf[1];
		minNum = read_buf[2];
		setNum = read_buf[3];
		//printf("读eeprom成功  \r\n");
		return 1;
	}
}



void first_page(void)//首页
{
	LCD_Clear(BLACK); //清屏
	Show_Str(32,5,BLUE,YELLOW,"山东传正",16,0);
	Show_Str(16,30,RED,YELLOW,"喷雾系统",24,1);
	Show_Str(5,50,RED,YELLOW,"当前:    ",16,1);
	curNum=(ADC_ConvertedValueLocal/3.3*(maxNum-minNum) + minNum);
	sprintf(string, "%d", curNum);
	Show_Str(5+48+16,50,RED,YELLOW,string,16,1);
	Show_Str(48,100,BLUE,YELLOW,"设定",16,0);
	//printf("diyiye");
}

void second_page(void)
{
	LCD_Clear(BLACK); //清屏
	Show_Str(15,5,RED,YELLOW,"1.延时设定",16,1);
	Show_Str(15,25,RED,YELLOW,"2.定时设定",16,1);
	Show_Str(15,45,RED,YELLOW,"3.模拟量设定",16,1);
	//printf("dierye");
}

void third_page_1(void)
{
	LCD_Clear(BLACK); //清屏
  Show_Str(32,5,BLUE,YELLOW,"延时设定",16,0);
	Show_Str(53,25,RED,YELLOW,"+",16,1);
	Show_Str(16,50,RED,YELLOW,"延时:    秒",16,1);
	Show_Str(5+48,50,RED,YELLOW,string,16,1);
	Show_Str(53,75,RED,YELLOW,"-",16,1);
	Show_Str(15,100,RED,YELLOW,"确认并返回",16,1);
}

void third_page_3(void)
{
	LCD_Clear(BLACK); //清屏
	Show_Str(15,25,RED,YELLOW,"1.最大值设定",16,1);
	Show_Str(15,50,RED,YELLOW,"2.最小值设定",16,1);
	Show_Str(15,75,RED,YELLOW,"3.设定值设定",16,1);
}

void third_page_3_1(void)
{
	LCD_Clear(BLACK); //清屏
  Show_Str(32,5,BLUE,YELLOW,"最大值设定",16,0);
	Show_Str(5+48+16,25,RED,YELLOW,"+",16,1);
	Show_Str(5,50,RED,YELLOW,"最大值:    ",16,1);
	Show_Str(5+48+16,50,RED,YELLOW,string,16,1);
	Show_Str(5+48+16,75,RED,YELLOW,"-",16,1);
	Show_Str(5,100,RED,YELLOW,"确认并返回",16,1);
}

void third_page_3_2(void)
{
	LCD_Clear(BLACK); //清屏
  Show_Str(32,5,BLUE,YELLOW,"最小值设定",16,0);
	Show_Str(5+48+16,25,RED,YELLOW,"+",16,1);
	Show_Str(5,50,RED,YELLOW,"最小值:    ",16,1);
	Show_Str(5+48+16,50,RED,YELLOW,string,16,1);
	Show_Str(5+48+16,75,RED,YELLOW,"-",16,1);
	Show_Str(5,100,RED,YELLOW,"确认并返回",16,1);
}

void third_page_3_3(void)
{
	LCD_Clear(BLACK); //清屏
  Show_Str(32,5,BLUE,YELLOW,"设定值设定",16,0);
	Show_Str(5+48+16,25,RED,YELLOW,"+",16,1);
	Show_Str(5,50,RED,YELLOW,"设定值:    ",16,1);
	Show_Str(5+48+16,50,RED,YELLOW,string,16,1);
	Show_Str(5+48+16,75,RED,YELLOW,"-",16,1);
	Show_Str(5,100,RED,YELLOW,"确认并返回",16,1);
}


void lcd_task1()  //显示任务
{
	//first_page();
	vTaskDelay(100);
		if( key==SETTING )                           //如果key等于176进入设置
    {
			second_page();												//显示第二页
			while(1)															//进入循环
			{
				vTaskDelay(100);
				if( key==RETURN )                       //退出设置
				{
					first_page();											
					break;
				}
				if( key==ONE )                        //进入延时设定
				{
					sec=delay_time/1000;								//将延时时间毫秒赋予sec秒
					sprintf(string, "%d", sec);
					third_page_1();											//进入延时设定页面
					while(1)														//进入循环
					{
						vTaskDelay(20);
						if( key==RETURN )                     //返回设置页面
						{
							delay_time=sec*1000;						
							i2c_saving();										//存储新改的值
							second_page();									//显示第二页
							while(key==RETURN){vTaskDelay(20);};
							break;
						}
						if( key==ADD )
						{
							sec++;
							if(sec>99){sec=99;};
							sprintf(string, "%d", sec);
							third_page_1();
							while(key==ADD){vTaskDelay(20);};
						}
						if( key ==LESS )
						{
							sec--;
							if(sec<0){sec=0;};
							sprintf(string, "%d", sec);
							third_page_1();
							while(key==LESS){vTaskDelay(20);};
						}
					}
				}
				if( key==THREE )                        //进入模拟量设定
				{			
					third_page_3();
					while(key==THREE){vTaskDelay(20);};
					while(1)
					{
						vTaskDelay(20);
						if( key==RETURN )                  //返回设置页面
						{
							second_page();
							while(key==RETURN){vTaskDelay(20);};
							break;
						}
						if( key==ONE )                        //进入最大值设定
						{
							uint8_t maxNum1=maxNum;
							sprintf(string, "%d", maxNum1);
							third_page_3_1();
							while(1)
							{
								vTaskDelay(20);
								if( key==RETURN )                  //返回设置页面
								{
									maxNum=maxNum1;
									i2c_saving();
									third_page_3();
									while(key==RETURN){vTaskDelay(20);};
									break;
								}
								if( key==ADD )
								{
									maxNum1++;
									if(maxNum1>100){maxNum1=100;};
									sprintf(string, "%d", maxNum1);
									third_page_3_1();
									while(key==ADD){vTaskDelay(20);};
								}
								if( key ==LESS )
								{
									maxNum1--;
									if(maxNum1<=0){maxNum1=0;};
									sprintf(string, "%d", maxNum1);
									third_page_3_1();
									while(key==LESS){vTaskDelay(20);};
								}
							}
						}
						if( key==TWO )                        //进入最小值设定
						{
							uint8_t minNum1=minNum;
							sprintf(string, "%d", minNum1);
							third_page_3_2();
							while(1)
							{
								vTaskDelay(20);
								if( key==RETURN )                  //返回设置页面
								{
									minNum=minNum1;
									i2c_saving();
									third_page_3();
									while(key==RETURN){vTaskDelay(20);};
									break;
								}
								if( key==ADD )
								{
									minNum1++;
									if(minNum1>100){minNum1=100;};
									sprintf(string, "%d", minNum1);
									third_page_3_2();
									while(key==ADD){vTaskDelay(20);};
								}
								if( key ==LESS )
								{
									minNum1--;
									if(minNum1<=0){minNum1=0;};
									sprintf(string, "%d", minNum1);
									third_page_3_2();
									while(key==LESS){vTaskDelay(20);};
								}
							}
						}
						if( key==THREE )                        //进入设定值设定
						{
							uint8_t setNum1=setNum;
							sprintf(string, "%d", setNum1);
							third_page_3_3();
							while(1)
							{
								vTaskDelay(20);
								if( key==RETURN )                  //返回设置页面
								{
									setNum=setNum1;
									i2c_saving();
									third_page_3();
									while(key==RETURN){vTaskDelay(20);};
									break;
								}
								if( key==ADD )
								{
									setNum1++;
									if(setNum1>100){setNum1=100;};
									sprintf(string, "%d", setNum1);
									third_page_3_1();
									while(key==ADD){vTaskDelay(20);};
								}
								if( key ==LESS )
								{
									setNum1--;
									if(setNum1<=0){setNum1=0;};
									sprintf(string, "%d", setNum1);
									third_page_3_3();
									while(key==LESS){vTaskDelay(20);};
								}
							}
						}
						
						
						
					}
				}				
			}
		}
	
}

