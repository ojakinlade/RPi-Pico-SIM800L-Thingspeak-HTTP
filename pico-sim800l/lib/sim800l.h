#ifndef SIM800L_H
#define SIM800L_H

enum OUTPUTS
{
    SUCCESS = 0,
    FAIL = 1,
    BAD_REQ = 2
};

typedef struct 
{
    char* host;
    uint16_t port;
    char* writeApiKey;
}server_t;

typedef struct
{
    char* apn;
    char* apn_user;
    char* apn_pass;
}sim_t;

typedef struct 
{
    sim_t sim;
    server_t server;
}SIM800_t;

extern void SIM800L_UART_Init(void);
extern void SIM800L_GPRS_Connect(void);
extern int SIM800L_Init(void);
extern int SIM800L_GPRS_Init(void);
extern int SIM800L_SendToServer(char* msg);

#endif /* SIM800L_H */