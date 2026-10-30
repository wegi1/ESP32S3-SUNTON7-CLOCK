/*
 * RTC.c
 *
 *  Created on: Jan 21, 2026
 *      Author: wegi0
 */

//===================================================================================================================

#include "time.h"
#include "stdio.h"
#include "my_RTC.h"
#include "ADT_F.h"
#include "types_h.h"
#include "inttypes.h"
#include <demos.h>

#include <fonty.h>
#include <boldfonts.h>
#include "display/Arduino_RPi_DPI_RGBPanel.h"
#include <DS3231.h>
#include <TAMC_GT911.h>
#include "main_app.h"

#include <SPIFFS.h>
#include <TJpg_Decoder.h>

#include <WiFi.h>
#include <NTPClient.h>

#include <lvgl.h>
#include <demos/lv_demos.h>

// #include <freertos/task.h>

void LVGL_Timer_Callback(TimerHandle_t xTimer);
static uint32_t my_tick_cb(void);

//================================================================================================
// #define RTC_BKP_DR1   1 // * LIGHT VALUE 0 TO 100 brightness
// #define RTC_BKP_DR2   2 // * COLOR FONTS ENABLED 0 = ENABLED  != 0 DISABLED
// #define RTC_BKP_DR3   3 // * NIGHT MODE ENABLED 0 = ENABLED  != 0 DISABLED
// #define RTC_BKP_DR6   6 // * slideshow mode enabled/disabled
// #define RTC_BKP_DR7   7 // * night light value
// #define RTC_BKP_DR8   8 // * READ NTP ENABLED 0 = ENABLED  != 0 DISABLED
// #define RTC_BKP_DR9   9 // * YEAR mod 100 work with CENTURY
// #define RTC_BKP_DR10 10 // * CENTURY
//===================================================================================================================
extern Arduino_RPi_DPI_RGBPanel *Screen;
extern DS3231 *wegi_clock;
extern volatile bool pressed;
extern volatile uint16_t possition_x;
extern volatile uint16_t possition_y;
//===================================================================================================================
extern TAMC_GT911 Touch;
extern char tekst[];

extern uint16_t lcd_text_color;
extern uint16_t lcd_background_color;
extern TaskHandle_t xHandle;
extern TimerHandle_t LVGL_Timer;

uint8_t brightness = 40; // RTC_BKP_DR1 & RTC_BKP_DR7

extern uint32_t shifted;
extern uint8_t RUN_LVGL;

extern struct tm old_time;
extern struct tm my_time;
extern struct tm *time_RD;
static const char cls_scr[] = {"\033[2J\033[3J\033[92m\033[0;0f"};

// struct timeval tvw;

int32_t century = 21; // RTC_BKP_DR10
int32_t SECONDS_TO_CALIBRATE = 0;
uint32_t HOURS_CALIBRATE_PERIOD = 1;
uint8_t UPDATE_FLAG = 0;

uint8_t change_colors = 1; // RTC_BKP_DR2
uint8_t run_slideshow = 0; // RTC_BKP_DR6
uint8_t mode_night = 0;    // RTC_BKP_DR3

String ssid = "EmptySSID_DATA";
String pswd = "EmptyPSWD_DATA";
uint8_t WiFI_connection = 0;
uint8_t USE_NTP;
//---
uint32_t NTP_counter = 0;
uint32_t DS3231_counter = 0;
uint8_t reRead_time = 0; // 77 = not reread time after 4h
uint8_t time_Reread = 0;
//---
uint8_t max_day; // maximum day in month
const char *file_names[] = {"/0.jpg", "/1.jpg", "/2.jpg", "/3.jpg", "/4.jpg", "/5.jpg", "/6.jpg", "/7.jpg", "/8.jpg", "/9.jpg", "/10.jpg", "/11.jpg", "/12.jpg"};
//---

//===================================================================================================================
//	RTC_BKP_DR0 ROTATION TEST MODE - NOT USED
//
//	RTC_BKP_DR1 LIGHT VALUE 0 TO 100 brightness
//	RTC_BKP_DR2 COLOR FONTS ENABLED 0 = ENABLED  != 0 DISABLED
//	RTC_BKP_DR3 NIGHT MODE ENABLED 0 = ENABLED  != 0 DISABLED
//	RTC_BKP_DR4 HOURS of calibrate interval
//	RTC_BKP_DR5  SECONDS TO CALIBRATE
//	RTC_BKP_DR6 slideshow mode enabled/disabled
//	RTC_BKP_DR7 night light value
//	RTC_BKP_DR8 epoch LOW 32 bits
//	RTC_BKP_DR9 epoch high 32 bits
//	RTC_BKP_DR10 century
//===================================================================================================================
// * RTC_BKP_DR1 LIGHT VALUE 0 TO 100 brightness
// * RTC_BKP_DR2 COLOR FONTS ENABLED 0 = ENABLED  != 0 DISABLED
// * RTC_BKP_DR3 NIGHT MODE ENABLED 0 = ENABLED  != 0 DISABLED
// * RTC_BKP_DR6 slideshow mode enabled/disabled
// * RTC_BKP_DR7 night light value
// * RTC_BKP_DR10 century
// * RTC_BKP_DR11 reRead_time // 77 = not reread time after 4h
//===================================================================================================================

