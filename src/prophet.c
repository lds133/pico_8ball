


#include "prophet.h"
#include "prophet_card.h"
#include "image.h"
#include "common.h"

#include <stdio.h>
#include <assert.h>



extern FTCARD CARDS;
extern const unsigned int CARDS_MAX;

FTCARD* _cards = &CARDS;


int _maxindex=0;

FTCARD* _selectedcard = NULL;

void prp_init()
{
	
	_selectedcard = &(_cards[_maxindex - 1]);
	
	printf("connecting with the astral... OK\n");	
	
	
	
}



void prp_new(PHYOBJ* p)
{
	uint8_t rnd = p->random;
	
	
	_maxindex = CARDS_MAX;

	float r = ((float)rnd) / 255;
	int index =  (int)(r*(_maxindex-1)); 
	_selectedcard  = &_cards[index];

	p->va = 0;
	p->a = (int)(360.0 * r);
	
	printf("new prophesy granted  %i (%i) - %s %s %s %s \n", index, rnd, _selectedcard->line[0], _selectedcard->line[1], _selectedcard->line[2], _selectedcard->line[3]);
	
}







void prp_draw_up(SPRITE* s,int angle_deg)
{
	
	int y0 = 0;
	int yn = 0;
	
	int h = img_textheight();
	int cy = ((s->height - h) / 2);
	
	
	switch (_selectedcard->nlines)
	{
	
		case 1:
			img_textcenter(s, cy, _selectedcard->line[0],SCR_WHITE);
			y0 = cy;
			yn = cy + h;
			break;
		case 2:
			img_textcenter(s, cy-h, _selectedcard->line[0],SCR_WHITE);
			img_textcenter(s, cy, _selectedcard->line[1],SCR_WHITE);
			y0 = cy-h;
			yn = cy + h;
			break;
		case 3:
			cy -= 5;
			img_textcenter(s, cy - h, _selectedcard->line[0],SCR_WHITE);
			img_textcenter(s, cy, _selectedcard->line[1],SCR_WHITE);
			img_textcenter(s, cy+h, _selectedcard->line[2],SCR_WHITE);
			y0 = cy-h;
			yn = cy + 2*h;
			break;
		
		default:
			assert(false);
			img_textcenter(s, cy, "nlines error",SCR_WHITE);
			break;
		
		
	}
	
	img_rotate_ex(s, y0, yn, -(int)angle_deg);
	
	//img_rotate(st,  -(int)a);

	
	
	

	
}

void prp_draw_dn(SPRITE* s, int angle_deg)
{
	
	int y0 = 0;
	int yn = 0;
	
	int h = img_textheight();
	int cy = ((s->height - h) / 2) ;
	
	
	switch (_selectedcard->nlines)
	{
	
	case 1:
		img_textcenter(s, cy, _selectedcard->line[0], SCR_WHITE);
		y0 = cy;
		yn = cy + h;
		break;
	case 2:
		cy += h;
		img_textcenter(s, cy - h, _selectedcard->line[0], SCR_WHITE);
		img_textcenter(s, cy, _selectedcard->line[1], SCR_WHITE);
		y0 = cy - h;
		yn = cy + h;
		break;
	case 3:
		cy += 10;
		img_textcenter(s, cy - h, _selectedcard->line[0], SCR_WHITE);
		img_textcenter(s, cy, _selectedcard->line[1], SCR_WHITE);
		img_textcenter(s, cy + h, _selectedcard->line[2], SCR_WHITE);
		y0 = cy - h;
		yn = cy + 2*h;
		break;
		
	default:
		assert(false);
		img_textcenter(s, cy, "nlines error", SCR_WHITE);
		break;
		
		
	}
	
	img_rotate_ex(s, y0, yn, -(int)angle_deg+180);
	
	//img_rotate(st,  -(int)a);

	
	
	

	
}


void prp_draw(SPRITE* s, int angle_deg)
{
	if (_selectedcard->direction == UP)
	{
		return prp_draw_up(s, angle_deg);	
	} else
	{
		return prp_draw_dn(s, angle_deg);	
	}
	
}











