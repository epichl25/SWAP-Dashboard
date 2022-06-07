/**
 * @file disp.h
 * 
 */

#ifndef DISP_H
#define DISP_H

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>
#include "lvgl/lvgl.h"

/*********************
 *      DEFINES
 *********************/
#define TFT_HOR_RES     800
#define TFT_VER_RES     480
#define TFT_NO_TEARING  1    /*1: no tearing but slower*/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void tft_init(void);
extern void SCREEN_ON(void);
extern void SCREEN_OFF(void);
/**********************
 *      MACROS
 **********************/
extern uint8_t BrightnessValue;
extern int scrn_stat_flag;
extern int scrnTimeout_flag;

#endif
