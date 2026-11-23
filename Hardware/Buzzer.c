#include "stm32f10x.h"
#include "Delay.h"

#define BUZZER_PIN GPIO_Pin_5

void Buzzer_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = BUZZER_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_SetBits(GPIOB, BUZZER_PIN);
}

void Buzzer_On(void)
{
    GPIO_ResetBits(GPIOB, BUZZER_PIN);
}

void Buzzer_Off(void)
{
    GPIO_SetBits(GPIOB, BUZZER_PIN);
}

void Buzzer_Beep(uint16_t ms)
{
    Buzzer_On();
    Delay_ms(ms);
    Buzzer_Off();
}

void Buzzer_Alarm(uint8_t times)
{
    uint8_t i;
    for(i = 0; i < times; i++)
    {
        Buzzer_On();
        Delay_ms(200);
        Buzzer_Off();
        Delay_ms(200);
    }
}
