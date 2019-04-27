
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/i2c.h>

#include "common.h"
#include "iic.h"


#define M_IIC I2C1
#define DELAY 20




static int iic1_gpio_init(void)
{
	rcc_periph_clock_enable(RCC_GPIOB);

	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_OPENDRAIN,
		      GPIO6 | GPIO7);
	return 0;
}



static void iic_start(void)
{
	
	SDA_H;						//拉高SDA线
	SCL_H;						//拉高SCL线
	delay_us(DELAY);		//延时，速度控制

	SDA_L;						//当SCL线为高时，SDA线一个下降沿代表开始信号
	delay_us(DELAY);		//延时，速度控制
	SCL_L;						//钳住SCL线，以便发送数据
}

static void iic_stop(void)
{
	SDA_L;						//拉低SDA线
	SCL_L;						//拉低SCL先
	delay_us(DELAY);		//延时，速度控制
	
	SCL_H;						//拉高SCL线
	SDA_H;						//拉高SDA线，当SCL线为高时，SDA线一个上升沿代表停止信号
	delay_us(DELAY);
}

static uint8_t iic_wack(unsigned int timeOut)
{
	SDA_H;delay_us(DELAY);			//拉高SDA线
	SCL_H;delay_us(DELAY);			//拉高SCL线
	
	while(SDA_R){
		if(--timeOut){
			iic_stop();						//超时未收到应答，则停止总线	
			return -1;					//返回失败
		}
		
		delay_us(DELAY);
	}
	SCL_L;									//拉低SCL线，以便继续收发数据
	return 0;							//返回成功
	
}

static void iic_ack(void)
{
	
	SCL_L;						//拉低SCL线
	SDA_L;						//拉低SDA线
	delay_us(DELAY);
	SCL_H;						//拉高SCL线
	delay_us(DELAY);
	SCL_L;						//拉低SCL线
	
}


static void iic_nack(void)
{
	SCL_L;						//拉低SCL线
	SDA_H;						//拉高SDA线
	delay_us(DELAY);
	SCL_H;						//拉高SCL线
	delay_us(DELAY);
	SCL_L;						//拉低SCL线
	
}


static void iic_sendbyte(uint8_t byte)
{
	uint8_t count = 0;
    SCL_L;							//拉低时钟开始数据传输
    for(; count < 8; count++){
		if(byte & 0x80)				//发送最高位
			SDA_H;
		else
			SDA_L;
		byte <<= 1;					//byte左移1位
		delay_us(DELAY);
		SCL_H;
		delay_us(DELAY);
		SCL_L;
    }
}

static uint8_t iic_recvbyte(void)
{
	
	uint8_t count = 0, receive = 0;
	SDA_H;							//拉高SDA线，开漏状态下，需线拉高以便读取数据
	
    for(; count < 8; count++ ){
		SCL_L;
		delay_us(DELAY);
		SCL_H;
        receive <<= 1;				//左移一位
        if(SDA_R)					//如果SDA线为1，则receive变量自增，每次自增都是对bit0的+1，然后下一次循环会先左移一次
			receive++;
		delay_us(DELAY);
    }
    return receive;
}



int soft_iic1_init(void)
{
	iic1_gpio_init();
	SDA_H;
	SCL_H;
	return 0;
} 


int soft_iic1_write(uint8_t dev_addr, uint8_t *buff, uint32_t len)
{
	uint8_t addr = 0;
	addr = dev_addr << 1;		//IIC地址是7bit，这里需要左移1位，bit0：1-读	0-写
	iic_start();				//起始信号
	iic_sendbyte(addr);			//发送设备地址(写)
	if(iic_wack(500))		//等待应答
		return -1;
	
	while(len--) {
		// printf("%s-%d:%#x\n", __FUNCTION__, __LINE__, *buff);
		iic_sendbyte(*buff);		//发送数据
		if(iic_wack(500))	//等待应答
			return -1;
		buff++;					//数据指针偏移到下一个
		delay_us(10);
	}
	// iic_stop();					//停止信号
	return 0;
}

