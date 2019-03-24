
#include "driver_face.h"
#include "led_breath.h"
#include "printf.h"

int driver_init(void)
{
    led_breath_init();
    printf_init();
    return 0;
}

int driver_task(void)
{
    led_breath_task();
    return 0;
}