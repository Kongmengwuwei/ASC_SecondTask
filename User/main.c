#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Key.h"
#include "Timer.h"
#include "Motor.h"
#include "Encoder.h"
#include "Serial.h"

int16_t Speed;
int8_t PWM;

float Target,Actual,Out;
float Kp=0.2,Ki=0.2,Kd=0;
float Error0,Error1,ErrorInt;

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
		if(Serial_GetRxFlag()==1){
			Target=Serial_GetRxData();
			Serial_SendByte(Target);
		}
		OLED_ShowNum(1,1,Target,5);
		OLED_ShowNum(2,1,Actual,5);
		OLED_ShowNum(3,1,Out,5);
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
			Count=0;
			Actual=Encoder_Get();
			
			Error1=Error0;
			Error0=Target-Actual;
		
			ErrorInt+=Error0;
			
			Out=Kp*Error0+Ki*ErrorInt+Kd*(Error0-Error1);
			
			if(Out>100)Out=100;
			if(Out<-100)Out=-100;
			
			Motor_SetPWM(Out);
		}
		
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}
}
