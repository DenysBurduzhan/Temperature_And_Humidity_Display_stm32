
#ifndef TM1637_H
#define TM1637_H

#include "stm32f4xx_hal.h"

void TM1637_Init(GPIO_TypeDef *clkPort, uint16_t clkPin, GPIO_TypeDef *dioPort, uint16_t dioPin);


void TM1637_DisplayDigit(uint8_t digit, uint8_t position);


void TM1637_DisplayNumber(int16_t number);


void TM1637_SetBrightness(uint8_t brightness);

void TM1637_ClearDisplay(void);

#endif