int soft_iic1_read(uint8_t dev_addr, uint8_t *buff, uint32_t len)
{
	uint16_t over_time = 0;
	uint8_t addr = 0;
	addr = dev_addr << 1; //IIC地址是7bit，这里需要左移1位，bit0：1-读	0-写
	iic_start(); //重启信号

	iic_sendbyte(addr + 1); //发送设备地址(读)
	while(iic_wack(500)) //等待应答
	{
		iic_start();
		iic_sendbyte(addr + 1);
		over_time++;
		if(over_time > 10){
			return -1;
		}
	}

	while (len--) {
		*buff = iic_recvbyte();
		// printf("%s-%d:%#x\n", __FUNCTION__, __LINE__, *buff);
		buff++; //偏移到下一个数据存储地址

		if (len == 0) {
			iic_nack(); //最后一个数据需要回NOACK
		} else {
			iic_ack(); //回应ACK
		}
	}
	iic_stop();
	return 0;
}
int soft_iic1_reg_write(uint8_t dev_addr, uint8_t reg, uint8_t *buff, uint32_t len)
{
	uint8_t addr = 0;
	addr = dev_addr << 1;		//IIC地址是7bit，这里需要左移1位，bit0：1-读	0-写
	iic_start();				//起始信号
	iic_sendbyte(addr);			//发送设备地址(写)
	if(iic_wack(5000))		//等待应答
		return -1;
	
	iic_sendbyte(reg);		//发送寄存器地址
	if(iic_wack(5000))		//等待应答
		return -1;
	
	while(len--) {
		iic_sendbyte(*buff);		//发送数据
		if(iic_wack(5000))	//等待应答
			return -1;
		
		buff++;					//数据指针偏移到下一个
		
		delay_us(10);
	}
	iic_stop();					//停止信号
	return 0;
}


int soft_iic1_reg_read(uint8_t dev_addr, uint8_t reg, uint8_t *buff, uint32_t len)
{
	uint8_t addr = 0;
	addr = dev_addr << 1; //IIC地址是7bit，这里需要左移1位，bit0：1-读	0-写
	iic_start();		   //起始信号

	iic_sendbyte(addr);	//发送设备地址(写)
	if (iic_wack(5000)) //等待应答
		return -1;

	iic_sendbyte(reg); //发送寄存器地址
	if (iic_wack(5000)) //等待应答
		return -1;

	iic_start(); //重启信号

	iic_sendbyte(addr + 1); //发送设备地址(读)
	if (iic_wack(5000))  //等待应答
		return -1;

	while (len--) {
		*buff = iic_recvbyte();
		buff++; //偏移到下一个数据存储地址

		if (len == 0) {
			iic_nack(); //最后一个数据需要回NOACK
		} else {
			iic_ack(); //回应ACK
		}
	}
	iic_stop();
	return 0;
}

int hard_iic1_init(void)
{
	/* Enable clocks for I2C2 and AFIO. */
	rcc_periph_clock_enable(M_IIC);
	rcc_periph_clock_enable(RCC_AFIO);

	iic1_gpio_init();
	/* Disable the I2C before changing any configuration. */
	i2c_peripheral_disable(M_IIC);

	/* APB1 is running at 36MHz. */
	i2c_set_clock_frequency(M_IIC, I2C_CR2_FREQ_36MHZ);

	/* 400KHz - I2C Fast Mode */
	i2c_set_fast_mode(M_IIC);

	/*
	 * fclock for I2C is 36MHz APB2 -> cycle time 28ns, low time at 400kHz
	 * incl trise -> Thigh = 1600ns; CCR = tlow/tcycle = 0x1C,9;
	 * Datasheet suggests 0x1e.
	 */
	i2c_set_ccr(M_IIC, 0x1e);

	/*
	 * fclock for I2C is 36MHz -> cycle time 28ns, rise time for
	 * 400kHz => 300ns and 100kHz => 1000ns; 300ns/28ns = 10;
	 * Incremented by 1 -> 11.
	 */
	i2c_set_trise(M_IIC, 0x0b);

	/*
	 * This is our slave address - needed only if we want to receive from
	 * other masters.
	 */
	i2c_set_own_7bit_slave_address(M_IIC, 0x32);

	/* If everything is configured -> enable the peripheral. */
	i2c_peripheral_enable(M_IIC);
}

