#include "timer.h"
#include "lcd.h"
#include "motor.h"
#include "sensor.h"
#include "led.h"
#include "tools.h"

#define CENTER_POS   0x3C
#define BASE_PWM    200
#define MAX_PWM    1000
#define MIN_PWM    200
#define PWM_STEP   10

u8 ledDisp[]={'S',':','0','T',':','b','c','d'};
const char TIME_TOPIC[] = {0xFF,0xFE,0x08,0x00,0xF7,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF5}; 
volatile u8 tim3HeartBeatCounter;

float Velocity_KP=0.5,Velocity_KI=0.5;  

extern long long leftWheelCounter;
extern long long rightWheelCounter;

const tim_func_t g_tim_func[] = {
    {TIM_OC1Init, TIM_OC1PreloadConfig, TIM_SetCompare1},
    {TIM_OC2Init, TIM_OC2PreloadConfig, TIM_SetCompare2},
    {TIM_OC3Init, TIM_OC3PreloadConfig, TIM_SetCompare3},
    {TIM_OC4Init, TIM_OC4PreloadConfig, TIM_SetCompare4},
};

void TIM3_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //ʱ��ʹ��
	
	//��ʱ��TIM3��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIM3�ж�,��������ж�

	//�ж����ȼ�NVIC����
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //��ʼ��NVIC�Ĵ���

	TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx					 
}

long long timeNow = 0;
//��ʱ��3�жϷ������
volatile u16 pwmChangeCounter = 0;

struct MotorControlMsg mbedODOMMotorControlMsg = {BASE_PWM,BASE_PWM,FORWARD,FORWARD};

void testMotor(void){
    pwmChangeCounter = pwmChangeCounter + 1;
        
    if(pwmChangeCounter == 5000){
        pwmChangeCounter = 0;
    }
        
    mbedODOMMotorControlMsg.pLeft = pwmChangeCounter;
    mbedODOMMotorControlMsg.pRight = pwmChangeCounter;
        
    //motorControlMsg.pRight= 10000-pwmChangeCounter;
    setWalkingmotorSpeed( mbedODOMMotorControlMsg ); 

}

void avoidZero(void){
    if(mbedODOMMotorControlMsg.pLeft > MAX_PWM){
        mbedODOMMotorControlMsg.pLeft = MAX_PWM;
    }else if( mbedODOMMotorControlMsg.pLeft < MIN_PWM){
        mbedODOMMotorControlMsg.pLeft = MIN_PWM;
    }
    
    if(mbedODOMMotorControlMsg.pRight > MAX_PWM){
        mbedODOMMotorControlMsg.pRight = MAX_PWM;
    }else if( mbedODOMMotorControlMsg.pRight < MIN_PWM){
        mbedODOMMotorControlMsg.pRight = MIN_PWM;    
    }
  
}



int Incremental_PI_Left (int Encoder,int Target)
{ 	
	 static float leftBias,leftPwm,leftLast_bias;
	 leftBias = Encoder - Target;                              //����ƫ��
	 leftPwm += Velocity_KP * (leftBias-leftLast_bias) + Velocity_KI * leftBias;   //����ʽPI������
	 leftLast_bias = leftBias;	                                     //������һ��ƫ�� 
	 return leftPwm;                                           //�������
}

int Incremental_PI_Right (int Encoder,int Target)
{ 	
	 static float rightBias,rightPwm,rightLast_bias;
	 rightBias = Encoder - Target;                              //����ƫ��
	 rightPwm += Velocity_KP * (rightBias-rightLast_bias) + Velocity_KI * rightBias;   //����ʽPI������
	 rightLast_bias = rightBias;	                                     //������һ��ƫ�� 
	 return rightPwm;                                           //�������
}

u8 backwardByte(u8 input){
    u8 anotherDirection=0;
    u8 i = 0;
    
    if(input !=0){
        for(i = 0 ; i< 8 ; i++){
            if(input & 0x01){
                anotherDirection = anotherDirection + ( 1 << (7-i));
  
            }
            
            input = input >> 1;
        
        }    
        
    }else{
        anotherDirection = input;
    }
    
    return anotherDirection;

}

void setWheelPWM(u8 magSensor){
    if(magSensor != 0xFF){
        u8 leftDirectionSensorByte = backwardByte(magSensor);
        u8 rightDirectionSensorByte = magSensor;
        
        int leftPwmErr = abs(Incremental_PI_Left(leftDirectionSensorByte,CENTER_POS));
        int rightPwmErr = abs(Incremental_PI_Right(rightDirectionSensorByte,CENTER_POS));

        printf("leftPwmErr: %d rightPwmErr: %d \r\n",leftPwmErr,rightPwmErr);
       
        mbedODOMMotorControlMsg.pRight = rightPwmErr ;
        mbedODOMMotorControlMsg.pLeft = leftPwmErr;       
        
        
        //printf("pleft: %d, pright: %d \r\n",mbedODOMMotorControlMsg.pLeft, mbedODOMMotorControlMsg.pRight);
        //avoidZero();
        //printf("after limit pleft: %d, pright: %d \r\n",mbedODOMMotorControlMsg.pLeft, mbedODOMMotorControlMsg.pRight);
     
        setWalkingmotorSpeed( mbedODOMMotorControlMsg ); 
    }
}

