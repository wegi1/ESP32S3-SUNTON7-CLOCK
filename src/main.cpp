//=====================================================================================================================
#include <Arduino.h>
#include <7inch_board.h>
#include <sys/time.h>
#include "time.h"
#include <DS3231.h>
#include <WiFi.h>
#include <NTPClient.h>

//======================================================================================================================

#include "my_RTC.h"
#include <ADT_F.h>
#include <SPIFFS.h>

#include <TJpg_Decoder.h>
#include "soc/rtc_cntl_reg.h"

#include <lvgl.h>
#include <demos/lv_demos.h>

//=====================================================================================================================
//=====================================================================================================================
#define I2C_SDA 17
#define I2C_SCL 18
//=====================================================================================================================
//---
static const char clr_scr[] = {"\033[2J\033[3J\033[92m\033[0;0f"};
static const char init_scr[] = {"\033[92m"};
//---
//=====================================================================================================================

//=====================================================================================================================
volatile bool pressed = false;
volatile uint16_t possition_x = 1111;
volatile uint16_t possition_y = 1111;
TimerHandle_t LVGL_Timer;
TimerHandle_t Manual_Timer;
uint32_t adr_bmp = 0;
TaskHandle_t xHandle = 0;
uint16_t *ramka;
// tm czas;
struct tm old_time;
struct tm my_time;
//---
struct tm *time_RD = new tm;
uint32_t YEAR_NOW;
//---
// DS3231 wegi_clock(I2C_SDA, I2C_SCL) ;
DS3231 *wegi_clock = new DS3231(I2C_SDA, I2C_SCL);
//=====================================================================================================================
extern uint16_t lcd_text_color;
extern uint16_t lcd_background_color;
extern uint32_t now_time;

extern uint8_t brightness;    // RTC_BKP_DR1
extern uint8_t change_colors; // RTC_BKP_DR2
extern uint8_t mode_night;    // RTC_BKP_DR3
extern uint8_t run_slideshow; // RTC_BKP_DR6
extern int32_t century;       // RTC_BKP_DR10
extern uint32_t reRead_time;  // RTC_BKP_DR11

uint8_t RUN_LVGL = 0;
//=====================================================================================================================
//=====================================================================================================================

//=====================================================================================================================
// #define RTC_BKP_DR1 1   // * LIGHT VALUE 0 TO 100 brightness
// #define RTC_BKP_DR2 2   // * COLOR FONTS ENABLED 0 = ENABLED  != 0 DISABLED
// #define RTC_BKP_DR3 3   // * NIGHT MODE ENABLED 0 = ENABLED  != 0 DISABLED
// #define RTC_BKP_DR6 6   // * slideshow mode enabled/disabled
// #define RTC_BKP_DR7 7   // * night light value
// #define RTC_BKP_DR8 8   // * READ NTP ENABLED 0 = ENABLED  != 0 DISABLED
// #define RTC_BKP_DR8 9   // * YEAR mod 100 work with CENTURY
// #define RTC_BKP_DR10 10 // * CENTURY
//=====================================================================================================================

