#ifndef __IR_H__
#define __IR_H__
#include <stm32f10x.h>
#include <stm32f10x_gpio.h>
#include <stdbool.h>
#include <stdio.h>

#define TOLERANCE 20

typedef enum {
	IR_NEC_NONE = 0,
	IR_NEC_FIRST_BURST = 9000,
	IR_NEC_SECOND_BURST = 4500,
	IR_NEC_1 = 560,
	IR_NEC_NDEF = 1,
	IR_NEC_DATA = 2
} ir_nec_state;

void ir_nec_init(u16 pin, GPIO_TypeDef *gpio);
void ir_nec_state_machine(unsigned int time);
void ir_nec_timeout();

#endif // __IR_H__
