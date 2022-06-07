#include "../lvgl/lvgl.h" //Include lvgl library

//tabs
extern lv_obj_t * tab1;
extern lv_obj_t * tab2;
extern lv_obj_t * tab3;

//Time & date
extern char time[16];
extern char date[16];
extern char year[8];

//Update funcs
extern void update_tab1();
extern void update_tab2();

//daily stat variables
extern char basin_Val[32];
extern char kit_tp_Val[32];
extern char showerhead_Val[32];
extern char toicstrn_Val[32];
extern char wshmch_Val[32];

//Monthly stat variables
extern int y_axis_max;
extern int y_axis_scaling;
extern int mnths;
extern int jan;
extern int feb;
extern int mar;
extern int apr;
extern int may;
extern int jun;
extern int jul;
extern int aug;
extern int sep;
extern int oct;
extern int nov;
extern int dec;

//Settings
extern volatile uint32_t connt_cnt;  //Checks the state of the connect/disconnect button
extern void DISPLAY_CNTED(void);
extern void DISPLAY_DISCNTED(void);
extern void CNT_FAILED(void);


#ifndef LV_WIDGETS_BASE_OBJ_H_
#define LV_WIDGETS_BASE_OBJ_H_

#endif /* LV_WIDGETS_BASE_OBJ_H_*/
