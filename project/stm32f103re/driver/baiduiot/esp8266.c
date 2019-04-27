
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "esp8266.h"
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/usart.h>

uint8_t reve_buff[256] = { 0 };
uint8_t reve_buff_len = 0;

uint8_t send_buff[256] = { 0 };
uint8_t send_buff_len = 0;

static int put_buff(uint8_t ch)
{
    // usart_send_blocking(USART1, ch);
    reve_buff[reve_buff_len++] = ch;
}

static int clear_buf(void)
{
    reve_buff_len = 0;
    send_buff_len = 0;
    memset(reve_buff, 0, 256);
    return 0;
}

static void usart2_setup(void)
{

    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_AFIO);
    rcc_periph_clock_enable(RCC_USART2);
    nvic_enable_irq(NVIC_USART2_IRQ);
    USART_CR1(USART2) = 0;
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
        GPIO_USART2_TX);
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO_USART2_RX);
    /* Setup UART parameters. */
    usart_set_baudrate(USART2, 115200);
    usart_set_databits(USART2, 8);
    usart_set_stopbits(USART2, USART_STOPBITS_1);
    usart_set_parity(USART2, USART_PARITY_NONE);
    usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);
    usart_set_mode(USART2, USART_MODE_TX_RX);

    USART_CR1(USART2) |= USART_CR1_RXNEIE;
    /* Finally enable the USART. */
    usart_enable(USART2);
}

void usart2_isr(void)
{
    /* Check if we were called because of RXNE. */
    if (((USART_CR1(USART2) & USART_CR1_RXNEIE) != 0) && ((USART_SR(USART2) & USART_SR_RXNE) != 0)) {

        USART_SR(USART2) &= ~USART_SR_RXNE;
        /* Retrieve the data from the peripheral. */
        put_buff(usart_recv(USART2));
    }
    /* Check if we were called because of TXE. */
    if (((USART_CR1(USART2) & USART_CR1_TXEIE) != 0) && ((USART_SR(USART2) & USART_SR_TXE) != 0)) {
        USART_SR(USART2) = ~USART_SR_TXE;
        /* Indicate that we are sending out data. */
        // gpio_toggle(GPIOA, GPIO7);

        /* Put data into the transmit register. */
        usart_send(USART2, 0xAA);

        /* Disable the TXE interrupt as we don't need it anymore. */
        USART_CR1(USART2) &= ~USART_CR1_TXEIE;
    }
}
static int esp8266_usart_send(uint8_t* buff, uint32_t len)
{
    while (len--) {
        usart_send_blocking(USART2, *buff);
        buff++;
    }
    return 0;
}

int esp8266_sendcmd(uint8_t* cmd, uint8_t* res, uint8_t retry)
{
    uint8_t old_reve_len = 0;
    clear_buf();
    esp8266_usart_send(cmd, strlen((char*)cmd));
    delay_ms(10);
    while (retry--) {
        while ((reve_buff_len != 0) && (reve_buff_len != old_reve_len)) {
            if (strstr(reve_buff, res) != 0) {
                return 0;
            }
            delay_ms(1);
            old_reve_len = reve_buff_len;
        }
        delay_ms(200);
    }
    return -1;
}
int esp8266_senddata(uint8_t* buf, uint32_t count)
{
    reve_buff_len = 0;
    send_buff_len = 0;
    memcpy(send_buff, buf, count);
    esp8266_usart_send(buf, count);
    return 0;
}

int esp8266_init(uint8_t* wifi_name, uint8_t* wifi_password, uint8_t* tcp_ip,
    uint8_t* tcp_port)
{
    uint8_t cmdbuf[125] = { 0 };

    usart2_setup();
    esp8266_sendcmd("+++", "OK", 10);
    delay_ms(50);
    esp8266_sendcmd("AT+CIPCLOSE\r\n", "OK", 10);
    delay_ms(50);
    if (esp8266_sendcmd("AT+PING=\"www.baidu.com\"\r\n", "OK", 5) != 0) {
        if (esp8266_sendcmd("AT+RESTORE\r\n", "OK", 400) != 0) {
            return -1;
        }
        delay_ms(50);
        esp8266_sendcmd("AT\r\n", "OK", 10);
        delay_ms(50);
        esp8266_sendcmd("AT+CWMODE_DEF=1\r\n", "OK", 10);
        delay_ms(50);
        //连接到WIFI
        sprintf(cmdbuf, "AT+CWJAP_DEF=\"%s\",\"%s\"\r\n", wifi_name, wifi_password);
        esp8266_sendcmd(cmdbuf, "OK", 200);
        delay_ms(50);
    }
    //建立TCP 链接
    sprintf(cmdbuf, "AT+CIPSTART=\"TCP\",\"%s\",%s\r\n", tcp_ip, tcp_port);
    esp8266_sendcmd(cmdbuf, "OK", 20);
    delay_ms(50);

    //进入透传模式
    esp8266_sendcmd("AT+CIPMODE=1\r\n", "OK", 10);
    delay_ms(50);
    esp8266_sendcmd("AT+CIPSEND\r\n", "<", 10);
    delay_ms(50);
    return 0;
}
