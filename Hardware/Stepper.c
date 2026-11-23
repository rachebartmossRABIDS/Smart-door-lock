#include "stm32f10x.h"
#include "Delay.h"

#define IN1 GPIO_Pin_15
#define IN2 GPIO_Pin_14
#define IN3 GPIO_Pin_13
#define IN4 GPIO_Pin_12

uint8_t step_table[8] = {
    0x08,0x0C,0x04,0x06,
    0x02,0x03,0x01,0x09
};

static void Stepper_Output(uint8_t data)
{
    GPIO_WriteBit(GPIOB, IN1, (BitAction)(data & 0x01));
    GPIO_WriteBit(GPIOB, IN2, (BitAction)((data>>1)&0x01));
    GPIO_WriteBit(GPIOB, IN3, (BitAction)((data>>2)&0x01));
    GPIO_WriteBit(GPIOB, IN4, (BitAction)((data>>3)&0x01));
}

void Stepper_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = IN1|IN2|IN3|IN4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void Stepper_Move(uint32_t steps, uint8_t dir, uint16_t speed)
{
    uint32_t i;
    static int8_t step = 0;

    for(i=0; i<steps; i++)
    {
        if(dir)  step++;
        else     step--;

        if(step > 7) step = 0;
        if(step < 0) step = 7;

        Stepper_Output(step_table[step]);
        Delay_ms(speed);
    }

    Stepper_Output(0x00); // 断电防止发热
}
