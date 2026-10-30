/*
 * demos.cpp
 *
 *  Created on: Feb 1, 2026
 *      Author: wegi0
 */
//===================================================================================================================
#include "demos.h"
#include "my_RTC.h"
#include "types_h.h"

//===================================================================================================================
#include "display/Arduino_RPi_DPI_RGBPanel.h"
#include <TAMC_GT911.h>
#include <esp_random.h>
#include <rom/cache.h>

//===================================================================================================================

void half_sec_delay(void);
void LCD_Put_Pixel(uint32_t x, uint32_t y, uint16_t Color);
void DelayMicro(uint32_t micros);

void demo_01(void);
void demo_02(void);
void demo_03(void);
void demo_04(void);
void demo_05(void);
void demo_06(void);
void demo_07(void);
void demo_08(void);
void demo_09(void);
void demo_10(void);
void demo_11(void);
void demo_12(void);
void demo_13(void);

uint32_t randomize(void);
//---
static void wait_t(void);
static void LCD_CLS_BLACK(void);
static uint32_t test_stop(void);
static void prepare_screen_line_address(void);
static void LCD_CLS(uint16_t color);

//===================================================================================================================
#define LCD_ROW_NUM 480 // Y lines
#define LCD_COL_NUM 800 // X pixels

#define sq_size 200
#define rect_size 100
#define t_wait 2
#define circle_size 230
//--------------------------------------------------------
//===================================================================================================================
extern Arduino_RPi_DPI_RGBPanel *Screen;
extern volatile bool pressed;
extern volatile uint16_t possition_x;
extern volatile uint16_t possition_y;
//===================================================================================================================
extern TAMC_GT911 Touch;
//===================================================================================================================
static uint16_t *pframe_Buff = NULL;
static uint32_t *lines_addr = NULL;

