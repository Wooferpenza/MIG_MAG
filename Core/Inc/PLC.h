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
	bool state:1;
	bool oldState:1;
}M_Type;
typedef struct
{
	uint32_t time;		
	uint32_t timeBegin;
    bool curr:1;
    bool old:1;
	bool enableOld:1;
}T_Type;

bool LD(M_Type);
bool LDI(M_Type);
bool LDP(M_Type);
bool LDF(M_Type);
bool TMR(T_Type *timer,bool enable, uint16_t time);
M_Type OUT(bool);
#endif