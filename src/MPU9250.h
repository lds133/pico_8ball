#pragma once
#include <stdbool.h>
#include <stddef.h>

#define MPU_PIN_SCL 17
#define MPU_PIN_SDA 16
#define MPU_PIN_INT 18



#define MPU_FIFO_OVERFLOW 999



typedef struct {
	uint8_t whoami;
	
	int fifopacketsize;
	
	float afsr;
	float gfsr;
	
	
	uint64_t fifocounter;
	
	int freq_hz;
	
	
} MPUINFO;


typedef struct {
	float ax;
	float ay;
	float az;
	float gx;
	float gy;
	float gz;
	
} MPUDATA;




MPUINFO* mpu_init();

void mpu_reset();

void mpu_runfifo(bool isrun);
size_t  mpu_getfifo(MPUDATA* d, size_t maxbytesize);
int mpu_wom();
