/*
 * dsp.h
 *
 *  Created on: 3 июн. 2021 г.
 *      Author: woofer
 */

#ifndef INC_DSP_H_
#define INC_DSP_H_
#include "main.h"
float RCfilter(float input,float old,float factor);
float calibration(float code, float minCode, float minValue, float maxCode, float maxValue);
uint16_t rangeLimitInt(uint16_t value, uint16_t minValue, uint16_t maxValue);
#endif /* INC_DSP_H_ */
