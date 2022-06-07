//Extern funcs
extern void RESET_WIFI(void);

extern void UART_START(void);

extern void CMD_CHK(void);
extern void JN_WIFI(void);

extern void GT_DAI_DAT(void);
extern void GT_MNTHLY_DAT(void);

extern void testUART(void);

extern void MX_USART6_DeInit(void);

//Extern variables
extern char cmd[32];
extern char recv_data[1024];

//Wi-Fi credentials
extern char *WIFI_SSID;
extern char *WIFI_PWD;

//Flags
extern int JNING_WIFI_flag; //Joining Wi-Fi flag
extern int chk_cmd_flag;  //Check command flag


