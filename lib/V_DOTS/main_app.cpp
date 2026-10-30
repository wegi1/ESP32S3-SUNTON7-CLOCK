/*******************************************************************************
 ******************************************************************************/
#include "display/Arduino_RPi_DPI_RGBPanel.h"
#include <TAMC_GT911.h>
#include <esp_random.h>
#include <rom/cache.h>
#include "table.h"

#define many_circles 16
#define many_plots many_circles * 2 * 32
#define screen_X 800
#define screen_Y 480
#define zooming ((uint16_t)0x2800)
#define frames_count 1500
#define x_min (2048 - screen_X / 2)
#define x_max (2048 + screen_X / 2)
#define y_min (2048 - screen_Y / 2)
#define y_max (2048 + screen_Y / 2)
#define x_offset (2048 - screen_X / 2)
#define y_offset (2048 - screen_Y / 2)

extern Arduino_RPi_DPI_RGBPanel *Screen;
extern volatile bool pressed;

void insert_dots(void);
void calculate_matrix(void);
void calculate_8_dots(void);
void calc_all_dots_01(void);

int8_t mat_A, mat_B, mat_C, mat_D, mat_E, mat_F;

int8_t *tb_fact1 = NULL; // circle prescale factors 4 circles top and 4 mirroring to bottom
int8_t *tb_fact2 = NULL;
int8_t *v_5tab = NULL;
int8_t *v_6tab = NULL;
int8_t *v_7tab = NULL;
uint8_t *x_data_valid = NULL; // maximum size calculated dot from - 2048 to 2047
uint8_t *y_data_valid = NULL; // after translate to 2D max values are from 0 to 4095

uint8_t multable[128][256]; // a prescale table

int8_t *wynik_X = NULL; // X poss first 8 plots
int8_t *wynik_Y = NULL; // Y poss first 8 plots
int8_t *plot_1X = NULL; // first buffer many_plots plot X values
int8_t *plot_1Y = NULL; // first buffer many_plots plot Y values
uint16_t *plots01_X = NULL;
uint16_t *plots01_Y = NULL;
uint16_t *plots02_X = NULL;
uint16_t *plots02_Y = NULL;

uint32_t pl01_calc = 1;
uint32_t pl02_calc = 1;

uint16_t zoom_scale = zooming;
uint8_t zoom_side = 0x01;

uint8_t zoom = 128; // 128;
uint32_t frames = 0;

 

