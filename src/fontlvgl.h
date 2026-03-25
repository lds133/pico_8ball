#pragma once

#include <stdint.h>

typedef struct
{
    uint32_t w_px         :8;
    uint32_t glyph_index  :24;
} lv_font_glyph_dsc_t;

typedef struct
{
    uint32_t unicode         :21;
    uint32_t glyph_dsc_index :11;
} lv_font_unicode_map_t;




typedef struct _lv_font_struct
{
    uint32_t unicode_first;
    uint32_t unicode_last;
    const uint8_t * glyph_bitmap;
    const lv_font_glyph_dsc_t * glyph_dsc;
    const uint32_t * unicode_list;
	
		
	const lv_font_glyph_dsc_t* (*get_glyph_dsc)(const struct _lv_font_struct *,char c);    
	
    const uint8_t * (*get_bitmap)(const struct _lv_font_struct *,uint32_t);     /*Get a glyph's  bitmap from a font*/
    //int16_t (*get_width)(const struct _lv_font_struct *,uint32_t);        /*Get a glyph's with with a given font*/
    struct _lv_font_struct * next_page;    /*Pointer to a font extension*/
    uint32_t h_px       :8;
    uint32_t bpp        :4;                /*Bit per pixel: 1, 2 or 4*/
    uint32_t monospace  :8;                /*Fix width (0: normal width)*/
    uint16_t glyph_cnt;                    /*Number of glyphs (letters) in the font*/
} lv_font_t;


extern lv_font_t font24eng;
extern lv_font_t font24cyr;