//===================================================================================================================
//===================================================================================================================
void print_RTC(void)
{

    if (reRead_time != 77)
    {
        if (time_Reread != 0)
        {
            setManualTime();
            time_Reread = 0;
            Serial.println("reRead time done...");
        }
    }
    check_night_mode();

    int hh = my_time.tm_hour, mm = my_time.tm_min, ss = my_time.tm_sec;
    int yy;

    set_color_RTC();

    old_time.tm_sec = my_time.tm_sec;

    // Seconds_print(ss);
    sprintf(tekst, "%02d", ss);                                  // prepare seconds
    render_text_monospace(0, 295, &tekst[0], digital_7_ttf, 65); // print prepared seconds

    sprintf(tekst, "%02d", mm);
    render_text_monospace(396, 0, tekst, digital_7_ttf, 230);
    sprintf(tekst, "%02d", hh);
    render_shiftedtext_monospace(0, 0, &tekst[0], digital_7_ttf, 230);

    if (ss == 0)
    { // update weekday field
        if (hh == 0)
        { // between 00:00:00 and 00:07:00
            if (mm < 8)
            {
                clear_week_field();
            } // END if(mm < 8)
        } // END if(hh == 0) { // between 00:00:00 and 00:07:00
    } // END if(ss == 0) { // update weekday field
    my_time.tm_wday = calc_dow((int)(((century - 1) * 100) + (my_time.tm_year % 100)), (int)(my_time.tm_mon + 1), (int)my_time.tm_mday);
    lcd_text_boxed(190, 300, getDayofweek((uint8_t)my_time.tm_wday), SourceCodePro_Bold_ttf, 140);

    yy = (YEAR_NOW) + ((century - 1) * 100);
    sprintf(tekst, "%04d-%02d-%02d", yy, (my_time.tm_mon + 1), my_time.tm_mday);
    render_text_monospace(0, 380, &tekst[0], digital_7_ttf, 80);
}
//===================================================================================================================
void Seconds_print(int ss)
{
    resume_colon();                                              // print colon for half second blink
    sprintf(tekst, "%02d", ss);                                  // prepare seconds
    render_text_monospace(0, 295, &tekst[0], digital_7_ttf, 65); // print prepared seconds
}
//===================================================================================================================
void copy_my_time_to_old(void)
{
    old_time.tm_hour = my_time.tm_hour;
    old_time.tm_isdst = my_time.tm_isdst;
    old_time.tm_mday = my_time.tm_mday;
    old_time.tm_min = my_time.tm_min;
    old_time.tm_mon = my_time.tm_mon;
    old_time.tm_sec = my_time.tm_sec;
    old_time.tm_wday = my_time.tm_wday;
    old_time.tm_yday = my_time.tm_yday;
    old_time.tm_year = my_time.tm_year;
}
//===================================================================================================================
void read_RTC(void)
{
    time_t now;
    time(&now); // get actual TIME from RTC as EPOCH time

    localtime_r(&now, &my_time); // convert readed time from EPOCH data into tm structure
    if ((my_time.tm_year & 0x03) != (YEAR_NOW & 0x03))
    {
        YEAR_NOW++;
        YEAR_NOW = YEAR_NOW % 100;
    }
    my_time.tm_year = YEAR_NOW;
}
//===================================================================================================================
const char *getDayofweek(uint8_t number)
{
    if (number == 0)
    {
        number = 1;
    }
    if (number > 7)
    {
        number = 7;
    }
    const char *weekday[] = {"MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY", "SUNDAY"};
    return weekday[number - 1];
}
// DS3231
// reg 04 date (day of month) range 0-31 BCD
// reg 05 month range 1-12 BCD
// reg 06 years range 0-99 BCD
//===================================================================================================================
int calc_dow(int y, int m, int d)
{
    int result;

    y -= m < 3;
    result = (y + y / 4 - y / 100 + y / 400 + "-bed=pen+mad."[m] + d) % 7;
    if (result == 0)
    {
        result = 7;
    }
    return result;
}
//===================================================================================================================
//===================================================================================================================
void set_color_RTC(void)
{
    lcd_text_color = 0xffff;
    lcd_background_color = 0x0000;

    if ((change_colors & 0x01) != 0)
    {
        return;
    }

    if (my_time.tm_min < 15)
    {
        lcd_text_color = LCD_COLOR565_GREEN;
        return;
    }
    if (my_time.tm_min < 30)
    {
        lcd_text_color = LCD_COLOR565_RED;
        return;
    }
    if (my_time.tm_min < 45)
    {
        lcd_text_color = LCD_COLOR565_MAGENTA;
        return;
    }
}
//===================================================================================================================
void test_print_RTC(void)
{
    uint8_t tmpr2, tmpr3;
    read_RTC();

    tmpr2 = my_time.tm_min;
    int ss = my_time.tm_sec;

    if (my_time.tm_sec != old_time.tm_sec)
    {
        old_time.tm_sec = my_time.tm_sec;
        if (my_time.tm_sec == 0)
        {
            print_RTC();
            return;
        }
        if (ss == 5)
        {
            if (run_slideshow == 0)
            {
                if (tmpr2 % 5 == 0)
                {
                    (void)Screen->setBackLight(100);
                    tmpr3 = tmpr2 / 5;

                    read_Pic_From_Flash(tmpr3);
                    delay(8000);
                    for (int t = 100; t > -1; t--)
                    {
                        (void)Screen->setBackLight((uint8_t)t);
                        delay(50);
                    }
                    LCD_ClrScr(lcd_background_color);
                    // (void) Screen->setBackLight( 33);
                    check_night_mode();
                    delay(50);
                    print_RTC();
                    return;
                }
            }
        }

        Seconds_print(my_time.tm_sec);
        return;
    }
}
//===================================================================================================================
//-------------------------------------
extern void clear_week_field(void)
{
    Screen->fillRect(190, 294, 610, 84, lcd_background_color);
}
//-------------------------------------
//=====================================================================================================================
void print_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
#define PIXEL_BYTES_WIDTH 2
#define PIXELS_SCREEN_WIDTH 800

    uint16_t *SCREEN_BUFF = Screen->getFramebuffer();
    uint16_t *pixel_addr = SCREEN_BUFF;
    pixel_addr = pixel_addr + (y * PIXELS_SCREEN_WIDTH);
    pixel_addr = pixel_addr + x;
    uint32_t size = w * PIXEL_BYTES_WIDTH;

    for (uint32_t line = 0; line < h; line++)
    {
        for (uint32_t pixels_count = 0; pixels_count < w; pixels_count++)
        {
            pixel_addr[pixels_count] = color;
        }
        Cache_WriteBack_Addr((uint32_t)pixel_addr, size);
        pixel_addr = pixel_addr + PIXELS_SCREEN_WIDTH;
    }
}
void colon_print(uint16_t color)
{
    Screen->fillRect(385, 80, 30, 30, color);
    Screen->fillRect(385, 181, 30, 30, color);
    // print_rect(385, 80, 30, 30, color);
    // print_rect(385, 181, 30, 30, color);
}
//=====================================================================================================================
void clear_colon(void)
{
    colon_print(lcd_background_color);
}
//=====================================================================================================================
void print_txt_colon(void)
{
    colon_print(lcd_text_color);
}
//===================================================================================================================
bool test_pressed_point(uint16_t xmin, uint16_t xmax, uint16_t ymin, uint16_t ymax)
{
    if (possition_x > xmin)
    {
        if (possition_x < xmax)
        {
            if (possition_y >= ymin)
            {
                if (possition_y < ymax)
                {
                    return true;
                }
            }
        }
    }
    return false;
}
//===================================================================================================================
void wait_for_releasseTS(void)
{
    pressed = true;
    while (1)
    {
        delay(30);
        Touch.read();
        if (Touch.isTouched == false)
        {
            break;
        }
    }
    Touch.read();
    pressed = false;
}
//===================================================================================================================
void test_setup(void)
{
    if (pressed == 0)
    {
        return; // no touch pressed so good bye
    }
    vTaskSuspend(xHandle);
    //---
    if (clear_ss() == true)
    {
        return; // clear seconds test}
    }
    //---
    if (set_hours() == true)
    {
        return;
    }
    //---
    if (set_minuts() == true)
    {
        return;
    }
    //---
    if (set_year() == true)
    {
        return;
    }
    //---
    if (set_month() == true)
    {
        return;
    }
    //---
    if (set_day() == true)
    {
        return;
    }
    //---
    if (setting_globals() == true)
    {
        LCD_ClrScr(0);
        print_RTC();
        wait_for_releasseTS(); // wait for releasse TS
        delay(55);
        return;
    }
    //---

    wait_for_releasseTS();
}
//===================================================================================================================
bool clear_ss(void)
{
    if (possition_x < 170)
    {
        if (possition_y > 290)
        {
            if (possition_y < 380)
            {
                //---
                wegi_clock->Read_Time(&my_time);

                my_time.tm_sec = 59;

                wegi_clock->SECONDS_write(59);
                setManualTime();
                delay(110);
                print_RTC();
                wait_for_releasseTS(); // wait for releasse TS
                return true;
            }
        }
    }
    return false;
}
//===================================================================================================================
void LCD_ClrScr(uint16_t color)
{
    Screen->cls(color);
}
//===================================================================================================================
bool set_hours(void)
{
    uint8_t action, bckp, chk_hrs;
    read_RTC();
    chk_hrs = my_time.tm_hour;
    if ((test_pressed_point(0, 390, 0, 290)) == true)
    {

        LCD_ClrScr(lcd_background_color);
        while (1)
        {
            lcd_text_color = 0xffff;
            lcd_text_boxed(100, 0, "SET HOURS", SourceCodePro_Bold_ttf, 140);
            put_number(my_time.tm_hour);
            Touch.read();
            pressed = false;
            action = test_action();
            if ((action == 3))
            { // DONE pressed
                bckp = my_time.tm_hour;
                wegi_clock->Read_Time(&my_time); // refresh time data
                my_time.tm_hour = bckp;
                wegi_clock->HOURS_write(bckp);
                setManualTime();
                //---
                LCD_ClrScr(lcd_background_color);
                wait_for_releasseTS();
                print_RTC();

                return true;
            }
            else
            {
                if (action == 2)
                { // PLUS pressed
                    if (my_time.tm_hour == 23)
                    {
                        my_time.tm_hour = 0;
                    }
                    else
                    {
                        my_time.tm_hour++;
                    }
                }
                else
                { // MINUS pressed
                    if (my_time.tm_hour == 0)
                    {
                        my_time.tm_hour = 23;
                    }
                    else
                    {
                        my_time.tm_hour--;
                    }
                }
            }
        }
    }
    return false;
}
//===================================================================================================================
void put_number(uint8_t data)
{
    put_DATE_number();
    sprintf(tekst, "%02d", data);
    lcd_mono_text_boxed(200, 100, tekst, digital_7_ttf, 220);
}
//===================================================================================================================
uint8_t test_action(void)
{
    while (1)
    {

        if (pressed != 0)
        {
            delay(100);
            pressed = 0;
            // DONE test
            if ((test_pressed_point(210, 590, 390, 479)) == true)
            {
                return 3;
            } // DONE
            if ((test_pressed_point(600, 799, 100, 390)) == true)
            {
                return 2;
            } // PLUS
            if ((test_pressed_point(0, 200, 100, 389)) == true)
            {
                return 1;
            } // MINUS
        }
    }
}
//===================================================================================================================
void put_DATE_number(void)
{
    lcd_text_color = 0xffff;
    lcd_mono_text_boxed(565, 100, "+", digital_7_ttf, 230);
    uint32_t old_shifted = shifted;
    shifted = 35;
    render_shiftedtext_monospace(0, 100, "-", digital_7_ttf, 230);
    lcd_text_boxed(300, 400, "DONE", SourceCodePro_Bold_ttf, 120);

    Screen->drawRect(197, 100, 400, 290, 0xffff); // number rect
    Screen->drawRect(0, 100, 190, 290, 0xffff);   // MINUS rect
    Screen->drawRect(605, 100, 190, 290, 0xffff); // PLUS rect
    Screen->drawRect(197, 390, 400, 89, 0xffff);  // DONE rect

    shifted = old_shifted;
}
//===================================================================================================================
bool set_minuts(void)
{

    uint8_t action, bckp, chk_min;
    read_RTC();
    chk_min = my_time.tm_min;
    if ((test_pressed_point(400, 799, 0, 290)) == true)
    {
        LCD_ClrScr(lcd_background_color);
        while (1)
        {
            lcd_text_color = 0xffff;
            lcd_text_boxed(80, 0, "SET MINUTS", SourceCodePro_Bold_ttf, 140);
            put_number(my_time.tm_min);
            Touch.read();
            pressed = false;
            action = test_action();
            if ((action == 3))
            { // DONE pressed
                bckp = my_time.tm_min;
                wegi_clock->Read_Time(&my_time); // refresh time data
                my_time.tm_min = bckp;
                wegi_clock->MINUTS_write(bckp);
                setManualTime();
                //---

                LCD_ClrScr(lcd_background_color);
                wait_for_releasseTS();
                print_RTC();

                return true;
            }
            else
            {
                if (action == 2)
                { // PLUS pressed
                    if (my_time.tm_min == 59)
                    {
                        my_time.tm_min = 0;
                    }
                    else
                    {
                        my_time.tm_min++;
                    }
                }
                else
                { // MINUS pressed
                    if (my_time.tm_min == 0)
                    {
                        my_time.tm_min = 59;
                    }
                    else
                    {
                        my_time.tm_min--;
                    }
                }
            }
        }
    }
    return false; // doesn't choose set hours
}
//===================================================================================================================
// Todo: test
bool set_year(void)
{
    uint8_t action;
    int bckp;

    if ((test_pressed_point(0, 280, 380, 479)) == true)
    {
        Serial.printf("year check : %04d\n\r", my_time.tm_year);
        LCD_ClrScr(lcd_background_color);

        while (1)
        {
            lcd_text_color = 0xffff;
            lcd_text_boxed(120, 0, "SET YEAR", SourceCodePro_Bold_ttf, 140);
            sprintf(tekst, "%04d", (int)((my_time.tm_year % 100) + ((century - 1) * 100)));
            lcd_mono_text_boxed(250, 190, tekst, digital_7_ttf, 80);
            put_DATE_number();
            Touch.read();
            pressed = false;
            action = test_action();
            check_RTC_DATE(); // check and set Year between 100-199

            if ((action == 3))
            {                                    // DONE pressed
                bckp = my_time.tm_year;          // store changed value
                wegi_clock->Read_Time(&my_time); // refresh time data
                my_time.tm_year = bckp;          // set the new year value

                set_RTC_DATE(); // store date to RTC from my_time structure
                YEAR_NOW = (uint32_t)(my_time.tm_year % 100);
                LCD_ClrScr(lcd_background_color);
                //---
                wegi_clock->Set_Time(&my_time);

                setManualTime();
                //---
                wait_for_releasseTS();
                print_RTC();
                return true;
            }
            else
            {
                if (action == 2)
                { // PLUS pressed
                    if (my_time.tm_year == 199)
                    {
                        my_time.tm_year = 100;
                    }
                    else
                    {
                        my_time.tm_year++;
                    }
                }
                else
                { // MINUS pressed
                    if (my_time.tm_year == 100)
                    {
                        my_time.tm_year = 199;
                    }
                    else
                    {
                        my_time.tm_year--;
                    }
                    // Serial.printf("my_time.tm_year = %d\n", my_time.tm_year);
                }
            }
        }
    }
    return false; // doesn't choose set year
}
//===================================================================================================================
void set_RTC_DATE(void)
{
    struct timeval tvw;
    check_RTC_DATE();
    // store setup value
    time_t t = mktime(&my_time); // convert my_time struct to EPOCH RTC values
    tvw.tv_sec = t;
    tvw.tv_usec = 0;
    settimeofday(&tvw, NULL); // Set the RTC clock from the EPOCH value
}
void check_RTC_DATE(void)
{
    static const uint8_t month_maxdays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint8_t luty_add = 0;

    my_time.tm_year = 100 + (my_time.tm_year % 100);

    if ((my_time.tm_year % 4) == 0)
    {
        luty_add = 1;
    }
    if (my_time.tm_mon == -1)
    {
        my_time.tm_mon = 11;
    }
    if (my_time.tm_mon > 11)
    {
        my_time.tm_mon = 0;
    }
    max_day = month_maxdays[my_time.tm_mon];
    if (my_time.tm_mon == 1)
    {
        max_day = max_day + luty_add;
    }
    if (my_time.tm_mday == 0)
    {
        my_time.tm_mday = 1;
    }

    if (my_time.tm_mday > max_day)
    {
        my_time.tm_mday = max_day;
    }

    my_time.tm_wday = calc_dow((int)(((century - 1) * 100) + (my_time.tm_year % 100)), (int)(my_time.tm_mon + 1), (int)my_time.tm_mday);
}
//===================================================================================================================
bool set_month(void)
{
    uint8_t action;
    if ((test_pressed_point(290, 500, 380, 479)) == true)
    {
        LCD_ClrScr(lcd_background_color);
        wait_for_releasseTS();
        delay(150);
        while (1)
        {
            lcd_text_color = 0xffff;
            lcd_text_boxed(100, 0, "SET MONTH", SourceCodePro_Bold_ttf, 140);
            sprintf(tekst, "%02d", (my_time.tm_mon + 1));
            lcd_mono_text_boxed(320, 190, tekst, digital_7_ttf, 80);

            put_DATE_number();
            Touch.read();
            pressed = false;
            action = test_action();
            check_RTC_DATE();
            if ((action == 3))
            { // DONE pressed
                set_RTC_DATE();
                //---
                wegi_clock->MONTH_write((uint8_t)(my_time.tm_mon + 1));
                setManualTime();
                setManualTime();
                //---

                LCD_ClrScr(lcd_background_color);
                wait_for_releasseTS();
                print_RTC();
                return true;
            }
            else
            {
                if (action == 2)
                { // PLUS pressed
                    if (my_time.tm_mon == 11)
                    {
                        my_time.tm_mon = 0;
                    }
                    else
                    {
                        my_time.tm_mon++;
                    }
                }
                else
                { // MINUS pressed
                    if (my_time.tm_mon == 0)
                    {
                        my_time.tm_mon = 11;
                    }
                    else
                    {
                        my_time.tm_mon--;
                    }
                }
            }
        }
    }
    return false; // doesn't choose set year
}
//===================================================================================================================
bool set_day(void)
{
    uint8_t action;
    if ((test_pressed_point(520, 799, 380, 479)) == true)
    {
        wait_for_releasseTS();
        delay(150);
        LCD_ClrScr(lcd_background_color);
        check_RTC_DATE(); // get max day in month
        while (1)
        {
            lcd_text_color = 0xffff;
            lcd_text_boxed(160, 0, "SET DAY", SourceCodePro_Bold_ttf, 140);
            sprintf(tekst, "%02d", (my_time.tm_mday));
            lcd_mono_text_boxed(320, 190, tekst, digital_7_ttf, 80);
            put_DATE_number();
            Touch.read();
            pressed = false;
            action = test_action();
            check_RTC_DATE();
            if ((action == 3))
            { // DONE pressed
                set_RTC_DATE();
                LCD_ClrScr(lcd_background_color);
                //---
                wegi_clock->MDAY_write((uint8_t)my_time.tm_mday);
                setManualTime();
                setManualTime();
                //---
                print_RTC();
                wait_for_releasseTS();
                return true;
            }
            else
            {
                if (action == 2)
                { // PLUS pressed
                    if (my_time.tm_mday == max_day)
                    {
                        my_time.tm_mday = 1;
                    }
                    else
                    {
                        my_time.tm_mday++;
                    }
                }
                else
                { // MINUS pressed
                    if (my_time.tm_mday == 1)
                    {
                        my_time.tm_mday = max_day;
                    }
                    else
                    {
                        my_time.tm_mday--;
                    }
                }
            }
        }
    }
    return false; // doesn't choose set year
}
//===================================================================================================================
bool setting_globals(void)
{
    if ((test_pressed_point(170, 800, 290, 380)) != true)
    { // was pressed SETTING GLOBALS menu?
        return false;
    }
    LCD_ClrScr(lcd_background_color);

    while (1)
    {
        Screen->setBackLight(100);
        lcd_text_color = 0XFFFF;
        lcd_background_color = 0;

        //---
        lcd_text_boxed(25, 10, "GLOBAL FUNCTIONS", SourceCodePro_Bold_ttf, 100);
        //---
        lcd_text_boxed(90, 116, "PAINT", SourceCodePro_Bold_ttf, 100);
        lcd_text_boxed(470, 116, "BIGTTF", SourceCodePro_Bold_ttf, 100);
        lcd_text_boxed(90, 212, "LIGHT", SourceCodePro_Bold_ttf, 100);
        lcd_text_boxed(470, 212, "COLORS", SourceCodePro_Bold_ttf, 100);
        lcd_text_boxed(111, 308, "AUTO", SourceCodePro_Bold_ttf, 100);
        lcd_text_boxed(470, 308, "*NTP*", SourceCodePro_Bold_ttf, 100);
        lcd_text_boxed(110, 404, "EXIT", SourceCodePro_Bold_ttf, 100);
        lcd_text_boxed(510, 404, "NEXT", SourceCodePro_Bold_ttf, 100);
        //---
        Screen->drawFastVLine(400, 96, 384, 0xffff); // set partial menu draw

        Screen->drawRect(0, 96, 800, 384, 0xffff); // draw full frame of menu

        Screen->drawFastHLine(0, 192, 800, 0xffff); // 1st 2 pos of menu
        Screen->drawFastHLine(0, 288, 800, 0xffff); // 2nd 2 pos of menu
        Screen->drawFastHLine(0, 384, 800, 0xffff); // 3rd 2 pos of menu

        wait_for_releasseTS();
        delay(250);
        pressed = 0;
        while (pressed == 0)
        {
            asm volatile("NOP");
        } // wait for pressed

        if (go_to_back() == true)
        {
            Screen->setBackLight(brightness);
            return true;
        }
        paint_test();
        TTF_test();
        set_LIGHT();
        COLOR_FONTS();
        night_mode();
        NTP_CALIBRATE_SETUP();

        test_page();
    }
}
//===================================================================================================================
bool go_to_back(void)
{
    if ((test_pressed_point(0, 400, 384, 480)) == true)
    {
        wait_for_releasseTS(); // wait for releasse TS
        LCD_ClrScr(lcd_background_color);
        delay(100);
        return true;
    }
    return false; // doesn't choose set year
}
//===================================================================================================================
extern void TTF_test(void)
{
    if ((test_pressed_point(400, 800, 96, 192)) != true)
    {
        return;
    }
    Big_TTF_Demo();
    wait_for_releasseTS();
    LCD_ClrScr(lcd_background_color);
}
//===================================================================================================================
void set_LIGHT(void)
{

    if ((test_pressed_point(0, 400, 192, 288)) != true)
    {
        return;
    }
    uint32_t lendraw;

    LCD_ClrScr(lcd_background_color);
    lcd_text_boxed(120, 0, "SET LIGHT", SourceCodePro_Bold_ttf, 140);
    lcd_text_boxed(300, 400, "DONE", SourceCodePro_Bold_ttf, 120);
    Screen->drawRect(200, 390, 400, 89, 0xffff); // DONE rect
    Screen->drawRect(0, 185, 800, 115, 0xffff);  // slider
    wait_for_releasseTS();
    brightness = (uint8_t)wegi_clock->CLOCK_READ_REG(EEPROM_AD, RTC_BKP_DR1);
    possition_x = brightness << 3;
    possition_y = 200; // set first time pressed values
    pressed = 1;

    lendraw = (brightness << 3);
    Serial.printf("lendraw = %03d\n\r", lendraw);
    Screen->fillRect(4, 190, lendraw, 105, LCD_COLOR565_GREEN);
    Screen->fillRect(4 + lendraw, 190, 795 - lendraw, 105, 0);
    sprintf(tekst, "%03d%%", (int)brightness);
    lcd_render_just_text(250, 190, tekst, digital_7_ttf, 80, 0, 0);

    while (1)
    {
        if (pressed != 0)
        {
            pressed = 0;
            if ((test_pressed_point(210, 590, 390, 479)) == true)
            { // done pressed

                wegi_clock->CLOCK_WRITE_REG(EEPROM_AD, RTC_BKP_DR1, (uint32_t)brightness);
                wait_for_releasseTS();
                delay(111);
                LCD_ClrScr(lcd_background_color);
                return;
            } // DONE
            if (test_pressed_point(0, 800, 185, 300) == true)
            {
                brightness = (possition_x >> 3); // x / 8
                if (brightness >= 100)
                {
                    brightness = 99;
                }
                if (brightness < 2)
                {
                    brightness = 2;
                }
                (void)Screen->setBackLight(brightness);

                lendraw = (brightness << 3);

                Screen->fillRect(4, 190, lendraw, 105, LCD_COLOR565_GREEN);
                Screen->fillRect(4 + lendraw, 190, 795 - lendraw, 105, 0);
                sprintf(tekst, "%03d%%", (int)brightness);
                lcd_render_just_text(250, 190, tekst, digital_7_ttf, 80, 0, 0);

                pressed = 0;
                delay(10);
            }
        }
    }
}
//===================================================================================================================
void night_mode(void)
{
    uint8_t action;
    uint32_t n_mode;

    if ((test_pressed_point(0, 400, 288, 384)) != true)
    {
        return;
    }

    LCD_ClrScr(lcd_background_color);
    lcd_text_boxed(80, 0, "NIGHT MODE", SourceCodePro_Bold_ttf, 140);
    put_DATE_number();

    n_mode = (uint32_t)wegi_clock->CLOCK_READ_REG(EEPROM_AD, RTC_BKP_DR3); // check enabled night mode

    while (1)
    {
        //===================================================================================================

        n_mode = n_mode & 0x01;

        lcd_text_color = 0;
        lcd_mono_text_boxed(300, 190, "000", digital_7_ttf, 80);
        lcd_text_color = 0xffff;

        if (n_mode != 0)
        {
            sprintf(tekst, "NO!");
            brightness = (uint8_t)wegi_clock->CLOCK_READ_REG(EEPROM_AD, RTC_BKP_DR1);
        }
        else
        {
            sprintf(tekst, "YES");
            brightness = (uint8_t)wegi_clock->CLOCK_READ_REG(EEPROM_AD, RTC_BKP_DR7);
        }

        if (brightness < 2)
        {
            brightness = 2;
        }
        if (brightness > 100)
        {
            brightness = 100;
        }

        Screen->setBackLight(brightness);

        lcd_mono_text_boxed(300, 190, tekst, digital_7_ttf, 80);
        // if(n_mode  == 0) {
        // 	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR7, brightness);
        // }
        //===================================================================================================
        check_RTC_DATE();
        action = test_action();
        if ((action == 3))
        { // DONE pressed
            read_RTC();
            if (n_mode == 0)
            {
                wegi_clock->CLOCK_WRITE_REG(EEPROM_AD, RTC_BKP_DR7, (uint32_t)brightness);
            }
            n_mode = n_mode & 0x01;
            wegi_clock->CLOCK_WRITE_REG(EEPROM_AD, RTC_BKP_DR3, n_mode);
            // read back data from backup register to variable
            mode_night = (uint8_t)wegi_clock->CLOCK_READ_REG(EEPROM_AD, RTC_BKP_DR3);

            LCD_ClrScr(lcd_background_color);
            wait_for_releasseTS(); // wait for releasse TS

            if (mode_night == 0)
            {
                night_light();
            }
            check_night_mode();

            return;
        }
        else
        {
            wait_for_releasseTS(); // wait for releasse TS
            if (action == 2)
            { // PLUS pressed
                n_mode++;
            }
            else
            { // MINUS pressed
                n_mode--;
            }
        }
    }
}
//===================================================================================================================
void check_night_mode(void)
{

#define nigh_value 20

    int hh;
    read_RTC();
    hh = my_time.tm_hour;

    if ((mode_night & 1) == 0)
    {
        if (hh < 6)
        {
            set_night_PWM();
            return;
        }
        if (hh > (nigh_value - 1))
        {
            set_night_PWM();
            return;
        }
    }
    set_PWM(); // out of night mode turn on the light as day mode
}
//===================================================================================================================
void set_PWM(void)
{
    uint32_t light_val;
    light_val = (uint32_t)wegi_clock->CLOCK_READ_REG(EEPROM_AD, RTC_BKP_DR1);

    if (light_val > 100)
    {
        light_val = 100;
    }
    if (light_val < 2)
    {
        light_val = 2;
    }
    wegi_clock->CLOCK_WRITE_REG(EEPROM_AD, RTC_BKP_DR1, light_val);
    brightness = light_val;
    Screen->setBackLight((uint8_t)brightness);
}
//---
void set_night_PWM(void)
{
    uint32_t light_val;
    light_val = (uint32_t)wegi_clock->CLOCK_READ_REG(EEPROM_AD, RTC_BKP_DR7);
    if (light_val > 100)
    {
        light_val = 100;
    }
    if (light_val < 2)
    {
        light_val = 2;
    }
    wegi_clock->CLOCK_WRITE_REG(EEPROM_AD, RTC_BKP_DR7, light_val);
    brightness = (uint8_t)light_val;
    Screen->setBackLight((uint8_t)brightness);
}
//===================================================================================================================
void night_light(void)
{
    pressed = 1;

    LCD_ClrScr(lcd_background_color);
    lcd_text_boxed(120, 0, "SET NIGHT", SourceCodePro_Bold_ttf, 140);
    lcd_text_boxed(300, 400, "DONE", SourceCodePro_Bold_ttf, 120);
    Screen->drawRect(200, 390, 400, 89, 0xffff); // DONE rect
    Screen->drawRect(0, 185, 800, 115, 0xffff);  // slider
    wait_for_releasseTS();
    possition_x = brightness << 3;
    possition_y = 200; // set first time pressed values
    pressed = 1;

    while (1)
    {
        if (pressed != 0)
        {
            pressed = 0;
            if ((test_pressed_point(210, 590, 390, 479)) == true)
            { // done pressed
                wegi_clock->CLOCK_WRITE_REG(EEPROM_AD, RTC_BKP_DR7, (uint32_t)brightness);
                wait_for_releasseTS();
                delay(111);
                LCD_ClrScr(lcd_background_color);
                return;
            } // DONE
            if (test_pressed_point(0, 800, 185, 300) == true)
            {
                brightness = (possition_x >> 3); // x / 8
                if (brightness >= 100)
                {
                    brightness = 99;
                }
                if (brightness < 2)
                {
                    brightness = 2;
                }
                (void)Screen->setBackLight(brightness);

                uint32_t lendraw = (brightness << 3);

                Screen->fillRect(4, 190, lendraw, 105, LCD_COLOR565_RED);
                Screen->fillRect(4 + lendraw, 190, 795 - lendraw, 105, 0);
                sprintf(tekst, "%03d%%", (int)brightness);

                lcd_render_just_text(250, 190, tekst, digital_7_ttf, 80, 0, 0);

                pressed = 0;
                delay(10);
            }
        }
    }
}
//===================================================================================================================
void COLOR_FONTS(void)
{
    uint8_t action;
    // uint32_t font_VAL = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR2);

    uint32_t font_VAL = (uint32_t)wegi_clock->CLOCK_READ_REG(EEPROM_AD, RTC_BKP_DR2);

    if ((test_pressed_point(400, 800, 192, 288)) != true)
    {
        return;
    }

    LCD_ClrScr(lcd_background_color);
    lcd_text_color = 0xffff;
    lcd_text_boxed(40, 0, "COLOR FONTS", SourceCodePro_Bold_ttf, 140);
    put_DATE_number();

    while (1)
    {
        font_VAL = font_VAL & 0x01;
        lcd_text_color = 0;
        lcd_mono_text_boxed(300, 190, "000", digital_7_ttf, 80);
        lcd_text_color = 0xffff;

        if (font_VAL != 0)
        {
            sprintf(tekst, "NO!");
        }
        else
        {
            sprintf(tekst, "YES");
        }
        lcd_mono_text_boxed(300, 190, tekst, digital_7_ttf, 80);

        wait_for_releasseTS();
        action = test_action();
        if ((action == 3))
        { // DONE pressed
            wegi_clock->CLOCK_WRITE_REG(EEPROM_AD, RTC_BKP_DR2, font_VAL);
            change_colors = (uint32_t)wegi_clock->CLOCK_READ_REG(EEPROM_AD, RTC_BKP_DR2);
            LCD_ClrScr(lcd_background_color);
            wait_for_releasseTS(); // wait for releasse TS
            delay(100);
            return;
        }
        else
        {
            if (action == 2)
            { // PLUS pressed
                font_VAL++;
            }
            else
            { // MINUS pressed
                font_VAL--;
            }
        }
    }
}
//===================================================================================================================
void test_page(void)
{
    if ((test_pressed_point(400, 800, 384, 480)) == true)
    {
        wait_for_releasseTS(); // wait for releasse TS
        LCD_ClrScr(lcd_background_color);

        Serial.println("test page = true");
        next_page();

        wait_for_releasseTS(); // wait for releasse TS
        LCD_ClrScr(lcd_background_color);
    }
}
//===================================================================================================================
void next_page(void)
{
    LCD_ClrScr(lcd_background_color);

    while (1)
    {
        Screen->setBackLight(100);
        lcd_text_color = 0XFFFF;
        lcd_background_color = 0;
        //---
        lcd_text_boxed(50, 25, "OTHER FUNCTIONS", SourceCodePro_Bold_ttf, 100);
        //---
        lcd_text_boxed(90, 154, "DEMOS", SourceCodePro_Bold_ttf, 100);
        lcd_mono_text_boxed(450, 140, "V_BALL", SourceCodePro_Bold_ttf, 100);
        lcd_text_boxed(70, 274, "SLIDES", SourceCodePro_Bold_ttf, 100);
        lcd_text_boxed(510, 274, "PICS", SourceCodePro_Bold_ttf, 100);
        lcd_text_boxed(110, 394, "EXIT", SourceCodePro_Bold_ttf, 100);
        lcd_text_boxed(510, 394, "NEXT", SourceCodePro_Bold_ttf, 100);

        Screen->drawFastVLine(400, 120, 360, 0xffff); // set partial menu draw
        Screen->drawRect(0, 120, 800, 360, 0xffff);   // draw full frame of menu
        Screen->drawFastHLine(0, 240, 800, 0xffff);   // 1st 2 pos of menu
        Screen->drawFastHLine(0, 360, 800, 0xffff);   // 2nd 2 pos of menu

        wait_for_releasseTS();
        delay(250);
        pressed = 0;
        while (pressed == 0)
        {
            asm volatile("NOP");
        }

        if (go_to_back2() == true)
        {
            return;
        }

        DEMOS_RUN();
        SLIDE_SHOW();
        V_BALL();
        slideshow_disable();
        next3_globals();

    } // end while(1)
}
//===================================================================================================================
bool go_to_back2(void)
{
    if ((test_pressed_point(0, 400, 360, 480)) == true)
    {
        wait_for_releasseTS(); // wait for releasse TS
        LCD_ClrScr(lcd_background_color);
        delay(100);
        return true;
    }
    return false; // doesn't choose set year
}
//===================================================================================================================
//=====================================================================================================================
void TP_task(void *pParams)
{
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    while (1)
    {
        if (pressed == false)
        {
            possition_x = 1111;
            possition_y = 1111;
            Touch.read();
            if (Touch.isTouched == true)
            {
                if ((Touch.points[0].x) < 800)
                {
                    if ((Touch.points[0].y) < 480)
                    {
                        pressed = true;
                        possition_x = Touch.points[0].x;
                        possition_y = Touch.points[0].y;
                    }
                }
            }
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}
//================================
//===================================================================================================================
//=====================================================================================================================
void colon_task(void *pParams)
{
    vTaskSuspend(NULL);

    while (1)
    {
        print_txt_colon();
        vTaskDelay(500 / portTICK_PERIOD_MS);
        clear_colon();
        vTaskSuspend(NULL);
    }
}
//================================
//===================================================================================================================
extern void resume_colon(void)
{
    vTaskResume(xHandle);
}
//===================================================================================================================
void setManualTime(void)
{
    struct tm tm;
    struct timeval tvw;
    time_t now;

    READ_YEAR();

    time_RD->tm_year = 100 + (YEAR_NOW & 0x03);

    now = mktime(time_RD); // convert time_RD struct to EPOCH RTC values
    tvw.tv_sec = now;
    tvw.tv_usec = 0;

    // Ustawienie czasu systemowego (RTC)
    settimeofday(&tvw, NULL); // Set the RTC clock from the EPOCH value
    //---

    read_RTC();
    copy_my_time_to_old();
}
//===================================================================================================================
void displayTime(void)
{
    // retrieve data from DS3231
    Read_DS3231();
    READ_YEAR();

    int m_yr = (((century - 1) * 100) + (YEAR_NOW));

    Serial.printf("%04d-", m_yr);

    Serial.printf("%02d-", time_RD->tm_mon);
    Serial.printf("%02d *** ", time_RD->tm_mday);

    // send it to the serial monitor
    Serial.printf("%02d:", time_RD->tm_hour);
    // convert the byte variable to a decimal number when displayed
    Serial.printf("%02d:", time_RD->tm_min);
    Serial.printf("%02d *** ", time_RD->tm_sec);

    Serial.print("Day of week: ");
    uint8_t dow = (uint8_t)calc_dow(m_yr, time_RD->tm_mon, time_RD->tm_mday);
    Serial.println(getDayofweek(dow));
}
//===================================================================================================================
void paint_test(void)
{
    if ((test_pressed_point(0, 400, 96, 192)) != true)
    {
        return;
    }
    wait_for_releasseTS();
    delay(111);

    //=======================================
    Screen->cls(0);

    render_text_monospace(0, 0, "CLS", digital_7_ttf, 40);
    render_text_monospace(660, 420, "EXIT", digital_7_ttf, 40);

    while (1)
    {
        // Touch.read();
        // if (Touch.isTouched)
        if (pressed == true)
        {
            Screen->fillCircle(possition_x, possition_y, 10, RED);

            if (possition_x < 100)
            {
                if (possition_y < 60)
                {
                    Screen->cls(lcd_background_color);
                    render_text_monospace(0, 0, "CLS", digital_7_ttf, 40);
                    render_text_monospace(660, 420, "EXIT", digital_7_ttf, 40);
                    wait_for_releasseTS();
                    delay(111);
                }
            }
            else if (possition_x > 660)
            {
                if (possition_y > 420)
                {
                    wait_for_releasseTS();
                    Screen->cls(lcd_background_color);
                    delay(111);
                    return;
                }
            }
            pressed = false;
        }
    }
    //=======================================
}
//===================================================================================================================
void DEMOS_RUN(void)
{
    if ((test_pressed_point(0, 400, 120, 240)) != true)
    {
        return;
    }

    wait_for_releasseTS();
    delay(50);
    start_demos();
    return;
}
//===================================================================================================================
extern void V_BALL(void)
{
    if ((test_pressed_point(400, 800, 120, 240)) != true)
    {
        return;
    }
    wait_for_releasseTS();
    delay(111);
    main_app();
    wait_for_releasseTS();
    delay(111);
    LCD_ClrScr(lcd_background_color);
    return;
}
//===================================================================================================================
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
    // tft.pushImage(x, y, w, h, bitmap);

    uint32_t row, col, i;
    i = 0;

    for (row = 0; row < h; row++)
    {
        for (col = 0; col < w; col++)
        {
            Screen->writePixel((x + col), (y + row), (uint16_t)bitmap[i]);
            i++;
        }
    }

    return true; // Zwróć true, aby kontynuować dekodowanie
}
//===================================================================================================================
void read_Pic_From_Flash(uint32_t pic_no)
{
    if (pic_no > 12)
    {
        pic_no = 12;
    }
    TJpgDec.drawJpg(0, 0, file_names[pic_no]); // Lub drawJpg dla SPIFFS
}
//===================================================================================================================
void SLIDE_SHOW(void)
{
    uint8_t i;

    if ((test_pressed_point(0, 400, 240, 360)) != true)
    {
        return;
    }

    wait_for_releasseTS(); // wait for releasse TS

    LCD_ClrScr(lcd_background_color);
    (void)Screen->setBackLight(100);

    for (i = 0; i < 13; i++)
    {
        read_Pic_From_Flash(i);
        pressed = 0;
        while (pressed == 0)
        {
            asm volatile("NOP");
        }
        wait_for_releasseTS(); // wait for releasse TS
    }

    for (int t = 100; t > -1; t--)
    {
        (void)Screen->setBackLight((uint8_t)t);
        delay(50);
    }
    wait_for_releasseTS(); // wait for releasse TS
    LCD_ClrScr(lcd_background_color);
    check_night_mode();
}
//===================================================================================================================
extern void slideshow_disable(void)
{
    uint8_t s_mode, action;
    if ((test_pressed_point(400, 800, 240, 360)) != true)
    {
        return;
    }

    lcd_text_color = 0xffff;

    LCD_ClrScr(lcd_background_color);
    check_slideshow(); // read slide setup
    s_mode = run_slideshow;
    lcd_text_boxed(80, 0, "SLIDE MODE", SourceCodePro_Bold_ttf, 140);

    put_DATE_number();
    wait_for_releasseTS();
    delay(100);

    while (1)
    {
        s_mode = s_mode & 0x01;

        lcd_text_color = 0;
        lcd_mono_text_boxed(300, 190, "000", digital_7_ttf, 80);
        lcd_text_color = 0xffff;

        if (s_mode != 0)
        {
            sprintf(tekst, "NO!");
        }
        else
        {
            sprintf(tekst, "YES");
        }
        lcd_mono_text_boxed(300, 190, tekst, digital_7_ttf, 80);

        wait_for_releasseTS();
        action = test_action();
        if ((action == 3))
        { // DONE pressed
            wegi_clock->CLOCK_WRITE_REG(EEPROM_AD, RTC_BKP_DR6, (uint32_t)s_mode);
            check_slideshow();
            wait_for_releasseTS(); // wait for releasse TS
            LCD_ClrScr(lcd_background_color);
            delay(100);
            run_slideshow = s_mode;
            return;
        }
        else
        {
            if (action == 2)
            { // PLUS pressed
                s_mode++;
            }
            else
            { // MINUS pressed
                s_mode--;
            }
        }
    }
}
//===================================================================================================================
void SET_CENTURY(void)
{

    uint8_t action;
    read_RTC();
    READ_CENTURY();

    // if ((test_pressed_point(400, 800, 360, 480)) != true)
    if ((test_pressed_point(0, 400, 96, 192)) != true)
    {
        return;
    }
    wait_for_releasseTS();
    delay(150);

    LCD_ClrScr(lcd_background_color);
    lcd_text_boxed(40, 0, "SET CENTURY", SourceCodePro_Bold_ttf, 140);
    put_DATE_number();
    while (1)
    {
        //===================================================================================================
        sprintf(tekst, "%d", (int)century);
        lcd_text_color = 0;
        lcd_mono_text_boxed(300, 190, "000", digital_7_ttf, 80);
        lcd_text_color = 0xffff;
        lcd_mono_text_boxed(300, 190, tekst, digital_7_ttf, 80);
        //===================================================================================================
        check_RTC_DATE();

        Touch.read();
        pressed = false;

        action = test_action();
        delay(55);

        if ((action == 3))
        { // DONE pressed

            wegi_clock->CLOCK_WRITE_REG(EEPROM_AD, RTC_BKP_DR10, (uint32_t)century);

            read_RTC();
            set_RTC_DATE();
            LCD_ClrScr(lcd_background_color);
            wait_for_releasseTS();
            delay(100);
            return;
        }
        else
        {
            if (action == 2)
            { // PLUS pressed
                if (century == 99)
                {
                    century = -99;
                }
                else
                {
                    century++;
                }
            }
            else
            { // MINUS pressed
                if (century == -99)
                {
                    century = 99;
                }
                else
                {
                    century--;
                }
            }
        }
    }
}

//===================================================================================================================
void READ_CENTURY(void)
{
    century = (int32_t)wegi_clock->CLOCK_READ_REG(EEPROM_AD, RTC_BKP_DR10);
    if (century > 99)
    {
        century = 21;
        wegi_clock->CLOCK_WRITE_REG(EEPROM_AD, RTC_BKP_DR10, (uint32_t)century);
    }
    if (century < -99)
    {
        century = 21;
        wegi_clock->CLOCK_WRITE_REG(EEPROM_AD, RTC_BKP_DR10, (uint32_t)century);
    }
}
//===================================================================================================================
void check_slideshow()
{
    run_slideshow = (uint8_t)wegi_clock->CLOCK_READ_REG(EEPROM_AD, RTC_BKP_DR6) & 0x01;
}
//===================================================================================================================

void start_WiFi(void)
{

    TaskHandle_t WiFi_Task_Handle = 0;
    File file;

    WiFI_connection = 0;

    USE_NTP = (uint8_t)wegi_clock->CLOCK_READ_REG(EEPROM_AD, RTC_BKP_DR8);
    Serial.printf("USE_NTP value = %d\n", USE_NTP);
    if (USE_NTP != 0)
    {
        return;
    }

    file = SPIFFS.open("/prefs.ini");

    if (!file)
    {
        Serial.println("File /prefs.ini does not exists !");
        return;
    }

    Serial.println("File: '/prefs.ini exists'");
    WiFI_connection = 2; // file exist mark

    ssid = file.readStringUntil('\n');
    pswd = file.readStringUntil('\n');
    file.close();
    ssid = ssid.substring(0, ssid.length() - 1);
    pswd = pswd.substring(0, pswd.length() - 1);

    xTaskCreatePinnedToCore(WiFi_connect_task, "WiFi_connect", 8192, 0, 1, &WiFi_Task_Handle, 1);
}

//===================================================================================================================
void WiFi_connect_task(void *pParams)
{
    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    tm NTP_TIME;
    time_t EP_TIME;
    time_t reg_time;
    WiFiUDP Udp;

    NTPClient Ntp(Udp, "0.pl.pool.ntp.org", 0, 60 * 1000);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pswd.c_str());
    for (uint32_t i = 0; i < 360; i++)
    {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        Serial.print(".");

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());

            Ntp.begin();
            Ntp.update();
            WiFI_connection = 1;
            Serial.printf("WiFI_connection value = 1\n");

            WiFi.disconnect();
            Serial.println(Ntp.getFormattedTime()); // Wyświetla HH:MM:SS
            i = 360;
        }
    }

    if (WiFI_connection != 1)
    {
        Serial.printf("\nWiFI_connection value = %d\n", WiFI_connection);
    }

    if (WiFI_connection == 1)
    {
        if (USE_NTP == 0) // if store NTP time
        {

            EP_TIME = Ntp.getEpochTime();     // get EPOCH time from NTP server data into the EP_TIME time_t structure
            localtime_r(&EP_TIME, &NTP_TIME); // convert readed time from EPOCH data into tm structure

            wegi_clock->Set_Time(&NTP_TIME); // set the minuts and the seconds from the NTP server into DS3231

            setManualTime();
            Screen->cls(0);
            print_RTC();

            //---
            time(&reg_time); // get actual TIME from RTC as EPOCH time
            Serial.printf("RTC value = 0x%08X\n", reg_time);
            Serial.printf("RTC value =   %d\n", reg_time);
            Serial.printf("EPOCH EP_TIME = %d\n", EP_TIME);
            Serial.printf("localtime_r(&EP_TIME, &NTP_TIME) tm structure NTP_TIME :\n");
            Print_structure(&NTP_TIME);
            //---
        }
    }

    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL); // NULL dla obecnego zadania
    // Serial.print("Min. free stack (bytes): ");
    // Serial.println(uxHighWaterMark);

    vTaskDelete(NULL);
}
//===================================================================================================================
//===================================================================================================================

