#include "image.h"



#include <math.h>
#include <float.h>
#include <stdlib.h>

#include <string.h>

#include "fontlvgl.h"
#include "common.h"


#include <assert.h>
#include <stdio.h>







#define FONTDELIM  3	
#define SPC  ' '	
#define SPCWIDTH  7



int img_charwidth(char ch)
{
	if (ch == SPC)
		return SPCWIDTH;
	return FONT.get_glyph_dsc(&FONT, ch)->w_px;
}


int img_textwidth(const char* text)
{
	/*
	{
		for (int i = 0; i < strlen(text); i++)
		{
			printf("%c 0x%02X\n", text[i], text[i] );
		}
	}
	*/
	int n = strlen(text);
	int s = 0;
	for (int i = 0; i < n; i++)
	{	if (i != 0)
			s += FONTDELIM;
		s += img_charwidth(text[i]);
	}

	return s;
}
int img_textheight()
{
	return FONT.h_px;
	
}




int img_char(SPRITE* s, char ch, int x, int y, uint16_t color)
{
	if (ch == SPC)
		return SPCWIDTH;
	
	const lv_font_glyph_dsc_t* d = FONT.get_glyph_dsc(&FONT, ch);

	int gi = d->glyph_index;
	int wpx = d->w_px;
	int wbt = d->w_px/8 + ( (d->w_px % 8 !=0) ? 1 : 0);
	const uint8_t* chptr = &(FONT.glyph_bitmap[gi]);
	int hpx = FONT.h_px;

	for (int i = 0; i < hpx; i++)
	{
		for (int k = 0; k < wbt; k++)
		{
			uint8_t c = *(chptr + i*wbt + k);
			for (int j = 0; j < 8; j++)
				if ((1 << (8 - j)) & c)
					s->setdot(s, x + j + 7 * k, y + i, color);
		}
	}
	
	return wpx;

}


void img_text(SPRITE* s, const char* text, int x, int y, uint16_t color)
{

	int n = strlen(text);
	int dx = 0;
	for (int i = 0; i < n; i++)
	{
		if (i != 0)
			dx += FONTDELIM;
		int chwidth = img_char(s, text[i], x + dx, y, color);
		dx += chwidth;
	}
	
}

int img_textcenter(SPRITE* s, int y, const char* text, uint16_t color)
{
	int x = (s->width - img_textwidth(text)) / 2;
	img_text(s, text, x, y, color);
}






void img_dot(SPRITE* s, int x, int y, uint16_t c)
{
	if ((x >= s->width) || (x < 0) || (y >= s->height) || (y < 0))
		return;
	s->buf[x + y*s->width] = c;		
	
}

uint16_t img_dotcolor(SPRITE* s, int x, int y)
{
	if ((x >= s->width) || (x < 0) || (y >= s->height) || (y < 0))
		return SCR_BLACK;	
	return s->buf[x + y*s->width];		
}


void img_dotbw(SPRITE* s, int x, int y, uint16_t c)
{
	if ((x >= s->width) || (x < 0) || (y >= s->height) || (y < 0))
		return;
	int pos = s->bytesperline*y +  x / 8;
	int offset = x % 8;
	if (c == SCR_BLACK)
	{
		s->bbuf[pos] &= ~(1 << offset);
	}
	else
	{
		s->bbuf[pos] |= (1 << offset);
	}
}

uint16_t img_dotcolorbw(SPRITE* s, int x, int y)
{
	
	if ((x >= s->width) || (x < 0) || (y >= s->height) || (y < 0))
		return SCR_BLACK;
	int pos = s->bytesperline*y +  x / 8;
	int offset = x % 8;
	return (s->bbuf[pos] & (1 << offset)) == 0 ? SCR_BLACK : SCR_WHITE;
}











void img_dotbyte(SPRITE* s, int x, int y, uint16_t c)
{
	if ((x >= s->width) || (x < 0) || (y >= s->height) || (y < 0))
		return;
	s->bbuf[ s->bytesperline*y +  x ] = c;

	
}

