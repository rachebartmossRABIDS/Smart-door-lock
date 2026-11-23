#ifndef __USART3_H
#define __USART3_H

#include "stm32f10x.h"
#include "oled.h"
#include "usart.h"

extern uint8_t Usart3_RxPacket[6];
extern uint8_t Usart3_RxFlag;

void USART3_Config(void);
uint8_t Usart3_GetRxFlag(void);

#endif
