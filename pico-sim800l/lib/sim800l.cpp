#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "SIM800l.h"

#define GSM_UART            uart1
#define UART_BAUD_RATE      9600
#define UART_TX_PIN         8
#define UART_RX_PIN         9
#define UART_DATA_BITS      8
#define UART_STOP_BITS      1
#define CMD_DELAY           2000

extern SIM800_t SIM800L;

uint8_t rx_data = 0;
char rx_buffer[1460] = {0};
uint16_t rx_index = 0;

char mqtt_buffer[1460] = {0};
uint16_t mqtt_index = 0;

bool uartDoneReceiving = false;
uint8_t mqttMessage[127];

static int SIM800L_SendCommand(char* command, char* reply, uint16_t delay, bool flag = false);

// const char APN[] = "gloflat\r\n";
//const char APN[] = "fast.m2m\r\n";
// const char APN[] = "airtelgprs.com\r\n";

static void SIM800L_RxCallback(void);

static void SIM800L_UART_RX(uint8_t* rxData)
{
    if(uart_is_readable(GSM_UART))
    {
        *rxData = uart_getc(GSM_UART);  
    }
    uartDoneReceiving = true;
}

void UART1_IRQHandler(void)
{
     if(uart_is_readable(GSM_UART))
     {
        rx_buffer[rx_index++] = uart_getc(GSM_UART);
     }
}

/**
 * @brief Clears SIM800L UART RX buffer
 * @param NONE
 * @return NONE
 * 
 */
void ClearRxBuffer(void)
{
    rx_index = 0;
    memset(rx_buffer, 0, sizeof(rx_buffer));
}

void SIM800L_RxCallback(void)
{
    rx_buffer[rx_index++] = rx_data; 
}

static void SIM800L_UART_TX(char* command)
{
    uart_puts(GSM_UART, command);
}

int SIM800L_SendCommand(char* command, char* reply, uint16_t delay,bool flag)
{
    if(!flag)
    {
        SIM800L_UART_TX(command);
        sleep_ms(delay);
        if(strstr(rx_buffer, reply) != NULL)
        {
            printf("%s",rx_buffer);
            ClearRxBuffer();
            return 0;
        }
        ClearRxBuffer();
        return 1;
    }
    else
    {
        SIM800L_UART_TX(command);
        sleep_ms(delay);
        if(strstr(rx_buffer, reply) != NULL)
        {
            printf("%s",rx_buffer);
            ClearRxBuffer();
            return 2;
        }
        ClearRxBuffer();
        return 0;
    }   
}

static void SIM800L_SendCommand(char* command, uint16_t delay)
{
    SIM800L_UART_TX(command);
    sleep_ms(delay);
}


void SIM800L_UART_Init(void)
{
    uart_init(GSM_UART, UART_BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(GSM_UART,false,false); //Disable hardware flow control
    uart_set_format(GSM_UART,UART_DATA_BITS,UART_STOP_BITS,UART_PARITY_NONE);
    uart_set_fifo_enabled(GSM_UART,true);
    //RX Interrupt Init
    irq_set_exclusive_handler(UART1_IRQ, UART1_IRQHandler);
    irq_set_enabled(UART1_IRQ, true);
    uart_set_irq_enables(GSM_UART, true, false);
}

int SIM800L_Init(void)
{
    int error = 0;

    error += SIM800L_SendCommand("AT\r\n", "OK\r\n", CMD_DELAY);
    error += SIM800L_SendCommand("ATE0\r\n", "OK\r\n", CMD_DELAY);
    error += SIM800L_SendCommand("AT+CPIN?\r\n", "READY\r\n", CMD_DELAY);
    if(error == 0)
    {
        return error;
    }
    return error;
}

int SIM800L_GPRS_Init(void)
{
    int error = 0;
    error += SIM800L_SendCommand("AT+CIPSHUT\r\n", "SHUT OK\r\n", CMD_DELAY);
    error += SIM800L_SendCommand("AT+CGATT=1\r\n", "OK\r\n", CMD_DELAY);
    error += SIM800L_SendCommand("AT+CIPMODE=1\r\n", "OK\r\n", CMD_DELAY);
    char apnStr[128] = "AT+CSTT=";
    strcat(apnStr, SIM800L.sim.apn);
    strcat(apnStr, "\r\n");
    error += SIM800L_SendCommand(apnStr, "OK\r\n", CMD_DELAY);
    error += SIM800L_SendCommand("AT+CIICR\r\n", "OK\r\n", CMD_DELAY);
    SIM800L_SendCommand("AT+CIFSR\r\n", CMD_DELAY);
    SIM800L_SendCommand("AT+CIPSPRT=0\r\n", CMD_DELAY);
    if(error == 0)
    {
        return error;
    }
    return error;
}

int SIM800L_SendToServer(char* msg)
{
    char str[128] = {0};
    char field_buffer[25] = "&field1=";
    const uint8_t api_key_index = 45;
    char* apiKey = "X5SID5NFEBPL8U3F";
    char url_template[200] = "GET http://api.thingspeak.com/update?api_key=0000000000000000";

    sprintf(str, "AT+CIPSTART=\"TCP\",\"%s\",\"%d\"\r\n",SIM800L.server.host, SIM800L.server.port); 
    SIM800L_SendCommand(str, "OK\r\n", CMD_DELAY);
    SIM800L_SendCommand("AT+CIPSEND\r\n",4000);
    for(int i = 0; i < strlen(SIM800L.server.writeApiKey); i++)
    {
        url_template[api_key_index + i] = SIM800L.server.writeApiKey[i];
    }
    strcat(field_buffer, msg);
    strcat(url_template, field_buffer);
    strcat(url_template,"\r\n");

    int ret = SIM800L_SendCommand(url_template," ",4000,true);
    uart_putc(GSM_UART, uint8_t(26));
    sleep_ms(1000);
    return ret;
}