uint16_t img_dotcolorbyte(SPRITE* s, int x, int y)
{
	if ((x >= s->width) || (x < 0) || (y >= s->height) || (y < 0))
		return SCR_BLACK;	
	return s->bbuf[s->bytesperline*y +  x];		
}




	
SPRITE * img_newsprite(int w, int h, uint8_t  mode)
{
	SPRITE* s = (SPRITE*) malloc(sizeof(SPRITE));
	if (s == NULL)
		return NULL;
	s->mode = mode;
	switch (s->mode)
	{
		case IMG_1BIT:
			s->setdot = &img_dotbw;
			s->getdot = &img_dotcolorbw;
			s->bytesperline = (w / 8 + (w % 8 == 0 ? 0 : 1));
			break;
		case IMG_16BIT:
			s->setdot = &img_dot;
			s->getdot = &img_dotcolor;
			s->bytesperline = w * 2;
			break;
		case IMG_8BIT:
			s->setdot = &img_dotbyte;
			s->getdot = &img_dotcolorbyte;
			s->bytesperline = w;
			break;
	
		default:
			free(s);
			assert(false);
			return NULL;
			break;
	}
	s->bytesize = s->bytesperline * h;	
	s->width = w;
	s->height = h;
	s->buf = (uint16_t*) malloc(s->bytesize);
	if (s->buf == NULL)
	{	free(s);
		return NULL;
	}
	uint8_t* b = (uint8_t*)s->buf;
	s->bbuf = b;
	img_clear_raw(s, SCR_BLACK);
	return s;
}




void img_delsprite(SPRITE* s)
{
	free(s->buf);
	free(s);
}

	



uint8_t* img_getdata(SPRITE* s)
{
	return (uint8_t*)s->buf;
}



void img_stamp(SPRITE* s, int x, int y, SPRITE* stamp, uint16_t color) 
{
	for (int i = 0; i < stamp->width; i++)
		for (int j = 0; j < stamp->height; j++)
			if (stamp->getdot(stamp,i, j) != SCR_BLACK)
				s->setdot(s,x + i, y + j, color);
}



void img_clear(SPRITE* s, uint16_t c)
{
	for (int x = 0; x < s->width; x++)
		for (int y = 0; y < s->height; y++)
			s->setdot(s, x, y, c);
}

void img_clear_raw(SPRITE* s,uint8_t value)
{
	memset(s->buf, value, s->bytesize);
}





#define SIND(a) (sin((((float)a)* M_PI)/180.0  ) )
#define COSD(a) (cos((((float)a)* M_PI)/180.0  ) )




void img_rotate_ex(SPRITE* s,int y0,int yn, int  angle_deg)
{
	
	SPRITE* ss = img_newsprite(s->width, s->height,s->mode);
	
	assert(ss != NULL);
	
	if (ss == NULL)
		return;

	float sinx = SIND(angle_deg); // -degrees
	float cosx = COSD(angle_deg);

	uint16_t defaultpixel = 0;
	
	int xCenter = s->height/2 ;
	int yCenter = s->width/2;
	
	int x0 = 0;
	int xn = s->width;

	for (int x = x0; x < xn; x++) 
	{
		int xt = x - xCenter;
		float xt_cosx = xt*cosx;
		float xt_sinx = xt*sinx;
		for (int y = y0; y < yn; y++) 
		{
			int  yt = y - yCenter;
			long xRotate = lround(xt_cosx - (yt*sinx)) + xCenter;
			long yRotate = lround((yt*cosx) + xt_sinx) + yCenter;   

	 	    //s->setdot(ss, x, y, s->getdot(s,xRotate,yRotate));
			s->setdot(ss, xRotate, yRotate, s->getdot(s, x, y));
			
		}
	}
	
	free(s->buf);
	s->buf = ss->buf;
	s->bbuf = ss->bbuf;
	free(ss);
	
}


void img_rotate(SPRITE* s, int  angle_deg)
{
	return img_rotate_ex(s, s->width, s->height, angle_deg);
	
}





