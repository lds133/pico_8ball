#include "common.h"

#include "hardware/spi.h"
#include "pico/binary_info.h"
#include "hardware/dma.h"
#include "hardware/pwm.h"

#include "GC9A01.h"


#define PIN_MISO 4
#define PIN_CS   (SCR_PIN_CS)    //5
#define PIN_SCK  (SCR_PIN_CLK)   //6
#define PIN_MOSI (SCR_PIN_DIN)   //7

#define SPI_PORT spi0


#define RESET_0			gpio_put(SCR_PIN_RST, 0);
#define RESET_1			gpio_put(SCR_PIN_RST, 1);
#define DC_0	    	gpio_put(SCR_PIN_DC, 0);
#define DC_1	    	gpio_put(SCR_PIN_DC, 1);

#define CS_0    		asm volatile("nop \n nop \n nop");gpio_put(PIN_CS, 0);asm volatile("nop \n nop \n nop");
#define CS_1    		asm volatile("nop \n nop \n nop");gpio_put(PIN_CS, 1);asm volatile("nop \n nop \n nop");







#pragma region BACK LIGHT


uint16_t _bl_curr;
uint16_t _bl_target;
uint64_t _bl_timestamp;

#define BL_MAX 255



void scr_backlight_set_raw(uint16_t level)
{
	pwm_set_gpio_level(SCR_PIN_BL, level * level);
	_bl_curr = level;
}


void scr_backlight_init()
{
	_bl_curr = SCR_BLOFF;
	_bl_target = SCR_BLOFF;
	_bl_timestamp = MILLIS;

	gpio_set_function(SCR_PIN_BL, GPIO_FUNC_PWM);
	uint slice_num = pwm_gpio_to_slice_num(SCR_PIN_BL);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.f);

    pwm_init(slice_num, &config, true);
	
	scr_backlight_set_raw(SCR_BLOFF);
	
}

void scr_backlight_loop()
{
	if (_bl_curr == _bl_target)
		return;
	if (MILLIS - _bl_timestamp < SCR_BL_CHANGE_PERIOD_MS)
		return;
	
	scr_backlight_set_raw(  _bl_curr + ((_bl_curr < _bl_target) ? 1 : (-1))   );
	_bl_timestamp = MILLIS;
}


void scr_backlight_set(uint16_t level)
{
	_bl_target = (level > BL_MAX) ? BL_MAX : level;
}

uint16_t scr_backlight_get()
{
	return _bl_curr;
}

#pragma endregion




void scr_hw_setup()
{
	gpio_init(SCR_PIN_RST);
	gpio_set_dir(SCR_PIN_RST, GPIO_OUT);
	
	gpio_init(SCR_PIN_BL);
	gpio_set_dir(SCR_PIN_BL, GPIO_OUT);
	
	gpio_init(SCR_PIN_DC);
	gpio_set_dir(SCR_PIN_DC, GPIO_OUT);
	
	
	
	
	// This example will use SPI0 at 0.5MHz.
	spi_init(SPI_PORT, 133000 * 1000);
	gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
	gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
	gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
	
	// Make the SPI pins available to picotool
	bi_decl(bi_3pins_with_func(PIN_MISO, PIN_MOSI, PIN_SCK, GPIO_FUNC_SPI));

	// Chip select is active-low, so we'll initialise it to a driven-high state
	gpio_init(PIN_CS);
	gpio_set_dir(PIN_CS, GPIO_OUT);
	gpio_put(PIN_CS, 1);
	// Make the CS pin available to picotool
	bi_decl(bi_1pin_with_name(PIN_CS, "SPI CS"));
	
	scr_backlight_init();	
	
	
}






void scr_writeN(uint8_t* dataptr, size_t datasize)
{

	CS_0;	
	
	const uint dma_tx = dma_claim_unused_channel(true);
	
	hw_set_bits(&spi_get_hw(spi_default)->cr1, SPI_SSPCR1_LBM_BITS);		
	
	dma_channel_config c = dma_channel_get_default_config(dma_tx);
	channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
	channel_config_set_dreq(&c, spi_get_index(spi_default) ? DREQ_SPI1_TX : DREQ_SPI0_TX);
	dma_channel_configure(dma_tx,
		&c,
		&spi_get_hw(spi_default)->dr, // write address
		dataptr, // read address
		datasize, // element count (each element is of size transfer_data_size)
		true); // don't start yet
	
	dma_channel_wait_for_finish_blocking(dma_tx);
	
	dma_channel_unclaim(dma_tx);
	
	
	CS_1;	
	
}

















