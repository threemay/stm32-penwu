/**
  *********************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   FreeRTOS V9.0.0  + STM32 固件库例程
  *********************************************************************
  * @attention
  *
  * 实验平台:野火 STM32 全系列开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  **********************************************************************
  */ 
 
/*
*************************************************************************
*                             包含的头文件
*************************************************************************
*/ 
#include <stdlib.h>
#include <stdio.h>
/* FreeRTOS头文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
/* 开发板硬件bsp头文件 */
#include "bsp_led.h"
#include "bsp_key.h"
#include "delay.h"
#include "lcd.h"
#include "gui.h"
#include "./IrDa/bsp_irda.h" 
#include "./usart/bsp_usart.h"
#include "tasks.h"
#include "bsp_adc.h"
#include "./led/bsp_led.h" 
#include "./i2c/bsp_i2c_ee.h"


extern const unsigned char BMP1[];

extern int delay_time;              //延时时间单位毫秒 
extern int sec;											//延时时间单位秒
extern char string[20];							//字符串,用于显示LCD
extern uint8_t key;									//红外遥控按键数值
extern __IO uint16_t ADC_ConvertedValue;  //ADC模拟转换电压
extern float ADC_ConvertedValueLocal;     // 局部变量，用于保存转换计算后的电压值 	 

/**************************** 任务句柄 ********************************/
/* 
 * 任务句柄是一个指针，用于指向一个任务，当任务创建好之后，它就具有了一个任务句柄
 * 以后我们要想操作这个任务都需要通过这个任务句柄，如果是自身的任务操作自己，那么
 * 这个句柄可以为NULL。
 */
static TaskHandle_t AppTaskCreate_Handle = NULL;/* 创建任务句柄 */
static TaskHandle_t LCD_Task_Handle = NULL;/* LED任务句柄 */
static TaskHandle_t Main_Task_Handle = NULL;/* KEY任务句柄 */
static TaskHandle_t Remote_Task_Handle = NULL;/* KEY任务句柄 */
static TaskHandle_t ADC_Task_Handle = NULL;/* KEY任务句柄 */
static TimerHandle_t Swtmr1_Handle =NULL;   /* 软件定时器句柄 */

/********************************** 内核对象句柄 *********************************/
/*
 * 信号量，消息队列，事件标志组，软件定时器这些都属于内核的对象，要想使用这些内核
 * 对象，必须先创建，创建成功之后会返回一个相应的句柄。实际上就是一个指针，后续我
 * 们就可以通过这个句柄操作这些内核对象。
 *
 * 内核对象说白了就是一种全局的数据结构，通过这些数据结构我们可以实现任务间的通信，
 * 任务间的事件同步等各种功能。至于这些功能的实现我们是通过调用这些内核对象的函数
 * 来完成的
 * 
 */
QueueHandle_t Test_Queue =NULL;

/******************************* 全局变量声明 ************************************/
/*
 * 当我们在写应用程序的时候，可能需要用到一些全局变量。
 */

/******************************* 宏定义 ************************************/
/*
 * 当我们在写应用程序的时候，可能需要用到一些宏定义。
 */
#define  QUEUE_LEN    4   /* 队列的长度，最大可包含多少个消息 */
#define  QUEUE_SIZE   4   /* 队列中每个消息大小（字节） */


/*
*************************************************************************
*                             函数声明
*************************************************************************
*/
static void AppTaskCreate(void);/* 用于创建任务 */

static void LCD_Task(void* pvParameters);/* Test_Task任务实现 */
static void Main_Task(void* pvParameters);/* KEY_Task任务实现 */
static void Remote_Task(void* pvParameters);/* KEY_Task任务实现 */
static void ADC_Task(void* pvParameters);/* KEY_Task任务实现 */
static void Swtmr1_Callback(void* parameter);

static void BSP_Init(void);/* 用于初始化板载相关资源 */

/*****************************************************************
  * @brief  主函数
  * @param  无
  * @retval 无
  * @note   第一步：开发板硬件初始化 
            第二步：创建APP应用任务
            第三步：启动FreeRTOS，开始多任务调度
  ****************************************************************/
int main(void)
{	
  BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
	
	SystemInit();           //系统初始化
	
  /* 开发板硬件初始化 */
  BSP_Init();  
 
  /* 创建Test_Queue */
  Test_Queue = xQueueCreate((UBaseType_t ) QUEUE_LEN,/* 消息队列的长度 */
                            (UBaseType_t ) QUEUE_SIZE);/* 消息的大小 */
  
  
   /* 创建AppTaskCreate任务 */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* 任务入口函数 */
                        (const char*    )"AppTaskCreate",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )1, /* 任务的优先级 */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* 任务控制块指针 */ 
  /* 启动任务调度 */           
  if(pdPASS == xReturn)
    vTaskStartScheduler();   /* 启动任务，开启调度 */
  else
			NVIC_SysReset();
    return -1;  
  
  while(1);   /* 正常不会执行到这里 */    
}


/***********************************************************************
  * @ 函数名  ： AppTaskCreate
  * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
  * @ 参数    ： 无  
  * @ 返回值  ： 无
  **********************************************************************/
