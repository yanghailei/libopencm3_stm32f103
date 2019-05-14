
#include "mqtt_client.h"

static int mqtt_malloc(mqtt_pack_t* pack, mqtt_pack_t* vhead, mqtt_pack_t* payload)
{
    if (pack == NULL) {
        return -1;
    }
    pack->maxlen = MQTT_PACK_MAX_LEN + 10;
    pack->buff = malloc(pack->maxlen);
    if (pack->buff == NULL) {
        free(pack->buff);
        return -1;
    }
    memset(pack->buff, 0, pack->maxlen);

    if (vhead != NULL) {
        vhead->maxlen = MQTT_PACK_MAX_LEN / 2;
        vhead->buff = malloc(vhead->maxlen);
        if (vhead->buff == NULL) {
            free(pack->buff);
            free(vhead->buff);
            return -1;
        }
        memset(vhead->buff, 0, vhead->maxlen);
    }

    if (payload != NULL) {
        payload->maxlen = MQTT_PACK_MAX_LEN / 2;
        payload->buff = malloc(payload->maxlen);
        if (payload->buff == NULL) {
            free(pack->buff);
            if (vhead != NULL) {
                free(vhead->buff);
            }
            free(payload->buff);
            return -1;
        }
        memset(payload->buff, 0, payload->maxlen);
    }

    return 0;
}

static int mqtt_free(mqtt_pack_t* pack, mqtt_pack_t* vhead, mqtt_pack_t* payload)
{
    if (pack == NULL) {
        return -1;
    }
    free(pack->buff);

    if (vhead != NULL) {
        free(vhead->buff);
    }

    if (payload != NULL) {
        free(payload->buff);
    }
    return 0;
}

static int mqtt_connect_vhead_mqtt_protocol(mqtt_cfg_connect_t* mqtt_cfg, mqtt_pack_t* vhead)
{
    uint8_t* pstrtmp = NULL;
    uint32_t len = 0;
    if (10 > vhead->maxlen) {
        return -1;
    }
    if (mqtt_cfg->protocol_level != 4) {
        return -1;
    }
    pstrtmp = vhead->buff;

    pstrtmp[len++] = 0;
    pstrtmp[len++] = strlen("MQTT");
    memcpy(&pstrtmp[len], "MQTT", strlen("MQTT"));
    len += strlen("MQTT");

    pstrtmp[len++] = MQTT_LEVEL_4;

    pstrtmp[len++] = MQTT_USERNAME_FLAG | MQTT_PASSWORD_FLAG | MQTT_CLSESSION_FLAG;
    pstrtmp[len++] = mqtt_cfg->keepliave >> 8;
    pstrtmp[len++] = mqtt_cfg->keepliave;
    if (len != 10) {
        return -1;
    }
    vhead->len = len;
    return 0;
}
static int mqtt_connect_vhead_get(mqtt_cfg_connect_t* mqtt_cfg, mqtt_pack_t* vhead)
{
    uint32_t protocol_level = 0;

    protocol_level = mqtt_cfg->protocol_level;
    if (protocol_level != MQTT_LEVEL_4) {
        return -1;
    }
    mqtt_connect_vhead_mqtt_protocol(mqtt_cfg, vhead);

    return 0;
}

static int mqtt_connect_payload_get(mqtt_cfg_connect_t* mqtt_cfg, mqtt_pack_t* payload)
{
    uint8_t* pstrtmp = NULL;
    uint32_t client_id_len = 0;
    uint32_t usrname_len = 0;
    uint32_t password_len = 0;
    uint32_t payload_len = 0;
    uint32_t len = 0;

    client_id_len = strlen((char*)mqtt_cfg->client_id);
    usrname_len = strlen((char*)mqtt_cfg->usrname);
    password_len = strlen((char*)mqtt_cfg->password);
    if ((client_id_len == 0)
        || (usrname_len == 0)
        || (password_len == 0)) {
        return -1;
    }

    payload_len = client_id_len + client_id_len + password_len + 6;

    if (payload_len > payload->maxlen) {
        return -1;
    }

    pstrtmp = payload->buff;

    pstrtmp[len++] = client_id_len >> 8;
    pstrtmp[len++] = client_id_len;
    memcpy(&pstrtmp[len], mqtt_cfg->client_id, client_id_len);
    len += client_id_len;

    pstrtmp[len++] = usrname_len >> 8;
    pstrtmp[len++] = usrname_len;
    memcpy(&pstrtmp[len], mqtt_cfg->usrname, usrname_len);
    len += usrname_len;

    pstrtmp[len++] = password_len >> 8;
    pstrtmp[len++] = password_len;
    memcpy(&pstrtmp[len], mqtt_cfg->password, password_len);
    len += password_len;

    payload->len = len;

    return 0;
}

