/*
 * ADT_F.c
 *
 *  Created on: Sep 23, 2025
 *      Author: BOLO
 */


//=====================================================================================================================
 
#include "stdio.h"
#include <Arduino.h>

#include "stb_truetype.h"
#include "fonty.h"
#include "boldfonts.h"

#include "ADT_F.h"
#include "display/Arduino_RPi_DPI_RGBPanel.h"
extern Arduino_RPi_DPI_RGBPanel *Screen;
//=====================================================================================================================


//=====================================================================================================================
//=====================================================================================================================


char tekst[30];
stbtt_fontinfo font;
uint16_t lcd_text_color = 0xffff;
uint16_t lcd_background_color = 0;
int efect_dx = 1;
uint8_t fade_effect = 0;
uint32_t shifted = 25;
uint16_t * SCREEN_BUFF;

uint32_t * clear_addr;
uint32_t * SCREEN32;
uint32_t now_time ;

extern uint32_t adr_bmp;
//=====================================================================================================================
extern "C" void Cache_WriteBack_All(void);
//=====================================================================================================================
static void lcd_text_bounds(stbtt_fontinfo *font, const char *text, float pixel_height, int *out_width, int *out_height);

//=====================================================================================================================
//=====================================================================================
int lcd_mono_text_boxed(int pozx, int pozy, const char *text, const unsigned char *font_data, float pixel_height)
{
	return pozy + render_text_monospace(pozx, pozy, text, font_data, pixel_height);
}
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

//=====================================================================================================================
int render_text_monospace(int pozx, int pozy, const char *text, const unsigned char *font_data, float pixel_height)
{

	if (!stbtt_InitFont(&font, font_data, stbtt_GetFontOffsetForIndex(font_data, 0)))
	{
		return 0;
	}

	float scale = stbtt_ScaleForPixelHeight(&font, pixel_height);

	int ascent, descent, line_gap;
	stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);
	int baseline = (int) (ascent * scale);

	//  Ustalamy rozmiar glifu (monospace)
	int adv_width, lsb;
	stbtt_GetCodepointHMetrics(&font, 'M', &adv_width, &lsb);  //
	int mono_w = (int) (adv_width * scale);
	int text_len = strlen(text);

	int x0, y0, x1, y1;
	stbtt_GetCodepointBitmapBox(&font, 'M', scale, scale, &x0, &y0, &x1, &y1);
	int total_h = baseline + y1;



	//  Renderowanie znaeden po drugim
	int pen_x = pozx;
	for (int i = 0; i < text_len; i++)
	{
		int cp = text[i];
		int x0, y0, x1, y1;
		stbtt_GetCodepointBitmapBox(&font, cp, scale, scale, &x0, &y0, &x1, &y1);

		int gw = x1 - x0;
		int gh = y1 - y0;
		unsigned char *g_bitmap = stbtt_GetCodepointBitmap(&font, scale, scale, cp, &gw, &gh, 0, 0);
		uint16_t center_y = ((total_h - gh) >> 1) + pozy; // Y center in monospace fields
		uint16_t buforek[480];


		for (int row = 0; row < gh; row++)
		{
			for(uint16_t it = 0; it< (mono_w) ; it++) { buforek[it] = lcd_background_color  ; }
			for (int col = 0; col < gw; col++)
			{
				unsigned char val = g_bitmap[row * gw + col];
				int px = col + (mono_w - gw)  ;  // centrowanie w polu monospace
				if (val > 0) {	buforek[px] = lcd_text_color; }
			}
			// for(uint16_t it = 0 ; it < mono_w ; it++) { Screen->drawPixel((pen_x+it), (pozy + row) , buforek[it]);}
			for(uint16_t it = 0 ; it < mono_w ; it++) { Screen->drawPixel((pen_x+it), (center_y + row) , buforek[it]);}
		}
		stbtt_FreeBitmap(g_bitmap, NULL);
		pen_x += mono_w;
	}

	return total_h;
}
//=====================================================================================================================
//=====================================================================================================================

