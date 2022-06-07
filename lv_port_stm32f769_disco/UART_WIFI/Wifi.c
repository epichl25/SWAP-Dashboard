/******************************************************************
 *
 * NOTE: "ESP8266_WIFI_UART_Arduino" contains code for Wi-Fi module
 *
 *D0=RX, D1=TX
 ******************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Wifi.h"
#include "../lvgl/lvgl.h"
#include "../lv_widgets/base_obj.h"
#include "../Timers/Timers.h"
/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart6;

//UART
char byte;
char uart_recv[1024];
char cmd[32];
char recv_data[1024];
int RX_read_flag=0;  //Check if data from UART is valid. If valid data format, RX_read_flag==1

//Command
int chk_cmd_flag=0;  //Check command flag

//Wi-Fi
int JNING_WIFI_flag=0;  //Joining Wi-Fi flag

//MQTT
char init_buff[2048];
char JN_WIFI_CMD[]="JN_WIFI";
char *WIFI_SSID;
char *WIFI_PWD;
char mqtt_broker[]="MQTT_Address";
char mqtt_username[]="USERNAME";  //authentication credentials
char mqtt_password[]="PASSWORD";  //authentication credentials
char mqtt_port[]="PORT";  //port
char device_ID[]="TOPIC";  //Device name
char opening[]="<";
char closing[]=">";
char equal[]="=";
char semicln[]=";";

/* Private function prototypes -----------------------------------------------*/
static void MX_USART1_UART_Init(void);
static void MX_USART6_UART_Init(void);
void MX_USART6_DeInit(void);
/* Private user code ---------------------------------------------------------*/

//Soft reset Wi-Fi Module
void RESET_WIFI(void)  //ESP8266 reset by pull low
{
	HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_1,1);
	HAL_Delay(200);
	HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_1,0);
	HAL_Delay(200);
	HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_1,1);
	HAL_Delay(100);
}

//Initialise UARTs
void UART_START(void)
{
	MX_USART1_UART_Init();
	MX_USART6_UART_Init();
	uint8_t Test[] = "USART Initialised!";
	HAL_UART_Transmit(&huart1,Test,strlen(Test),100);
	HAL_UART_Receive_IT (&huart6, &byte, 1);
}


/*********************
 *
 * UART RX
 *
 ********************/
//Receive data from Wi-Fi module with non-blocking UART function
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART6)
	{
		HAL_UART_Receive_IT(&huart6, &byte, 1);
	    if (byte=='<')
	    {
	    	RX_read_flag=1;
	    	memset(uart_recv, 0, sizeof(uart_recv));
	    }
	    else if (byte=='>')
	    {
	    	RX_read_flag=0;
	    	memset(cmd, 0, sizeof(cmd));
	    	memset(recv_data, 0, sizeof(recv_data));

	    	// Find index of '='
			char *equalChar=strchr(uart_recv,'=');
			if (equalChar==NULL)
			{
				int index_equalChar=strlen(uart_recv);
				strncpy(&cmd,&uart_recv,index_equalChar);
			}
			else
			{
				int index_equalChar=(int)(equalChar-uart_recv);
				strncpy(&cmd,&uart_recv,index_equalChar);
				int uart_recv_len=strlen(uart_recv);
				strncpy(&recv_data,&uart_recv[index_equalChar+1],uart_recv_len-index_equalChar);
			}
	    	chk_cmd_flag=1;
	    }
	    else if (RX_read_flag==1)
	    {
	    	strncat(uart_recv,&byte,1);
	    }
	}
}

/********************
 *
 * Settings
 *
 *******************/
void JN_WIFI(void)  //Concatenate char array
{
	snprintf(init_buff, sizeof(init_buff), "<%s=%s;%s;%s;%s;%s;%s;%s>",JN_WIFI_CMD,WIFI_SSID,WIFI_PWD,mqtt_broker,mqtt_username,mqtt_password,mqtt_port,device_ID);
	HAL_UART_Transmit(&huart6, init_buff, strlen(init_buff),100);
	JNING_WIFI_flag=1;
	START_TIMER2();
}

/********************
 *
 * CMD Check
 *
 *******************/
void CMD_CHK(void) //Check cmd
{
	if (strcmp(cmd, "RECV_DAI") == 0)  //Data for daily stats
	{
		GT_DAI_DAT();
	}
	else if (strcmp(cmd, "RECV_MNT") == 0)  //Data for monthly stats
	{
		GT_MNTHLY_DAT();
	}
    chk_cmd_flag=0;
}

/********************
 *
 * Data Processing
 *
 *******************/
