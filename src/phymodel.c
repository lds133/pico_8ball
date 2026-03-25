
#include  "phymodel.h"
#include "common.h"

#include <string.h>
#include <math.h>

#include <stdio.h>


PHYOBJ _obj =  { 0 };



void phy_init(MPUINFO* mpu)
{
	memset(&_obj, 0, sizeof(PHYOBJ));
	
	_obj.mpu = mpu;
	
	_obj.x = 0;
	_obj.y = 0;
	_obj.vx = 0;
	_obj.vy = 0;
	_obj.a = 0;
	_obj.va = 0;
	
	_obj.isfaceup=false;
	
	_obj.rmax = 100000;
	_obj.friction = PHY_FRICTION;
	_obj.isdormant = false;
	_obj.dormanttimer = MILLIS;
	_obj.isdormantready = false;
	_obj.dxyz = 0.0;
	_obj.random = 0;
	
	memset(&(_obj.lastdata), 0, sizeof(MPUDATA));
	
}


int phy_limit(int n, int max)
{
	if (n > max)
		return max;
	if (n < -max)
		return -max;	
	return n;	
}







void phy_update(MPUDATA* d)
{
	
	
	PHYOBJ* p = &_obj;
	
	float dt = 1.0f / p->mpu->freq_hz;
	
	if (dt != 0)
	{	
		p->vx += -d->ax / dt; 
		p->vy += -d->ay / dt; 
	
		p->x += p->vx / dt;
		p->y += p->vy / dt;
	
	
		p->vx *= _obj.friction;
		p->vy *= _obj.friction;
	

		float r = sqrt(p->x*p->x + p->y* p->y);
		if (r > p->rmax)
		{
			float sina = p->y / r;
			p->y = sina * p->rmax;
			p->vy *= -1;
		
			float cosb = p->x / r;
			p->x = cosb * p->rmax;
			p->vx *= -1;
		}

		p->a += d->gz * dt * PHY_ROTATE_KOEF;
		p->a += p->va * dt;
		//p->va *= _obj.friction;
		if (p->a > 360)
			p->a -= 360;
		if (p->a < -360)
			p->a += 360;
	}

	
	p->dxyz =  fabs(d->ax - p->lastdata.ax) + fabs(d->ay - p->lastdata.ay) + fabs(d->az - p->lastdata.az);
	
	bool isdormant = p->dxyz < PHY_DORMANT_THRESHOLD; 
	if (isdormant) 
	{
		if (!p->isdormant)
		{
			if (!p->isdormantready)
			{
				p->isdormantready = true;
				p->dormanttimer = MILLIS;
				//printf("?");
			}
			else
			{
				if ((MILLIS - p->dormanttimer) > PHY_DORMANT_TIME_MS)
				{
					p->isdormant = true;
					//printf("+");
				}
			}
		}
	}
	else
	{	
		if (p->isdormantready)
		{
			//printf("-");
		}
		p->isdormant = false;
		p->isdormantready = false;
	}	
	
	
	_obj.isfaceup = d->az > 0;	
	
	float sum = (d->ax + d->ay + d->az + d->gx + d->gy + d->gz) * 1000 ;
	float fraction = sum - ((long)sum);
	p->random = (int)(fraction * 0xFF);
	
	
	memcpy(&(p->lastdata), d, sizeof(MPUDATA));
	
}

PHYOBJ* phy_get()
{
	return &_obj;
}