static int mqtt_publish_payload_get(mqtt_pack_t* msg, mqtt_pack_t* payload)
{
    uint8_t* pstrtmp = NULL;
    uint32_t len = 0;

    pstrtmp = payload->buff;

    // pstrtmp[len++] = msg->len >> 8;
    // pstrtmp[len++] = msg->len;
    memcpy(&pstrtmp[len], msg->buff, msg->len);
    len += msg->len;

    payload->len = len;

    return 0;
}

static int mqtt_publish_vhead_get(mqtt_cfg_publish_t* mqtt_cfg, mqtt_pack_t* vhead)
{
    uint8_t* pstrtmp = NULL;
    uint32_t topic_len = 0;
    uint32_t payload_len = 0;
    uint32_t len = 0;

    topic_len = strlen((char*)mqtt_cfg->topic);
    payload_len = 2 + topic_len + 2;
    if (payload_len > vhead->maxlen) {
        return -1;
    }

    pstrtmp = vhead->buff;

    pstrtmp[len++] = topic_len >> 8;
    pstrtmp[len++] = topic_len;
    memcpy(&pstrtmp[len], mqtt_cfg->topic, topic_len);
    len += topic_len;
    pstrtmp[len++] = mqtt_cfg->identifier >> 8;
    pstrtmp[len++] = mqtt_cfg->identifier;
    vhead->len = len;

    return 0;
}

mqtt_link_t link;
int mqtt_init(mqtt_t* mqtt)
{
    uint32_t tmp;
    tmp = link.connect.wait_num;
    memset(&link, 0, sizeof(link));
    link.wait_max_sum = mqtt->wait_max;
    link.connect.wait_num = tmp;
    return 0;
}
/*
Must fill with clientid, name and password
*/
int mqtt_connect(mqtt_t* mqtt)
{
    mqtt_cfg_connect_t* mqtt_cfg;
    mqtt_pack_t pack, mqtt_vhead, mqtt_payload;
    uint32_t lastlen = 0;

    mqtt_cfg = &mqtt->cfg_connect;

    if (link.connect.wait_num > 3) {
        mqtt->link_status |= MQTT_LINK_ERROR;
        return -1;
    }

    if (mqtt_malloc(&pack, &mqtt_vhead, &mqtt_payload) != 0) {
        return -1;
    }

    if (mqtt_connect_payload_get(mqtt_cfg, &mqtt_payload) != 0) {
        mqtt_free(&pack, &mqtt_vhead, &mqtt_payload);
        return -1;
    }
    if (mqtt_connect_vhead_get(mqtt_cfg, &mqtt_vhead) != 0) {
        mqtt_free(&pack, &mqtt_vhead, &mqtt_payload);
        return -1;
    }

    //fullfill pack
    lastlen = mqtt_payload.len + mqtt_vhead.len;
    pack.len = 0;

    pack.buff[pack.len++] = MQTT_CONNECT;
    do {
        pack.buff[pack.len] = lastlen % 128;
        lastlen >>= 7;
        // if there are more data to encode, set the top bit of this byte
        if (lastlen > 0)
            pack.buff[pack.len] = pack.buff[pack.len] | 128;
        pack.len++;
    } while (lastlen > 0);

    memcpy(&pack.buff[pack.len], mqtt_vhead.buff, mqtt_vhead.len);
    pack.len += mqtt_vhead.len;
    memcpy(&pack.buff[pack.len], mqtt_payload.buff, mqtt_payload.len);
    pack.len += mqtt_payload.len;

    mqtt->send(pack.buff, pack.len);
    mqtt_free(&pack, &mqtt_vhead, &mqtt_payload);

    link.connect.wait_num++;
    link.wait_sum++;
    if (link.connect.wait_num > 3) {
        mqtt->link_status |= MQTT_LINK_ERROR;
    }

    return 0;
}