//===================================================================================================================
//===================================================================================================================
void start_demos(void)
{
    pframe_Buff = Screen->getFramebuffer();
    uint32_t *my_lines = new uint32_t[LCD_ROW_NUM];
    lines_addr = my_lines;
    prepare_screen_line_address();
    Serial.printf("line_addr pointer : 0x%08X\n\r", (uint32_t)lines_addr);
    Serial.printf("line_addr address : 0x%08X\n\r", (uint32_t)&lines_addr);
    Serial.printf("framebuffer address : 0x%08X\n\r", (uint32_t)Screen->getFramebuffer());
    Serial.printf("lines_addr[0] : 0x%08X\n\r", (uint32_t)lines_addr[0]);
    Serial.printf("lines_addr[1] : 0x%08X\n\r", (uint32_t)lines_addr[1]);
    Serial.printf("lines_addr[2] : 0x%08X\n\r", (uint32_t)lines_addr[2]);
    Serial.printf("&lines_addr[0] : 0x%08X\n\r", (uint32_t)&lines_addr[0]);
    Serial.printf("&lines_addr[1] : 0x%08X\n\r", (uint32_t)&lines_addr[1]);
    Serial.printf("&lines_addr[2] : 0x%08X\n\r", (uint32_t)&lines_addr[2]);
    demo_01();
    demo_02();
    demo_03();
    demo_04();
    demo_05();
    demo_06();
    demo_07();
    demo_08();
    demo_09();
    demo_10();
    demo_11();
    demo_12();
    demo_13();

    LCD_CLS_BLACK();
    delete[] my_lines;
    lines_addr = NULL;
}
//===================================================================================================================
void demo_01(void)
{
    LCD_CLS_BLACK();
    (void)Screen->setBackLight(100);
}
//===================================================================================================================
uint32_t randomize(void)
{
    return esp_random();
}
//===================================================================================================================
void half_sec_delay(void)
{
    delay(500);
}
//----------
static void wait_t(void) { delay(t_wait); }
static void LCD_CLS_BLACK(void) { LCD_CLS(0); }
static uint32_t test_stop(void) { return (uint32_t)pressed; }
static void LCD_CLS(uint16_t color)
{
    for (uint32_t i = 0; i < (LCD_COL_NUM * LCD_ROW_NUM); i++)
    {
        pframe_Buff[i] = color;
    }
    Cache_WriteBack_All();
}
//===================================================================================================================
static void prepare_screen_line_address(void)
{

    uint32_t data = (uint32_t)Screen->getFramebuffer();
    for (uint32_t i = 0; i < LCD_ROW_NUM; i++)
    {
        lines_addr[i] = data;
        data += 1600;
    }
}
//===================================================================================================================
void DelayMicro(uint32_t micros)
{
    delayMicroseconds(micros);
}
//===================================================================================================================
void LCD_Put_Pixel(uint32_t x, uint32_t y, uint16_t Color)
{
    uint16_t *PUT;
    PUT = (uint16_t *)(lines_addr[y]);
    PUT[x] = Color;
}
//===================================================================================================================
void demo_02(void)
{
    uint32_t i;
    LCD_CLS_BLACK();
    for (i = 0; i < 50000; i++)
    {
        LCD_Put_Pixel(randomize() % LCD_COL_NUM,
                      randomize() % LCD_ROW_NUM,
                      randomize() % 0xffff);
        DelayMicro(10);
        if (test_stop() != 0)
        {
            wait_for_releasseTS();
            half_sec_delay();
            LCD_CLS_BLACK();
            return;
        } // check the stop action
    }
    half_sec_delay();
}
//===================================================================================================================
void demo_03(void)
{
    uint32_t i;
    LCD_CLS_BLACK();

    for (i = 0; i < 1000; i++)
    {
        Screen->drawLine(randomize() % LCD_COL_NUM,
                         randomize() % LCD_ROW_NUM, randomize() % LCD_COL_NUM,
                         randomize() % LCD_ROW_NUM, randomize() % 0xFFFF);
        delay(1);
        if (test_stop() != 0)
        {
            wait_for_releasseTS();
            half_sec_delay();
            return;
        } // check the stop action
    }
    half_sec_delay();
}
//===================================================================================================================
void demo_04(void)
{
    u32 i;
    LCD_CLS_BLACK();

    for (i = 0; i < 1500; i++)
    {
        Screen->drawCircle(randomize() % 720 + 40,
                           randomize() % 400 + 40,
                           randomize() % 40 + 1,
                           randomize() % 0xFFFF);
        delay(1);
        if (test_stop() != 0)
        {
            wait_for_releasseTS();
            half_sec_delay();
            return;
        } // check the stop action
    }
    half_sec_delay();
}
//===================================================================================================================
void demo_05(void)
{
    u32 i, j, k, L, m;
    u16 x0, y0, x1, y1;
    u32 color;
    u16 linev[LCD_COL_NUM];
    u16 *pbuff_addr;
    LCD_CLS_BLACK();

    for (i = 0; i < LCD_ROW_NUM; i++)
    {
        Screen->drawFastHLine(0, i, 800, randomize() % 0xFFFF);
        if (test_stop() != 0)
        {
            wait_for_releasseTS();
            half_sec_delay();
            return;
        } // check the stop action
    }

    half_sec_delay();
    for (i = 0; i < LCD_COL_NUM; i++)
    {
        linev[i] = randomize() % 0xFFFF;
    }

    k = 0;
    L = (uint32_t)Screen->getFramebuffer();

    for (j = 0; j < LCD_ROW_NUM; j++)
    {
        m = k + L;
        pbuff_addr = (u16 *)m;
        for (i = 0; i < LCD_COL_NUM; i++)
        {
            pbuff_addr[i] = linev[i];
        }
        if (test_stop() != 0)
        {
            wait_for_releasseTS();
            half_sec_delay();
            return;
        } // check the stop action
        k = k + 1600;
    }
    Cache_WriteBack_All();

    half_sec_delay();

    x0 = 0;
    x1 = LCD_COL_NUM - 1;
    y0 = 0;
    y1 = LCD_ROW_NUM - 1;

    for (i = 0; i < LCD_ROW_NUM / 2; i++)
    {
        color = randomize() % 0xFFFF;
        Screen->drawRect(x0, y0, (x1 - x0), (y1 - y0), color);
        Cache_WriteBack_All();
        x0++;
        x1--;
        y0++;
        y1--;
        if (test_stop() != 0)
        {
            wait_for_releasseTS();
            half_sec_delay();
            return;
        } // check the stop action
    }
    half_sec_delay();
}
//===================================================================================================================
void demo_06(void)
{
    u32 x0, y0, x1, y1, i;
    u32 Color, tmpr;
    LCD_CLS_BLACK();
    for (i = 0; i < 700; i++)
    {
        Color = randomize() % 0xFFFF;
        x0 = randomize() % LCD_COL_NUM;
        x1 = randomize() % LCD_COL_NUM;
        y0 = randomize() % LCD_ROW_NUM;
        y1 = randomize() % LCD_ROW_NUM;

        if (x1 < x0)
        {
            tmpr = x0;
            x0 = x1;
            x1 = tmpr;
        }
        if (y1 < y0)
        {
            tmpr = y0;
            y0 = y1;
            y1 = tmpr;
        }

        Screen->fillRect(x0, y0, (x1 - x0), (y1 - y0), Color);

        if (test_stop() != 0)
        {
            wait_for_releasseTS();
            half_sec_delay();
            return;
        } // check the stop action
    }
    half_sec_delay();
}
//===================================================================================================================
void demo_07(void)
{
    u32 Color = RED; // COLOR_565_RED;
    u32 x0, y0, x1, y1;
    u8 movetype = 0; // x++ y++
    u32 counter = 0;
    x0 = 0;
    x1 = x0 + sq_size;
    y0 = 0;
    y1 = y0 + sq_size;

    LCD_CLS_BLACK();
    Screen->fillRect(x0, y0, (x1 - x0), (y1 - y0), Color);

    while (counter < 2000)
    {
        if (test_stop() != 0)
        {
            wait_for_releasseTS();
            half_sec_delay();
            return;
        } // check the stop action
        //-----------
        if (movetype == 0)
        {
            if (x0 < LCD_COL_NUM - sq_size)
            {
                if (y0 < LCD_ROW_NUM - sq_size)
                {
                    Screen->drawFastHLine(x0, y0, sq_size, 0); // gorna
                    Screen->drawFastVLine(x0, y0, sq_size, 0); // lewa

                    Screen->drawFastHLine(x0 + 1, y1, sq_size, Color); // dolna
                    Screen->drawFastVLine(x1, y0 + 1, sq_size, Color); // prawa

                    x0++;
                    x1++;
                    y0++;
                    y1++;
                    Cache_WriteBack_All();
                    wait_t();
                }
                else
                {
                    movetype = 1; // x++ y--
                }
            }
            else
            {
                movetype = 3; //
            }
        }
        //--------------
        if (movetype == 1) // x++ y--
        {
            if (x0 < LCD_COL_NUM - sq_size)
            {
                if (y0 > 0)
                {
                    Screen->drawFastHLine(x0, y1, sq_size, 0); // dolna
                    Screen->drawFastVLine(x0, y0, sq_size, 0); // lewa

                    Screen->drawFastHLine(x0 + 1, y0, sq_size, Color); // gorna
                    Screen->drawFastVLine(x1, y0 - 1, sq_size, Color); // prawa

                    x0++;
                    x1++;
                    y0--;
                    y1--;
                    Cache_WriteBack_All();
                    wait_t();
                }
                else
                {
                    movetype = 0; //
                }
            }
            else
            {
                movetype = 2;
            }
        }
        //--------------
        if (movetype == 2) // x-- y--
        {
            if (x0 > 0)
            {
                if (y0 > 0)
                {
                    Screen->drawFastHLine(x0, y1, sq_size + 1, 0); // dolna
                    Screen->drawFastVLine(x1, y0, sq_size, 0);     // prawa

                    Screen->drawFastHLine(x0 - 1, y0, sq_size, Color); // gorna
                    Screen->drawFastVLine(x0, y0 - 1, sq_size, Color); // lewa

                    x0--;
                    x1--;
                    y0--;
                    y1--;
                    Cache_WriteBack_All();
                    wait_t();
                }
                else
                {
                    movetype = 3; // x-- y++
                }
            }
            else
            {
                movetype = 1; //
            }
        }
        //--------------
        if (movetype == 3) // x-- y++
        {
            if (x0 > 0)
            {
                if (y0 < LCD_ROW_NUM - sq_size)
                {
                    Screen->drawFastHLine(x0, y0, sq_size, 0); // gorna
                    Screen->drawFastVLine(x1, y0, sq_size, 0); // prawa

                    Screen->drawFastHLine(x0 - 1, y1, sq_size, Color); // dolna
                    Screen->drawFastVLine(x0, y0 + 1, sq_size, Color); // lewa

                    x0--;
                    x1--;
                    y0++;
                    y1++;
                    Cache_WriteBack_All();
                    wait_t();
                }
                else
                {
                    movetype = 2;
                }
            }
            else
            {
                movetype = 0; // x-- y--
            }
        }
        //--------------

        counter++;

    } // end while(counter < 2000)
}
//===================================================================================================================
void demo_08(void)
{
    LCD_CLS_BLACK();
    int32_t x0, y0, x1, y1;

    x0 = 0;
    y0 = 0;
    x1 = 4;
    y1 = 479;

    while (y0 < 479)
    {
        Screen->drawLine(x0, y0, x1, y1, BLUE);

        y0 = y0 + 7;
        x1 = x1 + 12;
        delay(10);
        if (test_stop() != 0)
        {
            wait_for_releasseTS();
            half_sec_delay();
            return;
        } // check the stop action
    }

    x0 = 799;
    y0 = 470;
    x1 = 798;
    y1 = 0;

    while (y0 > 0)
    {
        Screen->drawLine(x0, y0, x1, y1, BLUE);

        y0 = y0 - 7;
        x1 = x1 - 12;
        delay(10);
        if (test_stop() != 0)
        {
            wait_for_releasseTS();
            half_sec_delay();
            return;
        } // check the stop action
    }
    //-----------------------------
    x0 = 0;
    y0 = 479;
    x1 = 4;
    y1 = 0;

    while (y0 > 0)
    {
        Screen->drawLine(x0, y0, x1, y1, RED);

        y0 = y0 - 7;
        x1 = x1 + 12;
        delay(10);
        if (test_stop() != 0)
        {
            wait_for_releasseTS();
            half_sec_delay();
            return;
        } // check the stop action
    }

    x0 = 799;
    y0 = 0;
    x1 = 798;
    y1 = 479;

    while (y0 < 479)
    {
        Screen->drawLine(x0, y0, x1, y1, RED);

        y0 = y0 + 7;
        x1 = x1 - 12;
        delay(10);
        if (test_stop() != 0)
        {
            wait_for_releasseTS();
            half_sec_delay();
            return;
        } // check the stop action
    }

    delay(1111);
}
//===================================================================================================================
void demo_09(void)
{
    u32 x0, y0, x1, y1, Color, i, tmpr;

    LCD_CLS_BLACK();

    for (i = 0; i < 100; i++)
    {
        Color = randomize() % 0xffff;
        y0 = randomize() % LCD_ROW_NUM;
        y1 = randomize() % LCD_ROW_NUM;
        x0 = randomize() % LCD_COL_NUM;
        x1 = randomize() % LCD_COL_NUM;
        if (x1 < x0)
        {
            tmpr = x0;
            x0 = x1;
            x1 = tmpr;
        }
        if (y1 < y0)
        {
            tmpr = y0;
            y0 = y1;
            y1 = tmpr;
        }

        Screen->drawRect(x0, y0, (x1 - x0), (y1 - y0), Color);
        delay(10);
        if (test_stop() != 0)
        {
            wait_for_releasseTS();
            half_sec_delay();
            return;
        } // check the stop action
    }
    delay(1000);
}
//===================================================================================================================
void demo_10(void)
{
    u32 i;
    LCD_CLS_BLACK();

    for (i = 0; i < 1200; i++)
    {
        Screen->fillCircle(randomize() % 600 + 100, randomize() % 280 + 100, randomize() % 100 + 1, randomize() % 0xffff);
        if (test_stop() != 0)
        {
            wait_for_releasseTS();
            half_sec_delay();
            return;
        } // check the stop action
    }

    delay(1000);
}
//===================================================================================================================
void demo_11(void)
{
    u32 color = RED;
    u32 x0, y0;
    u8 movetype = 0; // x++ y++

    x0 = circle_size;
    y0 = LCD_ROW_NUM >> 1;

    LCD_CLS_BLACK();

    Screen->fillCircle(x0, y0, circle_size, color);

    //-----------
    while (movetype == 0)
    {
        if (test_stop() != 0)
        {
            wait_for_releasseTS();
            half_sec_delay();
            return;
        } // check the stop action
        //---
        if (x0 < LCD_COL_NUM - circle_size)
        {
            Screen->drawCircle(x0, y0, circle_size + 2, 0);
            Screen->drawCircle(x0, y0, circle_size + 1, 0);
            x0++;
            Screen->drawCircle(x0, y0, circle_size, color);
            Screen->drawCircle(x0, y0, circle_size + 1, color);
            Screen->drawCircle(x0, y0, circle_size + 2, color);
            Cache_WriteBack_All();
        }
        else
        {
            movetype = 1; //
        }
    }
    //--------------
    while (movetype == 1)
    {
        if (test_stop() != 0)
        {
            wait_for_releasseTS();
            half_sec_delay();
            return;
        } // check the stop action
        //---
        if (x0 > circle_size + 2)
        {
            Screen->drawCircle(x0, y0, circle_size + 2, 0);
            Screen->drawCircle(x0, y0, circle_size + 1, 0);
            x0--;
            Screen->drawCircle(x0, y0, circle_size, color);
            Screen->drawCircle(x0, y0, circle_size + 1, color);
            Screen->drawCircle(x0, y0, circle_size + 2, color);
            Cache_WriteBack_All();
        }
        else
        {
            movetype = 0; // x-- y--
        }
    }

    half_sec_delay();
}
//===================================================================================================================
void demo_12(void)
{
    u32 color = BLUE; // blue
    u32 x0, y0;
    u8 movetype = 0; // x++ y++
    u32 counter = 0;

    x0 = circle_size / 2;
    y0 = circle_size / 2;

    LCD_CLS_BLACK();

    Screen->fillCircle(x0, y0, (circle_size >> 1), color);

    while (counter < 1000)
    {
        if (test_stop() != 0)
        {
            wait_for_releasseTS();
            half_sec_delay();
            return;
        } // check the stop action
        //-----------
        if (movetype == 0)
        {
            if (x0 < LCD_COL_NUM - circle_size / 2)
            {
                if (y0 < LCD_ROW_NUM - circle_size / 2)
                {
                    Screen->drawCircle(x0, y0, circle_size / 2 + 2, 0);
                    Screen->drawCircle(x0, y0, circle_size / 2 + 1, 0);
                    Screen->drawCircle(x0, y0, circle_size / 2 + 3, 0);
                    x0++;
                    y0++;
                    Screen->fillCircle(x0, y0, circle_size / 2, color);
                    Cache_WriteBack_All();
                    // wait_t();
                }
                else
                {
                    movetype = 1; // x++ y--
                }
            }
            else
            {
                movetype = 3; //
            }
        }
        //--------------
        if (movetype == 1)
        {
            if (x0 < LCD_COL_NUM - circle_size / 2)
            {
                if (y0 > circle_size / 2)
                {
                    Screen->drawCircle(x0, y0, circle_size / 2 + 2, 0);
                    Screen->drawCircle(x0, y0, circle_size / 2 + 1, 0);
                    Screen->drawCircle(x0, y0, circle_size / 2 + 3, 0);
                    x0++;
                    y0--;
                    Screen->fillCircle(x0, y0, circle_size / 2, color);
                    Cache_WriteBack_All();
                    // wait_t();
                }
                else
                {
                    movetype = 0; //
                }
            }
            else
            {
                movetype = 2; // x-- y--
            }
        }
        //--------------
        if (movetype == 2)
        {
            if (x0 > circle_size / 2)
            {
                if (y0 > circle_size / 2)
                {
                    Screen->drawCircle(x0, y0, circle_size / 2 + 2, 0);
                    Screen->drawCircle(x0, y0, circle_size / 2 + 1, 0);
                    Screen->drawCircle(x0, y0, circle_size / 2 + 3, 0);

                    x0--;
                    y0--;
                    Screen->fillCircle(x0, y0, circle_size / 2, color);
                    Cache_WriteBack_All();
                    // wait_t();
                }
                else
                {
                    movetype = 3; // x-- y++
                }
            }
            else
            {
                movetype = 1; //
            }
        }
        //--------------
        if (movetype == 3)
        {
            if (x0 > circle_size / 2)
            {
                if (y0 < LCD_ROW_NUM - circle_size / 2)
                {
                    Screen->drawCircle(x0, y0, circle_size / 2 + 2, 0);
                    Screen->drawCircle(x0, y0, circle_size / 2 + 1, 0);
                    Screen->drawCircle(x0, y0, circle_size / 2 + 3, 0);
                    x0--;
                    y0++;
                    Screen->fillCircle(x0, y0, circle_size / 2, color);
                    Cache_WriteBack_All();
                    // wait_t();
                }
                else
                {
                    movetype = 2;
                }
            }
            else
            {
                movetype = 0; // x-- y--
            }
        }
        //--------------

        counter++;

    } // while(counter < 2000)
    delay(1000);
}
//===================================================================================================================
void demo_13(void)
{
    LCD_CLS(0x001f);
    delay(1000);
    LCD_CLS_BLACK();
}
//===================================================================================================================
