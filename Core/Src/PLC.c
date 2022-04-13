#include "PLC.h"
#include "FreeRTOS.h"
bool LD(M_Type m)
{
	return m.state;
}
bool LDI(M_Type m)
{
	return !m.state;
}
bool LDP( M_Type m)
{
	return m.state&&!m.oldState;
}
bool LDF( M_Type m)
{
	return !m.state&&m.oldState;
}
bool TMR(T_Type *timer,bool enable,uint16_t time)
{
  if (enable)
  {
	  uint32_t curretTime=osKernelSysTick();
	  uint32_t deltaTime=0;
	  if (!timer->enableOld)
	  {
		timer->timeBegin=curretTime;
		timer->curr=false;
	  }
	  if (curretTime>=timer->timeBegin)
	  {
		  deltaTime=curretTime-timer->timeBegin;
	  }
	  else
	  {
		  deltaTime=4294967296+curretTime-timer->timeBegin;
	  }
	  if (deltaTime>=time)
	  {
		timer->curr=true;	  
	  }
  }
  else
  {
	  timer->curr=false;
  }
  timer->enableOld=enable;
  timer->old=timer->curr;
  return timer->curr;
}

//------------------------------------------------------------------
M_Type OUT(bool enable)
{
  M_Type output;
  output.state=enable;
  return output;
}