void main_app(void)
{
  uint16_t x, y;
  uint16_t *pBuffer;
  uint32_t i;
  //---
  //--- init values ---
  //---
  pl01_calc = 1;
  pl02_calc = 1;
  zoom_scale = zooming;
  zoom_side = 0x01;
  zoom = 128; // 128;
  frames = 0;
  //---

  uint8_t *x_data_vld = new uint8_t[4096]; // maximum size calculated dot from - 2048 to 2047
  x_data_valid = x_data_vld;
  
  uint8_t *y_data_vld = new uint8_t[4096]; // maximum size calculated dot from - 2048 to 2047
  y_data_valid = y_data_vld;
  int8_t *tb_fct1 = new int8_t[8];
  tb_fact1 = tb_fct1;
  int8_t *tb_fct2 = new int8_t[8];
  tb_fact2 = tb_fct2;

  int8_t *v5tab = new int8_t[many_circles]; //
  v_5tab = v5tab;
  int8_t *v6tab = new int8_t[many_circles]; //
  v_6tab = v6tab;
  int8_t *v7tab = new int8_t[many_circles * 2]; //
  v_7tab = v7tab;

  int8_t *wyn_X = new int8_t[16]; // X poss first 8 plots
  wynik_X = wyn_X;
  int8_t *wyn_Y = new int8_t[16]; // Y poss first 8 plots
  wynik_Y = wyn_Y;

  // int8_t *plt_1X = new int8_t[many_plots];
  int8_t *plt_1X = (int8_t *)ps_malloc(many_plots);
  plot_1X = plt_1X;
  int8_t *plt_1Y = new int8_t[many_plots];
  plot_1Y = plt_1Y;

  uint16_t *plts01_X = new uint16_t[many_plots];
  uint16_t *plts01_Y = new uint16_t[many_plots];
  plots01_X = plts01_X;
  plots01_Y = plts01_Y;

  uint16_t *plts02_X = new uint16_t[many_plots];
  uint16_t *plts02_Y = new uint16_t[many_plots];
  plots02_X = plts02_X;
  plots02_Y = plts02_Y;

  //---
  //================================================
  plots02_X[0] = 0U;
  plots02_Y[0] = 0U;
  insert_dots();
  uint32_t FRAME_AD = (uint32_t)Screen->getFramebuffer();
  char tekst[32];

  Serial.printf("multable buffer at  : 0x%08X\n\r", (uint32_t)&multable[0]);
  Serial.printf("frame    buffer at  : 0x%08X\n\r", (uint32_t)FRAME_AD);
  Serial.printf("plt_1X   buffer at  : 0x%08X\n\r", (uint32_t)plt_1X);
  Serial.printf("&plt_1X  buffer at  : 0x%08X\n\r", (uint32_t)&plt_1X);
  Serial.printf("var (uint32_t) i at : 0x%08X\n\r", (uint32_t)&i);

  pBuffer = (uint16_t *)FRAME_AD;
  for (y = 0; y < 480; y++)
  {
    for (x = 0; x < 800; x++)
    {
      pBuffer[(y * 800) + x] = 0;
    }
  }

  while (1)
  {
    if (pressed == true)
    {
      delete[] x_data_vld;
      delete[] y_data_vld;
      x_data_valid = NULL;
      y_data_valid = NULL;

      delete[] tb_fct1;
      delete[] tb_fct2;

      tb_fact1 = NULL;
      tb_fact2 = NULL;

      delete[] v5tab;
      delete[] v6tab;
      delete[] v7tab;
      v_5tab = NULL;
      v_6tab = NULL;
      v_7tab = NULL;

      delete[] wyn_X;
      delete[] wyn_Y;
      wynik_X = NULL;
      wynik_Y = NULL;

      delete[] plt_1X;
      delete[] plt_1Y;
      plot_1X = NULL;
      plot_1Y = NULL;

      delete[] plts01_X;
      delete[] plts01_Y;
      plots01_X = NULL;
      plots01_Y = NULL;

      delete[] plts02_X;
      delete[] plts02_Y;
      plots02_X = NULL;
      plots02_Y = NULL;

      return;
    }
    frames++;
    // huge zoom manage
    //====================================================================
    if (frames > frames_count)
    {
      if ((zoom_scale < 0x5400) && (zoom_side != 0))
      {
        zoom_scale += 0x0020;
        if (zoom_scale >= 0x5400)
        {
          zoom_side = 0;
        }
      }
      else
      {
        zoom_scale -= 0x0020;
        if (zoom_scale < 0x0020)
        {
          zoom_side = 1;
        }
      }
      if (frames > 3000)
      {
        // zoom_scale = zooming;
        zoom_side = 1;
        frames = 0;
      }
    }
    //====================================================================

    calculate_matrix();
    calculate_8_dots();
    calc_all_dots_01(); // store plots values in first buffer

    for (i = 0; i < pl01_calc; i++)
    {
      x = plots02_X[i];
      y = plots02_Y[i];
      Screen->drawPixel(x, y, 0);

      x = plots01_X[i];
      y = plots01_Y[i];
      if ((x != 65535) && (y != 65535))
        Screen->drawPixel(x, y, WHITE);

      plots02_X[i] = plots01_X[i];
      plots02_Y[i] = plots01_Y[i];
    }

    pl02_calc = pl01_calc;
  }

  delay(50);
}

void insert_dots()

