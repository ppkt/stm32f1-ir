#include <stm32f10x.h>
#include <system_stm32f10x.h>
#include <stm32f10x_exti.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_tim.h>
#include <misc.h>
#include <stdio.h>

#include "usart.h"
#include "ir.h"

#define DELAY 10

void LED_Init() {
	/* Enable GPIO clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	// Use PC8 and PC9 // Discovery LEDs
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

}

void IR_Init() {
	/* Enable GPIO clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	// Use PC6 as input from IR receiver
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// Enable clock and its interrupts
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource6);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

	EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line6;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);


}

void TIM_Init() {
	// Enable clock and its interrupts
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

	TIM_TimeBaseInitTypeDef TIM_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_InitStructure.TIM_Prescaler = 24 - 1;
	TIM_InitStructure.TIM_Period = 10000 - 1; // Update event every 10000 us / 10 ms
	TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_InitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIM_InitStructure);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);
}

int main(void)
{
	LED_Init();
	IR_Init();
	TIM_Init();
	USART1_Init(921600);
	ir_nec_init(GPIO_Pin_6, GPIOC);

	printf("\r\nWorking\r\n ");

    while(1)
    {
    }
}

void TIM2_IRQHandler()
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        // Timeout
        ir_nec_reset_transmission();
    }
}

void EXTI9_5_IRQHandler(void)
{
    static unsigned int counter;

    if (EXTI_GetITStatus(EXTI_Line6) != RESET)
    {
        u8 bit = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_6);
        if (bit) {
        	GPIO_ResetBits(GPIOC, GPIO_Pin_9);
        } else {
        	GPIO_SetBits(GPIOC, GPIO_Pin_9);
        }

        // Restart Timer
        counter = TIM_GetCounter(TIM2);
        TIM_SetCounter(TIM2, 0);
        ir_nec_state_machine(counter);

        EXTI_ClearITPendingBit(EXTI_Line6);
    }
}
