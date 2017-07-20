#include "tools.h"
#define BASE_SPEED 100
#define Km 40  //平顺性可以调整这个参数


void getDesireSpeed(u8* statusCode,u16* leftDesireSpeed,u16* rightDesireSpeed){
    switch(*statusCode){
        case 0x80:
            *leftDesireSpeed = BASE_SPEED + 4* Km;
            *rightDesireSpeed = BASE_SPEED;
            break;
        case 0x40:
            *leftDesireSpeed = BASE_SPEED + 3 * Km;
            *rightDesireSpeed = BASE_SPEED;
            break;
        case 0x20:
            *leftDesireSpeed = BASE_SPEED + 2 * Km;
            *rightDesireSpeed = BASE_SPEED;            
        
            break;
        case 0x10:
            *leftDesireSpeed = BASE_SPEED + 1 * Km;
            *rightDesireSpeed = BASE_SPEED;
            break;
        case 0x08:
            *leftDesireSpeed = BASE_SPEED;
            *rightDesireSpeed = BASE_SPEED + 1 * Km;
            break;
        case 0x04:
            *leftDesireSpeed = BASE_SPEED;
            *rightDesireSpeed = BASE_SPEED + 2 * Km;
            break;
        case 0x02:
            *leftDesireSpeed = BASE_SPEED;
            *rightDesireSpeed = BASE_SPEED + 3 * Km;            
            break;
        case 0x01:
            *leftDesireSpeed = BASE_SPEED;
            *rightDesireSpeed = BASE_SPEED + 4 * Km;            
            break;
        case 0x00:
            *leftDesireSpeed = BASE_SPEED;
            *rightDesireSpeed = BASE_SPEED;
            break;
                   
    }

}

u16 lengthOfInt(long long intNumber){
	
    u16 length;
    if(intNumber <= 9 && intNumber > 0){
        length=1;
    }else{
        length=0;
    }
    
    while(intNumber>=1){
        intNumber=intNumber/10;
        ++length;
   }
   return length;
    
}


u16 filEncoderCounter(u8 offSet, long long encoderCounter){
    u16 tempLength,i;   
    long long tempLong;
    long long tempCounter;
    tempCounter = encoderCounter;
    tempLength = lengthOfInt(encoderCounter);

   
    for(i = 0; i < tempLength; i++){
        tempLong = pow(10, tempLength - 1 - i);
        uartTxBuffer[offSet + i] = ASCII_BASE + (u8)(tempCounter / tempLong);
        tempCounter = tempCounter - tempLong * (u8)(tempCounter / tempLong);
    }
    
    return tempLength;
    
    //offSet = offSet + tempLength;
    
}


u8 publishTopic(long long leftEncoderCounter, long long rightEncoderCounter, long long timer, u16 topicID){
    u16 msgLength;
    u16 msgLengthChk;
    u8 frameChk;
    u16 msgSum;
	u32 publisherLength;
    u16 tempLength=0;
	u8 offSet = 0;
    long long tempLong;
    long long tempCounter;
    u16 i;
	//msg + 2 "," + msgLenth 4 bytes
    msgLength= 4 + 2 + lengthOfInt(leftEncoderCounter) + lengthOfInt(rightEncoderCounter) + lengthOfInt(timer);

    uartTxBuffer[offSet]=0xff;
	offSet++;
    uartTxBuffer[offSet]=0xfe;
    offSet++;
    uartTxBuffer[offSet] = (u8)(msgLength);
    uartTxBuffer[offSet+1] = (u8)(msgLength>>8);

    offSet = offSet + 2;	
    
	msgLengthChk = 255 - ( (( msgLength&255 ) + ( msgLength >>8 )) % 256 );

	uartTxBuffer[offSet] = msgLengthChk;
	offSet++;
	
	uartTxBuffer[offSet] = (u8)(topicID);
	uartTxBuffer[offSet+1] = (u8)(topicID>>8);
    
	offSet = offSet + 2;
	
	publisherLength = msgLength - 4;	
	
	uartTxBuffer[offSet] =  (u8)(publisherLength);
	uartTxBuffer[offSet+1] =  (u8)(publisherLength>>8);
	uartTxBuffer[offSet+2] =  (u8)(publisherLength>>16);
	uartTxBuffer[offSet+3] =  (u8)(publisherLength>>24);
 	offSet = offSet + 4;
	
    tempLength = filEncoderCounter(offSet,leftEncoderCounter);    
    offSet = offSet + tempLength;
    uartTxBuffer[offSet] =  0x2c;
    offSet++;
    
    tempLength = filEncoderCounter(offSet,rightEncoderCounter);    
    offSet = offSet + tempLength;    
    uartTxBuffer[offSet] =  0x2c;
    offSet++;

    tempLength = filEncoderCounter(offSet,timer);    
    offSet = offSet + tempLength;    

    //msg_checksum = 255 - ( ((topic&255) + (topic>>8) + sum([ord(x) for x in msg]))%256 )
    for(i=5; i<offSet ;i++){
        msgSum = msgSum + uartTxBuffer[i];
    }
    
    frameChk = 255 - msgSum % 256;
    uartTxBuffer[offSet] = frameChk;
    offSet++;
	return offSet;
}

