#include <stdio.h>
#include "pico/stdlib.h"
#include "sim800l.h"
#include "sysTimer.h"

sysTimer_t ledTimer;
sysTimer uploadTimer;
SIM800_t SIM800L;

int response = 0;
char* message = "58";

int main()
{
    stdio_init_all();
    SysTimer_Init(&ledTimer,1000);
    SysTimer_Init(&uploadTimer,20000);
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN,GPIO_OUT);
    //SIM and Server Param settings
    SIM800L.sim.apn = "airtelgprs.com";
    SIM800L.server.host = "api.thingspeak.com";
    SIM800L.server.port = 80;
    SIM800L.server.writeApiKey = "X5SID5NFEBPL8U3F";

    SIM800L_UART_Init();
    int ret = SIM800L_Init();
    while(ret != 0)
    {
        ret = SIM800L_Init();
        printf("%d",ret);
    }
    printf("%d",ret);

    ret = SIM800L_GPRS_Init();
    while(ret != 0)
    {
        ret = SIM800L_GPRS_Init();
        printf("%d",ret);
    }

    while(true)
    {
        response = SIM800L_SendToServer(message);
        while(response == BAD_REQ)
        {
            sleep_ms(5000);
            response = SIM800L_SendToServer(message);
        }
        sleep_ms(25000); 
    }
}