{
  volatile uint8_t i, i2, i3, dana, t9, t10, sx, sy;
  volatile uint8_t t_x;
  volatile uint16_t wynik, x;
  //*************************************************

  // make prescale/multiply table
  dana = 0;
  for (i = 0; i < 128; i++)
  {
    wynik = 0;
    i3 = 0;
    for (i2 = 0; i2 < 128; i2++)
    {
      multable[i][i2] = (uint8_t)(((wynik) >> 8) & 0xFF);
      wynik = wynik + dana;
      i3--;
      multable[i][i3] = (uint8_t)(0 - multable[i][i2]);
    }
    dana = dana + 2;
  }

  //*************************************************

  for (x = 0; x < 4096; x++)
  {
    x_data_valid[x] = 0;
    y_data_valid[x] = 0;
    if ((x > x_min) && (x < x_max))
    {
      x_data_valid[x] = 1;
    }
    if ((x > y_min) && (x < y_max))
    {
      y_data_valid[x] = 2;
    }
  }

  //*************************************************

  //*************************************************

  // calculate and place dots

  t_x = 0x44;
  i2 = 0;
  for (i = 7; i != 0xff; i--)
  {
    t_x = t_x - 0x08;
    sx = t_x & 0x3F;
    sy = 0;
    //;---------------------
    //; F = invsin(t9)-invsin(t10)
    //;---------------------
    t9 = sy - sx;
    t10 = sy + sx;
    tb_fact1[i] = (uint8_t)((sinus[128 + t9] - sinus[128 + t10]) >> 8) & 0xFF;
    tb_fact2[i2] = tb_fact1[i];
    i2++;
  }
  //*************************************************
  // calc data next object #3
  t_x = 0x42;
  i2 = 0;
  for (i = many_circles - 1; i != 0xff; i--)
  {
    t_x = t_x - 0x04;
    sx = t_x & 0x3F;
    sy = 0;
    //;---------------------
    //; F = invsin(t9)-invsin(t10)
    //;---------------------
    t9 = sy - sx;
    t10 = sy + sx;

    v_5tab[i] = (uint8_t)((sinus[128 + t9] - sinus[128 + t10]) >> 8) & 0xFF;
    v_6tab[i2] = v_5tab[i];
    i2++;
  }
}
//===========================================================================
void calculate_matrix()
{
  uint8_t t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;

  static uint16_t delta_x = 0x030;
  static uint16_t delta_y = 0x040;
  static uint16_t delta_z = 0x020;

  static uint16_t angle_x = 0x00;
  static uint16_t angle_y = 0x00;
  static uint16_t angle_z = 0x00;

  static uint8_t sx;
  static uint8_t sy;
  static uint8_t sz;

  /*    update_angles   */
  angle_x = angle_x + delta_x;
  angle_y = angle_y + delta_y;
  angle_z = angle_z + delta_z;

  sx = (uint8_t)((angle_x) >> 8) & 0xFF;
  sy = (uint8_t)((angle_y) >> 8) & 0xFF;
  sz = (uint8_t)((angle_z) >> 8) & 0xFF;

  t1 = sy - sz;
  t2 = sy + sz;
  t3 = sx + sz;
  t4 = sx - sz;
  t5 = sx + sy + sz;
  t6 = sx - sy + sz;
  t7 = sx + sy - sz;
  t8 = sy + sz - sx;
  t9 = sy - sx;
  t10 = sy + sx;

  mat_A = (uint8_t)((sinus[64 + t1] + sinus[64 + t2]) >> 8) & 0xFF;
  mat_B = (uint8_t)((sinus[t1] - sinus[t2]) >> 8) & 0xFF;
  mat_C = (uint8_t)((sinus[128 + sy] + sinus[128 + sy]) >> 8) & 0xFF;
  mat_D = (uint8_t)((sinus[t3] - sinus[t4] + ((sinus[64 + t6] - sinus[64 + t5] + sinus[64 + t8] - sinus[64 + t7]) / 2)) >> 8) & 0xFF;
  mat_E = (uint8_t)((sinus[64 + t3] + sinus[64 + t4] + ((sinus[t5] - sinus[t6] - sinus[t7] - sinus[t8]) / 2)) >> 8) & 0xFF;
  mat_F = (uint8_t)((sinus[128 + t9] - sinus[128 + t10]) >> 8) & 0xFF;
  if (zoom < 128)
  {
    mat_C = multable[zoom][mat_C];
    mat_F = multable[zoom][mat_F];
  }
}
//====================================================================================================
void calc_all_dots_01()
{
  uint16_t row1, row2, i, i2, i3;
  uint8_t add_x, add_y;
  int32_t data1;
  uint32_t helper_y, helper_x;
  int16_t plot_16X;
  int16_t plot_16Y;

  i2 = 0;
  // 4 times calculate and mirroring circles = 4*16*2 = 128 vectordots
  for (i3 = 0; i3 < many_circles; i3++)
  {
    row1 = v_6tab[i3]; // prescale factor for X/Y offset data (add_x;add_y)

    add_x = multable[row1][mat_C]; // X offset  after prescale circle = possitoin X;Y 0;0
    add_y = multable[row1][mat_F]; // Y offset  after prescale circle

    row2 = v_5tab[i3]; // prescale factor for actual circle

    // calculate 32 X;Y values of plots in mirroring 2nd circles
    for (i = 0; i < 16; i++)
    {
      plot_1X[i2] = ((multable[row2][wynik_X[i]] + add_x));
      plot_1X[i2 + 1] = (plot_1X[i2] ^ 255);
      plot_1Y[i2] = ((multable[row2][wynik_Y[i]] + add_y));
      plot_1Y[i2 + 1] = (plot_1Y[i2] ^ 255);

      plot_1X[i2 + 2] = ((multable[row2][wynik_X[i]] - add_x));
      plot_1X[i2 + 3] = (plot_1X[i2 + 2] ^ 255);
      plot_1Y[i2 + 2] = ((multable[row2][wynik_Y[i]] - add_y));
      plot_1Y[i2 + 3] = (plot_1Y[i2 + 2] ^ 255);
      i2 = i2 + 4;
    }
  }
  pl01_calc = 0;
  for (i = 0; i < many_plots; i++)
  {
    plot_16X = plot_1X[i] * 16;
    plot_16Y = plot_1Y[i] * 16;
    data1 = plot_16X * zoom_scale;
    plot_16X = data1 >> 16;
    data1 = plot_16Y * zoom_scale;
    plot_16Y = data1 >> 16;
    plot_16X += 2048;
    plot_16Y += 2048;

    // validate pixel range
    if ((x_data_valid[plot_16X] | y_data_valid[plot_16Y]) == 0x03)
    {
      helper_y = plot_16Y - y_offset;
      helper_x = plot_16X - x_offset;
      plots01_X[pl01_calc] = helper_x;
      plots01_Y[pl01_calc] = helper_y;
      pl01_calc++;
    }
    else
    {
      plots01_X[pl01_calc] = 65535;
      plots01_Y[pl01_calc] = 65535;
      pl01_calc++;
    }
  }
}
//==================================================================================
void calculate_8_dots()
{
  uint8_t row1, row2;
  uint8_t v1, v2, v4, v5;
  uint8_t i, i2;

  // calculate first 8 dots to following prescale and mirroring
  i2 = 0;
  for (i = 0; i < 8; i++)
  {
    row1 = tb_fact1[i];
    row2 = tb_fact2[i];

    v5 = multable[row1][mat_E];
    v2 = multable[row1][mat_B];
    v4 = multable[row2][mat_D];
    v1 = multable[row2][mat_A];

    wynik_X[i2] = v1 + v2;
    wynik_Y[i2] = v4 + v5;
    wynik_X[i2 + 1] = v1 - v2;
    wynik_Y[i2 + 1] = v4 - v5;
    i2 = i2 + 2;
  }
}