//===================================================================================================================
// TODO: setup time into i2c
// TODO: LVGL WIFI
// TODO: Calibr net time
//===================================================================================================================
//-------------------------------------------------------

//---
void READ_YEAR(void)
{
    Read_DS3231();
    YEAR_NOW = (uint32_t)time_RD->tm_year % 100;
}
//---
void Read_DS3231(void)
{
    wegi_clock->Read_Time(time_RD);
}
//---
//---
void Print_structure(struct tm *s_time)
{
    Serial.printf("\n");
    Serial.printf("Secs. value : %02d\n", s_time->tm_sec);
    Serial.printf("Mins. value : %02d\n", s_time->tm_min);
    Serial.printf("Hours value : %02d\n", s_time->tm_hour);
    Serial.printf("Mdays value : %02d\n", s_time->tm_mday);
    Serial.printf("Month value : %02d\n", s_time->tm_mon);
    Serial.printf("Year  value : %d \n", s_time->tm_year);
    Serial.printf("Wdays value : %d \n", s_time->tm_wday);
    Serial.printf("Ydays value : %d \n", s_time->tm_yday);
    Serial.printf("isdst value : %d \n\n", s_time->tm_isdst);

    Serial.print("DateTime : ");
    Serial.println(s_time, "%A, %Y-%m-%d %H:%M:%S");
}
//---
#include "soc/rtc_cntl_reg.h"
//===================================================================================================================
void NTP_CALIBRATE_SETUP(void)
{
    uint8_t WiFI_connection = 0;

    File file;

    WiFiUDP Udp;
    time_t EP_TIME;

    tm NTP_TIME;
    time_t reg_time;

    if ((test_pressed_point(400, 800, 288, 384)) != true)
    {
        return;
    }
    //=============================================
    Serial.printf(cls_scr);
    WiFI_connection = 0;

    file = SPIFFS.open("/prefs.ini");

    if (!file)
    {
        Serial.println("File /prefs.ini does not exists !");
        Screen->cls(0xF100);
        delay(3333);
        Screen->cls(0);
        return;
    }

    Serial.println("File: '/prefs.ini exists'");
    WiFI_connection = 2; // file exist mark

    ssid = file.readStringUntil('\n');
    pswd = file.readStringUntil('\n');
    file.close();
    ssid = ssid.substring(0, ssid.length() - 1);
    pswd = pswd.substring(0, pswd.length() - 1);
    Screen->cls(0x07E0); // green scrren

    //=============================================

    NTPClient Ntp(Udp, "0.pl.pool.ntp.org", 0, 60 * 1000);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pswd.c_str());
    for (uint32_t i = 0; i < 360; i++)
    {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        Serial.print(".");

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());

            Ntp.begin();
            Ntp.update();
            WiFI_connection = 1;
            Serial.printf("WiFI_connection value = 1\n");

            WiFi.disconnect();
            Serial.println(Ntp.getFormattedTime()); // Wyświetla HH:MM:SS
            i = 360;
        }
    }

    if (WiFI_connection != 1)
    {
        Serial.printf("\nWiFI_connection value = %d\n", WiFI_connection);
        Screen->cls(0xF100);
        delay(3333);
        Screen->cls(0);
        return;
    }

    //******************************************************************** */
    if (WiFI_connection == 1)
    {
        EP_TIME = Ntp.getEpochTime();     // get EPOCH time from NTP server data into the EP_TIME time_t structure
        localtime_r(&EP_TIME, &NTP_TIME); // convert readed time from EPOCH data into tm structure

        wegi_clock->Set_Time(&NTP_TIME); // set the minuts and the seconds from the NTP server into DS3231

        setManualTime();

        //****************************************************************** */
        read_RTC();

        Screen->cls(0x001F);
        delay(1111);
        Screen->cls(0);
        print_RTC();
        delay(1111);
        Screen->cls(0);

        //---
        time(&reg_time); // get actual TIME from RTC as EPOCH time
        Serial.printf("RTC value = 0x%08X\n", reg_time);
        Serial.printf("RTC value =   %d\n", reg_time);
        Serial.printf("EPOCH EP_TIME = %d\n", EP_TIME);
        Serial.printf("localtime_r(&EP_TIME, &NTP_TIME) tm structure NTP_TIME :\n");
        Print_structure(&NTP_TIME);
        //---
    }
}

