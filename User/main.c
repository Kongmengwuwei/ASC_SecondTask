#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Key.h"
#include "Timer.h"
#include "Motor.h"
#include "Encoder.h"
#include "Serial.h"

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
			
			
		}
		
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}
}