//Get daily data
void GT_DAI_DAT(void)
{
	//Declare local variables
	int recv_data_lstChar1;
	int semicln_index_1;
	int semicln_index_2;
	int semicln_index_3;
	int semicln_index_4;
	int Cma_index_1;
	int Cma_index_2;
	int Cma_index_3;
	int Cma_index_4;
	int Cma_index_5;

	//Flush buffers
	memset(date, 0, sizeof(date));
	memset(basin_Val, 0, sizeof(basin_Val));
	memset(kit_tp_Val, 0, sizeof(kit_tp_Val));
	memset(showerhead_Val, 0, sizeof(showerhead_Val));
	memset(toicstrn_Val, 0, sizeof(toicstrn_Val));
	memset(wshmch_Val, 0, sizeof(wshmch_Val));

	recv_data_lstChar1=strlen(recv_data);

	//Get date of data
	char *dateChar=strchr(recv_data,':');
	int index_dateChar=(int)(dateChar-recv_data);
	strncpy(&date,&recv_data,index_dateChar);

	//Find indexes of semicolons
	char *semiclnPtr1;
	int DAI_Cnt=0;
	semiclnPtr1=strchr(recv_data,';');
	while (semiclnPtr1 != NULL)
	{
		if (DAI_Cnt==0)  //Index of Basin Tap
	    {
			semicln_index_1=(int)(semiclnPtr1-recv_data);
	    }
	    else if (DAI_Cnt==1)  //Index of Kitchen Tap
	    {
		    semicln_index_2=(int)(semiclnPtr1-recv_data);
	    }
	    else if (DAI_Cnt==2) //Index of Showerhead
	    {
	        semicln_index_3=(int)(semiclnPtr1-recv_data);
	    }
	    else if (DAI_Cnt==3) //Index of Toilet Cistern
	    {
	        semicln_index_4=(int)(semiclnPtr1-recv_data);
	    }
	    semiclnPtr1 = strchr (semiclnPtr1 + 1, ';');
	    DAI_Cnt++;
	}

	//Find indexes of commas
	char *apsPtr1;
	int DAI_Cnt2=0;
	apsPtr1=strchr(recv_data,',');
	while (apsPtr1 != NULL)
	{
		if (DAI_Cnt2==0)  //Index of Basin Tap
		{
			Cma_index_1=(int)(apsPtr1-recv_data);
		}
		else if (DAI_Cnt2==1)  //Index of Kitchen Tap
		{
			Cma_index_2=(int)(apsPtr1-recv_data);
		}
		else if (DAI_Cnt2==2) //Index of Showerhead
		{
		    Cma_index_3=(int)(apsPtr1-recv_data);
		}
		else if (DAI_Cnt2==3) //Index of Toilet Cistern
		{
		    Cma_index_4=(int)(apsPtr1-recv_data);
		}
		else if (DAI_Cnt2==4)  //Index of Washing machine
		{
			Cma_index_5=(int)(apsPtr1-recv_data);
		}
		apsPtr1 = strchr (apsPtr1 + 1, ',');
		DAI_Cnt2++;
	}

	//Extract data
	//Get Basin Tap data
	strncpy(&basin_Val,&recv_data[Cma_index_1+1],(semicln_index_1-Cma_index_1-1));

	//Get Kitchen Tap data
	strncpy(&kit_tp_Val,&recv_data[Cma_index_2+1],(semicln_index_2-Cma_index_2-1));

	//Get Showerhead data
	strncpy(&showerhead_Val,&recv_data[Cma_index_3+1],(semicln_index_3-Cma_index_3-1));

	//Get Toilet Cistern data
	strncpy(&toicstrn_Val,&recv_data[Cma_index_4+1],(semicln_index_4-Cma_index_4-1));

	//Get Washing Machine data
	strncpy(&wshmch_Val,&recv_data[Cma_index_5+1],(recv_data_lstChar1-Cma_index_5-1));

	//Update tab 1 of GUI
	update_tab1();
}


