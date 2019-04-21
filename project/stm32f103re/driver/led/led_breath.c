

#include <stdlib.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include "led_breath.h"


extern uint16_t breath_table[];
extern uint16_t breath_len;

static void clock_setup(void)
{
	/* Enable TIM3 clock. */
	rcc_periph_clock_enable(RCC_TIM3);

	/* Enable GPIOC, Alternate Function clocks. */
	rcc_periph_clock_enable(RCC_GPIOA);
	// rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_AFIO);
}

static void gpio_setup(void)
{
	/*
	 * Set GPIO6 and 7 (in GPIO port A) to
	 * 'output alternate function push-pull'.
	 */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
		      GPIO_TIM3_CH1 | GPIO_TIM3_CH2);

}

static void tim_setup(void)
{
	/* Clock division and mode */
	TIM3_CR1 = TIM_CR1_CKD_CK_INT | TIM_CR1_CMS_EDGE;
	/* Period */
	TIM3_ARR = 500;
	/* Prescaler */
	TIM3_PSC = 9-1;
	TIM3_EGR = TIM_EGR_UG;

	/* ---- */
	/* Output compare 1 mode and preload */
	TIM3_CCMR1 |= TIM_CCMR1_OC1M_PWM1 | TIM_CCMR1_OC1PE;

	/* Polarity and state */
	TIM3_CCER |=  TIM_CCER_CC1E;
	//TIM3_CCER |= TIM_CCER_CC1E;

	/* Capture compare value */
	TIM3_CCR1 = 0;

	/* ---- */
	/* Output compare 2 mode and preload */
	TIM3_CCMR1 |= TIM_CCMR1_OC2M_PWM1 | TIM_CCMR1_OC2PE;

	/* Polarity and state */
	TIM3_CCER |= TIM_CCER_CC2P | TIM_CCER_CC2E;
	//TIM3_CCER |= TIM_CCER_CC2E;

	/* Capture compare value */
	TIM3_CCR2 = 0;
	/* ---- */
	/* ARR reload enable */
	TIM3_CR1 |= TIM_CR1_ARPE;

	/* Counter enable */
	TIM3_CR1 |= TIM_CR1_CEN;
}

int led_breath_init(void)
{
	clock_setup();
	gpio_setup();
	tim_setup();
	return 0;
}
uint16_t breath_i=0;

int led_breath_task(void)
{
	TIM3_CCR1 = breath_table[breath_i];
	TIM3_CCR2 = breath_table[breath_i];
	breath_i++;
	if (breath_i > breath_len){
		breath_i = 0;
	}
	return 0;
}