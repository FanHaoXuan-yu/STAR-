#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "LED.h"
#include "Key.h"

uint8_t KeyNum;

int main(void)
{
	LED_Init();
	Key_Init();
	
	while (1)
	{
		KeyNum = Key_GetNum();
		if (KeyNum == 1)
		{
			if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13) == 0)
			{
				LED_OFF();
            }
		    else
		    {
				LED_ON();
			}
		} 
		Delay_ms(10);
	}
}
