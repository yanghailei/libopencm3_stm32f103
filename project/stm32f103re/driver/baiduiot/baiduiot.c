
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "baiduiot.h"
#include "common.h"
#include "esp8266.h"
#include "mqtt_client.h"
#include "sht20.h"

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
    if (mqtt_publish(&gmqtt, pack, strlen((char*)pack)) != 0) {
        printf("%s-%d: mqtt_publish error\n", __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}
static int iot_port_init(void)
{
    //create tcp link
    esp8266_init((uint8_t*)WIFI_NAME,
        (uint8_t*)WIFI_PASSWORD,
        (uint8_t*)TCP_IP,
        (uint8_t*)TCP_PORT);
    gmqtt.send = esp8266_senddata;
    gmqtt.wait_max = 3;
    mqtt_init(&gmqtt);
    return 0;
}

static int iot_ping(void)
{
    if (mqtt_ping(&gmqtt) != 0) {
        printf("%s-%d: mqtt_ping error\n", __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

static int iot_recv(uint8_t* buff, uint32_t* len)
{
    esp8266_recvdata(buff, len);
    return 0;
}

int iot_init(void)
{
    memset(&gmqtt, 0, sizeof(gmqtt));
    //建立TCP链接 + send port
    iot_port_init();

    //建立MQTT链接
    strcpy((char*)gmqtt.cfg_connect.client_id, MQTT_CLIENT_ID);
    strcpy((char*)gmqtt.cfg_connect.usrname, MQTT_USER_NAME);
    strcpy((char*)gmqtt.cfg_connect.password, MQTT_PASSWORD);
    gmqtt.cfg_connect.keepliave = 300;
    gmqtt.cfg_connect.protocol_level = 4;
    if (mqtt_connect(&gmqtt) == 0) {
        printf("%s-%d: mqtt_connect success\n", __FUNCTION__, __LINE__);
        baiduiot.link = 1;
    }

    return 0;
}

static int iot_relink(void)
{
    if (gmqtt.link_status & MQTT_LINK_ERROR) {
        if (iot_init() == 0) {
            printf("%s-%d: iot init success\n", __FUNCTION__, __LINE__);
            baiduiot.link = 1;
            gmqtt.link_status = 0;
        } else {
            printf("%s-%d: iot init error\n", __FUNCTION__, __LINE__);
            return -1;
        }
    } else if (gmqtt.link_status & MQTT_LINK_BREAK) {
        if (mqtt_connect(&gmqtt) == 0) {
            printf("%s-%d: mqtt_connect success\n", __FUNCTION__, __LINE__);
            baiduiot.link = 1;
            gmqtt.link_status = 0;
        } else {
            printf("%s-%d: mqtt_connect error\n", __FUNCTION__, __LINE__);
            return -1;
        }
    }
    return 0;
}

int iot_task(void)
{
    uint8_t buff[256];
    uint32_t len;
    float tmp_1, tmp_2;
    extern sht20_t sht20;
    if (baiduiot.task_delay < 1000) {
        baiduiot.task_delay++;
        return 0;
    }
    baiduiot.task_delay = 0;
    tmp_1 = fabs(baiduiot.humidity - sht20.humidity);
    tmp_2 = fabs(baiduiot.tempreture - sht20.tempreture);
    // printf("%f\n", tmp_1);
    iot_recv(buff, &len);
    if (len) {
        mqtt_ack(&gmqtt, buff, len);
    }
    if (baiduiot.link == 0) {
        printf("%s-%d: retry create link\n", __FUNCTION__, __LINE__);
        if (iot_relink() != 0) {
            return -1;
        }
    }
    if ((tmp_1 > 0.2) || (tmp_2 > 0.2)) {
        baiduiot.humidity = sht20.humidity;
        baiduiot.tempreture = sht20.tempreture;
        if (iot_publish_sensor() != 0) {
            baiduiot.link = 0;
        }
    } else {
        if (iot_ping() != 0) {
            baiduiot.link = 0;
        }
    }
    return 0;
}
