#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "base_obj.h"
#include "../lvgl/lvgl.h"
#include "../UART_WIFI/Wifi.h"
#include "../Timers/Timers.h"
#include "hal_stm_lvgl/tft/tft.h"
#include "../HAL_Driver/Inc/main.h"
/**********************
 *  STATIC PROTOTYPES
 **********************/
//GUI tabs
static void tab_1(lv_obj_t * parent);
static void tab_2(lv_obj_t * parent);
void tab_3(lv_obj_t * parent);

//GUI's objects to be updated
void update_tab1(void);
void update_tab_2(void);

//Tab 2 - Line chart event handlers
static void line_chart_event_cb(lv_event_t * e);
void line_chart_set_para(void);  //Set line chart set parameters
int digit_rounder(int int_y_axis_max);  //Round up line chart values

//Tab 3 - event handlers
static void brightness_slider_event_cb(lv_event_t * e);
static void scrntimeout_event_handler(lv_event_t * e);
static void ta_event_cb(lv_event_t * e);
static void wifi_swch_event_handler(lv_event_t * e);
static void wifi_connt_event_handler(lv_event_t * e);
//Wi-Fi status object updates
void DISPLAY_CNTED(void);
void DISPLAY_DISCNTED(void);
void CNT_FAIL(void);

/****************************
 *  STATIC VARIABLES
 ***************************/
static lv_style_t style_main_obj;  //Style mainly for main background obj
static lv_style_t style_bkgnd;  //Style for stat bar 
static lv_style_t style_obj1;  //Style mainly for stat bar obj
static lv_style_t style_txt1;  //Style mainly for stat bar txt
static lv_obj_t * tabview;

//Styles
static lv_style_t grn_style;
static lv_style_t blue_style;
static lv_style_t purple_style;
static lv_style_t pink_style;
static lv_style_t orange_style;
static lv_style_t tab1_val_style;

//Main loop
static lv_obj_t * status_bar;
static lv_obj_t * tab_btns;
lv_obj_t * tab1;
lv_obj_t * tab2;
lv_obj_t * tab3;

//Daily stats:
lv_obj_t * stat_date;
lv_obj_t * tab_1_obj;
lv_obj_t * dai_stat_title;

lv_obj_t * basin_pro_obj;
lv_obj_t * basin_label;
lv_obj_t * basin_val;
lv_obj_t * SI_unit_label1;

lv_obj_t * kit_tp_pro_obj;
lv_obj_t * kit_tp_label;
lv_obj_t * kit_tp_val;
lv_obj_t * SI_unit_label2;

lv_obj_t * shwrhd_pro_obj;
lv_obj_t * shwrhd_label;
lv_obj_t * shwrhd_val;
lv_obj_t * SI_unit_label3;

lv_obj_t * toicstrn_pro_obj;
lv_obj_t * toicstrn_label;
lv_obj_t * toicstrn_val;
lv_obj_t * SI_unit_label4;

lv_obj_t * wshmch_pro_obj;
lv_obj_t * wshmch_label;
lv_obj_t * wshmch_val;
lv_obj_t * SI_unit_label5;

//Monthly stats:
lv_obj_t * stat_year;
lv_obj_t * tab_2_obj;
lv_obj_t * line_chart_title;
lv_obj_t * line_chart_Y_axis;
lv_obj_t * line_chart;
lv_chart_series_t * line_data;

//Settings
lv_obj_t * brightness_level;
static lv_obj_t * brightness_obj;
static lv_obj_t * slider;
static lv_style_t style_knob;
static lv_style_t style_knob_pressed_color;
static lv_obj_t * brightness_label;
static lv_obj_t * brightness_level_label;
static lv_obj_t * sleep_obj;
static lv_obj_t * sleep_label;
static lv_obj_t * scrntimeout_dd;

static lv_obj_t * kb;
static lv_obj_t * wifi_obj;
static lv_obj_t * wifi_label;
static lv_obj_t * wifi_switch;
static lv_obj_t * wifi_status_obj;
static lv_obj_t * wifi_status_label;
static lv_obj_t * wifi_name_label;
static lv_obj_t * wifi_name_txt;
static lv_obj_t * wifi_pwd_label;
static lv_obj_t * wifi_pwd_txt;
static lv_obj_t * connt_btn;
static lv_obj_t * connt_label;
static lv_obj_t * dev_obj;
static lv_obj_t * dev_title_label;

/****************************
 *  GLOBAL VARIABLES
 ***************************/
//Time and date
char date[16];
char time[16];
char year[8];

//daily stat variables
char basin_Val[32];
char kit_tp_Val[32];
char showerhead_Val[32];
char toicstrn_Val[32];
char wshmch_Val[32];

//Monthly stat variables
char temp_y_axis_max[32];
int int_temp_y_axis_max;
int y_axis_max;
int y_axis_scaling;
int jan;
int feb;
int mar;
int apr;
int may;
int jun;
int jul;
int aug;
int sep;
int oct;
int nov;
int dec;

