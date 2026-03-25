#pragma once
#include <stdint.h>
#include <stdbool.h>

#define SCR_PIN_DIN 3
#define SCR_PIN_CLK 2
#define SCR_PIN_CS  5
#define SCR_PIN_DC  6
#define SCR_PIN_RST 7
#define SCR_PIN_BL  8


#define SCR_HEIGHT 240
#define SCR_WIDTH 240
#define SCR_HORIZONTAL 0
#define SCR_VERTICAL   1

#define SCR_WHITE          0xFFFF
#define SCR_BLACK          0x0000
#define SCR_GREEN          0xE007
#define SCR_RED            0x00F8
#define SCR_BLUE           0x1F00
#define SCR_YELLOW         0xE0FF
#define SCR_LBLUE          0x9E06
#define SCR_PINK           0x3BF0
#define SCR_VIOLET         0x3E88
#define SCR_ORANGE         0x60F4
#define SCR_GRAY           0xEF7B


#define SCR_BLON      255
#define SCR_BLOFF     0

#define IMG_1BIT     1
#define IMG_8BIT     8
#define IMG_16BIT    16




struct sprite_t;

typedef void(*setdotproc)(struct sprite_t*, int, int, uint16_t);
typedef uint16_t(*getdotproc)(struct sprite_t*, int, int);


typedef struct sprite_t 
{
	uint16_t width;
	uint16_t height;
	uint16_t* buf;
	
	uint8_t* bbuf;
	
	unsigned int bytesize;
	
	uint8_t mode;
	
	setdotproc setdot;
	getdotproc getdot;
	
	unsigned int bytesperline;
	
} SPRITE;



void scr_init();
void scr_set(int x,int y,SPRITE* data);
void scr_clear(uint16_t color);
void scr_clear_ex(int x, int y, int w, int h, uint16_t color);


void scr_backlight_loop();
void scr_backlight_set(uint16_t level);
void scr_backlight_set_raw(uint16_t level);
uint16_t scr_backlight_get();


void scr_wakeup();
void scr_sleep();