void scr_writeN_1(uint8_t* dataptr, size_t n)
{
	CS_0;	
	
	spi_write_blocking(spi_default, dataptr, n);
	
	CS_1;	
}


void scr_write1(uint8_t data)
{
	scr_writeN_1(&data, 1);
}



static void scr_cmd(uint8_t reg)
{
	DC_0;
	scr_write1(reg);
}

static void scr_dat8(uint8_t data)
{
	DC_1;
	scr_write1(data);
}


static void scr_dat16(uint16_t data)
{
	DC_1;
	scr_write1(data >> 8);
	scr_write1(data);
	
}

void scr_initreg()
{
	scr_cmd(0xEF);
	scr_cmd(0xEB);
	scr_dat8(0x14); 
	
	scr_cmd(0xFE);			 
	scr_cmd(0xEF); 

	scr_cmd(0xEB);	
	scr_dat8(0x14); 

	scr_cmd(0x84);			
	scr_dat8(0x40); 

	scr_cmd(0x85);			
	scr_dat8(0xFF); 

	scr_cmd(0x86);			
	scr_dat8(0xFF); 

	scr_cmd(0x87);			
	scr_dat8(0xFF);

	scr_cmd(0x88);			
	scr_dat8(0x0A);

	scr_cmd(0x89);			
	scr_dat8(0x21); 

	scr_cmd(0x8A);			
	scr_dat8(0x00); 

	scr_cmd(0x8B);			
	scr_dat8(0x80); 

	scr_cmd(0x8C);			
	scr_dat8(0x01); 

	scr_cmd(0x8D);			
	scr_dat8(0x01); 

	scr_cmd(0x8E);			
	scr_dat8(0xFF); 

	scr_cmd(0x8F);			
	scr_dat8(0xFF); 


	scr_cmd(0xB6);
	scr_dat8(0x00);
	scr_dat8(0x20);

	scr_cmd(0x36);
	scr_dat8(0x08); //Set as vertical screen

	scr_cmd(0x3A);			
	scr_dat8(0x05); 


	scr_cmd(0x90);			
	scr_dat8(0x08);
	scr_dat8(0x08);
	scr_dat8(0x08);
	scr_dat8(0x08); 

	scr_cmd(0xBD);			
	scr_dat8(0x06);
	
	scr_cmd(0xBC);			
	scr_dat8(0x00);	

	scr_cmd(0xFF);			
	scr_dat8(0x60);
	scr_dat8(0x01);
	scr_dat8(0x04);

	scr_cmd(0xC3);			
	scr_dat8(0x13);
	scr_cmd(0xC4);			
	scr_dat8(0x13);

	scr_cmd(0xC9);			
	scr_dat8(0x22);

	scr_cmd(0xBE);			
	scr_dat8(0x11); 

	scr_cmd(0xE1);			
	scr_dat8(0x10);
	scr_dat8(0x0E);

	scr_cmd(0xDF);			
	scr_dat8(0x21);
	scr_dat8(0x0c);
	scr_dat8(0x02);

	scr_cmd(0xF0);   
	scr_dat8(0x45);
	scr_dat8(0x09);
	scr_dat8(0x08);
	scr_dat8(0x08);
	scr_dat8(0x26);
	scr_dat8(0x2A);

	scr_cmd(0xF1);    
	scr_dat8(0x43);
	scr_dat8(0x70);
	scr_dat8(0x72);
	scr_dat8(0x36);
	scr_dat8(0x37);  
	scr_dat8(0x6F);


	scr_cmd(0xF2);   
	scr_dat8(0x45);
	scr_dat8(0x09);
	scr_dat8(0x08);
	scr_dat8(0x08);
	scr_dat8(0x26);
	scr_dat8(0x2A);

	scr_cmd(0xF3);   
	scr_dat8(0x43);
	scr_dat8(0x70);
	scr_dat8(0x72);
	scr_dat8(0x36);
	scr_dat8(0x37); 
	scr_dat8(0x6F);

	scr_cmd(0xED);	
	scr_dat8(0x1B); 
	scr_dat8(0x0B); 

	scr_cmd(0xAE);			
	scr_dat8(0x77);
	
	scr_cmd(0xCD);			
	scr_dat8(0x63);		


	scr_cmd(0x70);			
	scr_dat8(0x07);
	scr_dat8(0x07);
	scr_dat8(0x04);
	scr_dat8(0x0E); 
	scr_dat8(0x0F); 
	scr_dat8(0x09);
	scr_dat8(0x07);
	scr_dat8(0x08);
	scr_dat8(0x03);

	scr_cmd(0xE8);			
	scr_dat8(0x34);

	scr_cmd(0x62);			
	scr_dat8(0x18);
	scr_dat8(0x0D);
	scr_dat8(0x71);
	scr_dat8(0xED);
	scr_dat8(0x70); 
	scr_dat8(0x70);
	scr_dat8(0x18);
	scr_dat8(0x0F);
	scr_dat8(0x71);
	scr_dat8(0xEF);
	scr_dat8(0x70); 
	scr_dat8(0x70);

	scr_cmd(0x63);			
	scr_dat8(0x18);
	scr_dat8(0x11);
	scr_dat8(0x71);
	scr_dat8(0xF1);
	scr_dat8(0x70); 
	scr_dat8(0x70);
	scr_dat8(0x18);
	scr_dat8(0x13);
	scr_dat8(0x71);
	scr_dat8(0xF3);
	scr_dat8(0x70); 
	scr_dat8(0x70);

	scr_cmd(0x64);			
	scr_dat8(0x28);
	scr_dat8(0x29);
	scr_dat8(0xF1);
	scr_dat8(0x01);
	scr_dat8(0xF1);
	scr_dat8(0x00);
	scr_dat8(0x07);

	scr_cmd(0x66);			
	scr_dat8(0x3C);
	scr_dat8(0x00);
	scr_dat8(0xCD);
	scr_dat8(0x67);
	scr_dat8(0x45);
	scr_dat8(0x45);
	scr_dat8(0x10);
	scr_dat8(0x00);
	scr_dat8(0x00);
	scr_dat8(0x00);

	scr_cmd(0x67);			
	scr_dat8(0x00);
	scr_dat8(0x3C);
	scr_dat8(0x00);
	scr_dat8(0x00);
	scr_dat8(0x00);
	scr_dat8(0x01);
	scr_dat8(0x54);
	scr_dat8(0x10);
	scr_dat8(0x32);
	scr_dat8(0x98);

	scr_cmd(0x74);			
	scr_dat8(0x10);	
	scr_dat8(0x85);	
	scr_dat8(0x80);
	scr_dat8(0x00); 
	scr_dat8(0x00); 
	scr_dat8(0x4E);
	scr_dat8(0x00);					
	
	scr_cmd(0x98);			
	scr_dat8(0x3e);
	scr_dat8(0x07);

	scr_cmd(0x35);	
	scr_cmd(0x21);

	scr_cmd(0x11);
	sleep_ms(120);
	scr_cmd(0x29);
	sleep_ms(20);
}





