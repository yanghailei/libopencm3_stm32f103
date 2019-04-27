

#ifndef IIC_H
#define IIC_H

#include <stdint.h>
#include <stdlib.h>

#define SDA_H	gpio_set(GPIOB, GPIO7)
// ;GPIO_SetBits(GPIOB, GPIO_Pin_7)
#define SDA_L	gpio_clear(GPIOB, GPIO7)
// ;GPIO_ResetBits(GPIOB, GPIO_Pin_7)
#define SDA_R	gpio_get(GPIOB, GPIO7)
// GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7)

#define SCL_H	gpio_set(GPIOB, GPIO6)
// GPIO_SetBits(GPIOB, GPIO_Pin_6)
#define SCL_L	gpio_clear(GPIOB, GPIO6)
// GPIO_ResetBits(GPIOB, GPIO_Pin_6)


int soft_delay(uint32_t us);

int soft_iic1_init(void);
int soft_iic1_checkack(uint8_t dev_addr);
int soft_iic1_read(uint8_t dev_addr, uint8_t *buff, uint32_t len);
int soft_iic1_write(uint8_t dev_addr, uint8_t *buff, uint32_t len);
int soft_iic1_reg_write(uint8_t slaveAddr, uint8_t regAddr, uint8_t *buf, uint32_t len);
int soft_iic1_reg_read(uint8_t slaveAddr, uint8_t regAddr, uint8_t *buf, uint32_t len);


int hard_iic1_init(void);
int hard_iic1_read(uint8_t dev_addr, uint8_t *buff, uint32_t len);
int hard_iic1_write(uint8_t dev_addr, uint8_t *buff, uint32_t len);

float sht2x_measure(uint8_t dev_add, uint8_t cmd, uint8_t *buff);
#endif
