#include "stm32f10x.h"
#include <stdio.h>
#include <stdlib.h>

#define MODE_POLLING    0
#define MODE_INTERRUPT  1
#define MODE_DMA        2

// 舵机引脚 PA1
#define SERVO_PWM_PORT   GPIOA
#define SERVO_PWM_PIN    GPIO_Pin_1

uint8_t currentMode = MODE_POLLING;
uint8_t currentAngle = 90;
uint8_t rxBuffer[10];
uint8_t rxIndex = 0;
uint8_t rxComplete = 0;

// ========== 微秒延时（保证PWM准确）==========
void delay_us(uint32_t us) {
    for(uint32_t i = 0; i < us * 8; i++);
}

// ========== 毫秒延时 ==========
void delay_ms(uint32_t ms) {
    for(uint32_t i = 0; i < ms * 8000; i++);
}

// ========== 舵机初始化 ==========
void Servo_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = SERVO_PWM_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SERVO_PWM_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(SERVO_PWM_PORT, SERVO_PWM_PIN);
}

// ========== 舵机控制（直接输出PWM脉冲）==========
void Servo_SetAngle(uint8_t angle) {
    if(angle > 180) angle = 180;
    
    // 连续输出30个脉冲，确保舵机转到目标角度
    for(int i = 0; i < 30; i++) {
        // 高电平：500us (0°) ~ 2500us (180°)
        GPIO_SetBits(SERVO_PWM_PORT, SERVO_PWM_PIN);
        delay_us(500 + angle * 2000 / 180);
        
        // 低电平：补满20ms周期
        GPIO_ResetBits(SERVO_PWM_PORT, SERVO_PWM_PIN);
        delay_us(20000 - (500 + angle * 2000 / 180));
    }
}

// ========== 串口初始化 ==========
void UART_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStructure);
    
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    USART_Cmd(USART1, ENABLE);
}

// ========== 串口发送函数 ==========
void UART_Polling_SendString(char *str) {
    while(*str) {
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, *str++);
    }
}

void UART_IT_SendString(char *str) {
    UART_Polling_SendString("[IT] ");
    UART_Polling_SendString(str);
}

void UART_DMA_SendString(char *str) {
    UART_Polling_SendString("[DMA] ");
    UART_Polling_SendString(str);
}

void SendAngle(uint8_t mode, uint8_t angle) {
    char buf[30];
    sprintf(buf, "Angle: %d\r\n", angle);
    
    switch(mode) {
        case MODE_POLLING:
            UART_Polling_SendString("[POLLING] ");
            UART_Polling_SendString(buf);
            break;
        case MODE_INTERRUPT:
            UART_IT_SendString(buf);
            break;
        case MODE_DMA:
            UART_DMA_SendString(buf);
            break;
    }
}

// ========== 串口中断接收 ==========
void USART1_IRQHandler(void) {
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        uint8_t data = USART_ReceiveData(USART1);
        
        // 回显
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, data);
        
        if(data == '\n' || data == '\r') {
            if(rxIndex > 0) {
                rxBuffer[rxIndex] = '\0';
                rxComplete = 1;
                rxIndex = 0;
            }
        } else {
            if(rxIndex < 9) {
                rxBuffer[rxIndex++] = data;
            }
        }
    }
}

// ========== 主函数 ==========
int main(void) {
    UART_Init();
    Servo_Init();
    
    UART_Polling_SendString("\r\n======= \u4efb\u52a1\u4e00 =======\r\n");
    UART_Polling_SendString("\u6a21\u5f0f0: [POLLING] \u8f6e\u8be2\r\n");
    UART_Polling_SendString("\u6a21\u5f0f1: [IT] \u4e2d\u65ad\r\n");
    UART_Polling_SendString("\u6a21\u5f0f2: [DMA] DMA\r\n");
    UART_Polling_SendString("====================\r\n\r\n");
    
    uint8_t counter = 0;
    uint8_t demoAngle = 0;
    
    while(1) {
        counter++;
        if(counter >= 100) {  // 约1秒
            counter = 0;
            
            // 切换模式
            currentMode++;
            if(currentMode > MODE_DMA) currentMode = MODE_POLLING;
            
            // 改变角度
            demoAngle += 30;
            if(demoAngle > 180) demoAngle = 0;
            
            // 先让舵机转
            Servo_SetAngle(demoAngle);
            
            // 再通过串口发送
            SendAngle(currentMode, demoAngle);
        }
        
        // 处理手动输入
        if(rxComplete) {
            int newAngle = atoi((char*)rxBuffer);
            if(newAngle >= 0 && newAngle <= 180) {
                Servo_SetAngle(newAngle);
                SendAngle(currentMode, newAngle);
            }
            rxComplete = 0;
        }
        
        delay_ms(10);
    }
}
