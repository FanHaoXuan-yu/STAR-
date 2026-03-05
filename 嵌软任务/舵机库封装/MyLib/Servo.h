#ifndef __SERVO_H
#define __SERVO_H

#include "stm32f10x.h"

// 舵机引脚配置
#define SERVO_GPIO_PORT     GPIOA
#define SERVO_GPIO_PIN      GPIO_Pin_1
#define SERVO_GPIO_CLK      RCC_APB2Periph_GPIOA

// 舵机参数配置
#define SERVO_MIN_ANGLE      0
#define SERVO_MAX_ANGLE      180
#define SERVO_MIN_PULSE      500     // 0度对应的脉宽(us)
#define SERVO_MAX_PULSE      2500    // 180度对应的脉宽(us)
#define SERVO_PERIOD         20000   // 20ms周期

// 初始化舵机
void Servo_Init(void);

// 设置角度（0-180）
void Servo_SetAngle(uint8_t angle);

// 获取当前角度
uint8_t Servo_GetAngle(void);

// 舵机更新函数（需要在主循环中调用）
void Servo_Update(void);

#endif