//===================================================================================================================
void LVGL_Timer_Callback(TimerHandle_t xTimer)
{
    asm volatile("NOP");

    // Serial.println("BOYSAR od LVGL_Timer_Callback...");
    // xTimerStart(LVGL_Timer, 0);
    // xTimerStop(LVGL_Timer, 0);
}
//---
//===================================================================================================================
void manual_time_setup(TimerHandle_t xTimer)
{
    time_Reread = 1; // time up to reREad time into the RTC from DS3231
    // xTimerStart(timer_VAR, 0);
    // xTimerStop(timer_VAR, 0);
}
//---
//===================================================================================================================
extern void NTP_SET(void)
{
    uint8_t action;
    if ((test_pressed_point(400, 800, 96, 192)) != true)
    {
        return;
    }

    USE_NTP = (uint8_t)wegi_clock->CLOCK_READ_REG(EEPROM_AD, RTC_BKP_DR8);

    LCD_ClrScr(lcd_background_color);
    lcd_text_color = 0xffff;
    lcd_text_boxed(140, 0, "NTP USE", SourceCodePro_Bold_ttf, 140);
    put_DATE_number();
    //----------------------------------------------------------------------
    while (1)
    {
        USE_NTP = USE_NTP & 0x01;
        lcd_text_color = 0;
        lcd_mono_text_boxed(300, 190, "000", digital_7_ttf, 80);
        lcd_text_color = 0xffff;

        if (USE_NTP != 0)
        {
            sprintf(tekst, "NO!");
        }
        else
        {
            sprintf(tekst, "YES");
        }
        lcd_mono_text_boxed(300, 190, tekst, digital_7_ttf, 80);

        wait_for_releasseTS();
        action = test_action();
        if ((action == 3))
        { // DONE pressed
            wegi_clock->CLOCK_WRITE_REG(EEPROM_AD, RTC_BKP_DR8, USE_NTP);
            change_colors = (uint32_t)wegi_clock->CLOCK_READ_REG(EEPROM_AD, RTC_BKP_DR8);
            LCD_ClrScr(lcd_background_color);
            wait_for_releasseTS(); // wait for releasse TS
            delay(100);
            return;
        }
        else
        {
            if (action == 2)
            { // PLUS pressed
                USE_NTP++;
            }
            else
            { // MINUS pressed
                USE_NTP--;
            }
        }
    }
    //----------------------------------------------------------------------
    wait_for_releasseTS();
    LCD_ClrScr(lcd_background_color);
}

