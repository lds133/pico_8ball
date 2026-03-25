#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"



#include "MPU9250.h"

#define MPU9255_WHOAMI_VALUE  0x73
#define MPU6500_WHOAMI_VALUE  0x70
#define MPU9250_WHOAMI_VALUE  0x71

#define MPU9250_ADDRESS   0x68
#define MAG_ADDRESS       0x0C

#define GYRO_FULL_SCALE_MASK       0x18
#define GYRO_FULL_SCALE_250_DPS    0x00
#define GYRO_FULL_SCALE_500_DPS     0x08
#define GYRO_FULL_SCALE_1000_DPS    0x10
#define GYRO_FULL_SCALE_2000_DPS    0x18

#define ACC_FULL_SCALE_MASK       0x18
#define ACC_FULL_SCALE_2_G         0x00 
#define ACC_FULL_SCALE_4_G         0x08
#define ACC_FULL_SCALE_8_G         0x10
#define ACC_FULL_SCALE_16_G        0x18

#define DEVID  0xE5

#define PWR_MGMT_1			     0x6B 
#define PWR_MGMT_2			     0x6C
#define INT_ENABLE			     0x38
#define INT_PIN_CFG			     0x37
#define ACCEL_CONFIG2		     0x1D
#define MOT_DETECT_CTRL		     0x69
#define WOM_THR				     0x1F
#define LP_ACCEL_ODR		     0x1E
#define FIFO_COUNTH			     0x72
#define FIFO_COUNTL			     0x73
#define FIFO_EN				     0x23
#define FIFO_R_W				 0x74
#define FIFO_EN_ACCEL_GYRO_BIT   0x78


#define USER_CTRL  0x6A
#define USER_CTRL_FIFO_EN_BIT  0x40
#define USER_CTRL_FIFO_RST_BIT  0x4

#define SMPLRT_DIV    0x19

#define GYRO_CONFIG  0x1B
#define ACCEL_CONFIG  0x1C
#define CONFIG  0x1A


#define PWR_RESET  0x80
#define DIS_GYRO  0x07
#define ACCEL_DLPF_184  0x01
#define INT_WOM_EN  0x40
#define ACCEL_INTEL_MODE  0x40
#define ACCEL_INTEL_EN  0x80
#define PWR_CYCLE  0x20




void mpu_write8(uint8_t addr, uint8_t data);
size_t mpu_read(uint8_t addr, uint8_t* buf, size_t len);
uint8_t mpu_read8(uint8_t addr);
uint16_t mpu_read16(uint8_t addr);
void  mpu_writemask(uint8_t reg, uint8_t data, uint8_t mask);



MPUINFO _info;


MPUINFO* mpu_init()
{
	
	gpio_init(MPU_PIN_INT);
	gpio_set_dir(MPU_PIN_INT, GPIO_IN);	
	
	i2c_init(i2c_default, 400 * 1000);
	gpio_set_function(MPU_PIN_SDA, GPIO_FUNC_I2C);
	gpio_set_function(MPU_PIN_SCL, GPIO_FUNC_I2C);
	gpio_pull_up(MPU_PIN_SDA);
	gpio_pull_up(MPU_PIN_SCL);
	// Make the I2C pins available to picotool
	bi_decl(bi_2pins_with_func(MPU_PIN_SDA, MPU_PIN_SCL, GPIO_FUNC_I2C));
	
	memset(&_info, 0, sizeof(MPUINFO));

	mpu_reset();
	
	_info.whoami = mpu_read8(0x75);
	
	return &_info;
}


void mpu_reset()
{
	mpu_write8(PWR_MGMT_1, 0x80); //  Write a one to bit 7 reset bit; toggle reset device
	sleep_ms(100);
	mpu_write8(PWR_MGMT_1, 0x00); // # Clear sleep mode bit(6), enable all sensors
	mpu_write8(PWR_MGMT_2, 0x00); 
	sleep_ms(100);

}



#define WOM_THRESHOLD  10
#define ACCEL_LP_ODR 10

int mpu_wom()
{
	mpu_write8(PWR_MGMT_1, PWR_RESET);
	sleep_ms(100);// wait for MPU-9250 to come back up
	mpu_write8(PWR_MGMT_1, 0x00);
	sleep_ms(100);
	
	mpu_write8(PWR_MGMT_2, DIS_GYRO);// disable gyro measurements
	mpu_write8(ACCEL_CONFIG2, ACCEL_DLPF_184);// setting accel bandwidth to 184Hz
	mpu_write8(INT_ENABLE, INT_WOM_EN);// enabling interrupt to wake on motion
	mpu_write8(MOT_DETECT_CTRL, (ACCEL_INTEL_EN | ACCEL_INTEL_MODE));// enabling accel hardware intelligence
	mpu_write8(WOM_THR, WOM_THRESHOLD);// setting wake on motion threshold
	mpu_write8(LP_ACCEL_ODR, ACCEL_LP_ODR);// set frequency of wakeup
	mpu_write8(PWR_MGMT_1, PWR_CYCLE);// switch to accel low power mode
	
	return MPU_PIN_INT;
	
}



