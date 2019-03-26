
#ifndef __SHT20_H
#define __SHT20_H

typedef struct _sht20_t{
    uint8_t dev_add;
    uint8_t rbuff[10];
    uint8_t wbuff[10];
    int *read(uint8_t add, uint8_t *buff, uint32_t len);
    int *write(uint8_t add, uint8_t *buff, uint32_t len);
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





#endif