//===================================================================================================================
/*****************************************************************************
 * @brief LVGL 1st demo run
 */
void LVGL_DEMO1(void)
{
    if ((test_pressed_point(0, 400, 192, 288)) != true)
    {
        return;
    }
    wait_for_releasseTS(); // wait for releasse TS
    LCD_ClrScr(lcd_background_color);

    setup_LVGL();

    RUN_LVGL = 1;
    Screen->setBackLight(brightness);
    lv_demo_benchmark();

    while (RUN_LVGL != 0)
    {
        lv_timer_handler();
        delay(5);
    }
}
//===================================================================================================================
/*****************************************************************************
 * @brief LVGL 2nd demo run
 */
void LVGL_DEMO2(void)
{
    if ((test_pressed_point(400, 800, 192, 288)) != true)
    {
        return;
    }
    wait_for_releasseTS(); // wait for releasse TS
    LCD_ClrScr(lcd_background_color);

    setup_LVGL();

    RUN_LVGL = 1;
    Screen->setBackLight(brightness);

    lv_demo_widgets();

    while (RUN_LVGL != 0)
    {
        lv_timer_handler();
        delay(5);
    }
}

//===================================================================================================================
//===================================================================================================================
/***********************************************************************
 * @brief LVGL tick callback code
 */
static uint32_t my_tick_cb(void)
{
    return millis();
}
//---
/**********************************************************************
 * @brief display flushing callback
 */
