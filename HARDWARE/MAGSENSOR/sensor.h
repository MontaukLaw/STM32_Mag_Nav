#ifndef __SENSOR_H
#define __SENSOR_H	

#include "sys.h"

#define SENSOR_BIT_8 GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_7)
#define SENSOR_BIT_7 GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_6)
#define SENSOR_BIT_6 GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_5)
#define SENSOR_BIT_5 GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_4)
#define SENSOR_BIT_4 GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_3)
#define SENSOR_BIT_3 GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_2)
#define SENSOR_BIT_2 GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_1)
#define SENSOR_BIT_1 GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_0)

u8 sensorScan(void);
void initMagSensor(void);


#endif

