
#ifndef __MQTT_CLIENT_H_
#define __MQTT_CLIENT_H_

#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#define MQTT_PACK_MAX_LEN   500
/* MQTT Control Packet type */
enum {
    MQTT_CONTROL_TYPE_NULL  = 0<<4,
    MQTT_CONNECT            = 1<<4,       /* Client request to connect to Server */
    MQTT_CONNACK            = 2<<4,       /* Connect acknowledgment */
    MQTT_PUBLISH            = 3<<4,      /* Publish message */
    MQTT_PUBACK             = 4<<4,       /* Publish acknowledgment */
    MQTT_PUBREC             = 5<<4,       /* Publish received (assured delivery part 1) */
    MQTT_PUBREL             = 6<<4,       /* Publish release (assured delivery part 2) */
    MQTT_PUBCOMP            = 7<<4,      /* Publish complete (assured delivery part 3) */
    MQTT_SUBSCRIBE          = 8<<4,    /* Client subscribe request */
    MQTT_SUBACK             = 9<<4,       /* Subscribe acknowledgment */
    MQTT_UNSUBSCRIBE        = 10<<4,  /* Unsubscribe request */
    MQTT_UNSUBACK           = 11<<4,     /* Unsubscribe acknowledgment */
    MQTT_PINGREQ            = 12<<4,      /* PING request */
    MQTT_PINGRESP           = 13<<4,     /* PING response */
    MQTT_DISCONNECT         = 14<<4,    /* Client is disconnecting */
};


/* Connect Return code */
enum {
    MQTT_CONNECTION_ACCEPTED,      /* Connection accepted */
    MQTT_UNACCEPTABLE_PROTOCOL,    /* Connection Refused, unacceptable protocol version */
    MQTT_IDENTIFIER_REJECTED,      /* Connection Refused, identifier rejected */
    MQTT_SERVER_UNAVAILABLE,       /* Connection Refused, Server unavailable */
    MQTT_BAD_USERNAME_OR_PASSWORD, /* Connection Refused, bad user name or password */
    MQTT_NOT_AUTHORIZED            /* Connection Refused, not authorized */
};



enum {
    MQTT_QOS0   = 0,
    MQTT_QOS1   = 1,
    MQTT_QOS2   = 2,
    MQTT_QOSMAX,
};

enum {
    MQTT_LEVEL_0   = 0,
    MQTT_LEVEL_1   = 1,
    MQTT_LEVEL_2   = 2,
    MQTT_LEVEL_3   = 3,
    MQTT_LEVEL_4   = 4,
    MQTT_LEVEL_5   = 5,
};


enum {
    MQTT_USERNAME_FLAG      = 1<<7,
    MQTT_PASSWORD_FLAG      = 1<<6,
    MQTT_WRETAIN_FLAG       = 1<<5,
    MQTT_WQOS_FLAG          = 3<<4,
    MQTT_WILL_FLAG          = 1<<2,
    MQTT_CLSESSION_FLAG     = 1<<1,
};

enum {
    MQTT_DUP      = 1<<3,
    MQTT_QOS_0    = 0<<1,
    MQTT_QOS_1    = 1<<1,
    MQTT_QOS_2    = 2<<1,
};

enum {
    MQTT_LINK_OK        = 0,
    MQTT_LINK_ERROR     = 1,
    MQTT_LINK_BREAK     = 1<<4,
};

typedef struct{
    uint8_t * buff;
    uint32_t len;
    uint32_t maxlen;
}mqtt_pack_t;

typedef struct{
    uint8_t client_id[50];
    uint8_t usrname[50];
    uint8_t password[50];
    uint8_t protocol_level;
    uint8_t clean_session;
    uint8_t wqos;
    uint8_t wretain;
    uint8_t wflag;
    uint32_t keepliave;
}mqtt_cfg_connect_t;

typedef struct{
    uint8_t qos;
    uint8_t dup;
    uint32_t identifier;
    uint8_t  topic[200];
}mqtt_cfg_publish_t;



typedef struct {
    uint8_t wait_id;
    uint8_t wait_num;
    uint8_t buff[10];
}mqtt_wait_ack_t;

typedef struct {
    uint8_t status;
    uint32_t wait_sum;
    uint32_t wait_max_sum;
    mqtt_wait_ack_t ping;
    mqtt_wait_ack_t pub;
    mqtt_wait_ack_t connect;
}mqtt_link_t;

typedef struct{
    mqtt_cfg_connect_t cfg_connect;
    mqtt_cfg_publish_t cfg_publish;
    uint32_t wait_max;
    uint8_t link_status;
    int (* send)(uint8_t *pack, uint32_t len);
}mqtt_t;

int mqtt_init(mqtt_t *mqtt);
int mqtt_connect(mqtt_t *mqtt);
int mqtt_publish(mqtt_t *mqtt,uint8_t *msg, uint32_t msglen);
int mqtt_ping(mqtt_t *mqtt);

int mqtt_ack(mqtt_t *mqtt, uint8_t *buff, uint32_t len);

#endif
