/*
 * Notes:
 *
 * Timer interrupt
 * APB1 timer clock=108MHz
 * Timer 2 uses a 32-bit register
 * Timer clock frequency= CPU clock/prescalar=108MHz/54000=2000Hz=0.002MHz
 * Hence, 1 sec = ~2000 ticks
 * Count period=2^(32)-1 <= set to max
 * 60 secs=120000 ticks
 *
 * Output compare timer interrupt
 * APB1 timer clock=108MHz
 * Timer5 uses a 32-bit register
 * Timer clock frequency= CPU clock/prescalar=108MHz/54000=2000Hz=0.002MHz
 * Hence, 1 sec = ~2000 ticks
 * Count period=2^(32)-1 <= set to max
 *
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Timers.h"
#include "../UART_WIFI/Wifi.h"
#include "../lvgl/lvgl.h"
#include "../lv_widgets/base_obj.h"
#include "hal_stm_lvgl/tft/tft.h"
#include "hal_stm_lvgl/touchpad/touchpad.h"

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
uint16_t timer2_val;
int shw_discnt_flag=0;
int tim2_frst_rnd_flag=0;
TIM_HandleTypeDef htim5;
uint32_t uhCapture = 0; //Capture buffer/register

/* Private function prototypes -----------------------------------------------*/
void MX_TIM2_Init(void);
void MX_TIM5_Init(void);
void TIMER5_CTRL(void);
void MX_TIM5_DeInit(void);


void TIMER2_EVENT(void)  //Timer 2 - check how much time has passed
{
	if (tim2_frst_rnd_flag==1)
	{
		timer2_val = __HAL_TIM_GET_COUNTER(&htim2);  //Save new timestamp
		tim2_frst_rnd_flag=0;
	}
	else if ( __HAL_TIM_GET_COUNTER(&htim2) - timer2_val >=120000)  // Approx. 60 secs has pasted
	{
		shw_discnt_flag=1;
	}
}

void START_TIMER2(void)  //Initialise timer 2
{
	MX_TIM2_Init();
	HAL_TIM_Base_Start(&htim2);
	tim2_frst_rnd_flag=1;
}

void STOP_TIMER2(void)  //De-initialise timer 2
{
	HAL_TIM_Base_Stop(&htim2);
	HAL_TIM_Base_DeInit(&htim2);
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 54001-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967296-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}


void TIMER5_CTRL(void)  //Timer 5 control.
{
	if (Touched==1)  //If screen is touched
	{
		//Restart timer
		MX_TIM5_DeInit();
		MX_TIM5_Init();
		Touched=0;
	}
	//Set timer
	if (scrnTimeout_flag==1)
    {
		HAL_TIM_OC_Start_IT(&htim5, TIM_CHANNEL_1);
	    HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_2);
	    HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_3);
	    HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_4);
    }
    else if (scrnTimeout_flag==2)
    {
	    HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_1);
	    HAL_TIM_OC_Start_IT(&htim5, TIM_CHANNEL_2);
	    HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_3);
	    HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_4);
    }
    else if (scrnTimeout_flag==3)
    {
	    HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_1);
	    HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_2);
	    HAL_TIM_OC_Start_IT(&htim5, TIM_CHANNEL_3);
	    HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_4);
    }
    else if (scrnTimeout_flag==4)
    {
	    HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_1);
	    HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_2);
	    HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_3);
	    HAL_TIM_OC_Start_IT(&htim5, TIM_CHANNEL_4);
    }
    else
    {
	    HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_1);
	    HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_2);
	    HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_3);
	    HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_4);
    }
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef* htim)  //Tim5's interrupt - Process when will be the next screen timeout
{

    if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1 )  //30 secs
    {
    	uhCapture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
    	if (__HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, (uhCapture + 60000-1)) && scrnTimeout_flag==1 && scrn_stat_flag==0)
    	{
    		SCREEN_OFF();
    		scrn_stat_flag=1;
    		uhCapture = 0;
    		HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_1);
    	}
    }
    else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2 )  //1 min
    {
		uhCapture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
		if (__HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_2, (uhCapture + 120000-1)) && scrnTimeout_flag==2 && scrn_stat_flag==0)
		{
			SCREEN_OFF();
			scrn_stat_flag=1;
			uhCapture = 0;
			HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_2);
		}
	}
    else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3 )  //2 mins
    {
		uhCapture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
		if (__HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_3, (uhCapture + 240000-1)) && scrnTimeout_flag==3 && scrn_stat_flag==0)
		{
			SCREEN_OFF();
			scrn_stat_flag=1;
			uhCapture = 0;
			HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_3);
		}
	}
    else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4 )  //5 mins
    {
		uhCapture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);
		if (__HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_4, (uhCapture + 600000-1)) && scrnTimeout_flag==4 && scrn_stat_flag==0)
		{
			SCREEN_OFF();
			scrn_stat_flag=1;
			uhCapture = 0;
			HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_4);
		}
	}
}

void TIM5_ReInit(void)  //Initialise timer 5
{
	HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_1);
	HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_2);
	HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_3);
	HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_4);
	HAL_TIM_OC_DeInit(&htim5);
	HAL_TIM_Base_DeInit(&htim5);

	MX_TIM5_Init();
}

void MX_TIM5_DeInit(void)  //De-initialise timer 5
{
	HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_1);
	HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_2);
	HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_3);
	HAL_TIM_OC_Stop_IT(&htim5, TIM_CHANNEL_4);
	HAL_TIM_OC_DeInit(&htim5);
	HAL_TIM_Base_DeInit(&htim5);
}

/**
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
void MX_TIM5_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 54001-1;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 4294967296-1;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OC_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 60000-1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.Pulse = 120000-1;
  if (HAL_TIM_OC_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.Pulse = 240000-1;
  if (HAL_TIM_OC_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.Pulse = 600000-1;
  if (HAL_TIM_OC_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
}

