#ifndef __STEPPER_H
#define __STEPPER_H

void Stepper_Init(void);
void Stepper_Move(uint32_t steps, uint8_t dir, uint16_t speed);

#endif
