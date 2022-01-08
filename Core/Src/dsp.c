/*
 * dsp.c
 *
 *  Created on: 3 июн. 2021 г.
 *      Author: woofer
 */
#include "dsp.h"
float RCfilter(float input,float old,float factor)
{
	return input*(1.0-factor)+old*factor;
}

float calibration(float code, float minCode, float minValue, float maxCode, float maxValue)
{
	float k=(maxValue-minValue)/(maxCode-minCode);
	float b=maxValue-(maxCode*k);
	return k*code+b;
}

uint16_t rangeLimitInt(uint16_t value, uint16_t minValue, uint16_t maxValue)
{
	if (value<minValue) {return minValue;}
	if (value>maxValue) {return maxValue;}
	return value;
}
