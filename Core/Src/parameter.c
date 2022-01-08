/*
 * parameter.c
 *
 *  Created on: 13 мар. 2021 г.
 *      Author: woofer
 */
#include "parameter.h"
void add(parameter_t *parameter,float value)
{
  parameter->value+=value;
  if (parameter->value<parameter->min) {parameter->value=parameter->min;}
  if (parameter->value>parameter->max) {parameter->value=parameter->max;}
return;
}

void inc(parameter_t *parameter,float stepNumber,float stepFactor)
{
    parameter->value+=(parameter->step*stepNumber*stepFactor);
    if (parameter->value<parameter->min) {parameter->value=parameter->min;}
    if (parameter->value>parameter->max) {parameter->value=parameter->max;}
    return;
}
