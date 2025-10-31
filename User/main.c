#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Key.h"
#include "Timer.h"
#include "Motor.h"
#include "Encoder.h"
#include "Serial.h"
#include <string.h>

uint8_t mode=0;

float Mode1_Target,Mode1_Actual,Mode1_Out;
float Mode1_Kp=0.5,Mode1_Ki=0.16,Mode1_Kd=0.15;
float Mode1_Error0,Mode1_Error1,Mode1_Error2;

float Mode2_Target,Mode2_Actual,Mode2_Out;
float Mode2_Kp=0.4,Mode2_Ki=0,Mode2_Kd=0;
float Mode2_Error0,Mode2_Error1,Mode2_Error2;

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
		
		if(Key_Check(KEY_1,KEY_SINGLE)){
			mode=(mode==1)?0:1;
		}
		// 功能一电机控速
		if(mode==0){
			Mode2_Out=0;
			Motor2_SetPWM(0);
			//读取上位机输入为目标
			if(Serial_GetRxFlag()==1){
				if (strstr(Serial_RxPacket, "speed%") != NULL) {
					
					sscanf(Serial_RxPacket, "speed%%%f", &Mode1_Target);
					
					Serial_Printf("Set_Speed:%d\r\n",(int)Mode1_Target);
				} else {
					Serial_SendString("ERROR_COMMAND\r\n");
				}
			}
			OLED_Clear();
			OLED_Printf(0,0,8,"Mode 1");
	        OLED_Printf(0,16,8,"电机控速");
			OLED_Printf(0,32,8,"Mode 2");
	        OLED_Printf(0,48,8,"电机传动");
			OLED_ReverseArea(0,0,128,32);
			OLED_Update();
		
			Serial_Printf("%f,%f,%f\n",Mode1_Target,Mode1_Actual,Mode1_Out);
		}
			
		//功能二主从电机
		if (mode==1){
			Mode1_Out=0;
			Motor1_SetPWM(0);
			
			OLED_Clear();
			OLED_Printf(0,0,8,"Mode 1");
	        OLED_Printf(0,16,8,"电机控速");
			OLED_Printf(0,32,8,"Mode 2");
	        OLED_Printf(0,48,8,"电机传动");
			OLED_ReverseArea(0,32,128,32);
			OLED_Update();
		
			Serial_Printf("%f,%f,%f\n",Mode2_Target,Mode2_Actual,Mode2_Out);
		}
	}
}

void TIM1_UP_IRQHandler(void){
	
	static uint16_t Count;
	
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
		Key_Tick();
		
		Count++;
		if(Count>=10)
		{
			// 功能一电机控速
			if(mode==0){
			Count=0;
				
			Mode2_Error0=0;
			Mode2_Error1=0;
			Mode2_Error2=0;	
				
			//增量式PID控制速度
			Mode1_Actual=Encoder1_Get();
			
			Mode1_Error2=Mode1_Error1;
			Mode1_Error1=Mode1_Error0;
			Mode1_Error0=Mode1_Target-Mode1_Actual;
			
			Mode1_Out += Mode1_Kp * (Mode1_Error0-Mode1_Error1) +
		    	         Mode1_Ki *  Mode1_Error0 +
			             Mode1_Kd * (Mode1_Error0-2*Mode1_Error1+Mode1_Error2);
			
			if(Mode1_Out>100)Mode1_Out=100;
			if(Mode1_Out<-100)Mode1_Out=-100;
			
			Motor1_SetPWM(Mode1_Out);
		}
	}
		//功能二主从电机
		if (mode==1){
			Count=0;
			
			Mode1_Error0=0;
			Mode1_Error1=0;
			Mode1_Error2=0;	
			
			//读取电机一位置为目标
			Mode2_Target+=Encoder1_Get();
			//增量式PID控制位置
			Mode2_Actual+=Encoder2_Get();
			
			Mode2_Error2=Mode2_Error1;
			Mode2_Error1=Mode2_Error0;
			Mode2_Error0=Mode2_Target-Mode2_Actual;
			
			Mode2_Out += Mode2_Kp * (Mode2_Error0-Mode2_Error1) +
		    	   Mode2_Ki *  Mode2_Error0 +
			       Mode2_Kd * (Mode2_Error0-2*Mode2_Error1+Mode2_Error2);
			
			if(Mode2_Out>100)Mode2_Out=100;
			if(Mode2_Out<-100)Mode2_Out=-100;
			
			Motor2_SetPWM(Mode2_Out);
		}
		
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}
}
