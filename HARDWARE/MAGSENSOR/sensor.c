#include "sensor.h"


//PF0-PF7
void initMagSensor(void){
    GPIO_InitTypeDef GPIO_InitStructure;
 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF,ENABLE);//使能PORTA,PORTE时钟

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;//KEY0-KEY2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOF, &GPIO_InitStructure);//初始化GPIOE2,3,4

}

u8 sensorScan(void){
    u8 i;
    u8 offset = 0;
    u8 sensorResult = 0;
    //从sensor的01开始, 到08
    
    //82998999-612
    
    if(SENSOR_BIT_8 == RESET){
        sensorResult = sensorResult + (1 << offset);
    }    
    offset++;
    
    if(SENSOR_BIT_7 == RESET){
        sensorResult = sensorResult + (1 << offset);
    }
    offset++; 
    
    if(SENSOR_BIT_6 == RESET){
        sensorResult = sensorResult + (1 << offset);
    }
    offset++; 
    
    if(SENSOR_BIT_5 == RESET){
        sensorResult = sensorResult + (1 << offset);
    }  
    offset++;
    
    if(SENSOR_BIT_4 == RESET){
        sensorResult = sensorResult + (1 << offset);
    }    
    offset++;
    
    if(SENSOR_BIT_3 == RESET){
        sensorResult = sensorResult + (1 << offset);
    }
    
    offset++;     
    if(SENSOR_BIT_2 == RESET){
        sensorResult = sensorResult + (1 << offset);
    }  
  
    offset++;     
    if(SENSOR_BIT_1 == RESET){
        sensorResult = sensorResult + (1 << offset);
    }   
   
    return sensorResult;    
    //3c is center
    
}
