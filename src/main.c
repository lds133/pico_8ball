
#include <stdio.h>
#include <assert.h>

#include "common.h"

#include "pico/multicore.h"
#include "pico/sleep.h"

#include "GC9A01.h"
#include "image.h"
#include "battery.h"
#include "MPU9250.h"
#include "phymodel.h"
#include "prophet.h"


#include "hardware/clocks.h"
#include "hardware/structs/scb.h"


#include "hardware/powman.h"
#include "hardware/pll.h"






#define LED_PIN  PICO_DEFAULT_LED_PIN

void error(char* msg)
{
	printf("ERROR: %s\n", msg);
	
	while (1) ;
	
}



bool _core1started = false;
bool _core1stopped = true;
bool _core1needtostop = false;


void core1_main() 
{
	
	printf("Core1 started\n");

	_core1stopped = false;
	
	const uint N = 25;
	MPUDATA d[N];		
	
	
	
	mpu_runfifo(true);
	
	while (1) 
	{
		
		scr_backlight_loop();

		size_t n = mpu_getfifo(d, sizeof(MPUDATA) * N);
		if (n == 0)
			continue;
		if (n == MPU_FIFO_OVERFLOW)
		{
			mpu_runfifo(false);
			printf("FIFO OVERFLOW!!!\n");
			mpu_runfifo(true);
		}
		
		for (int i = 0; i < n; i++)
		{
			phy_update(&d[i]);
		}
		if (!_core1started)
		{
			printf("PHY updated\n");
			_core1started = true;
		}
		
		
		if (_core1needtostop)
			break;
		
		
	}	
	
	mpu_runfifo(false);
	printf("Core1 stopped\n");
	_core1stopped = true;
	
	
}


void start_core1()
{
	_core1started = false;
	_core1needtostop = false;
	multicore_launch_core1(core1_main);
	while (!_core1started)
		tight_loop_contents();	

	printf("Core1 start OK\n");
}

void stop_core1()
{
	_core1needtostop = true;
	while (!_core1stopped)
		tight_loop_contents();
	
	multicore_reset_core1(); 
	
}


void init_clocks() // todo: AI generated check
{
	// Re-initialize clocks manually (replaces removed clocks_init())
    // Step 1: Switch sysclk back to ROSC while we reconfigure PLLs
    clock_configure(clk_sys,
                    CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
                    CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_ROSC_CLKSRC,
                    6 * MHZ,
                    6 * MHZ);

    // Step 2: Re-init system PLL (125 MHz on Pico 2)
    pll_init(pll_sys, 1, 1500 * MHZ, 6, 2);   // VCO=1500MHz, /6/2 = 125MHz

    // Step 3: Switch sysclk to PLL sys
    clock_configure(clk_sys,
                    CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
                    CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
                    125 * MHZ,
                    125 * MHZ);

    // Step 4: Re-init USB PLL (48 MHz)
    pll_init(pll_usb, 1, 1200 * MHZ, 5, 5);   // VCO=1200MHz, /5/5 = 48MHz

    // Step 5: Restore peripheral clock
    clock_configure(clk_peri,
                    0,
                    CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
                    125 * MHZ,
                    125 * MHZ);

    // Step 6: Restore USB clock
    clock_configure(clk_usb,
                    0,
                    CLOCKS_CLK_USB_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                    48 * MHZ,
                    48 * MHZ);

    // Step 7: Restore ADC clock
    clock_configure(clk_adc,
                    0,
                    CLOCKS_CLK_ADC_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                    48 * MHZ,
                    48 * MHZ);

}



void gotosleep()
{
	printf("GOTO SLEEP!!!\n");
	
	gpio_put(LED_PIN, 0); 
	
	
	scr_sleep();
	int wompin = mpu_wom();
	
	uint scb_orig = scb_hw->scr;
	uint clock0_orig = clocks_hw->sleep_en0;
	uint clock1_orig = clocks_hw->sleep_en1;	
	
	sleep_run_from_xosc();
	
	gpio_put(LED_PIN, 0); 
	
	sleep_goto_dormant_until_edge_high(wompin);

	
	rosc_write(&rosc_hw->ctrl, ROSC_CTRL_ENABLE_BITS);
	scb_hw->scr = scb_orig;
	clocks_hw->sleep_en0 = clock0_orig;
	clocks_hw->sleep_en1 = clock1_orig;

	init_clocks();
	
	
	stdio_init_all();
}


