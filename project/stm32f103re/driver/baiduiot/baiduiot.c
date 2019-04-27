
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "baiduiot.h"
#include "common.h"
#include "esp8266.h"
#include "sht20.h"
#include "mqtt_client.h"

//ChinaNet-Xc2x 3nxzfxvq
#define WIFI_NAME "ChinaNet-Xc2x"
#define WIFI_PASSWORD "3nxzfxvq"
#define TCP_IP "14.215.190.20"
#define TCP_PORT "1883"

// #define MQTT_USER_NAME "n87ieud/control_livingroom"
// #define MQTT_PASSWORD "4Sg3VOvR8supOidl"

// #define PUBLISH_ADD "$baidu/iot/shadow/living_room_cp/update"

#define MQTT_USER_NAME "n87ieud/34FFDA4D4734176625"
#define MQTT_PASSWORD "j0a96svtr02dprmx"
#define MQTT_CLIENT_ID "34FFDA4D4734176625"

#define PUBLISH_ADD "$baidu/iot/shadow/34FFDA4D4734176625/update"

mqtt_t gmqtt;
baiduiot_t baiduiot;

#define PROTOCOL "{\"requestId\":\"%08u\",\"reported\":{\"temperature\":%2.0f,\"humidity\":%2.0f}}"

static int create_msg(uint8_t* pack)
{
    
    sprintf((char*)pack, PROTOCOL,
        baiduiot.requestid,
        baiduiot.tempreture,
        baiduiot.humidity);
    return 0;
}

static int iot_publish_sensor(void)
{
    uint8_t pack[200];


    memset(pack, 0, 200);
    baiduiot.requestid++;
    create_msg(pack);
    gmqtt.cfg_publish.identifier = baiduiot.requestid;
    strcpy((char*)gmqtt.cfg_publish.topic, PUBLISH_ADD);


    mqtt_publish(&gmqtt.cfg_publish, pack, strlen((char*)pack), gmqtt.send);
    return 0;
}
static int iot_port_init(void)
{
    //create tcp link
    esp8266_init((uint8_t *)WIFI_NAME, 
                    (uint8_t *)WIFI_PASSWORD,
                    (uint8_t *)TCP_IP,
                    (uint8_t *)TCP_PORT);
    gmqtt.send = esp8266_senddata;
    return 0;
}

static int iot_ping(void)
{
    mqtt_ping(gmqtt.send);
    return 0;
}



int iot_init(void)
{
    //建立TCP链接 + send port
    iot_port_init();

    //建立MQTT链接
    strcpy((char *)gmqtt.cfg_connect.client_id, MQTT_CLIENT_ID);
    strcpy((char *)gmqtt.cfg_connect.usrname, MQTT_USER_NAME);
    strcpy((char *)gmqtt.cfg_connect.password, MQTT_PASSWORD);
    gmqtt.cfg_connect.keepliave = 300;
    gmqtt.cfg_connect.protocol_level = 4;
    mqtt_connect(&gmqtt.cfg_connect, gmqtt.send);
    delay_ms(500);
    return 0;
}

int iot_task(void)
{
	float tmp_1, tmp_2;
    extern sht20_t sht20;
    if (baiduiot.task_delay < 1000) {
        baiduiot.task_delay++;
        return 0;
    }
    baiduiot.task_delay = 0;
	tmp_1 = fabs(baiduiot.humidity - sht20.humidity);
	tmp_2 = fabs(baiduiot.tempreture - sht20.tempreture);
	printf("%f\n", tmp_1);
    if ((tmp_1 > 0.2) || ( tmp_2 > 0.2)) {
        baiduiot.humidity = sht20.humidity;
        baiduiot.tempreture = sht20.tempreture;
        iot_publish_sensor();
    }else{
		iot_ping();
	}
	return 0;
    
}

