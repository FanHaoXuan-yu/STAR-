#ifndef __TIMER_H
#define __TIMER_H

void Timer_Init(void);
void TIM2_IRQHandler(void);

extern volatile uint8_t timer_flag;
#endif
