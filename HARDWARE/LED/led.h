#ifndef __LED_H
#define __LED_H	 
#include "sys.h"


#define LED_ON    GPIO_SetBits(GPIOD,GPIO_Pin_13);
#define LED_OFF    GPIO_ResetBits(GPIOD,GPIO_Pin_13);

#define LED2_ON    GPIO_SetBits(GPIOG,GPIO_Pin_14);
#define LED2_OFF    GPIO_ResetBits(GPIOG,GPIO_Pin_14);

void LED_Init(void);//≥ı ºªØ

		 				    
#endif
