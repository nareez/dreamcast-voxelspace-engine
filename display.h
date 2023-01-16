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


       
       /**********************************************************************
		FSCA
***********************************************************************/
#define ANGLE_TO_RAD(r)		((r) / 10430.37835f)
#define RAD_TO_ANGLE(r)		((r) * 10430.37835f)

static inline void sh4FSCARadianF(float a, float *sine, float *cosine)
{
	register float __s __asm__("fr2");
	register float __c __asm__("fr3");
	a = RAD_TO_ANGLE(a);
	
         __asm__(    "ftrc	%2,fpul\n\t"
                "fsca	fpul,dr2\n\t"
		: "=f" (__s), "=f" (__c)
		: "f" (a)
		: "fpul");
	
        *sine = __s; *cosine = __c;
}
#define sh4FSCARadian(angle, sine, cosine) sh4FSCARadianF(angle,&sine,&cosine)
static inline float sh4FSCARadianSine(float angle) {
	float s, c;
	sh4FSCARadian(angle, s, c);
        return s;
}
static inline float sh4FSCARadianCosine(float angle) {
	float s, c;
	sh4FSCARadian(angle, s, c);
        return c;
}
static inline float sh4FSCARadianTangent(float angle) {
	float s, c;
	sh4FSCARadian(angle, s, c);
        return s / c;
}

static inline void sh4FSCAF(int angle, float *sine, float *cosine)
{
	register float __s __asm__("fr2");
	register float __c __asm__("fr3");
	
         __asm__(    "lds	%2,fpul\n\t"
                "fsca	fpul,dr2\n\t"
		: "=f" (__s), "=f" (__c)
		: "r" (angle)
		: "fpul");
	
        *sine = __s; *cosine = __c;
}
#define sh4FSCA(angle, sine, cosine) sh4FSCAF(angle, &sine, &cosine)

static inline float sh4FSCASine(int angle) {
	float s, c;
	sh4FSCA(angle, s, c);
        return s;
}
static inline float sh4FSCACosine(int angle) {
	float s, c;
	sh4FSCA(angle, s, c);
        return c;
}
static inline float sh4FSCATangent(int angle) {
	float s, c;
	sh4FSCA(angle, s, c);
        return s / c;
}


static inline float FSQRT( float n)
{
    register float __x __asm__("fr15") = n; 
	
	__asm__ __volatile__( 
		"fsqrt	fr15\n" 
		: "=f" (__x)
		: "0" (__x) );   
		
    return __x; 
}

//Variables
extern uint16_t* backbuffer;

//Functions
void dis_initializeDisplay(void);
void dis_initializeDoublebuffer(void);
void dis_flipBuffer(void);
void dis_clearBackBuffer(int r, int g, int b);

#endif
