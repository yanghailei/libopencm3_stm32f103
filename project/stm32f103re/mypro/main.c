
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>

#include "driver_face.h"
#include "common.h"

/* Set STM32 to 72 MHz. */
static void clock_setup(void)
{
	rcc_clock_setup_in_hse_12mhz_out_72mhz();
}

int main(void)
{
    clock_setup();
    delay_setup();
    
    driver_init();
    
    while(1){
        delay_us(20000);
        driver_task();
    }
    return 0;
}