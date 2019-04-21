
#include "driver_face.h"
#include "baiduiot.h"
#include "led_breath.h"
#include "sht20.h"
#include "printf.h"

int driver_init(void)
{
    printf_init();
    
    sht20_init(0);
    iot_init();
    led_breath_init();
    
    return 0;
}

int driver_task(void)
{
    sht20_task();
    iot_task();
    led_breath_task();

    return 0;
}