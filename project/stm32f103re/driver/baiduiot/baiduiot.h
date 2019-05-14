
#ifndef __BAIDUIOT_H
#define __BAIDUIOT_H

#include <stdint.h>
#include <stdlib.h>

typedef struct 
{
    char *head;
    char *temper_head;
    char *temper_value;
    char *hum_head;
    char *hum_value;
    char *end;
}report_t;

typedef struct 
{
    char *head;
    char *req_head;
    char *req_value;
    report_t report;
    char *end;
}prot_t;

typedef struct 
{
    uint32_t task_delay;
    uint32_t requestid;
    float tempreture;
    float humidity;
    uint8_t link;
    prot_t prot;
}baiduiot_t;


int iot_init(void);
int iot_task(void);

#endif

