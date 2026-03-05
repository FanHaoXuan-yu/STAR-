#include "stm32f10x.h"
#include "Servo.h"

// 简单延时函数
void delay_ms(uint32_t ms)
{
    for(uint32_t i = 0; i < ms * 8000; i++);
}

int main(void)
{
    Servo_Init();        // 初始化舵机
    
    uint8_t angle = 0;
    
    while(1)  // 你原来的while循环
    {
        Servo_SetAngle(angle);  // 设置角度
        
        angle += 30;
        if(angle > 180) angle = 0;
        
        // 延时1秒（通过多次调用Update实现）
        for(int i = 0; i < 100; i++)
        {
            Servo_Update();  // 更新PWM
            delay_ms(10);
        }
    }
}
