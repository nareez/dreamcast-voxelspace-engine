#ifndef display_h
#define display_h

#include <stdlib.h>
#include <kos.h>

//MACROS
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 320
#define PACK_PIXEL(r, g, b) ( ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3) )
#define DRAW_PIXEL(x, y, color) \
	if((x >= 0) && (x < SCREEN_HEIGHT) && (y >= 0) && (y < SCREEN_WIDTH)) \
		backbuffer[(y * SCREEN_WIDTH) + x] = color;

//Variables
extern uint16_t* backbuffer;

//Functions
void dis_initializeDisplay(void);
void dis_initializeDoublebuffer(void);
void dis_flipBuffer(void);
void dis_clearBackBuffer(int r, int g, int b);

#endif