/* Display flushing */
void my_disp_flush(lv_display_t *display, const lv_area_t *area, uint8_t *data)
{

    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);

    Screen->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)data, w, h);
    lv_display_flush_ready(display);
}
//---
/**************************************************************************
 * @breief read touchpad callback code
 */
/*Read the touchpad*/
void my_touchpad_read(lv_indev_t *indev_driver, lv_indev_data_t *data)
{
    if (Touch.touch_has_signal())
    {
        if (Touch.touch_touched())
        {
            data->state = LV_INDEV_STATE_PR;
            data->point.x = Touch.touch_last_x;
            data->point.y = Touch.touch_last_y;
            // Serial.printf("pressed : X = %d  Y = %d\n\r", data->point.x, data->point.y);
        }

        else if (Touch.touch_released())
        {
            data->state = LV_INDEV_STATE_REL;
        }
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
}
//---

//---
//===================================================================================================================
//===================================================================================================================
/******************************************************************************************
 * @brief initialzation of LVGL like screen buffer,
 *        LVGL register callbacks for touchscreen, tick and display flushing
 */
void setup_LVGL(void)
{
    static const uint16_t screenWidth = 800;
    static const uint16_t screenHeight = 480;
    static uint32_t buf_size = 800 * 480 * 2; // 800 * 480 * 2 / 10

    lv_init();
    lv_tick_set_cb(my_tick_cb);

    static lv_display_t *disp = lv_display_create(screenWidth, screenHeight);
    lv_display_set_flush_cb(disp, my_disp_flush);

    lv_color_t *buf1 = NULL;
    lv_color_t *buf2 = NULL;

    buf1 = (lv_color_t *)ps_malloc((size_t)buf_size);

    if (buf1 == NULL)
    {
        Serial.print("buf1 not initialized !!!\n\r");
        return;
    }
    Serial.printf("buf1  initialized successfully at : 0x%08X length : 0x%08X\n\r", buf1, buf_size);

    buf2 = (lv_color_t *)ps_malloc((size_t)buf_size);
    if (buf2 == NULL)
    {
        Serial.print("buf2 not initialized !!!\n\r");
        return;
    }
    Serial.printf("buf2  initialized successfully at : 0x%08X length : 0x%08X\n\r", buf2, buf_size);

    // LV_DISPLAY_RENDER_MODE_PARTIAL
    lv_display_set_buffers(disp, buf1, buf2, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);

    /*Initialize the display*/
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, my_touchpad_read);

    // lv_demo_benchmark();
}
//===================================================================================================================
//===================================================================================================================