int render_shiftedtext_monospace(int pozx, int pozy, const char *text, const unsigned char *font_data, float pixel_height)
{

	if (!stbtt_InitFont(&font, font_data, stbtt_GetFontOffsetForIndex(font_data, 0)))
	{
		return 0;
	}

	float scale = stbtt_ScaleForPixelHeight(&font, pixel_height);

	int ascent, descent, line_gap;
	stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);
	int baseline = (int) (ascent * scale);

	//  Ustalamy rozmiar glifu (monospace)
	int adv_width, lsb;
	stbtt_GetCodepointHMetrics(&font, 'M', &adv_width, &lsb);  //
	int mono_w = (int) (adv_width * scale);
	int text_len = strlen(text);

	int x0, y0, x1, y1;
	stbtt_GetCodepointBitmapBox(&font, 'M', scale, scale, &x0, &y0, &x1, &y1);
	int total_h = baseline + y1;

	//  Renderowanie znaeden po drugim
	int pen_x = pozx;
	for (int i = 0; i < text_len; i++)
	{
		int cp = text[i];
		int x0, y0, x1, y1;
		stbtt_GetCodepointBitmapBox(&font, cp, scale, scale, &x0, &y0, &x1, &y1);

		int gw = x1 - x0;
		int gh = y1 - y0;
		unsigned char *g_bitmap = stbtt_GetCodepointBitmap(&font, scale, scale, cp, &gw, &gh, 0, 0);

		uint16_t center_y = ((total_h - gh) >> 1) + pozy; // Y center in monospace fields
		uint16_t buforek[480];



		for (int row = 0; row < gh; row++)
		{
			for(uint16_t it = 0; it< (mono_w) ; it++) { buforek[it] = lcd_background_color  ; }
			for (int col = 0; col < gw; col++)
			{
				unsigned char val = g_bitmap[row * gw + col];
				int px = col + (mono_w - gw) -shifted ;  // centrowanie w polu monospace
				if (val > 0) {	buforek[px] = lcd_text_color; }
			}
			// for(uint16_t it = 0 ; it < mono_w ; it++)  { Screen->drawPixel((pen_x+it), (pozy + row) , buforek[it]);}
			for(uint16_t it = 0 ; it < mono_w ; it++) { Screen->drawPixel((pen_x+it), (center_y + row) , buforek[it]);}
		}
		stbtt_FreeBitmap(g_bitmap, NULL);
		pen_x += mono_w;
	}

	return total_h;
}
//=====================================================================================================================
//=====================================================================================================================
int lcd_text_boxed(int pozx, int pozy, const char *text, const unsigned char *font_data, float pixel_height)
{
	return pozy + lcd_render_text(pozx, pozy, text, font_data, pixel_height, 0, 0);
}
//=====================================================================================================================
//=====================================================================================================================
/**
 * @brief procedura rysująca
 * @param pozx
 * @param pozy
 * @param text
 * @param font_data
 * @param pixel_height
 * @param fast
 * @param boxed
 */
int lcd_render_text(int pozx, int pozy, const char *text, const unsigned char *font_data, float pixel_height, uint8_t fast, uint8_t boxed)
{

	int text_width, text_height;

	if (!stbtt_InitFont(&font, font_data, stbtt_GetFontOffsetForIndex(font_data, 0)))
	{
		return 0;  //
	}

	lcd_text_bounds(&font, text, pixel_height, &text_width, &text_height);
	float scale = stbtt_ScaleForPixelHeight(&font, pixel_height);

	int ascent, descent, line_gap;
	stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);