static void AppTaskCreate(void)
{
  BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
  
  taskENTER_CRITICAL();           //进入临界区
  

  xReturn = xTaskCreate((TaskFunction_t )Main_Task,  /* 任务入口函数 */
                        (const char*    )"Main_Task",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )4, /* 任务的优先级 */
                        (TaskHandle_t*  )&Main_Task_Handle);/* 任务控制块指针 */ 
												
									
	xReturn = xTaskCreate((TaskFunction_t )LCD_Task,  /* 任务入口函数 */
                        (const char*    )"LCD_Task",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )3, /* 任务的优先级 */
                        (TaskHandle_t*  )&LCD_Task_Handle);/* 任务控制块指针 */ 
	
	xReturn = xTaskCreate((TaskFunction_t )Remote_Task,  /* 任务入口函数 */
                        (const char*    )"Remote_Task",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )0, /* 任务的优先级 */
                        (TaskHandle_t*  )&Remote_Task_Handle);/* 任务控制块指针 */ 
												
	xReturn = xTaskCreate((TaskFunction_t )ADC_Task,  /* 任务入口函数 */
                        (const char*    )"ADC_Task",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )2, /* 任务的优先级 */
                        (TaskHandle_t*  )&ADC_Task_Handle);/* 任务控制块指针 */ 
												
	
	Swtmr1_Handle=xTimerCreate((const char*			)"OneShotTimer",
                             (TickType_t			)1000,/* 定时器周期 1s */
                             (UBaseType_t			)pdFALSE,/* 单次模式 */
                             (void*					  )1,/* 为每个计时器分配一个索引的唯一ID */
                             (TimerCallbackFunction_t)Swtmr1_Callback); 
 
  vTaskDelete(AppTaskCreate_Handle); //删除AppTaskCreate任务
  
  taskEXIT_CRITICAL();            //退出临界区
}



static void LCD_Task(void* parameter)
{
	vTaskDelay(20);			//延时20个tick
	first_page();				//显示第一页,首页
	i2c_reading();			//读取存储参数
  while(1)						//循环
	{
		lcd_task1();			//进入LCD自定任务在tasks里面	
		vTaskDelay(20);
		
	}
}

static void Remote_Task(void* parameter) //红外遥控
{
   while(1)
	 {
		key=Remote_Scan();	
		vTaskDelay(50);/* 延时50个tick */
		key=0;
	 }
}


static void ADC_Task(void* parameter) //模拟量
{
   while(1)
	 {
		//ADC_ConvertedValueLocal =(float) ADC_ConvertedValue/4096*3.3; 
	
		vTaskDelay(200);/* 延时20个tick */

	 }
}

/**********************************************************************
  * @ 函数名  ： Main_Task
  * @ 功能说明： Main_Task体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
static void Main_Task(void* parameter) //主任务
{
   while(1)
	 {
		 if( main_task1(0) == 1 )
		 {
			 LED1_ON; // 输出
			 xTimerChangePeriod(Swtmr1_Handle,delay_time,0);  //更改延时时间
			if(Swtmr1_Handle != NULL)
			{
				xTimerStart(Swtmr1_Handle,0);	//开启定时器
			}
		 }
		vTaskDelay(200);/* 延时20个tick */
	 }


}


/***********************************************************************
  * @ 函数名  ： Swtmr2_Callback
  * @ 功能说明： 软件定时器2 回调函数，打印回调函数信息&当前系统时间
  *              软件定时器请不要调用阻塞函数，也不要进行死循环，应快进快出
  * @ 参数    ： 无  
  * @ 返回值  ： 无
  **********************************************************************/
static void Swtmr1_Callback(void* parameter)
{	
	LED1_OFF;
}

/***********************************************************************
  * @ 函数名  ： BSP_Init
  * @ 功能说明： 板级外设初始化，所有板子上的初始化均可放在这个函数里面
  * @ 参数    ：   
  * @ 返回值  ： 无
  *********************************************************************/
static void BSP_Init(void)
{
	/*
	 * STM32中断优先级分组为4，即4bit都用来表示抢占优先级，范围为：0~15
	 * 优先级分组只需要分组一次即可，以后如果有其他的任务需要用到中断，
	 * 都统一用这个优先级分组，千万不要再分组，切忌。
	 */
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
	
	
	delay_init(72);	     //延时初始化
	
	/* LED 即输出 初始化 */
	LED_GPIO_Config();
  
  /* 按键初即输入初始化	*/
  Key_GPIO_Config();

	/* 红外遥控初始化	*/
	Remote_Init();
	
	/* 串口初始化,此程序不用	*/
	//USART_Config(); 
	//printf("//////////////////////////////// \r\n");
	
	/* 模拟量初始化	*/
	ADCx_Init();
	
	/* ATC24C02存储初始化	*/
	i2c_CfgGpio();

	
	LCD_Init();	   //液晶屏初始化
	LCD_Clear(BLACK); //清屏
	POINT_COLOR=WHITE; //白屏画笔



}

/********************************END OF FILE****************************/