void getStatusCodeBylightNumber1(u8* input,u8* statusCode){
    //u8 statusCode;
    switch(*input){
        //1000 0000                
        case 0x80:                   
            *statusCode = 1 << 7;              
        break;             
            
        //0000 0001        
        case 0x01:
            *statusCode = 1;
        break;            
    }  
    //return statusCode;
}

void getStatusCodeBylightNumber2(u8* input,u8*statusCode){
    //u8 statusCode;
    switch(*input){
        //1100 0000
        case 0xc0:
            *statusCode = 1 << 6;
            break;
            
        //0110 0000
        case 0x60:
            *statusCode = 1 << 5;
            break;
            
        //0011 0000
        case 0x30:
            *statusCode = 1 << 4;
            break;
            
        //0001 1000 稳态
        case 0x18:
            *statusCode = 0;
            break;
            
        //0000 1100
        case 0x0c:
            *statusCode = 1 << 3;
            break;
            
        //0000 0110
        case 0x06:
            *statusCode = 1 << 2;
            break;
            
        //0000 0011
       case 0x03:
            *statusCode = 1 << 1;
            break;                            
    }            
    
    //return statusCode;

}

void getStatusCodeBylightNumber3(u8* input, u8*statusCode){
    //u8 statusCode;
    switch(*input){
        //1110 0000
        case 0xE0:
            *statusCode = 1 << 6;
            break;
            
        //0111 0000
        case 0x70:
            *statusCode = 1 << 5;
            break;
            
        //0011 1000
        case 0x38:
            *statusCode = 1 << 4;
            break;           
            
        //0001 1100
        case 0x1c:
            *statusCode = 1 << 3;
            break;
            
        //0000 1110
        case 0x0E:
            *statusCode = 1 << 2;
            break;
            
        //0000 0011
       case 0x07:
            *statusCode = 1 << 1;
            break;                            
    }            
    
    //return statusCode;

}

void getStatusCodeBylightNumber4(u8* input,u8* statusCode){
    //u8 statusCode;
    switch(*input){
        //1111 0000
        case 0xF0:
            *statusCode = 1 << 5;
            break;
            
        //0111 1000
        case 0x78:
            *statusCode = 1 << 4;
            break;
            
        //0011 1100 稳态
        case 0x3C:
            *statusCode = 0;
            break;           
            
        //0001 1110
        case 0x1E:
            *statusCode = 1 << 3;
            break;
            
        //0000 1111
        case 0x0F:
            *statusCode = 1 << 2;
            break;
                                        
    }            
    
    //return statusCode;
}

void getStatusCodeBylightNumber5(u8* input, u8* statusCode){
    //u8 statusCode;
    switch(*input){
        //1111 1000
        case 0xF8:
            *statusCode = 1 << 5;
            break;
            
        //0111 1100
        case 0x7C:
            *statusCode = 1 << 4;
            break;                    
            
        //0011 1110
        case 0x3E:
            *statusCode = 1 << 3;
            break;
            
        //0001 1111
        case 0x1F:
            *statusCode = 1 << 2;
            break;
                                        
    }            
    
    //return statusCode;

}


u8 getGreenLights(u8 *input,u8* statusCode){
    
    u8 a;
    
    //绿灯数量
    u8 greenLightsQuo = 0;
    //是否有稳态
    u8 ifHasStableStatus = 0 ;
    //有多少种状态
    u8 statusQuo = 0;
    u8 i=0;
    //u8 statusCode = 0;
    u8 tempInput = *input;
    
    //首先获取触发数
    for(i = 0; i < 8; i++){
        if(tempInput & 0x01){
            greenLightsQuo ++;
        }
        tempInput = tempInput >> 1;
    }
    
    switch( greenLightsQuo ){
       
        case 1:
            //只有一个灯亮, 通常仅仅左右两种可能, 一左一右        
            statusQuo = 2;
            ifHasStableStatus = 0;      
            getStatusCodeBylightNumber1(input,statusCode);                           
            break;
            
        case 2:            
            //两个有效位, 有7种状态, 有稳态
            statusQuo = 7;
            ifHasStableStatus = 1;    
            getStatusCodeBylightNumber2(input,statusCode);            
            break;        
            
        case 3:          
            //3个有效位的情况下, 有6种状态, 没有稳态, 偶数都没有稳态...
            statusQuo = 6;
            ifHasStableStatus = 0;    
            getStatusCodeBylightNumber3(input,statusCode);                     
                
            break;
        
        case 4:
            //4个有效位, 有5种状态, 奇数, 有稳态
            statusQuo = 5;
            ifHasStableStatus = 1;    
            getStatusCodeBylightNumber4(input,statusCode); 
            break;
        
        case 5:
            //5个有效位, 4种状态, 偶数无稳态
            statusQuo = 4;
            ifHasStableStatus = 0;    
            getStatusCodeBylightNumber5(input,statusCode);         
            break;
    }
       
    
    return greenLightsQuo;
    
}