//	int baseline = (int) (ascent * scale);
	int x = pozx;

	for (const char *p = text; *p; p++)
	{
		int glyph = stbtt_FindGlyphIndex(&font, *p);

		int ax, lsb;
		stbtt_GetGlyphHMetrics(&font, glyph, &ax, &lsb);

		int x0, y0, x1, y1;
		stbtt_GetGlyphBitmapBox(&font, glyph, scale, scale, &x0, &y0, &x1, &y1);

		int width = x1 - x0;
		int height = y1 - y0;

		unsigned char *bitmap = stbtt_GetCodepointBitmap(&font, scale, scale, *p, &width, &height, &x0, &y0);


 

		for (int row = 0; row < height; row++)
		{
		 
			for (int col = 0; col < width; col++)
			{
				uint8_t value = bitmap[row * width + col];
				if (value > 0)
				{
					//pFBaddr[col] =  lcd_text_color; 
					Screen->drawPixel((x+col) , (pozy+row), lcd_text_color );
				}
				else
				{
					//pFBaddr[col] =  lcd_background_color;
					Screen->drawPixel((x+col) , (pozy+row), lcd_background_color );
				}
			}
			 
		}

		stbtt_FreeBitmap(bitmap, NULL);

		int kern = stbtt_GetGlyphKernAdvance(&font, glyph, *(p + 1));
		x += (int) (ax * scale) + (int) (kern * scale);
	}

	return text_height;
}
//=====================================================================================================================
//=====================================================================================================================
static void lcd_text_bounds(stbtt_fontinfo *font, const char *text, float pixel_height, int *out_width, int *out_height)
{
	float scale = stbtt_ScaleForPixelHeight(font, pixel_height);

	int ascent, descent, line_gap;
	stbtt_GetFontVMetrics(font, &ascent, &descent, &line_gap);

	int baseline = (int) (ascent * scale);
	int min_x = 999999, min_y = 999999;
	int max_x = -999999, max_y = -999999;

	int x = 0;
	for (const char *p = text; *p; p++)
	{

		int glyph = stbtt_FindGlyphIndex(font, *p);

		int ax, lsb;
		stbtt_GetGlyphHMetrics(font, glyph, &ax, &lsb);

		int x0, y0, x1, y1;
		stbtt_GetGlyphBitmapBox(font, glyph, scale, scale, &x0, &y0, &x1, &y1);

		int gx0 = x + x0;
		int gy0 = baseline + y0;
		int gx1 = x + x1;
		int gy1 = baseline + y1;

		if (gx0 < min_x) min_x = gx0;
		if (gy0 < min_y) min_y = gy0;
		if (gx1 > max_x) max_x = gx1;
		if (gy1 > max_y) max_y = gy1;

		int kern = stbtt_GetGlyphKernAdvance(font, glyph, *(p + 1));
		x += (int) (ax * scale) + (int) (kern * scale);
	}

	*out_width = (max_x - min_x);
	*out_height = (max_y - min_y) + 3;
}
//=====================================================================================================================
//=====================================================================================================================
int render_rotate_text_monospace00(int pozx, int pozy, const char *text, const unsigned char *font_data, float pixel_height)
{
 

	if (!stbtt_InitFont(&font, font_data, stbtt_GetFontOffsetForIndex(font_data, 0)))
	{
		return 0;
	}

	float scale = stbtt_ScaleForPixelHeight(&font, pixel_height);


	//  Ustalamy rozmiar glifu (monospace)
	int adv_width, lsb;
	stbtt_GetCodepointHMetrics(&font, 'M', &adv_width, &lsb);  //
	int mono_w = (int) (adv_width * scale);


	int x0, y0, x1, y1;
	int cp = text[0];

	stbtt_GetCodepointBitmapBox(&font, cp, scale, scale, &x0, &y0, &x1, &y1);

	int gw = x1 - x0;
	int gh = y1 - y0;
	unsigned char *g_bitmap = stbtt_GetCodepointBitmap(&font, scale, scale, cp, &gw, &gh, 0, 0);

	for (int row = 0; row < gh; row++)
	{
		for (int col = 0; col < gw; col++)
		{
			unsigned char val = g_bitmap[row * gw + col];
			int px = col + (mono_w - gw)  ;  // centrowanie w polu monospace
 
			if (val > 0) { Screen->drawPixel( (row+80) , (520-px), lcd_text_color); }
		}
	}

extern uint32_t adr_bmp;
adr_bmp = (uint32_t) &g_bitmap[0];

	stbtt_FreeBitmap(g_bitmap, NULL);


	return 0;
}
uint16_t * F_BUFFER;
void store_pixel(uint16_t x, uint16_t y, uint16_t color) {
	uint32_t data = (y * 800) + x;
	F_BUFFER[data] = color;
}
//=====================================================================================================================
//=====================================================================================================================
//TODO ROTATE 90 deg render
int render_rotate_text_monospace(int pozx, int pozy, const char *text, const unsigned char *font_data, float pixel_height)
{
 

	if (!stbtt_InitFont(&font, font_data, stbtt_GetFontOffsetForIndex(font_data, 0)))
	{
		return 0;
	}

	float scale = stbtt_ScaleForPixelHeight(&font, pixel_height);


	//  Ustalamy rozmiar glifu (monospace)
	int adv_width, lsb;
	stbtt_GetCodepointHMetrics(&font, 'M', &adv_width, &lsb);  //
	int mono_w = (int) (adv_width * scale);


	int x0, y0, x1, y1;
	int cp = text[0];

	stbtt_GetCodepointBitmapBox(&font, cp, scale, scale, &x0, &y0, &x1, &y1);

	int gw = x1 - x0;
	int gh = y1 - y0;
	unsigned char *g_bitmap = stbtt_GetCodepointBitmap(&font, scale, scale, cp, &gw, &gh, 0, 0);




	for (int row = 0; row < gh; row++)
	{
		for (int col = 0; col < gw; col++)
		{
			unsigned char val = g_bitmap[row * gw + col];
			int px = col + (mono_w - gw)  ;  // centrowanie w polu monospace
 
			// if (val > 0) { lcd_WritePixel( (row+80) , (520-px), lcd_text_color); }
			if (val > 0) { store_pixel( (row+80) , (520-px), lcd_text_color); }
		}
	}


adr_bmp = (uint32_t) &g_bitmap[0];

	stbtt_FreeBitmap(g_bitmap, NULL);

	return 0;
}
//=====================================================================================================================
//=====================================================================================================================
void copy_to_screen(void) {
	if(clear_addr == NULL) {return;} // memory not allocated
	for(uint32_t i = 0; i < (800*240) ; i++) { SCREEN32[i] = clear_addr[i]; }
	Cache_WriteBack_All();
}
//=====================================================================================================================
void fill_second_fb(uint32_t val){
	if(clear_addr == NULL) {return;} // memory not allocated
	for(uint32_t i = 0; i < (800*240) ; i++) { clear_addr[i] = val; }
	Cache_WriteBack_All();
}
//=====================================================================================================================
 