//Settings
volatile uint32_t connt_cnt;  //Check state of the connect/disconnect button

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
//Main loop - status bar, tabview
void base_obj(void) 
{
    /*********STYLES*******/
	lv_style_init(&style_bkgnd);  //Background style

	//Tab 1 styles
	lv_style_init(&grn_style);
	lv_style_set_bg_color(&grn_style, lv_palette_lighten(LV_PALETTE_GREEN, 1));

	lv_style_init(&blue_style);
	lv_style_set_bg_color(&blue_style, lv_palette_lighten(LV_PALETTE_BLUE, 1));

	lv_style_init(&purple_style);
	lv_style_set_bg_color(&purple_style, lv_palette_lighten(LV_PALETTE_PURPLE, 1));

	lv_style_init(&pink_style);
	lv_style_set_bg_color(&pink_style, lv_palette_lighten(LV_PALETTE_PINK, 1));

	lv_style_init(&orange_style);
	lv_style_set_bg_color(&orange_style, lv_palette_lighten(LV_PALETTE_ORANGE, 1));

	lv_style_init(&tab1_val_style); //Tab 1 - daily stats water consumption values' style
	lv_style_set_text_font(&tab1_val_style, &lv_font_montserrat_24);

    /**********************
     * Tabview
     *********************/
    //Create a tabview object
    tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 50);
    lv_obj_add_style(tabview , &style_bkgnd, 0);
    tab_btns = lv_tabview_get_tab_btns(tabview);
    lv_obj_set_size(tab_btns,800,50);
    lv_obj_set_scrollbar_mode(tab_btns, LV_SCROLLBAR_MODE_ON);
    lv_obj_set_scroll_dir(tab_btns, LV_DIR_HOR);
    lv_obj_add_style(tab_btns , &style_txt1, 0);

    //Add tabs
    tab1 = lv_tabview_add_tab(tabview, "Daily Stats");
    tab2 = lv_tabview_add_tab(tabview, "Monthly Stats");
    tab3 = lv_tabview_add_tab(tabview, "Settings");
    
    //Constrain panel movement
    lv_obj_clear_flag(tab1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(tab2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(tab3, LV_DIR_TOP);

    //Initialise tabs
    tab_1(tab1);
    tab_2(tab2);
    tab_3(tab3);

    fflush(NULL); //Flush all strings
}


/**********************
 *   STATIC FUNCTIONS
 **********************/
static void tab_1(lv_obj_t * parent)
{
	//Declare images
	LV_IMG_DECLARE(basin_tap);
	LV_IMG_DECLARE(kitchen_tap);
	LV_IMG_DECLARE(showerhead);
	LV_IMG_DECLARE(toilet_cistern);
	LV_IMG_DECLARE(washing_machine);

	//Create panel for tab 1
	tab_1_obj = lv_obj_create(parent);
	lv_obj_set_size(tab_1_obj,800,430);
	lv_obj_align( tab_1_obj,  LV_ALIGN_TOP_LEFT, -20, -20);
	lv_obj_set_scrollbar_mode(tab_1_obj, LV_SCROLLBAR_MODE_OFF);
	lv_obj_add_style(tab_1_obj, &style_bkgnd, 0);

	//Daily Water Usage
	dai_stat_title = lv_label_create(tab_1_obj);
	lv_label_set_text(dai_stat_title, "Daily Water Usage: ");
	lv_obj_add_style(dai_stat_title, &style_txt1, 0);
	lv_obj_align(dai_stat_title,  LV_ALIGN_TOP_MID, -45, 10);
	stat_date = lv_label_create(tab_1_obj);
	lv_label_set_text(stat_date, date);
	lv_obj_add_style(stat_date, &style_txt1, 0);
	lv_obj_align(stat_date,  LV_ALIGN_TOP_MID, 100, 10);

	//Basin profile
	basin_pro_obj = lv_obj_create(tab_1_obj );
	lv_obj_add_style(basin_pro_obj, &grn_style,0);
	lv_obj_set_size(basin_pro_obj , 150, 250);
	lv_obj_set_pos(basin_pro_obj, 0, 70);
	lv_obj_clear_flag(basin_pro_obj, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_t * basin_img = lv_img_create(basin_pro_obj);
	lv_img_set_src(basin_img, &basin_tap);
	lv_img_set_zoom(basin_img, 384);
	lv_obj_align(basin_img,  LV_ALIGN_TOP_MID, 0, 15);
	basin_label = lv_label_create(basin_pro_obj);
	lv_label_set_text(basin_label, "Basin Tap");
	lv_obj_align(basin_label,  LV_ALIGN_CENTER, 0, 5);
	basin_val = lv_label_create(basin_pro_obj);
	lv_label_set_text(basin_val, basin_Val);
	lv_obj_align(basin_val,  LV_ALIGN_CENTER, 0, 60);
	lv_obj_add_style(basin_val, &tab1_val_style, 0);
	SI_unit_label1 = lv_label_create(basin_pro_obj);
	lv_label_set_text(SI_unit_label1, "Litres");
	lv_obj_align(SI_unit_label1,  LV_ALIGN_BOTTOM_MID, 0, 0);
	lv_obj_add_style(SI_unit_label1, &tab1_val_style, 0);


	//kitchen tap profile
	kit_tp_pro_obj = lv_obj_create(tab_1_obj);
	lv_obj_add_style(kit_tp_pro_obj , &blue_style,0);
	lv_obj_set_size(kit_tp_pro_obj , 150, 250);
	lv_obj_set_pos(kit_tp_pro_obj, 151, 70);
	lv_obj_clear_flag(kit_tp_pro_obj, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_t * kit_tp_img = lv_img_create(kit_tp_pro_obj);
	lv_img_set_src(kit_tp_img, &kitchen_tap);
	lv_img_set_zoom(kit_tp_img, 384);
	lv_obj_align(kit_tp_img,  LV_ALIGN_TOP_MID, 0, 15);
	kit_tp_label = lv_label_create(kit_tp_pro_obj);
	lv_label_set_text(kit_tp_label, "Kitchen Tap");
	lv_obj_align(kit_tp_label,  LV_ALIGN_CENTER, 0, 5);
	kit_tp_val = lv_label_create(kit_tp_pro_obj);
	lv_label_set_text(kit_tp_val, kit_tp_Val);
	lv_obj_align(kit_tp_val,  LV_ALIGN_CENTER, 0, 60);
	lv_obj_add_style(kit_tp_val, &tab1_val_style, 0);
	SI_unit_label2 = lv_label_create(kit_tp_pro_obj);
	lv_label_set_text(SI_unit_label2, "Litres");
	lv_obj_align(SI_unit_label2,  LV_ALIGN_BOTTOM_MID, 0, 0);
	lv_obj_add_style(SI_unit_label2, &tab1_val_style, 0);

	//showerhead profile
	shwrhd_pro_obj = lv_obj_create(tab_1_obj);
	lv_obj_add_style(shwrhd_pro_obj  , &purple_style,0);
	lv_obj_set_size(shwrhd_pro_obj  , 150, 250);
	lv_obj_set_pos(shwrhd_pro_obj, 302, 70);
	lv_obj_clear_flag(shwrhd_pro_obj, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_t * shwrhd_img = lv_img_create(shwrhd_pro_obj);
	lv_img_set_src(shwrhd_img, &showerhead);
	lv_img_set_zoom(shwrhd_img, 384);
	lv_obj_align(shwrhd_img,  LV_ALIGN_TOP_MID, 0, 15);
	shwrhd_label = lv_label_create(shwrhd_pro_obj);
	lv_label_set_text(shwrhd_label, "Showerhead");
	lv_obj_align(shwrhd_label,  LV_ALIGN_CENTER, 0, 5);
	shwrhd_val = lv_label_create(shwrhd_pro_obj);
	lv_label_set_text(shwrhd_val, showerhead_Val);
	lv_obj_align(shwrhd_val,  LV_ALIGN_CENTER, 0, 60);
	lv_obj_add_style(shwrhd_val, &tab1_val_style, 0);
	SI_unit_label3 = lv_label_create(shwrhd_pro_obj);
	lv_label_set_text(SI_unit_label3, "Litres");
	lv_obj_align(SI_unit_label3,  LV_ALIGN_BOTTOM_MID, 0, 0);
	lv_obj_add_style(SI_unit_label3, &tab1_val_style, 0);

	//toilet cistern profile
	toicstrn_pro_obj = lv_obj_create(tab_1_obj);
	lv_obj_add_style(toicstrn_pro_obj , &pink_style,0);
	lv_obj_set_size(toicstrn_pro_obj  , 150, 250);
	lv_obj_set_pos(toicstrn_pro_obj, 453, 70);
	lv_obj_clear_flag(toicstrn_pro_obj, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_t * toicstrn_img = lv_img_create(toicstrn_pro_obj);
	lv_img_set_src(toicstrn_img, &toilet_cistern);
	lv_img_set_zoom(toicstrn_img, 384);
	lv_obj_align(toicstrn_img,  LV_ALIGN_TOP_MID, 0, 15);
	toicstrn_label = lv_label_create(toicstrn_pro_obj);
	lv_label_set_text(toicstrn_label, "Toilet Cistern");
	lv_obj_align(toicstrn_label,  LV_ALIGN_CENTER, 0, 5);
	toicstrn_val = lv_label_create(toicstrn_pro_obj);
	lv_label_set_text(toicstrn_val, toicstrn_Val);
	lv_obj_align(toicstrn_val,  LV_ALIGN_CENTER, 0, 60);
	lv_obj_add_style(toicstrn_val, &tab1_val_style, 0);
	SI_unit_label4 = lv_label_create(toicstrn_pro_obj);
	lv_label_set_text(SI_unit_label4, "Litres");
	lv_obj_align(SI_unit_label4,  LV_ALIGN_BOTTOM_MID, 0, 0);
	lv_obj_add_style(SI_unit_label4, &tab1_val_style, 0);

	//washing machine profile
	wshmch_pro_obj = lv_obj_create(tab_1_obj);
	lv_obj_add_style(wshmch_pro_obj , &orange_style,0);
	lv_obj_set_size(wshmch_pro_obj  , 150, 250);
	lv_obj_set_pos(wshmch_pro_obj, 604, 70);
	lv_obj_clear_flag(wshmch_pro_obj, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_t * wshmch_img = lv_img_create(wshmch_pro_obj);
	lv_img_set_src(wshmch_img, &washing_machine);
	lv_img_set_zoom(wshmch_img, 384);
	lv_obj_align(wshmch_img,  LV_ALIGN_TOP_MID, 0, 10);
	wshmch_label = lv_label_create(wshmch_pro_obj);
	lv_label_set_text(wshmch_label, "Washing\nMachine");
	lv_obj_align(wshmch_label,  LV_ALIGN_CENTER, 0, 5);
	wshmch_val = lv_label_create(wshmch_pro_obj);
	lv_label_set_text(wshmch_val, wshmch_Val);
	lv_obj_align(wshmch_val,  LV_ALIGN_CENTER, 0, 60);
	lv_obj_add_style(wshmch_val, &tab1_val_style, 0);
	SI_unit_label5 = lv_label_create(wshmch_pro_obj);
	lv_label_set_text(SI_unit_label5, "Litres");
	lv_obj_align(SI_unit_label5,  LV_ALIGN_BOTTOM_MID, 0, 0);
	lv_obj_add_style(SI_unit_label5, &tab1_val_style, 0);
}

void update_tab1(void)  //Tab 1 updates
{
	lv_label_set_text(stat_date, date);
	lv_label_set_text(basin_val, basin_Val);
	lv_label_set_text(kit_tp_val, kit_tp_Val);
	lv_label_set_text(shwrhd_val, showerhead_Val);
	lv_label_set_text(toicstrn_val, toicstrn_Val);
	lv_label_set_text(wshmch_val, wshmch_Val);
}

static void tab_2(lv_obj_t * parent)
{
    //Create panel
    tab_2_obj = lv_obj_create(parent); 
    lv_obj_set_size(tab_2_obj , 800, 430);
    lv_obj_align(tab_2_obj,  LV_ALIGN_TOP_LEFT, -20, -20);
    lv_obj_add_style(tab_2_obj, &style_bkgnd, 0);

    //Line chart title
    line_chart_title = lv_label_create(tab_2_obj);
    lv_label_set_text(line_chart_title, "Monthly Water Usage: ");
    lv_obj_add_style(line_chart_title, &style_txt1, 0);
    lv_obj_align(line_chart_title,  LV_ALIGN_TOP_MID, -40, -12);
    stat_year = lv_label_create(tab_2_obj);
    lv_label_set_text(stat_year, year);
    lv_obj_add_style(stat_year, &style_txt1, 0);
    lv_obj_align(stat_year,  LV_ALIGN_TOP_MID, 100, -12);

    //Line chart axis
    line_chart_Y_axis = lv_label_create(tab_2_obj);
    lv_label_set_text(line_chart_Y_axis, "Litres");
    lv_obj_set_pos(line_chart_Y_axis, -10, -15);

    //Line chart
    //Note: Line chart's plotting range is [-2147483648, 2147483647]
    line_chart = lv_chart_create(tab_2_obj);
    lv_obj_set_size(line_chart, 690, 360);
    lv_obj_align(line_chart, LV_ALIGN_CENTER, 20, 5);
    lv_chart_set_type(line_chart, LV_CHART_TYPE_LINE);
    lv_obj_add_flag(line_chart, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_grid_cell(line_chart, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_chart_set_axis_tick(line_chart, LV_CHART_AXIS_PRIMARY_X, 0, 0, 12, 1, true, 50);
    lv_chart_set_div_line_count(line_chart, 0, 12);
    lv_chart_set_point_count(line_chart, 12);
    lv_obj_add_event_cb(line_chart, line_chart_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_border_side(line_chart, LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_radius(line_chart, 0, 0);
    line_data = lv_chart_add_series(line_chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);
}

void update_tab_2(void)  //Tab 2 updates
{

	lv_label_set_text(stat_year, year);

	//Set line graph parameters
	y_axis_set_para();

	lv_chart_set_range(line_chart, LV_CHART_AXIS_PRIMARY_Y, 0, y_axis_max);
	lv_chart_set_axis_tick(line_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 0, y_axis_scaling, 1, true, 80);

	//Plot data
	line_data->y_points[0] = jan;
	line_data->y_points[1] = feb;
	line_data->y_points[2] = mar;
	line_data->y_points[3] = apr;
	line_data->y_points[4] = may;
	line_data->y_points[5] = jun;
	line_data->y_points[6] = jul;
	line_data->y_points[7] = aug;
	line_data->y_points[8] = sep;
	line_data->y_points[9] = oct;
	line_data->y_points[10] = nov;
	line_data->y_points[11] = dec;

	lv_chart_refresh(line_chart);
}

static void line_chart_event_cb(lv_event_t * e)  //Line chart event handler
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_PRESSED || code == LV_EVENT_RELEASED) 
    {
        lv_obj_invalidate(obj);
    }
    else if(code == LV_EVENT_DRAW_PART_BEGIN) 
    {
        lv_obj_draw_part_dsc_t * dsc = lv_event_get_param(e);
        //Set the markers' text
        if(dsc->part == LV_PART_TICKS && dsc->id == LV_CHART_AXIS_PRIMARY_X) 
        {
            const char * month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "July", "Aug", "Sept", "Oct", "Nov", "Dec"};
            lv_snprintf(dsc->text, sizeof(dsc->text), "%s", month[dsc->value]);
        }

        //Create pop-up when pressed
        //Add the faded area before the lines are drawn
        else if(dsc->part == LV_PART_ITEMS) 
        {

#if LV_DRAW_COMPLEX
        	//Add a line mask that keeps the area below the line
			if(dsc->p1 && dsc->p2) {
				lv_draw_mask_line_param_t line_mask_param;
				lv_draw_mask_line_points_init(&line_mask_param, dsc->p1->x, dsc->p1->y, dsc->p2->x, dsc->p2->y, LV_DRAW_MASK_LINE_SIDE_BOTTOM);
				int16_t line_mask_id = lv_draw_mask_add(&line_mask_param, NULL);

				//Add a fade effect: transparent bottom covering top
				lv_coord_t h = lv_obj_get_height(obj);
				lv_draw_mask_fade_param_t fade_mask_param;
				lv_draw_mask_fade_init(&fade_mask_param, &obj->coords, LV_OPA_COVER, obj->coords.y1 + h / 8, LV_OPA_TRANSP, obj->coords.y2);
				int16_t fade_mask_id = lv_draw_mask_add(&fade_mask_param, NULL);

				//Draw a rectangle that will be affected by the mask
				lv_draw_rect_dsc_t draw_rect_dsc;
				lv_draw_rect_dsc_init(&draw_rect_dsc);
				draw_rect_dsc.bg_opa = LV_OPA_50;
				draw_rect_dsc.bg_color = dsc->line_dsc->color;

				lv_area_t obj_clip_area;
				_lv_area_intersect(&obj_clip_area, dsc->clip_area, &obj->coords);

				lv_area_t a;
				a.x1 = dsc->p1->x;
				a.x2 = dsc->p2->x - 1;
				a.y1 = LV_MIN(dsc->p1->y, dsc->p2->y);
				a.y2 = obj->coords.y2;
				lv_draw_rect(&a, &obj_clip_area, &draw_rect_dsc);

				//Remove the masks
				lv_draw_mask_remove_id(line_mask_id);
				lv_draw_mask_remove_id(fade_mask_id);
			}
#endif
            const lv_chart_series_t * ser = dsc->sub_part_ptr;

            if(lv_chart_get_pressed_point(obj) == dsc->id) 
            {
                if(lv_chart_get_type(obj) == LV_CHART_TYPE_LINE) 
                {
                    dsc->rect_dsc->outline_color = lv_color_white();
                    dsc->rect_dsc->outline_width = 3;
                } 
                else 
                {
                    dsc->rect_dsc->shadow_color = ser->color;
                    dsc->rect_dsc->shadow_width = 5;
                    dsc->rect_dsc->shadow_spread = 0;
                }

                char buf[8];
                lv_snprintf(buf, sizeof(buf), "%d", dsc->value);

                lv_point_t text_size;
                lv_txt_get_size(&text_size, buf, LV_FONT_LARGE_24, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

                lv_area_t txt_area;
                if(lv_chart_get_type(obj) == LV_CHART_TYPE_BAR) 
                {
                    txt_area.y2 = dsc->draw_area->y1 - LV_DPX(5);
                    txt_area.y1 = txt_area.y2 - text_size.y;
                    if(ser == lv_chart_get_series_next(obj, NULL)) 
                    {
                        txt_area.x1 = dsc->draw_area->x1 + lv_area_get_width(dsc->draw_area) / 2;
                        txt_area.x2 = txt_area.x1 + text_size.x;
                    } 
                    else 
                    {
                        txt_area.x2 = dsc->draw_area->x1 + lv_area_get_width(dsc->draw_area) / 2;
                        txt_area.x1 = txt_area.x2 - text_size.x;
                    }
                } 
                else 
                {
                    txt_area.x1 = dsc->draw_area->x1 + lv_area_get_width(dsc->draw_area) / 2 - text_size.x / 2;
                    txt_area.x2 = txt_area.x1 + text_size.x;
                    txt_area.y2 = dsc->draw_area->y1 - LV_DPX(20);
                    txt_area.y1 = txt_area.y2 - text_size.y;
                }

                lv_area_t bg_area;
                bg_area.x1 = txt_area.x1 - LV_DPX(10);
                bg_area.x2 = txt_area.x2 + LV_DPX(10);
                bg_area.y1 = txt_area.y1 - LV_DPX(10);
                bg_area.y2 = txt_area.y2 + LV_DPX(10);

                lv_draw_rect_dsc_t rect_dsc;
                lv_draw_rect_dsc_init(&rect_dsc);
                rect_dsc.bg_color = ser->color;
                rect_dsc.radius = LV_DPX(8);
                lv_draw_rect(&bg_area, dsc->clip_area, &rect_dsc);

                lv_draw_label_dsc_t label_dsc;
                lv_draw_label_dsc_init(&label_dsc);
                label_dsc.color = lv_color_white();
                label_dsc.font = LV_FONT_LARGE_24;
                lv_draw_label(&txt_area, dsc->clip_area, &label_dsc, buf, NULL);
            } 
            else 
            {
                dsc->rect_dsc->outline_width = 0;
                dsc->rect_dsc->shadow_width = 0;
            }
        }
    }
}

void y_axis_set_para(void)  //Set Line chart's parameters
{
	//Set Y-axis scaling
	y_axis_scaling=5;

	//Flush char array
	memset(temp_y_axis_max, 0, sizeof(temp_y_axis_max));


	snprintf(temp_y_axis_max, sizeof(temp_y_axis_max), "%d", jan);
	if(feb>=atoi(temp_y_axis_max))
	{
		memset(temp_y_axis_max, 0, sizeof(temp_y_axis_max));
		snprintf(temp_y_axis_max, sizeof(temp_y_axis_max), "%d", feb);
	}
	if(mar>=atoi(temp_y_axis_max))
	{
		memset(temp_y_axis_max, 0, sizeof(temp_y_axis_max));
		snprintf(temp_y_axis_max, sizeof(temp_y_axis_max), "%d", mar);
	}
	if(apr>=atoi(temp_y_axis_max))
	{
		memset(temp_y_axis_max, 0, sizeof(temp_y_axis_max));
		snprintf(temp_y_axis_max, sizeof(temp_y_axis_max), "%d", apr);
	}
	if(may>=atoi(temp_y_axis_max))
	{
		memset(temp_y_axis_max, 0, sizeof(temp_y_axis_max));
		snprintf(temp_y_axis_max, sizeof(temp_y_axis_max), "%d", may);
	}
	if(jun>=atoi(temp_y_axis_max))
	{
		memset(temp_y_axis_max, 0, sizeof(temp_y_axis_max));
		snprintf(temp_y_axis_max, sizeof(temp_y_axis_max), "%d", jun);
	}
	if(jul>=atoi(temp_y_axis_max))
	{
		memset(temp_y_axis_max, 0, sizeof(temp_y_axis_max));
		snprintf(temp_y_axis_max, sizeof(temp_y_axis_max), "%d", jul);
	}
	if(aug>=atoi(temp_y_axis_max))
	{
		memset(temp_y_axis_max, 0, sizeof(temp_y_axis_max));
		snprintf(temp_y_axis_max, sizeof(temp_y_axis_max), "%d", aug);
	}
	if(sep>=atoi(temp_y_axis_max))
	{
		memset(temp_y_axis_max, 0, sizeof(temp_y_axis_max));
		snprintf(temp_y_axis_max, sizeof(temp_y_axis_max), "%d", sep);
	}
	if(oct>=atoi(temp_y_axis_max))
	{
		memset(temp_y_axis_max, 0, sizeof(temp_y_axis_max));
		snprintf(temp_y_axis_max, sizeof(temp_y_axis_max), "%d", oct);
	}
	if(nov>=atoi(temp_y_axis_max))
	{
		memset(temp_y_axis_max, 0, sizeof(temp_y_axis_max));
		snprintf(temp_y_axis_max, sizeof(temp_y_axis_max), "%d", nov);
	}
	if(dec>=atoi(temp_y_axis_max))
	{
		memset(temp_y_axis_max, 0, sizeof(temp_y_axis_max));
		snprintf(temp_y_axis_max, sizeof(temp_y_axis_max), "%d", dec);
	}

	//Find no. of digits in max value
	int_temp_y_axis_max=strlen(temp_y_axis_max);

	//Round up
	if (int_temp_y_axis_max==0 || int_temp_y_axis_max==1 || int_temp_y_axis_max==2) //Set default if units or tens
	{
		y_axis_max=100;
	}
	else //Round up for other values
	{
		y_axis_max=digit_rounder(int_temp_y_axis_max);
	}
}

int digit_rounder(int int_y_axis_max)  //Round up monthly stat's max. value
{
    char new_temp_y_axis_max[16];
    int new_y_axis_max;
    int zeros_cnter=0;

	for (int digt_lp=(int_temp_y_axis_max); digt_lp>=0; digt_lp--) //Check digits for '2345' in '12345'
	{
		if (strcmp(temp_y_axis_max[digt_lp], '0') == 0)  //Count number of zeros
		{
			zeros_cnter++;
		}
	}

	if (zeros_cnter==(int_temp_y_axis_max-1)) // if '0000' in 10000
	{
		new_y_axis_max=atoi(temp_y_axis_max); // No need to change the Y-axis's scaling

	}
	else
	{
		int extra_digit=0;
		int round_up_value=temp_y_axis_max[0]-'0';

		if (round_up_value<9)  //Check if need to increase the value
		{
			round_up_value++;
			extra_digit=0;
		}
		else
		{
			round_up_value=1;
			extra_digit=1;

		}

		for (int roundVal=0;roundVal<=(int_temp_y_axis_max-1+extra_digit); roundVal++)  //Round values
		{
			if (roundVal==0)
			{
				memset(new_temp_y_axis_max, 0, sizeof(new_temp_y_axis_max));
				new_temp_y_axis_max[0]=(round_up_value+'0');
			}
			else
			{
				new_temp_y_axis_max[roundVal]='0';
			}
		}

		new_y_axis_max=atoi(new_temp_y_axis_max);
	}


	return  new_y_axis_max;
}

void tab_3(lv_obj_t * parent)
{
	//Brightness
	brightness_obj = lv_obj_create(parent);
	lv_obj_set_size(brightness_obj,800,80);
	lv_obj_align(brightness_obj,  LV_ALIGN_TOP_LEFT, -20, -20);
	lv_obj_set_scrollbar_mode(brightness_obj, LV_SCROLLBAR_MODE_OFF);
	lv_obj_add_style(brightness_obj, &style_bkgnd, 0);

	//Brightness slider
	slider= lv_slider_create(brightness_obj);
	lv_obj_set_size(slider,300, 25);
	lv_obj_align(slider,  LV_ALIGN_CENTER, 0, 0);
	lv_slider_set_range(slider, 5, 100);
	lv_slider_set_value(slider, 100, LV_ANIM_ON);

	//Add style to knob
	lv_style_init(&style_knob);
	lv_style_set_bg_opa(&style_knob, LV_OPA_COVER);
	lv_style_set_bg_color(&style_knob, lv_palette_main(LV_PALETTE_BLUE));
	lv_style_set_border_color(&style_knob, lv_palette_darken(LV_PALETTE_BLUE, 3));
	lv_style_set_border_width(&style_knob, 2);
	lv_style_set_radius(&style_knob, LV_RADIUS_CIRCLE);
	lv_style_set_pad_all(&style_knob, 6);
	lv_obj_add_style(slider, &style_knob, LV_PART_KNOB);

	lv_style_init(&style_knob_pressed_color);
	lv_style_set_bg_color(&style_knob_pressed_color, lv_palette_darken(LV_PALETTE_BLUE, 2));
	lv_obj_add_style(slider, &style_knob_pressed_color, LV_PART_INDICATOR | LV_STATE_PRESSED);
	BrightnessValue=100;  //Set default screen brightness to 100%
	BSP_LCD_SetBrightness(BrightnessValue);

	lv_obj_add_event_cb(slider, brightness_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	//Brightness label
	brightness_label = lv_label_create(brightness_obj);
	lv_label_set_text(brightness_label,"Brightness:");
	lv_obj_align(brightness_label,  LV_ALIGN_LEFT_MID, 0, 0);
	//Create a label below the slider
	brightness_level = lv_label_create(brightness_obj);
	lv_label_set_text(brightness_level, "100%");
	lv_obj_align(brightness_level,  LV_ALIGN_LEFT_MID, 120, 0);

	//Screen timeout
	sleep_obj = lv_obj_create(parent);
	lv_obj_set_size(sleep_obj,800,80);
	lv_obj_align(sleep_obj,  LV_ALIGN_TOP_LEFT, -20, 60);
	lv_obj_clear_flag(sleep_obj, LV_OBJ_FLAG_SCROLLABLE);
	//Label for screen timeout
	sleep_label = lv_label_create(sleep_obj);
	lv_label_set_text(sleep_label,"Screen Timeout:");
	lv_obj_align(sleep_label,  LV_ALIGN_LEFT_MID, 0, 0);
	scrntimeout_dd=lv_dropdown_create(sleep_obj);
	lv_dropdown_set_options(scrntimeout_dd, "Never\n"
											"30 sec\n"
											"1 min\n"
											"2 min\n"
											"5 min");
	lv_obj_set_size(scrntimeout_dd,150, 50);  //set size
	lv_obj_align(scrntimeout_dd, LV_ALIGN_TOP_MID, 0, -5);
	lv_obj_add_event_cb(scrntimeout_dd, scrntimeout_event_handler, LV_EVENT_ALL, NULL);

    //WIFI
    //Create a keyboard
    kb = lv_keyboard_create(lv_scr_act());
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);

    //Wifi panel
    wifi_obj = lv_obj_create(parent);
    lv_obj_set_size(wifi_obj,800,270);
    lv_obj_align(wifi_obj,  LV_ALIGN_TOP_LEFT, -20, 140);
    lv_obj_set_scrollbar_mode(wifi_obj, LV_SCROLLBAR_MODE_OFF);

    //Wifi label
    wifi_label = lv_label_create(wifi_obj);
    lv_label_set_text(wifi_label,"Wi-Fi");
    lv_obj_align(wifi_label,  LV_ALIGN_TOP_LEFT, 0, 10);

    //ON/OFF button
    wifi_switch = lv_switch_create(wifi_obj);
    lv_obj_set_size(wifi_switch,80,35);
    lv_obj_set_pos(wifi_switch,60,4);
    lv_obj_set_align(wifi_label, wifi_switch);

    //WIFI Status
    wifi_status_obj =lv_obj_create(wifi_obj);
    lv_obj_set_size(wifi_status_obj,400,50);
    lv_obj_set_align(wifi_status_obj, wifi_switch);
    lv_obj_set_x(wifi_status_obj,190);
    lv_obj_set_scrollbar_mode(wifi_status_obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(wifi_status_obj, LV_OBJ_FLAG_SCROLLABLE);
    wifi_status_label =lv_label_create(wifi_status_obj);
    lv_label_set_text(wifi_status_label,"Wi-Fi Status: Off");
    lv_obj_center(wifi_status_label);

    //WIFI name
    wifi_name_label =lv_label_create(wifi_obj);
    lv_label_set_text(wifi_name_label,"SSID:");
    lv_obj_align(wifi_name_label,  LV_ALIGN_CENTER ,-290, 0);
    wifi_name_txt = lv_textarea_create(wifi_obj);
    lv_obj_align(wifi_name_txt,  LV_ALIGN_CENTER ,-150, 0);
    lv_textarea_set_one_line(wifi_name_txt, true);
    lv_obj_set_scroll_dir(wifi_name_txt,LV_DIR_HOR);
    lv_textarea_set_placeholder_text(wifi_name_txt, "Wi-Fi Name");
    lv_obj_set_size(wifi_name_txt,200,50);
    lv_obj_add_event_cb(wifi_name_txt, ta_event_cb, LV_EVENT_ALL, kb);
    lv_keyboard_set_textarea(kb, wifi_name_txt);
    lv_obj_add_state(wifi_name_txt, LV_STATE_DISABLED);

    //WIFI password
    wifi_pwd_label=lv_label_create(wifi_obj);
    lv_label_set_text(wifi_pwd_label,"Password:");
    lv_obj_align(wifi_pwd_label,  LV_ALIGN_CENTER , 70, 0);
    wifi_pwd_txt = lv_textarea_create(wifi_obj);
    lv_obj_align(wifi_pwd_txt,  LV_ALIGN_CENTER , 235, 0);
    lv_textarea_set_one_line(wifi_pwd_txt, true);
    lv_textarea_set_password_mode(wifi_pwd_txt, true);
    lv_obj_set_scroll_dir(wifi_pwd_txt,LV_DIR_HOR);
    lv_textarea_set_placeholder_text(wifi_pwd_txt, "Wi-Fi Password");
    lv_obj_set_size(wifi_pwd_txt,200,50);
    lv_obj_add_event_cb(wifi_pwd_txt, ta_event_cb, LV_EVENT_ALL, kb);
    lv_keyboard_set_textarea(kb, wifi_pwd_txt);
    lv_obj_add_state(wifi_pwd_txt, LV_STATE_DISABLED);

    //Connect button
    connt_btn=lv_btn_create(wifi_obj);
    lv_obj_set_size(connt_btn,150,50);
    lv_obj_align(connt_btn,  LV_ALIGN_BOTTOM_MID, 0, -10);
    connt_label =lv_label_create(connt_btn);
    lv_label_set_text(connt_label,"Connect");
    lv_obj_center(connt_label);
    lv_obj_add_state(connt_btn, LV_STATE_DISABLED);
    lv_obj_add_event_cb(connt_btn, wifi_connt_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(wifi_switch, wifi_swch_event_handler, LV_EVENT_ALL, NULL);
    connt_cnt=0;  //Connect button status flag
}

/**********************
 *   EVENT HANDLERS
 **********************/

//brightness slider event handler
static void brightness_slider_event_cb(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * slider = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED)
    {
		char buf[8];
		lv_snprintf(buf, sizeof(buf), "%d%%", lv_slider_get_value(slider));
		lv_label_set_text(brightness_level, buf);
		BrightnessValue=atoi(buf);  //Range: 0 to 100
		BSP_LCD_SetBrightness(BrightnessValue);
    }
}

//Screen timeout buttons event handler
static void scrntimeout_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        char buf[32];
        lv_dropdown_get_selected_str(obj, buf, sizeof(buf));

        if (strcmp(buf,"Never")==0)
		{
        	scrnTimeout_flag=0;
        	MX_TIM5_DeInit();
		}
        else if (strcmp(buf,"30 sec")==0)
        {
        	scrnTimeout_flag=1;
        	TIM5_ReInit();
        	TIMER5_CTRL();
        }
        else if (strcmp(buf,"1 min")==0)
        {
        	scrnTimeout_flag=2;
        	TIM5_ReInit();
        	TIMER5_CTRL();
        }
        else if (strcmp(buf,"2 min")==0)
		{
        	scrnTimeout_flag=3;
        	TIM5_ReInit();
        	TIMER5_CTRL();
		}
        else if (strcmp(buf,"5 min")==0)
		{
        	scrnTimeout_flag=4;
        	TIM5_ReInit();
        	TIMER5_CTRL();
		}
    }
}

static void ta_event_cb(lv_event_t * e)  //Keyboard
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    lv_obj_t * kb = lv_event_get_user_data(e);
    if(code == LV_EVENT_FOCUSED) {

        if(lv_indev_get_type(lv_indev_get_act()) != LV_INDEV_TYPE_KEYPAD) {
            lv_keyboard_set_textarea(kb, ta);
            lv_obj_set_style_max_height(kb, LV_HOR_RES * 2 / 3, 0);
            lv_obj_update_layout(tabview);
            lv_obj_set_height(tabview, LV_VER_RES - lv_obj_get_height(kb));
            lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
            lv_obj_scroll_to_view_recursive(ta, LV_ANIM_OFF);
        }
    }
    else if(code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_set_height(tabview, LV_VER_RES);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
    else if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        lv_obj_set_height(tabview, LV_VER_RES);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_state(ta, LV_STATE_FOCUSED);
        lv_indev_reset(NULL, ta);
    }
}

static void wifi_swch_event_handler(lv_event_t * e)  //Event handler to turn Wi-Fi switch on/off
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        if (lv_obj_has_state(obj, LV_STATE_CHECKED)==1)  //If Wi-Fi switch is turned on
        {
            LV_LOG_USER("State: %s\n", lv_obj_has_state(obj, LV_STATE_CHECKED) ? "On" : "Off");
            lv_obj_clear_state(wifi_name_txt, LV_STATE_DISABLED);
            lv_obj_clear_state(wifi_pwd_txt, LV_STATE_DISABLED);
            lv_obj_clear_state(connt_btn, LV_STATE_DISABLED);
            lv_label_set_text_fmt(wifi_status_label, "%s","Wi-Fi Status: On");

        }
        else if (lv_obj_has_state(obj, LV_STATE_CHECKED)==0)  //If Wi-Fi switch is turned off
        {
            LV_LOG_USER("State: %s\n", lv_obj_has_state(obj, LV_STATE_CHECKED) ? "On" : "Off");
            lv_obj_add_state(wifi_name_txt, LV_STATE_DISABLED);
            lv_obj_add_state(wifi_pwd_txt, LV_STATE_DISABLED);
            lv_obj_add_state(connt_btn, LV_STATE_DISABLED);
            lv_label_set_text_fmt(wifi_status_label, "%s","Wi-Fi Status: Off");
            lv_label_set_text(connt_label,"Connect");
            HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_5,0);
            RESET_WIFI();
        }
    }
}

static void wifi_connt_event_handler(lv_event_t * e)  //Event handler to connect to Wi-Fi
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * btn = lv_event_get_target(e);
	lv_obj_t * label = lv_obj_get_child(btn, 0);
    if (connt_cnt==0 && code==LV_EVENT_CLICKED)  //Connect to Wi-Fi, MQTT, and NTP
    {
    	//Set WIFI credentials
    	WIFI_SSID=(char*)lv_textarea_get_text(wifi_name_txt);
    	WIFI_PWD=(char*)lv_textarea_get_text(wifi_pwd_txt);
    	lv_label_set_text(label, "Disconnect");
    	lv_label_set_text(wifi_status_label, "Wi-Fi Status: Connecting");
    	JN_WIFI(); // Initiate join Wi-Fi func
    	connt_cnt=1;  //Set connect counter to 1
    	//Disable text input into text boxes
    	lv_obj_add_state(wifi_name_txt, LV_STATE_DISABLED);
    	lv_obj_add_state(wifi_pwd_txt, LV_STATE_DISABLED);
    }
    else if (connt_cnt==1 && code==LV_EVENT_CLICKED)  //Disconnect from Wifi
    {
    	JNING_WIFI_flag=0;
    	shw_discnt_flag=0;
    	STOP_TIMER2();
    	lv_label_set_text(label, "Connect");
    	lv_label_set_text(wifi_status_label, "Wi-Fi Status: Not connected");
    	RESET_WIFI();  //Reset Wi-Fi
    	connt_cnt=0;  //Reset counter to 0
    	//Enable text input into text boxes
    	lv_obj_clear_state(wifi_name_txt, LV_STATE_DISABLED);
    	lv_obj_clear_state(wifi_pwd_txt, LV_STATE_DISABLED);
    	HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_5,0);
    }
}

void DISPLAY_CNTED(void)  //Connected
{
	lv_label_set_text(connt_label,"Disconnect");
	lv_label_set_text_fmt(wifi_status_label, "%s","Wi-Fi Status: Connected");
	connt_cnt=1;  //Set connect counter to 1
	//Disable text input into text boxes
	lv_obj_add_state(wifi_name_txt, LV_STATE_DISABLED);
	lv_obj_add_state(wifi_pwd_txt, LV_STATE_DISABLED);
}


void CNT_FAILED(void)  //Connection failed
{
	lv_label_set_text(wifi_status_label,"Wi-Fi Status: Connection failed");
	//Enable text input into text boxes
	lv_obj_clear_state(wifi_name_txt, LV_STATE_DISABLED);
	lv_obj_clear_state(wifi_pwd_txt, LV_STATE_DISABLED);
}