static void scr_setattr(uint8_t scandir)
{
	uint8_t memoryaccessreg = 0x08;
	if (scandir == SCR_HORIZONTAL) 
	{
		memoryaccessreg = 0xC8;
	}
	else 
	{
		memoryaccessreg = 0x68;
	}
	scr_cmd(0x36);
	scr_dat8(memoryaccessreg); 
}




void scr_setwin(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend)
{

	scr_cmd(0x2A);
	scr_dat8(0x00);
	scr_dat8(Xstart);
	scr_dat8((Xend - 1) >> 8);
	scr_dat8(Xend - 1);

	scr_cmd(0x2B);
	scr_dat8(0x00);
	scr_dat8(Ystart);
	scr_dat8((Xend - 1) >> 8);
	scr_dat8(Yend - 1);

	scr_cmd(0X2C);
}



void scr_clear_ex(int x, int y, int w, int h, uint16_t color)
{
	uint16_t buf[SCR_WIDTH];
	for (int i = 0; i < w; i++)
		buf[i] = color;
	scr_setwin(x,y,x+w,x+h);
	DC_1;	
	for (int i = 0; i <= h; i++)// todo: figure out why '<='
		scr_writeN((uint8_t *)buf, w * 2);
}




void scr_clear(uint16_t color)
{
	scr_clear_ex(0, 0, SCR_WIDTH, SCR_HEIGHT, color);
}




void scr_set(int x, int y, SPRITE* data)
{
	scr_setwin(x, y, x+data->width, y+data->height);
	DC_1;
	scr_writeN((uint8_t *)data->buf, data->bytesize);
	
}



void scr_init()
{
	scr_hw_setup();


}





void scr_wakeup()
{
	RESET_1;
	sleep_ms(100);	
	RESET_0;
	sleep_ms(100);
	RESET_1;
	sleep_ms(100);
	
	
	scr_setattr(SCR_HORIZONTAL);
	scr_initreg();	
}


void scr_sleep()
{
	scr_backlight_set_raw(SCR_BLOFF);
	RESET_0;
	
}