//===================================================================================================================
//===================================================================================================================

//===================================================================================================================
//===================================================================================================================

//===================================================================================================================
/************************************************************************************
 * @brief 3rd menu to choice otions:
 * - set century of calendar
 * - use NTP (at the reset try read time from NTP server)
 * - LVGL1 - show demo of LVGL #1
 * - LVGL2 - show demo of LVGL #2
 * - set WiFi - setup of WiFi name and password
 * - set TZ - set string of TimeZone function
 * - Exit - back to 2nd menu
 * - DS3231 - function for enable/disable reread time from DS3231
 *   after every on 4 hours passed for eliminate RTC inaccuracy
 */
void next3_globals(void)
{

    if ((test_pressed_point(400, 800, 384, 480)) != true)
    {
        return;
    }
    wait_for_releasseTS(); // wait for releasse TS
    LCD_ClrScr(lcd_background_color);

    while (1)
    {
        Screen->setBackLight(100);
        lcd_text_color = 0XFFFF;
        lcd_background_color = 0;

        //---
        lcd_text_boxed(25, 10, "SETUP FUNCTIONS", SourceCodePro_Bold_ttf, 100);
        //---
        lcd_text_boxed(40, 116, "CENTURY", SourceCodePro_Bold_ttf, 100);
        lcd_text_boxed(440, 116, "USE NTP", SourceCodePro_Bold_ttf, 100);
        lcd_text_boxed(70, 212, "LVGL 1", SourceCodePro_Bold_ttf, 100);
        lcd_text_boxed(470, 212, "LVGL 2", SourceCodePro_Bold_ttf, 100);
        lcd_text_boxed(20, 308, "SET WiFi", SourceCodePro_Bold_ttf, 100);
        lcd_text_boxed(470, 308, "SET TZ", SourceCodePro_Bold_ttf, 100);
        lcd_text_boxed(110, 404, "EXIT", SourceCodePro_Bold_ttf, 100);
        lcd_text_boxed(470, 404, "DS3231", SourceCodePro_Bold_ttf, 100);
        //---
        Screen->drawFastVLine(400, 96, 384, 0xffff); // set partial menu draw

        Screen->drawRect(0, 96, 800, 384, 0xffff); // draw full frame of menu

        Screen->drawFastHLine(0, 192, 800, 0xffff); // 1st 2 pos of menu
        Screen->drawFastHLine(0, 288, 800, 0xffff); // 2nd 2 pos of menu
        Screen->drawFastHLine(0, 384, 800, 0xffff); // 3rd 2 pos of menu

        wait_for_releasseTS();
        delay(250);
        pressed = 0;
        while (pressed == 0)
        {
            asm volatile("NOP");
        } // wait for pressed

        if (go_to_back() == true)
        {
            Screen->setBackLight(brightness);
            wait_for_releasseTS(); // wait for releasse TS
            LCD_ClrScr(lcd_background_color);
            return;
        }
        SET_CENTURY();
        NTP_SET();

        ////LVGL_DEMO1(); // !!! restore
        ////LVGL_DEMO2(); // !!! restore

        // night_mode();
        // NTP_CALIBRATE_SETUP();
        DS3231_reRead_set();
    }
}