uint16_t img_rgb(float r, float g, float b)
{
	const int max5bit = 31;
	const int max6bit = 63;

	
	int rr = (int)(r*max5bit);
	int gg = (int)(g*max6bit);
	int bb = (int)(b*max5bit);
	
	uint16_t c = (uint16_t)(bb  | gg << 5 | rr << 11);
	return ((c << 8) & 0xff00) | (c >> 8);
	
}





uint16_t img_hsv8(uint8_t h, uint8_t s, uint8_t v) 
{
	int r, g, b;
	
	unsigned char region, remainder, p, q, t;

	if (s == 0)
	{	r = v;
		g = v;
		b = v;
	}	else
	{
		unsigned char region, remainder, p, q, t;

		region = h / 43;
		remainder = (h - (region * 43)) * 6;

		p = (v * (255 - s)) >> 8;
		q = (v * (255 - ((s * remainder) >> 8))) >> 8;
		t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

		switch (region)
		{
			case 0:    r = v; g = t; b = p;	 break;
			case 1:    r = q; g = v; b = p;	 break;
			case 2:    r = p; g = v; b = t;  break;
			case 3:    r = p; g = q; b = v;  break;
			case 4:    r = t; g = p; b = v;  break;
			default:   r = v; g = p; b = q;  break;
		}
	}
	
	return img_rgb((float)r / 255, (float)g / 255, (float)b / 255);

}


uint16_t img_hsv(float hh, float ss, float vv) 
{
	unsigned char h = (unsigned char)(hh * 255);
	unsigned char s = (unsigned char)(ss * 255);
	unsigned char v = (unsigned char)(vv * 255);		

	return img_hsv8(h, s, v); 
}














void img_rect(SPRITE* s, int x, int y, int w, int h, uint16_t c)
{
	
	for (int i = 0; i < w; i++)
		for (int j = 0; j < h; j++)
			s->setdot(s, x + i, y + j, c);
	
}




#pragma region TRIANGLE


#define SWAP(x,y) 		do { (x)=(x)^(y); (y)=(x)^(y); (x)=(x)^(y); } while(0)


//uint8_t _color_counter=0;

void img_hline(SPRITE* s, int x1, int x2, int y, uint16_t color) 
{
	if (x1 >= x2) 
		SWAP(x1, x2);

//	color = img_hsv8(_color_counter++, 255, 255);
	
	for (; x1 <= x2; x1++) 
		s->setdot(s, x1, y, color);
}