void wakeup(MPUINFO* mpu)
{
	gpio_put(LED_PIN, 1); 
	
	scr_wakeup();
	scr_clear(RGB(0, 0, 0));
	
	mpu_reset();
	
	phy_init(mpu);
	
	bat_reset();
	
	printf("WAKEY - WAKEY!\n");
	
}


void main_loop()
{
	
	scr_backlight_set(SCR_BLON);
	
	SPRITE* s;
	SPRITE* st;
	s = img_newsprite(240, 240, IMG_16BIT);
	assert(s != NULL);
	st = img_newsprite(200, 200, IMG_8BIT);
	assert(st != NULL);

	
	
	float x = 0;
	float y = 0;
	float a = 0;	
	bool isgoingtosleep = false;
	uint64_t ts = MILLIS;
	
	
	start_core1();
	
	
	prp_new(phy_get());
	
	
	while (true) 
	{
		
		
		scr_backlight_loop();

		uint16_t bgcolor = SCR_BLUE;

		bgcolor = img_hsv8(150, 255, 255);
		
		int v = bat_mvolt();
		int bp = 100;
		if (v > 0)
			bp = bat_persent();
			
		bgcolor = img_hsv8(250 - bp, 255, 255);
		

		img_clear_raw(s, SCR_BLACK);
		
		PHYOBJ* p = phy_get();		
		int tr_r = 160;
		int lim_r = 30;
		x = ((p->y / p->rmax) * lim_r) + 120;
		y = ((p->x / p->rmax) * lim_r) + 120; 
		a = -p->a;
		img_triangle_ex(s, x, y, tr_r, (int)a, bgcolor);
		
		
		
		uint64_t t0 = MILLIS;
		img_clear_raw(st, SCR_BLACK);
		prp_draw(st, (int)a);
		img_stamp(s, x - 100, y - 100, st, SCR_WHITE);
		uint64_t t1 = MILLIS;
		
		if (isgoingtosleep)
		{	if (!p->isdormant)
			{	scr_backlight_set(SCR_BLON);
				isgoingtosleep = false;
				printf("back alive...\n");
			}
			if (scr_backlight_get() == SCR_BLOFF)
			{
				break;
			}
			
		} else
		{	
			if (p->isdormant)
			{	scr_backlight_set(SCR_BLOFF);
				isgoingtosleep = true;
				printf("feel sleepy...\n");
			} else
			{	if (!p->isfaceup)
				{	scr_backlight_set_raw(SCR_BLOFF);
					prp_new(p);
				} else
				{	scr_backlight_set(SCR_BLON);
				}			
			}			
		}
		
	
		
		gpio_put(LED_PIN, 1); 
		scr_set(0, 0, s);
		gpio_put(LED_PIN, 0); 
		
		
		if (MILLIS - ts > 3000)
		{
			
			printf("ax=%.3f, ay=%.3f, az=%.3f, %.3f (%i,%i) %i rnd=%i  stamptime=%i\n",
				p->lastdata.ax,
				p->lastdata.ay,
				p->lastdata.az,
				p->dxyz,
				p->isdormantready,
				p->isdormant,
				isgoingtosleep,
				p->random,
				(uint)(t1-t0)
				);
			
			
			ts = MILLIS;
		}
		
		
		
		
	}
	
	stop_core1();
	
	img_delsprite(s);
	img_delsprite(st);	
		
	
}





int main() 
{
	set_sys_clock_khz(250000, true);
	
	stdio_init_all();
	
	printf("\r\n\r\n\r\n\r\n");
	printf("Start\n");
	
	
	//printf("GREEN   0x%04X\n", RGB(0, 255, 0));

	

	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);	
		
	bat_init();
	int bp = bat_persent();
	printf(" battery %i%%\n", bp);
	
	MPUINFO* mpu =  mpu_init();
	printf("MPU whoami = 0x%02X\n", mpu->whoami);
	

	scr_init();
	printf("scr_init ... OK\n");
	
	prp_init();

		

	wakeup(mpu);
	
	
	while (1)
	{

		main_loop();
		
		
		gotosleep();
		
		
		wakeup(mpu);
	}
	
	
	
	
	
	
	
	
	
	


}
