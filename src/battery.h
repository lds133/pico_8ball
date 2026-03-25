#pragma once




#define BATTERY_NOTREADY  -2
#define BATTERY_ERROR     -1

void bat_init();

void bat_reset();

int bat_persent();

int bat_mvolt();
