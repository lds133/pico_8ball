#pragma once


typedef struct {
	int nlines;
	int direction;
	const char* line[4];
} FTCARD;


#define  UP 0
#define  DN 1
