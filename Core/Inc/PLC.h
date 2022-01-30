#ifndef PLC_H_
#define PLC_H_
#include "stdbool.h"
#include "stdint.h"
#define X_MAX	8
#define Y_MAX	8
#define M_MAX	50
#define T_MAX	10
#define D_MAX	10

typedef struct
{
	bool Curr:1;
	bool Old:1;
}M_Type;
typedef struct
{
	uint16_t Time;		
	uint16_t Time_Old;
    bool Curr:1;
    bool Old:1;
}T_Type;

void TMR(T_Type *timer,uint16_t time);
#endif