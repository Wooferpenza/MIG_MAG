#include "PLC.h"
void TMR(T_Type *timer,uint16_t time)
{
    uint16_t Time_T=0;
	uint16_t Time_ms_Old=timer->Time_Old;
	// if (Time_ms>=Time_ms_Old)
	// {
	// 	Time_T=Time_ms-Time_ms_Old;
	// }
	// else
	// {
	// 	Time_T=TIMER_MAX-Time_ms_Old+Time_ms;
	// }
	// if (Read())
	// {
	// 	T[Tm].Time+=Time_T;
	// 	u16 Set_Time;
	// 	Set_Time=Detect_D(VM_PC+1);
	// 	if (T[Tm].Time>=(Set_Time*100))
	// 	{
	// 		M[Tm+T_BEGIN].Curr=true;
	// 		T[Tm].Time-=Time_T;
	// 	}
	// }
	// else
	// {
	// 	M[Tm+T_BEGIN].Curr=false;
	// 	T[Tm].Time=0;
	// }
	timer->Time_Old=Time_ms;
}