#define FIFO_PACKET_SIZE 12

void mpu_runfifo(bool isrun)
{
	if (!isrun)
	{
		mpu_writemask(USER_CTRL, 0x04, 0x44);
		return;
	}

	mpu_writemask(GYRO_CONFIG, GYRO_FULL_SCALE_1000_DPS, GYRO_FULL_SCALE_MASK);
	_info.gfsr = 1000;
	
	mpu_writemask(ACCEL_CONFIG, ACC_FULL_SCALE_4_G, ACC_FULL_SCALE_MASK);
	_info.afsr = 4;
	
	mpu_writemask(CONFIG, 0x06, 0x07);
	mpu_writemask(GYRO_CONFIG, 0x00, 0x03);
	mpu_write8(SMPLRT_DIV, 19); // # 0 - 1Khz, 9 - 100 Hz 19 - 50Hz
	_info.freq_hz = 50;
	
	mpu_writemask(FIFO_EN, FIFO_EN_ACCEL_GYRO_BIT, FIFO_EN_ACCEL_GYRO_BIT);
	mpu_writemask(USER_CTRL, 0x44, 0x44);
	
	_info.fifopacketsize = FIFO_PACKET_SIZE;
	_info.fifocounter = 0;
	
}


#define SWAP16(W)  ( ( (W << 8) & 0xff00) | (W >> 8) )
#define FIFOMAXBYTES 512 
#define HALF 0x7FFF

float mpu_convert(uint8_t* buf, int pos, int fsr)
{
	int16_t v;
	*((uint8_t*)&v) = buf[pos+1]; 
	*(((uint8_t*)&v)+1) = buf[pos];
	return ((float)v)*fsr/HALF;
}

size_t  mpu_getfifo(MPUDATA* d, size_t count)
{
	uint16_t fifocount = mpu_read16(FIFO_COUNTH);
	fifocount = SWAP16(fifocount);
	if (fifocount == FIFOMAXBYTES) 
		return MPU_FIFO_OVERFLOW;
	if (fifocount == 0) 
		return 0;
	
	uint16_t n = (int)(fifocount / _info.fifopacketsize);
	if (n > count)
		n = count;

	uint8_t buf[FIFOMAXBYTES];
	mpu_read(FIFO_R_W, buf, n* _info.fifopacketsize); 
	
	for (int i = 0; i < n; i++)
	{
		d[i].ax = mpu_convert(buf, 0, _info.afsr);
		d[i].ay = mpu_convert(buf, 2, _info.afsr);
		d[i].az = mpu_convert(buf, 4, _info.afsr);

		d[i].gx = mpu_convert(buf, 6, _info.gfsr);
		d[i].gy = mpu_convert(buf, 8, _info.gfsr);
		d[i].gz = mpu_convert(buf, 10, _info.gfsr);

	}
	
	_info.fifocounter += n;
	
	return n;
	
}





void  mpu_writemask(uint8_t reg, uint8_t data, uint8_t mask)
{
	uint8_t v = mpu_read8(reg);
	data &= mask;
	v &= ~mask;
	v |= (data & mask);
	mpu_write8(reg, v);
	//v = mpu_read8(reg);
}



void mpu_writeraw(uint8_t* buf,size_t len)
{
	i2c_write_blocking(i2c_default, MPU9250_ADDRESS, buf, len, false);
}


void mpu_write8(uint8_t addr, uint8_t data) 
{
	uint8_t buf[2] = { addr, data };
	mpu_writeraw(buf, 2);
}



size_t mpu_read(uint8_t addr, uint8_t* buf, size_t len) 
{
	if (len == 0)
		return 0;
    i2c_write_blocking(i2c_default, MPU9250_ADDRESS, &addr, 1, true); 
    i2c_read_blocking(i2c_default, MPU9250_ADDRESS, buf, len, false);	
	return len;

}

uint8_t mpu_read8(uint8_t addr)
{
	uint8_t val;
	mpu_read(addr, &val, 1);
	return val;
}

uint16_t mpu_read16(uint8_t addr)
{
	uint16_t val;
	mpu_read(addr, (uint8_t*)&val, 2);
	return val;
}