int mqtt_publish(mqtt_t* mqtt, uint8_t* msg, uint32_t msglen)
{
    mqtt_cfg_publish_t* mqtt_cfg;
    mqtt_pack_t pack, mqtt_vhead, mqtt_payload;
    mqtt_pack_t msgpack;

    uint32_t lastlen = 0;

    mqtt_cfg = &mqtt->cfg_publish;
    if (link.wait_max_sum != 0) {
        if (link.wait_sum > link.wait_max_sum) {
            mqtt->link_status = MQTT_LINK_BREAK;
            return -1;
        }
    }

    if (mqtt_malloc(&pack, &mqtt_vhead, &mqtt_payload) != 0) {
        return -1;
    }

    if (mqtt_publish_vhead_get(mqtt_cfg, &mqtt_vhead) != 0) {
        mqtt_free(&pack, &mqtt_vhead, &mqtt_payload);
        return -1;
    }
    msgpack.buff = msg;
    msgpack.len = msglen;
    if (mqtt_publish_payload_get(&msgpack, &mqtt_payload) != 0) {
        mqtt_free(&pack, &mqtt_vhead, &mqtt_payload);
        return -1;
    }
    //fullfill pack
    lastlen = mqtt_payload.len + mqtt_vhead.len;

    pack.len = 0;
    pack.buff[pack.len++] = MQTT_PUBLISH | MQTT_QOS_1;

    do {
        pack.buff[pack.len] = lastlen % 128;
        lastlen >>= 7;
        // if there are more data to encode, set the top bit of this byte
        if (lastlen > 0)
            pack.buff[pack.len] = pack.buff[pack.len] | 128;
        pack.len++;
    } while (lastlen > 0);

    memcpy(&pack.buff[pack.len], mqtt_vhead.buff, mqtt_vhead.len);
    pack.len += mqtt_vhead.len;
    memcpy(&pack.buff[pack.len], mqtt_payload.buff, mqtt_payload.len);
    pack.len += mqtt_payload.len;

    mqtt->send(pack.buff, pack.len);
    mqtt_free(&pack, &mqtt_vhead, &mqtt_payload);

    link.pub.wait_num++;
    link.wait_sum++;

    return 0;
}

int mqtt_ping(mqtt_t* mqtt)
{
    mqtt_pack_t pack;

    if (link.wait_max_sum != 0) {
        if (link.wait_sum > link.wait_max_sum) {
            mqtt->link_status = MQTT_LINK_BREAK;
            return -1;
        }
    }

    if (mqtt_malloc(&pack, NULL, NULL) != 0) {
        return -1;
    }
    pack.len = 0;
    pack.buff[pack.len++] = MQTT_PINGREQ;
    pack.buff[pack.len++] = 0;
    mqtt->send(pack.buff, pack.len);
    mqtt_free(&pack, NULL, NULL);

    link.ping.wait_num++;
    link.wait_sum++;
    return 0;
}

static int mqtt_ack_ping(mqtt_t* ack, mqtt_pack_t* pack)
{
    if (ack->wait_max == 0) {
        return 0;
    }
    if (link.wait_max_sum != 0) {
        link.ping.wait_num--;
        link.wait_sum--;
    }
    pack->len = 0;
    return 0;
}

static int mqtt_ack_pub_qos(mqtt_t* ack, mqtt_pack_t* pack)
{
    if (ack->wait_max == 0) {
        return 0;
    }
    if (link.wait_max_sum != 0) {
        link.pub.wait_num--;
        link.wait_sum--;
    }
    pack->len = 0;
    return 0;
}

int mqtt_ack(mqtt_t* mqtt, uint8_t* buff, uint32_t len)
{
    mqtt_pack_t pack;
    uint8_t head;
    head = buff[0];
    pack.buff = buff;
    pack.len = len;
    // if (mqtt->link_status != MQTT_LINK_OK) {
    //     return -1;
    // }
    switch (head) {
    case MQTT_CONNACK:
        break;
    case MQTT_PUBACK:
        mqtt_ack_pub_qos(mqtt, &pack);
        break;
    case MQTT_SUBACK:
        break;
    case MQTT_UNSUBACK:
        break;
    case MQTT_PINGRESP:
        mqtt_ack_ping(mqtt, &pack);
        break;
    case MQTT_DISCONNECT:
        break;
    default:
        break;
    }
    return 0;
}