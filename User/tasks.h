#ifndef __TASKS_H
#define	__TASKS_H

#include "stm32f10x.h"

#define SETTING	176
#define RETURN	56
#define ADD	    24
#define LESS	  74
#define ONE			162
#define TWO	    98
#define THREE	  226

uint8_t main_task1(uint8_t mode_num);
void lcd_task1(void);
void first_page(void);
uint8_t ADC_task(void);
void NVIC_SysReset(void);
uint8_t i2c_saving(void);
uint8_t i2c_reading(void);

extern int delay_time;
extern int sec;
extern char string[20];
extern uint8_t key;

extern float ADC_ConvertedValueLocal;   


#endif