//===================================================================================================================
/**************************************************************************
 * @brief function for enable/disable reread time from DS3231
 *        after every on 4 hours passed
 *        for eliminate RTC inaccuracy
 */
extern void DS3231_reRead_set()
{

    if ((test_pressed_point(400, 800, 384, 480)) != true)
    {
        return;
    }

    wait_for_releasseTS(); // wait for releasse TS
    LCD_ClrScr(lcd_background_color);

    uint8_t s_mode, action;

    lcd_text_color = 0xffff;

    LCD_ClrScr(lcd_background_color);
    check_slideshow(); // read slide setup
    s_mode = reRead_time & 0x01;

    lcd_text_boxed(40, 0, "DS3231 READ", SourceCodePro_Bold_ttf, 140);

    put_DATE_number();
    wait_for_releasseTS();
    delay(100);

    while (1)
    {
        s_mode = s_mode & 0x01;
        lcd_text_color = 0;
        lcd_mono_text_boxed(300, 190, "000", digital_7_ttf, 80);
        lcd_text_color = 0xffff;

        if (s_mode != 0)
        {
            sprintf(tekst, "NO!");
        }
        else
        {
            sprintf(tekst, "YES");
        }
        lcd_mono_text_boxed(300, 190, tekst, digital_7_ttf, 80);

        wait_for_releasseTS();
        action = test_action();
        if ((action == 3))
        { // DONE pressed
            if (s_mode != 0)
            {
                s_mode = 77;
            }
            wegi_clock->CLOCK_WRITE_REG(EEPROM_AD, RTC_BKP_DR11, (uint32_t)s_mode);
            check_slideshow();
            wait_for_releasseTS(); // wait for releasse TS
            LCD_ClrScr(lcd_background_color);
            delay(100);
            run_slideshow = s_mode;
            return;
        }
        else
        {
            if (action == 2)
            { // PLUS pressed
                s_mode++;
            }
            else
            { // MINUS pressed
                s_mode--;
            }
        }
    }
}
//===================================================================================================================