u16 outRailCounter = 0;

u16 pwmControlCounter = 0;
volatile long long lastLeftEncoderCounter = 0;
volatile long long lastRightEncoderCounter = 0;
   

volatile u8 ifFirstRun = 1;

void getSpeed(volatile long long* leftSpeed,volatile long long* rightSpeed){

    
    if(ifFirstRun == 0){
        *leftSpeed = leftWheelCounter - lastLeftEncoderCounter;
        *rightSpeed = rightWheelCounter - lastRightEncoderCounter;          
         
    }
    ifFirstRun = 0;
    
    lastLeftEncoderCounter = leftWheelCounter;
    lastRightEncoderCounter = rightWheelCounter;
}



void TIM3_IRQHandler(void)   //TIM3�ж�
{
    u8 txBufferSize;
    u8 magSensor = 0;
    u8 totalGreenLight = 0;
    u8 statusCode = 0;
    volatile long long leftSpeed = 0;
    volatile long long rightSpeed = 0; 
    
    u16 leftDesireSpeed = 0;
    u16 rightDesireSpeed = 0;    
    u8 backwardStatusCode = 0;
	
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //���TIM3�����жϷ������
	{
        //10ms�ж�һ��
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //���TIMx�����жϱ�־ 
           
        magSensor = sensorScan();  
        //����λ���ж�
        
        //test  
        //magSensor = 0x38;        
        totalGreenLight = getGreenLights( &magSensor, &statusCode ); 
          
        backwardStatusCode = backwardByte (statusCode);    
              
      
        //����ٶ�, ����������/10ms
        getSpeed(&leftSpeed,&rightSpeed);
        getDesireSpeed(&statusCode,&leftDesireSpeed,&rightDesireSpeed);
        

        //getDesireSpeed(&backwardStatusCode,&leftDesireSpeed,&rightDesireSpeed);
         
        mbedODOMMotorControlMsg.pLeft = abs(Incremental_PI_Left(leftSpeed, leftDesireSpeed));
        mbedODOMMotorControlMsg.pRight = abs(Incremental_PI_Right(rightSpeed, rightDesireSpeed));
        
        //mbedODOMMotorControlMsg.pLeft= 5000;
        //mbedODOMMotorControlMsg.pLeft= 5000;
        //mbedODOMMotorControlMsg.pRight = 500;
        //printf("pleft: %d, pright: %d \r\n",mbedODOMMotorControlMsg.pLeft, mbedODOMMotorControlMsg.pRight);
        //avoidZero();
        //printf("after limit pleft: %d, pright: %d \r\n",mbedODOMMotorControlMsg.pLeft, mbedODOMMotorControlMsg.pRight);
        setWalkingmotorSpeed( mbedODOMMotorControlMsg ); 
          
        if(magSensor == 0){
            outRailCounter++;
             //�ѹ�1��
            if(outRailCounter > 100){
                TIM_Cmd(TIM3, DISABLE);
                mbedODOMMotorControlMsg.leftDirection=BREAK;
                mbedODOMMotorControlMsg.rightDirection=BREAK;
                mbedODOMMotorControlMsg.pLeft = 0 ;
                mbedODOMMotorControlMsg.pRight= 0 ;
                setWalkingmotorSpeed(mbedODOMMotorControlMsg);
                LED2_ON;
            }
        }else{             
            pwmControlCounter++; 
             
            if(pwmControlCounter >10 ){
                //setWheelPWM(magSensor);
                //setWalkingmotorSpeed( mbedODOMMotorControlMsg ); 
                printf("statusCode:%d\r\n",statusCode);
                printf("SPD:%lld,%lld \r\n",leftSpeed,rightSpeed);
                printf("D-SPD:%d,%d \r\n",leftDesireSpeed,rightDesireSpeed);
        
                printf("pleft: %d, pright: %d \r\n",mbedODOMMotorControlMsg.pLeft, mbedODOMMotorControlMsg.pRight);
       
                pwmControlCounter = 0; 
            }
         
        }    
           
            
	}
}


#if 0    
    u8 err = abs(magSensor - CENTER_POS);
    
    //�ݶ���������ƫ
    if(magSensor < CENTER_POS){
        //����ƫ���������������, ͬʱ�����������
        mbedODOMMotorControlMsg.pRight = mbedODOMMotorControlMsg.pRight + PWM_STEP;
        mbedODOMMotorControlMsg.pLeft = mbedODOMMotorControlMsg.pLeft - PWM_STEP;
         
    }else if(magSensor > CENTER_POS){
        mbedODOMMotorControlMsg.pRight = mbedODOMMotorControlMsg.pRight - PWM_STEP;
        mbedODOMMotorControlMsg.pLeft = mbedODOMMotorControlMsg.pLeft + PWM_STEP;        
    
    }else{
        mbedODOMMotorControlMsg. pLeft = BASE_PWM;
        mbedODOMMotorControlMsg. pRight = BASE_PWM;
    
    }
#endif  
