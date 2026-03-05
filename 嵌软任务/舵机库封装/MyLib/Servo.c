#include "Servo.h"

static uint16_t targetPulse = 1500;  // 目标脉宽
static uint16_t currentPulse = 1500; // 当前实际脉宽
static uint8_t currentAngle = 90;    // 当前角度

void Servo_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 使能GPIO时钟
    RCC_APB2PeriphClockCmd(SERVO_GPIO_CLK, ENABLE);
    
    // 配置舵机引脚为推挽输出
    GPIO_InitStructure.GPIO_Pin = SERVO_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SERVO_GPIO_PORT, &GPIO_InitStructure);
    
    // 初始低电平
    GPIO_ResetBits(SERVO_GPIO_PORT, SERVO_GPIO_PIN);
}

void Servo_SetAngle(uint8_t angle)
{
    // 限幅处理
    if(angle > SERVO_MAX_ANGLE) angle = SERVO_MAX_ANGLE;
    if(angle < SERVO_MIN_ANGLE) angle = SERVO_MIN_ANGLE;
    
    currentAngle = angle;
    
    // 角度转脉宽：500 + (angle * 2000 / 180)
    targetPulse = SERVO_MIN_PULSE + 
                  (angle * (SERVO_MAX_PULSE - SERVO_MIN_PULSE)) / 
                  (SERVO_MAX_ANGLE - SERVO_MIN_ANGLE);
}

uint8_t Servo_GetAngle(void)
{
    return currentAngle;
}

void Servo_Update(void)
{
    static uint16_t timer = 0;
    static uint8_t phase = 0;  // 0:高电平 1:低电平
    
    timer++;
    
    if(phase == 0)  // 高电平阶段
    {
        GPIO_SetBits(SERVO_GPIO_PORT, SERVO_GPIO_PIN);
        if(timer >= currentPulse)
        {
            timer = 0;
            phase = 1;
        }
    }
    else  // 低电平阶段
    {
        GPIO_ResetBits(SERVO_GPIO_PORT, SERVO_GPIO_PIN);
        if(timer >= (SERVO_PERIOD - currentPulse))
        {
            timer = 0;
            phase = 0;
            
            // 平滑过渡（每次变化不超过5us）
            if(currentPulse < targetPulse) currentPulse += 5;
            else if(currentPulse > targetPulse) currentPulse -= 5;
        }
    }
}