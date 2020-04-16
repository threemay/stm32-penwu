/**
  ******************************************************************************
  * @file    bsp_key.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   按键应用bsp（扫描模式）
  ******************************************************************************
  * @attention
  *
  * 实验平台:秉火 F103-MINI STM32 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 
  
#include "bsp_key.h"  
#include "FreeRTOS.h"
#include "task.h"


/**
  * @brief  配置按键用到的I/O口
  * @param  无
  * @retval 无
  */
void Key_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	


  
  RCC_APB2PeriphClockCmd(KEY1_GPIO_CLK|KEY2_GPIO_CLK,ENABLE); //开启按键端口PB的时钟
	GPIO_InitStructure.GPIO_Pin = KEY1_GPIO_PIN; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //端口配置为上拉输入
	GPIO_Init(KEY1_GPIO_PORT, &GPIO_InitStructure);	//初始化端口

	GPIO_InitStructure.GPIO_Pin = KEY2_GPIO_PIN; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //端口配置为x拉输入
	GPIO_Init(KEY2_GPIO_PORT, &GPIO_InitStructure);	//初始化端口
	
	RCC_APB2PeriphClockCmd(KEY4_GPIO_CLK,ENABLE); //开启按键端口PB的时钟
	GPIO_InitStructure.GPIO_Pin = KEY4_GPIO_PIN; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //端口配置为x拉输入
	GPIO_Init(KEY4_GPIO_PORT, &GPIO_InitStructure);	//初始化端口*/

}
	


 /*
 * 函数名：Key_Scan
 * 描述  ：检测是否有按键按下
 * 输入  ：GPIOx：x 可以是 A，B，C，D或者 E
 *		     GPIO_Pin：待读取的端口位 	
 * 输出  ：KEY_OFF(没按下按键)、KEY_ON（按下按键）
 */
uint8_t Key_Scan_Low(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin)
{			
	/*检测是否有按键按下 */
	if(GPIO_ReadInputDataBit(GPIOx,GPIO_Pin) == KEY_ON )  
	{	 
		/*等待按键释放 */
		vTaskDelay(100);
		if(GPIO_ReadInputDataBit(GPIOx,GPIO_Pin) == KEY_ON )
		{
			while(GPIO_ReadInputDataBit(GPIOx,GPIO_Pin) == KEY_ON);   
			return 	KEY_ON;
		}
		else{
		return KEY_OFF;}
	}
	else{
		return KEY_OFF;}
}

uint8_t Key_Scan_High(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin)
{			
	/*检测是否有按键按下 */
	if(GPIO_ReadInputDataBit(GPIOx,GPIO_Pin) == 1 )  
	{	 
		/*等待按键释放 */
		vTaskDelay(100);
		if(GPIO_ReadInputDataBit(GPIOx,GPIO_Pin) == 1 )
		{
			//while(GPIO_ReadInputDataBit(GPIOx,GPIO_Pin) == 1);   
			return 	1;
		}
		else{
		return 0;}
	}
	else{
		return 0;}
}

uint8_t Key_Scan_High1(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin)
{			
	/*检测是否有按键按下 */
	if(GPIO_ReadInputDataBit(GPIOx,GPIO_Pin) == 1 )  
	{	 
		/*等待按键释放 */
		vTaskDelay(100);
		if(GPIO_ReadInputDataBit(GPIOx,GPIO_Pin) == 1 )
		{
			while(GPIO_ReadInputDataBit(GPIOx,GPIO_Pin) == 1);   
			return 	1;
		}
		else{
		return 0;}
	}
	else{
		return 0;}
}


/*********************************************END OF FILE**********************/
