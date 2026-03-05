#include "stm32f10x.h"

// 按键引脚定义
#define KEY_PORT    GPIOA
#define KEY_PIN     GPIO_Pin_0

// 状态定义
typedef enum {
    STATE_IDLE = 0,        // 空闲
    STATE_PRESS_DETECT,    // 检测到按下
    STATE_PRESS_CONFIRM,   // 确认按下
    STATE_RELEASE_DETECT,  // 检测到释放
    STATE_RELEASE_CONFIRM  // 确认释放
} KeyState;

static KeyState currentState = STATE_IDLE;
static uint8_t count = 0;

// 初始化
void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = KEY_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入
    GPIO_Init(KEY_PORT, &GPIO_InitStructure);
}

// 串口初始化
void UART_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    
    USART_Cmd(USART1, ENABLE);
}

void UART_SendString(char* str)
{
    while(*str)
    {
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, *str++);
    }
}

// 状态机扫描函数（每10ms调用一次）
void KEY_Scan(void)
{
    uint8_t keyValue = GPIO_ReadInputDataBit(KEY_PORT, KEY_PIN);
    
    switch(currentState)
    {
        case STATE_IDLE:
            if(keyValue == 0)  // 检测到按下
            {
                currentState = STATE_PRESS_DETECT;
                count = 1;
            }
            break;
            
        case STATE_PRESS_DETECT:
            if(keyValue == 0)
            {
                count++;
                if(count >= 3)  // 连续3次检测到按下（30ms）
                {
                    currentState = STATE_PRESS_CONFIRM;
                    UART_SendString("Key Pressed!\r\n");
                }
            }
            else
            {
                currentState = STATE_IDLE;  // 抖动，回到空闲
            }
            break;
            
        case STATE_PRESS_CONFIRM:
            if(keyValue == 1)  // 检测到释放
            {
                currentState = STATE_RELEASE_DETECT;
                count = 1;
            }
            break;
            
        case STATE_RELEASE_DETECT:
            if(keyValue == 1)
            {
                count++;
                if(count >= 3)  // 连续3次检测到释放
                {
                    currentState = STATE_RELEASE_CONFIRM;
                    UART_SendString("Key Released!\r\n");
                }
            }
            else
            {
                currentState = STATE_PRESS_CONFIRM;  // 抖动，回到按下态
            }
            break;
            
        case STATE_RELEASE_CONFIRM:
            currentState = STATE_IDLE;  // 回到空闲
            break;
    }
}

// 简单延时
void delay_ms(uint32_t ms)
{
    for(uint32_t i = 0; i < ms * 8000; i++);
}

int main(void)
{
    UART_Init();
    KEY_Init();
    
    UART_SendString("Key State Machine Started\r\n");
    
    uint32_t lastScan = 0;
    
    while(1)
    {
        // 每10ms扫描一次按键
        KEY_Scan();
        delay_ms(10);
    }
}
