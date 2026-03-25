#include "common.h"
#include "battery.h"

#include "hardware/gpio.h"
#include "hardware/adc.h"

#include <string.h>




uint8_t  _p[] = { 100,100, 95, 90, 85, 80, 75, 70, 65, 60, 55, 50, 45, 40, 35, 30, 25, 20, 15, 10,  5,  0,  0, 0};
uint16_t _v[] = { 999,420,415,411,408,402,398,395,391,387,385,384,382,380,379,377,375,373,371,369,361,327,250, 0};	
uint16_t  _size = sizeof(_v) / sizeof(uint16_t);



#define  HISTORYSIZE   20 

uint16_t _vv[HISTORYSIZE] ;

uint64_t _ts=0;


#define PERIOD 1000 //ms

uint16_t  _historypos;
bool  _isvalid;

int _historyvalue;


void bat_init()
{
	
	adc_init();
	adc_gpio_init(26);
	adc_select_input(0);
	
	bat_reset();

}

void bat_reset()
{
	_historyvalue = BATTERY_NOTREADY;
	_isvalid = false;
	_historypos = 0;
	memset(_vv, 0, sizeof(_vv));
}



int bat_persent()
{
	

	int v = bat_mvolt() / 10; 
	for (int i = 1; i < _size; i++)
	{
		if (v == _v[i])
			return _p[i];
		if ((v < _v[i - 1]) && (v > _v[i]))
		{
			int p = (_p[i - 1]*(_v[i] - v) + _p[i]*(v - _v[i - 1])) / (_v[i] - _v[i - 1]);
			return p;			
		}
	}
	return BATTERY_ERROR;

}


int bat_mvolt()
{

	
	if ( (MILLIS - _ts) < PERIOD )
		return _historyvalue; 
	
	const float conversion_factor = 3.3f / (1 << 12);
	uint16_t result = adc_read();
	int v =  (int)( (((float)result) * conversion_factor*2.0)*1000 ) ;
	
	_ts = MILLIS;
	_vv[_historypos] = v;
	_historypos++;
	if (_historypos >= HISTORYSIZE)
	{	_historypos = 0;
		_isvalid = true;
	}
	
	if (_isvalid)
	{	uint64_t s = 0;
		for (int i = 0; i < HISTORYSIZE; i++)
			s += _vv[i];
		_historyvalue =  (int)(s / HISTORYSIZE);	
	} 
	
	return _historyvalue;

}




