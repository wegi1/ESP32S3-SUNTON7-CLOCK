/*
 * ADT_F.h
 *
 *  Created on: Sep 23, 2025
 *      Author: BOLO
 */

#ifndef ADT_F_H_
#define ADT_F_H_

extern const unsigned char digital_7_ttf[];
extern const unsigned char SourceCodePro_Bold_ttf[];


extern void Big_TTF_Demo(void);
extern int render_biggestFont_portait(int pozx, int pozy, const char *text, const unsigned char *font_data, float pixel_height);
extern int lcd_text_boxed(int pozx, int pozy, const char *text, const unsigned char *font_data, float pixel_height);

extern int lcd_mono_text_boxed(int pozx, int pozy, const char *text, const unsigned char *font_data, float pixel_height);
extern int render_text_monospace(int pozx, int pozy, const char *text, const unsigned char *font_data, float pixel_height);
extern int lcd_render_text(int pozx, int pozy, const char *text, const unsigned char *font_data, float pixel_height, uint8_t fast, uint8_t boxed);

extern int render_rotate_text_monospace00(int pozx, int pozy, const char *text, const unsigned char *font_data, float pixel_height);
extern int render_rotate_text_monospace(int pozx, int pozy, const char *text, const unsigned char *font_data, float pixel_height);
extern int render_shiftedtext_monospace(int pozx, int pozy, const char *text, const unsigned char *font_data, float pixel_height);
extern void lcd_set_text_color(uint16_t kolor);
extern void lcd_set_background_color(uint16_t kolor);
extern int lcd_render_just_text(int pozx, int pozy, const char *text, const unsigned char *font_data, float pixel_height, uint8_t fast, uint8_t boxed);


#define LCD_COLOR565_BLACK         0x0000
#define LCD_COLOR565_BLUE          0x001F
#define LCD_COLOR565_GREEN         0x07E0
#define LCD_COLOR565_RED           0xF800
#define LCD_COLOR565_CYAN          0x07FF
#define LCD_COLOR565_MAGENTA       0xF81F
#define LCD_COLOR565_YELLOW        0xFFE0
#define LCD_COLOR565_LIGHTBLUE     0x841F
#define LCD_COLOR565_LIGHTGREEN    0x87F0
#define LCD_COLOR565_LIGHTRED      0xFC10
#define LCD_COLOR565_LIGHTCYAN     0x87FF
#define LCD_COLOR565_LIGHTMAGENTA  0xFC1F
#define LCD_COLOR565_LIGHTYELLOW   0xFFF0
#define LCD_COLOR565_DARKBLUE      0x0010
#define LCD_COLOR565_DARKGREEN     0x0400
#define LCD_COLOR565_DARKRED       0x8000
#define LCD_COLOR565_DARKCYAN      0x0410
#define LCD_COLOR565_DARKMAGENTA   0x8010
#define LCD_COLOR565_DARKYELLOW    0x8400
#define LCD_COLOR565_WHITE         0xFFFF
#define LCD_COLOR565_LIGHTGRAY     0xD69A
#define LCD_COLOR565_GRAY          0x8410
#define LCD_COLOR565_DARKGRAY      0x4208
#define LCD_COLOR565_BROWN         0xA145
#define LCD_COLOR565_ORANGE        0xFD20

#endif /* ADT_F_H_ */
