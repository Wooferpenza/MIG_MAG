/*
 * parameter.h
 *
 *  Created on: 13 мар. 2021 г.
 *      Author: woofer
 */

#ifndef INC_PARAMETER_H_
#define INC_PARAMETER_H_
typedef struct
{
  float min;
  float max;
} range_t;

typedef struct
{
  float value;
  const range_t *range;
  float step;
  const char *name;
  const char *unit;
} parameter_t;

void add(parameter_t *,float);
void inc(parameter_t *,float,float);
#endif /* INC_PARAMETER_H_ */
