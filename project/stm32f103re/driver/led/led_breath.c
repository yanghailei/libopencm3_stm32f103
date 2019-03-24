

#include <stdlib.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include "led_breath.h"


#ifndef ARRAY_LEN
#define ARRAY_LEN(array) (sizeof((array))/sizeof((array)[0]))
#endif

#define LED1_PORT GPIOA
#define LED1_PIN GPIO6

/* Morse standard timings */
#define ELEMENT_TIME 500
#define DIT (1*ELEMENT_TIME)
#define DAH (3*ELEMENT_TIME)
#define INTRA (1*ELEMENT_TIME)
#define INTER (3*ELEMENT_TIME)
#define WORD (7*ELEMENT_TIME)

uint16_t frequency_sequence[] = {
	DIT,
	INTRA,
	DIT,
	INTRA,
	DIT,
	INTER,
	DAH,
	INTRA,
	DAH,
	INTRA,
	DAH,
	INTER,
	DIT,
	INTRA,
	DIT,
	INTRA,
	DIT,
	WORD,
};

int frequency_sel = 0;



static void gpio_setup(void)
{
	/* Enable GPIO clock for leds. */
	rcc_periph_clock_enable(RCC_GPIOA);

	/* Enable led as output */
	gpio_set_mode(LED1_PORT, GPIO_MODE_OUTPUT_50_MHZ,
		GPIO_CNF_OUTPUT_PUSHPULL, LED1_PIN);
	gpio_set(LED1_PORT, LED1_PIN);

	/* Set GPIO12 (in GPIO port C) to 'output push-pull'. */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO4);

	/* Set GPIO13 (in GPIO port B) to 'output push-pull'. */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO5);
}

static void tim_setup(void)
{
	/* Enable TIM3 clock. */
	rcc_periph_clock_enable(RCC_TIM3);

	/* Enable TIM3 interrupt. */
	nvic_enable_irq(NVIC_TIM3_IRQ);

	/* Reset TIM3 peripheral to defaults. */
	rcc_periph_reset_pulse(RST_TIM3);

	/* Timer global mode:
	 * - No divider
	 * - Alignment edge
	 * - Direction up
	 * (These are actually default values after reset above, so this call
	 * is strictly unnecessary, but demos the api for alternative settings)
	 */
	timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT,
		TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

	/*
	 * Please take note that the clock source for STM32 timers
	 * might not be the raw APB1/APB2 clocks.  In various conditions they
	 * are doubled.  See the Reference Manual for full details!
	 * In our case, TIM3 on APB1 is running at double frequency, so this
	 * sets the prescaler to have the timer run at 5kHz
	 */
	timer_set_prescaler(TIM3, ((rcc_apb1_frequency * 2) / 5000));

	/* Disable preload. */
	timer_disable_preload(TIM3);
	timer_continuous_mode(TIM3);

	/* count full range, as we'll update compare value continuously */
	timer_set_period(TIM3, 65535);

	/* Set the initual output compare value for OC1. */
	timer_set_oc_value(TIM3, TIM_OC1, frequency_sequence[frequency_sel++]);

	/* Counter enable. */
	timer_enable_counter(TIM3);
	// timer_enable_oc_output(TIM3, TIM_OC1);
	/* Enable Channel 1 compare interrupt to recalculate compare values */
	timer_enable_irq(TIM3, TIM_DIER_CC1IE);
}

void tim3_isr(void)
{
	if (timer_get_flag(TIM3, TIM_SR_CC1IF)) {

		/* Clear compare interrupt flag. */
		timer_clear_flag(TIM3, TIM_SR_CC1IF);

		/*
		 * Get current timer value to calculate next
		 * compare register value.
		 */
		uint16_t compare_time = timer_get_counter(TIM3);

		/* Calculate and set the next compare value. */
		uint16_t frequency = frequency_sequence[frequency_sel++];
		uint16_t new_time = compare_time + frequency;

		timer_set_oc_value(TIM3, TIM_OC1, new_time);
		if (frequency_sel == ARRAY_LEN(frequency_sequence)) {
			frequency_sel = 0;
		}

		/* Toggle LED to indicate compare event. */
		gpio_toggle(LED1_PORT, LED1_PIN);
	}
}

int led_breath_init(void)
{
	gpio_setup();
	tim_setup();
	return 0;
}

int led_breath_task(void)
{
	gpio_toggle(GPIOA, GPIO4);	/* LED on/off */
	return 0;
}