void setup(void)
{
  extern void manual_time_setup(TimerHandle_t xTimer);
  extern void LVGL_Timer_Callback(TimerHandle_t xTimer);
  xTaskCreatePinnedToCore(colon_task, "colon_task", 2000, 0, 1, &xHandle, 0);
  xTaskCreatePinnedToCore(TP_task, "TP_task", 4096, 0, 1, NULL, 0);
  LVGL_Timer = xTimerCreate("LVGL_Timer", pdMS_TO_TICKS(5), pdTRUE, (void *)0, LVGL_Timer_Callback);
  Manual_Timer = xTimerCreate("Manual_Timer", pdMS_TO_TICKS(1000 * 3600 * 4), pdTRUE, (void *)1, manual_time_setup); // 4h period

  Serial.begin(115200);
  Serial.printf(init_scr);
  Serial.println("Setup complete");
  displayTime();
  Serial.printf("Manual_Timer value = 0x%08X \n", (int32_t)Manual_Timer);
  Serial.printf("LVGL_Timer value = 0x%08X \n", (int32_t)LVGL_Timer);

  xTimerStart(Manual_Timer, 0);

  // setenv("TZ", "GMT-1GMT-2,M3.5.0/2,M10.5.0/3", 1);
  setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1); //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
  tzset();

  BoardInit(true, 30);
  Screen->fillScreen(BLACK);

  if (!SPIFFS.begin())
  {
    Serial.println("Błąd SPIFFS");
    return;
  }

  Serial.println("SPIFFS initialized");
  // Inicjalizacja dekodera
  TJpgDec.setJpgScale(1); // Skala: 1, 2, 4, 8
  TJpgDec.setSwapBytes(false);
  TJpgDec.setCallback(tft_output);

  //---
  // wegi_clock->CLOCK_WRITE_REG(EEPROM_AD, 0, 0xdeadbeef);
  // wegi_clock->CLOCK_WRITE_REG(EEPROM_AD, 9, 26);  // write YEAR % 100
  // wegi_clock->CLOCK_WRITE_REG(EEPROM_AD, RTC_BKP_DR1, 40); // 40%
  // wegi_clock->CLOCK_WRITE_REG(EEPROM_AD, RTC_BKP_DR8, 0); // save enable of READ NTP
  //---

  brightness = (uint8_t)wegi_clock->CLOCK_READ_REG(EEPROM_AD, RTC_BKP_DR1);
  change_colors = (uint8_t)wegi_clock->CLOCK_READ_REG(EEPROM_AD, RTC_BKP_DR2);
  mode_night = (uint8_t)wegi_clock->CLOCK_READ_REG(EEPROM_AD, RTC_BKP_DR3);
  run_slideshow = (uint8_t)wegi_clock->CLOCK_READ_REG(EEPROM_AD, RTC_BKP_DR6);
  century = (int32_t)wegi_clock->CLOCK_READ_REG(EEPROM_AD, RTC_BKP_DR10);
  reRead_time = (uint8_t)wegi_clock->CLOCK_READ_REG(EEPROM_AD, RTC_BKP_DR11);
  READ_YEAR(); // read year from DS3231 into YEAR_NOW

  Serial.printf("reRead_time value   : %d \n", reRead_time);
  Serial.printf("beef value          : 0x%08X \n", wegi_clock->CLOCK_READ_REG(EEPROM_AD, 0));
  Serial.printf("brightness value    : %d percent\n", brightness);

  Serial.printf("change_colors value : %d \n", change_colors);
  Serial.printf("mode_night value    : %d \n", mode_night);
  Serial.printf("run_slideshow value : %d \n", run_slideshow);

  Serial.printf("century value       : %d \n", century);

  Screen->setCursor(0, 18);
  Screen->setTextColor(RED);
  Screen->setTextSize(4, 4, 2);

  int cpuSpeed = getCpuFrequencyMhz();
  Serial.println("Running at " + String(cpuSpeed) + "MHz");
}
//=====================================================================================================================
//=====================================================================================================================

//=====================================================================================================================
//=====================================================================================================================
void loop()
{
  struct timeval tv;
  asm volatile("NOP");

  Screen->setBackLight((brightness * 255 / 100) & 0xff);
  ramka = Screen->getFramebuffer();
  Serial.printf("Framebuffer addres : = 0x%.8lX\n", (uint32_t)ramka);

  setManualTime();

  delay(111);
  //===================================================
  // Screen->setBackLight(100);

  // read_Pic_From_Flash(12);
  // delay(5000);
  // for (int t = 100; t > -1; t--)
  // {
  //   (void)Screen->setBackLight((uint8_t)t);
  //   delay(50);
  // }

  //====================================================

  Screen->cls(0);
  Screen->setBackLight((brightness * 255 / 100) & 0xff);
  start_WiFi();

  print_RTC();

  //---

  //---

  while (1)
  {
    delay(50);
    test_print_RTC();
    test_setup();
  }
  //==================================
}
//=====================================================================================================================
//=====================================================================================================================

//=====================================================================================================================
//=====================================================================================================================

//=====================================================================================================================
//=====================================================================================================================
