#include "sht20.h"

sht20_t sht20;

static int sht20_reset(void)
{
    sht20.wbuff[0] = SHT20_SOFT_RESET;
    sht20->write(sht20.dev_add, sht20.wbuff, 1);
    return 0;
}

static int sht20_read_user_reg(void)
{
    sht20.wbuff[0] = SHT20_READ_REG;
    sht20->write(sht20.dev_add, sht20.wbuff, 1);

    sht20->read(sht20.dev_add, sht20.rbuff, 1);
    return 0;
}

static int sht20_checkcrc(char data[], char nbrOfBytes, char checksum)
{
    char crc = 0;
    char bit = 0;
    char byteCtr = 0;
	
    //calculates 8-Bit checksum with given polynomial
    for(byteCtr = 0; byteCtr < nbrOfBytes; ++byteCtr)
    {
        crc ^= (data[byteCtr]);
        for ( bit = 8; bit > 0; --bit)
        {
            if (crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
            else crc = (crc << 1);
        }
    }
	
    if(crc != checksum)
		return 1;
    else
		return 0;
}

static float sht20_calc_temperature(unsigned short u16sT)
{
	
    float temperature = 0;            // variable for result

    u16sT &= ~0x0003;           // clear bits [1..0] (status bits)
    //-- calculate temperature [癈] --
    temperature = -46.85 + 175.72 / 65536 * (float)u16sT; //T= -46.85 + 175.72 * ST/2^16
	
    return temperature;
}
static float sht20_calc_rh(unsigned short u16sRH)
{
	
    float humidity = 0;              // variable for result
	
    u16sRH &= ~0x0003;          // clear bits [1..0] (status bits)
    //-- calculate relative humidity [%RH] --
    //humidityRH = -6.0 + 125.0/65536 * (float)u16sRH; // RH= -6 + 125 * SRH/2^16
    humidity = ((float)u16sRH * 0.00190735) - 6;
	
    return humidity;
}

static int sht20_get_tmperature(void)
{
    sht20.wbuff[0] = SHT20_MEASURE_T_HM;
    sht20->write(sht20.dev_add, sht20.wbuff, 1);

    sht20->read(sht20.dev_add, sht20.rbuff, 3);
    sht20.tempreture = sht20_calc_temperature(sht20.rbuff, 2, &sht20.rbuff[2])
}
static int sht20_get_humidity(void)
{
    unsigned short tmp;
    sht20.wbuff[0] = SHT20_MEASURE_RH_HM;
    sht20->write(sht20.dev_add, sht20.wbuff, 1);

    sht20->read(sht20.dev_add, sht20.rbuff, 3);
    tmp = (sht20.rbuff[0]<<8) + sht20.rbuff[1];
    sht20.humidity = sht20_calc_rh(tmp);
}

int sht20_init(uint8_t value)
{
    memset(&sht20, 0, sizeof(sht20_t));
    sht20.dev_add = SHT20_ADDRESS;

}

int sht20_task(void)
{

}
