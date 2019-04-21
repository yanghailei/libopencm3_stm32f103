
#ifndef __SHT20_H
#define __SHT20_H

#include <stdint.h>
#include <stdlib.h>

typedef enum{
    SHT20_MODE_START = 0,
    SHT20_MODE_RH_START,
    SHT20_MODE_T_START,
    SHT20_MODE_RH_END,
    SHT20_MODE_T_END,
}sht20_mode;

typedef struct _sht20_t{
    uint8_t dev_add;
    sht20_mode mode;
    uint16_t measure_delay;
    uint8_t rbuff[10];
    uint8_t wbuff[10];
    int (*read)(uint8_t add, uint8_t *buff, uint32_t len);
    int (*write)(uint8_t add, uint8_t *buff, uint32_t len);
    int (*regread)(uint8_t add, uint8_t reg, uint8_t *buff, uint32_t len);
    int (*regwrite)(uint8_t add, uint8_t reg, uint8_t *buff, uint32_t len);
    float tempreture;
	float humidity;
}sht20_t;

/*SHT20 设备操作相关宏定义，详见手册*/
#define SHT20_ADDRESS  0X40
#define SHT20_MEASURE_RH_HM  0XE5
#define SHT20_MEASURE_T_HM  0XE3
#define SHT20_MEASURE_RH_NHM  0XF5
#define SHT20_MEASURE_T_NHM  0XF3
#define SHT20_READ_REG  0XE7
#define SHT20_WRITE_REG  0XE6
#define SHT20_SOFT_RESET  0XFE


int sht20_init(uint8_t value);
int sht20_task(void);


#endif

