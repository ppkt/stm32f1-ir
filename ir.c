#include "ir.h"

u16 pin;
GPIO_TypeDef *gpio;
ir_nec_state ir_nec_current_state;
u8 cnt = 0;

char data[33];

void ir_nec_init(u16 p, GPIO_TypeDef *g) {
	pin = p;
	gpio = g;
	ir_nec_current_state = IR_NEC_NONE;
}

void ir_nec_timeout() {
	if (ir_nec_current_state != IR_NEC_NONE) {
		printf("%s\r\n", data);
		ir_nec_current_state = IR_NEC_NONE;
		cnt = 0;
	}
}

bool ir_nec_check_tolerance(unsigned int received, unsigned int expected) {
	if (expected <= 2) {
		return true; // case of IR_NEC_NONE
	}

	u32 min = expected - (TOLERANCE / 100.0) * expected;
	u32 max = expected + (TOLERANCE / 100.0) * expected;
	if ((received >= min) && (received <= max)) {
		return true;
	}
//	printf("%d %d\r\n", received, TIM2->CNT);
	return false;
}

void ir_nec_state_machine(unsigned int time) {
	BitAction bit = 1 - GPIO_ReadInputDataBit(gpio, pin); // Invert received value

	GPIO_WriteBit(GPIOC, GPIO_Pin_8, bit);

	if (!ir_nec_check_tolerance(time, ir_nec_current_state)) {
		ir_nec_current_state = IR_NEC_NONE;
		cnt = 0;
		return;
	}

	switch (ir_nec_current_state) {
		case IR_NEC_NONE:
			if (bit == Bit_SET) {
				ir_nec_current_state = IR_NEC_FIRST_BURST;
			}
			break;
		case IR_NEC_FIRST_BURST:
			if (bit == Bit_RESET) {
				ir_nec_current_state = IR_NEC_SECOND_BURST;
//				printf("2\r\n", time);
			}
			break;
		case IR_NEC_SECOND_BURST:
		case IR_NEC_DATA:
			if (bit == Bit_SET) {
				ir_nec_current_state = IR_NEC_1;
//				printf("3\r\n", time);
			}
			break;
		case IR_NEC_1:
			if (bit == Bit_RESET) {
				ir_nec_current_state = IR_NEC_NDEF; // we can receive either 0 or 1
//				printf("4\r\n", time);
			}
			break;

		case IR_NEC_NDEF:
			ir_nec_current_state = IR_NEC_1;
			if (time < 1000) {
//				printf("5 - 1\r\n", time);
//				printf("%d", 1);
				data[cnt++] = '1';
			} else {
//				printf("5 - 0\r\n", time);
//				printf("%d", 0);
				data[cnt++] = '0';
			}
			break;
		default:
			ir_nec_current_state = IR_NEC_NONE;
			cnt = 0;
			break;
	}
}