void Big_TTF_Demo(void)
{
	 
	SCREEN_BUFF = Screen->getFramebuffer();
    clear_addr = (uint32_t*) malloc(800*480*4);
	if(clear_addr == NULL) {return;} // memory not allocated
	F_BUFFER = (uint16_t*) clear_addr;
	SCREEN32 = (uint32_t*) SCREEN_BUFF;

	fill_second_fb(0);

	now_time=0;
    uint32_t start_time;
	uint32_t end_time;

	for (uint8_t t=0;t<16;t++)
	{
		start_time = (uint32_t) millis();
		sprintf(tekst, "%01d", t%10);
		render_rotate_text_monospace(0, 0, tekst, digital_7_ttf, 540);
		Cache_WriteBack_All();
		copy_to_screen();
		fill_second_fb(0);
		end_time = (uint32_t) millis();
		now_time += end_time - start_time;
	}
    // SCREEN_CLS(0);
	Screen->cls(0);
    free(clear_addr);
}
 
//=====================================================================================================================
// list of print TTF types
//		render_biggestFont_portait(0, 0, tekst, digital_7_ttf, 380);
//		lcd_mono_text_boxed(0, 0, tekst, digital_7_ttf, 380);
//		lcd_mono_text_boxed(0, 0, "4321", digital_7_ttf, 230);
//		render_text_monospace(0, 0, "4321", digital_7_ttf, 230);
//		render_text_monospace(396, 0, "21", digital_7_ttf, 230);
//		render_text_monospace(0, 0, tekst, digital_7_ttf, 350);
//		lcd_mono_text_boxed(2, 0, tekst, digital_7_ttf, 230);
//		lcd_text_boxed(0, 0, tekst, digital_7_ttf, 380);
// 		render_rotate_text_monospace(0, 0, tekst, digital_7_ttf, 540);
//=====================================================================================================================
//=====================================================================================

/**
 * @brief procedura rysująca
 * @param pozx
 * @param pozy
 * @param text
 * @param font_data
 * @param pixel_height
 * @param fast
 * @param boxed
 */
int lcd_render_just_text(int pozx, int pozy, const char *text, const unsigned char *font_data, float pixel_height, uint8_t fast, uint8_t boxed)
{

	int text_width, text_height;

	if (!stbtt_InitFont(&font, font_data, stbtt_GetFontOffsetForIndex(font_data, 0)))
	{
		return 0;  //
	}

	lcd_text_bounds(&font, text, pixel_height, &text_width, &text_height);
	float scale = stbtt_ScaleForPixelHeight(&font, pixel_height);

	int ascent, descent, line_gap;
	stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);

//	int baseline = (int) (ascent * scale);
	int x = pozx;

	for (const char *p = text; *p; p++)
	{
		int glyph = stbtt_FindGlyphIndex(&font, *p);

		int ax, lsb;
		stbtt_GetGlyphHMetrics(&font, glyph, &ax, &lsb);

		int x0, y0, x1, y1;
		stbtt_GetGlyphBitmapBox(&font, glyph, scale, scale, &x0, &y0, &x1, &y1);

		int width = x1 - x0;
		int height = y1 - y0;

		unsigned char *bitmap = stbtt_GetCodepointBitmap(&font, scale, scale, *p, &width, &height, &x0, &y0);


		uint32_t FRAME_ADDRESS = (uint32_t) Screen->getFramebuffer();
		uint32_t x_offset = (x << 1);
		uint32_t y_offset = pozy * 1600;
		uint32_t GL_ADDR = FRAME_ADDRESS + x_offset + y_offset;
		uint32_t GLYPH_ADDR = 0;

		for (int row = 0; row < height; row++)
		{
			uint16_t* pFBaddr = (uint16_t *) GL_ADDR;
			for (int col = 0; col < width; col++)
			{
				// if (bitmap[GLYPH_ADDR + col] != 0)
				if (bitmap[GLYPH_ADDR] != 0)
				{
					pFBaddr[col] =  lcd_text_color;
				}
				GLYPH_ADDR++;
			}
			GL_ADDR = GL_ADDR + 1600;
		}

		stbtt_FreeBitmap(bitmap, NULL);

		int kern = stbtt_GetGlyphKernAdvance(&font, glyph, *(p + 1));
		x += (int) (ax * scale) + (int) (kern * scale);
	}
    Cache_WriteBack_All();
	return text_height;
}
//=====================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