/* Fill Triangle Function */
void img_triangle(SPRITE* s, int x1, int y1, int x2, int y2, int x3, int y3, uint16_t color) 
{
	
	//_color_counter = 0;
	
	int t1x, t2x, y, minx, maxx, t1xp, t2xp;
	int changed1 = 0;
	int changed2 = 0;
	int signx1, signx2, dx1, dy1, dx2, dy2;
	int e1, e2;

	// Sort vertices
	if (y1 > y2) { SWAP(y1, y2); SWAP(x1, x2); }
	if (y1 > y3) { SWAP(y1, y3); SWAP(x1, x3); }
	if (y2 > y3) { SWAP(y2, y3); SWAP(x2, x3); }

	t1x = t2x = x1; 
	y = y1; // Starting points

	dx1 = (x2 - x1); if (dx1 < 0) { dx1 = -dx1; signx1 = -1; }
	else signx1 = 1;
	dy1 = (y2 - y1);
 
	dx2 = (x3 - x1); if (dx2 < 0) { dx2 = -dx2; signx2 = -1; }
	else signx2 = 1;
	dy2 = (y3 - y1);
	
	if (dy1 > dx1) {
		// swap values
		SWAP(dx1, dy1);
		changed1 = 1;
	}
	if (dy2 > dx2) {
		// swap values
		SWAP(dy2, dx2);
		changed2 = 1;
	}
	
	e2 = (dx2 >> 1);
	// Flat top, just process the second half
	if (y1 == y2) goto next;
	e1 = (dx1 >> 1);
	
	for (int i = 0; i < dx1;) {
		t1xp = 0; t2xp = 0;
		if (t1x < t2x) { minx = t1x; maxx = t2x; }
		else		{ minx = t2x; maxx = t1x; }
		// process first line until y value is about to change
		while (i < dx1) {
			i++;			
			e1 += dy1;
			while (e1 >= dx1) {
				e1 -= dx1;
				if (changed1) t1xp = signx1; //t1x += signx1;
				else          goto next1;
			}
			if (changed1) break;
			else t1x += signx1;
		}
		// Move line
next1:
		// process second line until y value is about to change
		while (1) {
			e2 += dy2;		
			while (e2 >= dx2) {
				e2 -= dx2;
				if (changed2) t2xp = signx2; //t2x += signx2;
				else          goto next2;
			}
			if (changed2)     break;
			else              t2x += signx2;
		}
next2:
		if (minx > t1x) minx = t1x; if (minx > t2x) minx = t2x;
		if (maxx < t1x) maxx = t1x; if (maxx < t2x) maxx = t2x;
		img_hline(s,minx, maxx, y, color); // Draw line from min to max points found on the y
		// Now increase y
		if (!changed1) t1x += signx1;
		t1x += t1xp;
		if (!changed2) t2x += signx2;
		t2x += t2xp;
		y += 1;
		if (y == y2) break;
		
	}
next:
	// Second half
	dx1 = (x3 - x2); if (dx1 < 0) { dx1 = -dx1; signx1 = -1; }
	else signx1 = 1;
	dy1 = (y3 - y2);
	t1x = x2;
 
	if (dy1 > dx1) {
		// swap values
		SWAP(dy1, dx1);
		changed1 = 1;
	}
	else changed1 = 0;
	
	e1 = (dx1 >> 1);
	
	for (int i = 0; i <= dx1; i++) {
		t1xp = 0; t2xp = 0;
		if (t1x < t2x) { minx = t1x; maxx = t2x; }
		else		{ minx = t2x; maxx = t1x; }
		// process first line until y value is about to change
		while (i < dx1) {
			e1 += dy1;
			while (e1 >= dx1) {
				e1 -= dx1;
				if (changed1) { t1xp = signx1; break; }//t1x += signx1;
				else          goto next3;
			}
			if (changed1) break;
			else   	   	  t1x += signx1;
			if (i < dx1) i++;
		}
next3:
		// process second line until y value is about to change
		while (t2x != x3) {
			e2 += dy2;
			while (e2 >= dx2) {
				e2 -= dx2;
				if (changed2) t2xp = signx2;
				else          goto next4;
			}
			if (changed2)     break;
			else              t2x += signx2;
		}	   	   
next4:

		if (minx > t1x) minx = t1x; if (minx > t2x) minx = t2x;
		if (maxx < t1x) maxx = t1x; if (maxx < t2x) maxx = t2x;
		img_hline(s,minx, maxx, y, color); // Draw line from min to max points found on the y
		// Now increase y
		if (!changed1) t1x += signx1;
		t1x += t1xp;
		if (!changed2) t2x += signx2;
		t2x += t2xp;
		y += 1;
		if (y > y3) return;
	}
}


void img_triangle_ex(SPRITE* s, int x, int y, int n, int a, uint16_t color)
{
	

	int r = (int)(((float)n) / M_SQRT3);
	
	int x1 = r*SIND(a)+x;
	int y1 = r*COSD(a)+y;
	int x2 = r*SIND(a+120) + x;
	int y2 = r*COSD(a+120) + y;
	int x3 = r*SIND(a - 120) + x;
	int y3 = r*COSD(a - 120) + y;

	img_triangle(s, x1, y1, x2, y2, x3, y3, color);
	
	
	
}


void img_circle(SPRITE* s, int x, int y, int r, uint16_t color)
{
	for (int a = 0; a < 360; a++)
	{
		int xx = r*SIND(a) + x; 
		int yy = r*COSD(a) + y;		
		s->setdot(s, xx, yy, color);
	}
}




#pragma endregion