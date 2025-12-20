#include "stm32f10x.h"
#include "Delay.h"
#include "LED.h"
#include "Key.h"
#include "Timer.h"

// 模式枚举
typedef enum {
    MODE_OFF = 0,
    MODE_ON,
    MODE_BLINK  
} LED_ModeTypeDef;

// 全局变量
volatile LED_ModeTypeDef led_mode = MODE_BLINK;

int main(void)
{
    // 初始化
    LED_Init();
    Key_Init();
    Timer_Init();
    
    // 默认启动定时器（闪烁模式）
    TIM_Cmd(TIM2, ENABLE);
    
    // 初始测试：LED亮1秒、灭1秒，确保硬件正常
    LED_ON();
    Delay_ms(1000);
    LED_OFF();
    Delay_ms(1000);
    
    // 记录上次按键状态，用于检测按键释放
    uint8_t last_key_state = 1;  // 1表示未按下（上拉输入默认高电平）
    
    while (1)
    {
        uint8_t current_key_state = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0);
        
        // 检测按键按下（下降沿）
        if (last_key_state == 1 && current_key_state == 0)
        {
            Delay_ms(20);  // 消抖
            
            // 确认按键仍然按下
            if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == 0)
            {
                // 切换LED模式
                led_mode = (led_mode + 1) % 3;
                
                // 根据模式启停定时器
                if (led_mode != MODE_BLINK)
                {
                    TIM_Cmd(TIM2, DISABLE);  // 关闭定时器
                }
                else
                {
                    TIM_Cmd(TIM2, ENABLE);   // 开启定时器
                }
                
                // 等待按键释放
                while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == 0);
                Delay_ms(20);  // 释放消抖
            }
        }
        
        // 更新按键状态
        last_key_state = current_key_state;
        
        // 根据模式控制LED
        switch (led_mode)
        {
            case MODE_OFF:
                LED_OFF();
                break;
                
            case MODE_ON:
                LED_ON();
                break;
                
            case MODE_BLINK:
                // 定时器中断中设置的标志位控制闪烁
                if (timer_flag == 1)
                {
                    LED_Toggle();  // 使用LED_Toggle函数
                    timer_flag = 0;
                }
                break;
        }
        
        Delay_ms(10);  // 小延时，减少CPU占用
    }
}
