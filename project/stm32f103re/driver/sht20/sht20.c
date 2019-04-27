#include "iic.h"
#include "sht20.h"
#include "common.h"

sht20_t sht20;

const int16_t POLYNOMIAL = 0x131;



static int sht20_reset(void)
{
    sht20.wbuff[0] = SHT20_SOFT_RESET;
    sht20.write(sht20.dev_add, sht20.wbuff, 1);
    return 0;
}

static int sht20_read_user_reg(void)
{
    sht20.wbuff[0] = SHT20_READ_REG;
    sht20.write(sht20.dev_add, sht20.wbuff, 1);
    delay_us(10);
    sht20.read(sht20.dev_add, sht20.rbuff, 1);
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

static float sht20_calc_temperature(uint16_t u16sT)
{
	
    float temperature = 0;            // variable for result

    u16sT &= ~0x0003;           // clear bits [1..0] (status bits)
    //-- calculate temperature [癈] --
    temperature = -46.85 + 175.72 / 65536 * (float)u16sT; //T= -46.85 + 175.72 * ST/2^16
	
    return temperature;
}
static float sht20_calc_rh(uint16_t u16sRH)
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
    uint16_t tmp;
    if(sht20.read(sht20.dev_add, sht20.rbuff, 3) ==0){ 
        tmp = (sht20.rbuff[0] << 8) + sht20.rbuff[1];
        sht20.tempreture = sht20_calc_temperature(tmp);
        printf("tempreture = %0.1f\r\n", sht20.tempreture);
        sht20.mode = SHT20_MODE_T_END;
        return 0;
    }
    return -1;
}
static int sht20_get_humidity(void)
{
    unsigned short tmp;
    if(sht20.read(sht20.dev_add, sht20.rbuff, 3) ==0){
        tmp = (sht20.rbuff[0]<<8) + sht20.rbuff[1];
        sht20.humidity = sht20_calc_rh(tmp);
        printf("humidity = %0.1f\r\n", sht20.humidity);
        sht20.mode = SHT20_MODE_RH_END;
        return 0;
    }
    return -1;
}

int sht20_init(uint8_t value)
{
    soft_iic1_init();
    memset(&sht20, 0, sizeof(sht20_t));
    sht20.dev_add = SHT20_ADDRESS;
    sht20.write = soft_iic1_write;
    sht20.read = soft_iic1_read;
    sht20.regwrite = soft_iic1_reg_write;
    sht20.regread = soft_iic1_reg_read;
    return 0;
}

int sht20_task(void)
{
    sht20.measure_delay++;
    if(sht20.measure_delay % 300 == 0){
        sht20.mode = SHT20_MODE_START;
    }
    if(sht20.mode == SHT20_MODE_START){
        sht20.mode = SHT20_MODE_RH_START;
        sht20.wbuff[0] = SHT20_MEASURE_RH_NHM;
        sht20.write(sht20.dev_add, sht20.wbuff, 1);
    }else if(sht20.mode == SHT20_MODE_RH_END){
        sht20.mode = SHT20_MODE_T_START;
        sht20.wbuff[0] = SHT20_MEASURE_T_NHM;
        sht20.write(sht20.dev_add, sht20.wbuff, 1);
    }
    
    if (sht20.mode == SHT20_MODE_RH_START){
        if(sht20_get_humidity() == 0){
            sht20.mode = SHT20_MODE_RH_END;
        }
    } else if (sht20.mode == SHT20_MODE_T_START){
        if(sht20_get_tmperature() == 0){
            sht20.mode = SHT20_MODE_T_END;
        }
    }
    
    
    return 0;
}
