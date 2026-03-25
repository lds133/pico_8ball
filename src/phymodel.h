#pragma once

#include <stdint.h>
#include "MPU9250.h"




typedef struct {
	float x;
	float y;

	float a;
	float va;
	
	float vx;
	float vy;
	
	MPUINFO* mpu;
	
	
	float rmax;
	float friction;
	
	bool isdormant;
	bool isdormantready;
	uint64_t dormanttimer; 
	
	
	bool isfaceup;
	
	MPUDATA lastdata;
	
	float dxyz;
	
	uint8_t random;
	
} PHYOBJ;



void phy_init(MPUINFO* mpu);

void phy_update(MPUDATA* d);


PHYOBJ* phy_get();








