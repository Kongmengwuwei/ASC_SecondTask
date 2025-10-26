#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Key.h"
#include "Timer.h"
#include "Motor.h"
#include "Encoder.h"
#include "Serial.h"
#include <string.h>

int16_t Speed;
int8_t PWM;

float Target,Actual,Out;
float Kp=0.1,Ki=0.2,Kd=0.05;
float Error0,Error1,Error2;

int main(void)
{ 
	OLED_Init();
	Key_Init();
	Timer_Init();
	Motor_Init();
	Encoder_Init();
	Serial_Init();
	
	while (1)
	{
		//读取上位机输入
		if(Serial_GetRxFlag()==1){
			if (strstr(Serial_RxPacket, "speed%") != NULL) {
				
				sscanf(Serial_RxPacket, "speed%%%f", &Target);
				
				Serial_Printf("Set_Speed:%d\r\n",(int)Target);
			} else {
				Serial_SendString("ERROR_COMMAND\r\n");
			}
		}
		
		OLED_ShowSignedNum(1,1,Target,5);
		OLED_ShowSignedNum(2,1,Actual,5);
		OLED_ShowSignedNum(3,1,Out,5);
		
		//输出波形图
		Serial_Printf("%f,%f,%f\n",Target,Actual,Out);
	}
}

void TIM1_UP_IRQHandler(void){
	
	static uint16_t Count;
	
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
		Key_Tick();
		
		Count++;
		if(Count>=40)
		{
			//增量式PID控制
			Count=0;
			Actual=Encoder_Get();
			
			Error2=Error1;
			Error1=Error0;
			Error0=Target-Actual;
			
			Out += Kp * (Error0-Error1) +
		    	   Ki *  Error0 +
			       Kd * (Error0-2*Error1+Error2);
			
			if(Out>100)Out=100;
			if(Out<-100)Out=-100;
			
			Motor_SetPWM(Out);
		}
		
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}
}
