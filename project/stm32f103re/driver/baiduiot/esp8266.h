#ifndef __ESP8266_H
#define __ESP8266_H

#include <stdint.h>
#include <stdlib.h>

int esp8266_senddata(uint8_t* buf, uint32_t count);
int esp8266_recvdata(uint8_t* buf, uint32_t *count);
int esp8266_init(uint8_t* wifi_name,
    uint8_t* wifi_password,
    uint8_t* tcp_ip,
    uint8_t* tcp_port);

#endif