int hard_iic1_read(uint8_t dev_addr, uint8_t *buff, uint32_t len)
{
	uint32_t reg32 __attribute__((unused));
	/* Send START condition. */
	i2c_send_start(M_IIC);

	/* Waiting for START is send and switched to master mode. */
	while (!((I2C_SR1(M_IIC) & I2C_SR1_SB)
		& (I2C_SR2(M_IIC) & (I2C_SR2_MSL | I2C_SR2_BUSY))));

	/* Say to what address we want to talk to. */
	i2c_send_7bit_address(M_IIC, dev_addr, I2C_READ); 

	/* 2-byte receive is a special case. See datasheet POS bit. */
	I2C_CR1(M_IIC) |= (I2C_CR1_POS | I2C_CR1_ACK);

	/* Waiting for address is transferred. */
	while (!(I2C_SR1(M_IIC) & I2C_SR1_ADDR));

	/* Cleaning ADDR condition sequence. */
	reg32 = I2C_SR2(M_IIC);

	/* Cleaning I2C_SR1_ACK. */
	I2C_CR1(M_IIC) &= ~I2C_CR1_ACK;

	/* Now the slave should begin to send us the first byte. Await BTF. */
	while (!(I2C_SR1(M_IIC) & I2C_SR1_BTF));
	buff[0] = I2C_DR(M_IIC); /* MSB */

	/*
	 * Yes they mean it: we have to generate the STOP condition before
	 * saving the 1st byte.
	 */
	I2C_CR1(M_IIC) |= I2C_CR1_STOP;

	buff[1] = I2C_DR(M_IIC); /* LSB */

	/* Original state. */
	I2C_CR1(M_IIC) &= ~I2C_CR1_POS;
	return 0;
} 

int hard_iic1_write(uint8_t dev_addr, uint8_t *buff, uint32_t len)
{
	uint32_t reg32 __attribute__((unused));

	/* Send START condition. */
	i2c_send_start(M_IIC);

	/* Waiting for START is send and switched to master mode. */
	while (!((I2C_SR1(M_IIC) & I2C_SR1_SB)
		& (I2C_SR2(M_IIC) & (I2C_SR2_MSL | I2C_SR2_BUSY))));

	/* Send destination address. */
	i2c_send_7bit_address(M_IIC, dev_addr, I2C_WRITE);

	/* Waiting for address is transferred. */
	while (!(I2C_SR1(M_IIC) & I2C_SR1_ADDR));

	/* Cleaning ADDR condition sequence. */
	reg32 = I2C_SR2(M_IIC);

	/* Sending the data. */
	while( len > 1){
		i2c_send_data(M_IIC, *buff); /* stts75 config register */
		while (!(I2C_SR1(M_IIC) & I2C_SR1_BTF)); /* Await ByteTransferedFlag. */
		len--;
		buff++;
	}
	/* Polarity reverse - LED glows if temp is below Tos/Thyst. */
	i2c_send_data(M_IIC, *buff);
	while (!(I2C_SR1(M_IIC) & (I2C_SR1_BTF | I2C_SR1_TxE)));

	/* Send STOP condition. */
	i2c_send_stop(M_IIC);
	return 0;
}

float sht2x_measure(uint8_t dev_add, uint8_t cmd, uint8_t *buff)
{

	uint8_t addr = 0;
    float t = 0;
	
    addr = dev_add << 1;
	
	iic_start();
	
	iic_sendbyte(addr);
	if(iic_wack(50000)) //等待应答
		return 0.0;
	
	iic_sendbyte(cmd);
	if(iic_wack(50000)) //等待应答
		return 0.0;
	
	iic_start();
	
	iic_sendbyte(addr + 1);
	while(iic_wack(50000)) //等待应答
	{
		iic_start();
		iic_sendbyte(addr + 1);
	}
	
	delay_us(30000);
	delay_us(30000);
	delay_us(30000);
	
	buff[0] = iic_recvbyte();
	iic_ack();
	buff[1] = iic_recvbyte();
	iic_ack();
	
	buff[2] = iic_recvbyte();
	iic_nack();
	
	iic_stop();
	
	// SHT2x_CheckCrc(data, 2, checksum);
    // tmp = (data[0] << 8) + data[1];
    // if(cmd == SHT20_Measurement_T_HM)
    // {
    //     t = SHT2x_CalcTemperatureC(tmp);
    // }
    // else
    // {
    //     t = SHT2x_CalcRH(tmp);
    // }
	
    // if(pMeasurand)
    // {
    //     *pMeasurand = (unsigned short)t;
    // }
	
    return t;
	
}