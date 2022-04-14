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
  if (parameter->value<parameter->range->min) {parameter->value=parameter->range->min;}
  if (parameter->value>parameter->range->max) {parameter->value=parameter->range->max;}
return;
}

void inc(parameter_t *parameter,float stepNumber,float stepFactor)
{
    parameter->value+=(parameter->step*stepNumber*stepFactor);
    if (parameter->value<parameter->range->min) {parameter->value=parameter->range->min;}
    if (parameter->value>parameter->range->max) {parameter->value=parameter->range->max;}
    return;
}
