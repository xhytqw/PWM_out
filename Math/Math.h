#ifndef __MATH_H
#define __MATH_H

#include "stm32f10x.h"
#include "stdint.h"

uint32_t mypow(uint8_t base,uint8_t exp);
uint32_t freCal(uint8_t *fre,uint8_t dig);
uint8_t duCal(uint8_t *du,uint8_t dig);

#endif
