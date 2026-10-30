/*
 * RTC.h
 *
 *  Created on: Jan 21, 2026
 *      Author: wegi0
 */

#ifndef my_RTC_H_
#define my_RTC_H_

#define EEPROM_AD 0x57

#define RTC_BKP_DR1 1   // * LIGHT VALUE 0 TO 100 brightness
#define RTC_BKP_DR2 2   // * COLOR FONTS ENABLED 0 = ENABLED  != 0 DISABLED
#define RTC_BKP_DR3 3   // * NIGHT MODE ENABLED 0 = ENABLED  != 0 DISABLED
#define RTC_BKP_DR6 6   // * slideshow mode enabled/disabled
#define RTC_BKP_DR7 7   // * night light value
#define RTC_BKP_DR8 8   // * READ NTP ENABLED 0 = ENABLED  != 0 DISABLED
#define RTC_BKP_DR10 10 // * CENTURY
#define RTC_BKP_DR11 11 // * reRead_time // 77 = not reRead time after 4h

#include "stdbool.h"
#include <stdint.h>
 
 

extern void print_RTC(void);
extern void read_RTC(void);
extern void colon_print(void);
extern void clear_colon(void);
extern void print_txt_colon(void);
extern const char *getDayofweek(uint8_t number);
extern int calc_dow(int y, int m, int d);
extern void clear_week_field(void);
extern void stop_colon(void);

extern void test_print_RTC(void);
extern void test_setup(void);
extern void wait_for_releasseTS(void);
extern bool clear_ss(void);
extern void LCD_ClrScr(uint16_t color);
extern bool set_hours(void);
extern bool test_pressed_point(uint16_t xmin, uint16_t xmax, uint16_t ymin, uint16_t ymax);
extern void put_number(uint8_t data);
extern uint8_t test_action(void);
extern bool set_minuts(void);
extern bool set_year(void);
extern void set_RTC_DATE(void);
extern void check_RTC_DATE(void);
extern void put_DATE_number(void);
extern bool set_month(void);
extern bool set_day(void);
extern void Seconds_print(int ss);
extern bool setting_globals(void);
extern void CHECK_CALIBRATE_TIME_VALUES(void);
extern void compare_EPOCH();
extern void set_EPOCH(void);
extern void read_timestamp(void);
extern void READ_CENTURY(void);
extern void SET_CENTURY(void);
extern void paint_test(void);
extern void TTF_test(void);
extern void V_BALL(void);
extern void set_LIGHT(void);
extern void night_mode(void);
extern void set_PWM(void);
extern void set_night_PWM(void);
extern void night_light(void);
extern void check_night_mode(void);
extern void NTP_CALIBRATE_SETUP(void);
extern uint8_t calibr_action(void);
extern void sec_calibrate_print(void);
extern void hrs_calibrate_print(void);
extern void COLOR_FONTS(void);
extern void set_color_RTC(void);
extern void slideshow_disable(void);
extern void check_slideshow();
extern void test_page(void);
extern bool go_to_back(void);
extern bool go_to_back2(void);
extern uint32_t YEAR_NOW;

//---
extern void next_page(void);
//---
extern void DEMOS_RUN(void);
extern void SLIDE_SHOW(void);
extern void copy_my_time_to_old(void);
extern void clear_week_field(void);
//---
extern void TP_task(void *pParams);
extern void colon_task(void *pParams);
extern void resume_colon(void);
extern void setManualTime(void);
extern void displayTime(void);

extern bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap);
void read_Pic_From_Flash(uint32_t pic_no);
//---
extern void start_WiFi(void) ;
extern void WiFi_connect_task(void *pParams);
extern uint8_t BIN_to_BCD(uint8_t val);
extern void READ_YEAR (void);
extern void Print_structure(struct tm *s_time);
extern void Read_DS3231(void);
extern void store_TM_to_DS3231(struct tm * s_save);
extern void save_to_RTC(struct tm * s_save);
extern void resume_colon(void);

extern void next3_globals(void);
extern void NTP_SET(void);
extern void LVGL_DEMO1(void);
extern void LVGL_DEMO2(void);
extern void setup_LVGL(void);
extern void DS3231_reRead_set();

extern void print_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
//---

#endif /* my_RTC_H_ */
