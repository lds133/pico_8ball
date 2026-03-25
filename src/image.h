#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "GC9A01.h"


#define SWAP16(W)  ( ( (W << 8) & 0xff00) | (W >> 8) )
#define RGB(R,G,B)  SWAP16(  ((uint16_t)( (B*31/255)  | (G*63/255) << 5 | (R*31/255) << 11 ))  )



SPRITE* img_newsprite(int w, int h, uint8_t  mode);
void img_delsprite(SPRITE* s);

int img_textwidth(const char* text);
int img_textheight();
int img_char(SPRITE* s, char ch, int x, int y, uint16_t color);
void img_text(SPRITE* s, const char* text, int x, int y, uint16_t color);
int img_textcenter(SPRITE* s, int y, const char* text, uint16_t color);

void img_rotate(SPRITE* s, int  angle_deg);
void img_rotate_ex(SPRITE* s, int width, int height, int  angle_deg);
void img_clear(SPRITE* s,  uint16_t c);

void img_rect(SPRITE* s,int x, int y, int w, int h, uint16_t c);

void img_stamp(SPRITE* s, int x, int y, SPRITE* stamp, uint16_t color);



uint16_t img_rgb(float r, float g, float b);
uint16_t img_hsv(float hh, float ss, float vv);
uint16_t img_hsv8(uint8_t h, uint8_t s, uint8_t v);

void img_triangle_ex(SPRITE* s,int x, int y, int n, int a, uint16_t color);


void img_circle(SPRITE* s,int x, int y, int r, uint16_t color);


void img_triangle(SPRITE* s, int x1, int y1, int x2, int y2, int x3, int y3, uint16_t color);


void img_clear_raw(SPRITE* s, uint8_t value);