//Get monthly data
void GT_MNTHLY_DAT(void)
{
	//Declare local variables
	int recv_data_lstChar2;
	int semicln_indx1;
	int semicln_indx2;
	int semicln_indx3;
	int semicln_indx4;
	int semicln_indx5;
	int semicln_indx6;
	int semicln_indx7;
	int semicln_indx8;
	int semicln_indx9;
	int semicln_indx10;
	int semicln_indx11;
	char temp_jan[16];
	char temp_feb[16];
	char temp_mar[16];
	char temp_apr[16];
	char temp_may[16];
	char temp_jun[16];
	char temp_jul[16];
	char temp_aug[16];
	char temp_sep[16];
	char temp_oct[16];
	char temp_nov[16];
	char temp_dec[16];

	recv_data_lstChar2=strlen(recv_data);

	//Get year of data
	char *yearChar=strchr(recv_data,':');
	int index_yearChar=(int)(yearChar-recv_data);
	strncpy(&year,&recv_data,index_yearChar);

	//Find number of months with valid data & find indexes off semicolons
	char *semiclnPtr2;
	int MNTHLY_Cnt=2;

	semiclnPtr2=strchr(recv_data,';');
	while (semiclnPtr2 != NULL)
	{
		if (MNTHLY_Cnt==2)  //Check Feb
		{
			semicln_indx1=(int)(semiclnPtr2-recv_data);
		}
		else if (MNTHLY_Cnt==3)  //Check Mar
		{
			semicln_indx2=(int)(semiclnPtr2-recv_data);
		}
		else if (MNTHLY_Cnt==4)  //Check Apr
		{
			semicln_indx3=(int)(semiclnPtr2-recv_data);
		}
		else if (MNTHLY_Cnt==5)  //Check May
		{
			semicln_indx4=(int)(semiclnPtr2-recv_data);
		}
		else if (MNTHLY_Cnt==6)  //Check Jun
		{
			semicln_indx5=(int)(semiclnPtr2-recv_data);
		}
		else if (MNTHLY_Cnt==7)  //Check Jul
		{
			semicln_indx6=(int)(semiclnPtr2-recv_data);
		}
		else if (MNTHLY_Cnt==8)  //Check Aug
		{
			semicln_indx7=(int)(semiclnPtr2-recv_data);
		}
		else if (MNTHLY_Cnt==9)  //Check Sep
		{
			semicln_indx8=(int)(semiclnPtr2-recv_data);
		}
		else if (MNTHLY_Cnt==10)  //Check Oct
		{
			semicln_indx9=(int)(semiclnPtr2-recv_data);
		}
		else if (MNTHLY_Cnt==11)  //Check Nov
		{
			semicln_indx10=(int)(semiclnPtr2-recv_data);
		}
		else if (MNTHLY_Cnt==12)  //Check Dec
		{
			semicln_indx11=(int)(semiclnPtr2-recv_data);
		}
		semiclnPtr2 = strchr (semiclnPtr2 + 1, ';');
		MNTHLY_Cnt++;
	}

	//Extract data
	strncpy(&temp_jan,&recv_data[index_yearChar+1],(semicln_indx1-index_yearChar-1));
	jan=atoi(temp_jan);

	strncpy(&temp_feb,&recv_data[semicln_indx1+1],(semicln_indx2-semicln_indx1-1));
	feb=atoi(temp_feb);

	strncpy(&temp_mar,&recv_data[semicln_indx2+1],(semicln_indx3-semicln_indx2-1));
	mar=atoi(temp_mar);

	strncpy(&temp_apr,&recv_data[semicln_indx3+1],(semicln_indx4-semicln_indx3-1));
	apr=atoi(temp_apr);

	strncpy(&temp_may,&recv_data[semicln_indx4+1],(semicln_indx5-semicln_indx4-1));
	may=atoi(temp_may);

	strncpy(&temp_jun,&recv_data[semicln_indx5+1],(semicln_indx6-semicln_indx5-1));
	jun=atoi(temp_jun);

	strncpy(&temp_jul,&recv_data[semicln_indx6+1],(semicln_indx7-semicln_indx6-1));
	jul=atoi(temp_jul);

	strncpy(&temp_aug,&recv_data[semicln_indx7+1],(semicln_indx8-semicln_indx7-1));
	aug=atoi(temp_aug);

	strncpy(&temp_sep,&recv_data[semicln_indx8+1],(semicln_indx9-semicln_indx8-1));
	sep=atoi(temp_sep);

	strncpy(&temp_oct,&recv_data[semicln_indx9+1],(semicln_indx10-semicln_indx9-1));
	oct=atoi(temp_oct);

	strncpy(&temp_nov,&recv_data[semicln_indx10+1],(semicln_indx11-semicln_indx10-1));
	nov=atoi(temp_nov);

	strncpy(&temp_dec,&recv_data[semicln_indx11+1],(recv_data_lstChar2-semicln_indx11-1));
	dec=atoi(temp_dec);

	//Update tab 2 of GUI
	update_tab_2();
}


/*************************************
 *
 * UART CONFIG
 *
 *************************************/
static void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
}
static void MX_USART6_UART_Init(void)
{
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  huart6.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart6.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
}

void MX_USART6_DeInit(void)
{
	HAL_UART_DeInit(&huart6);
}
