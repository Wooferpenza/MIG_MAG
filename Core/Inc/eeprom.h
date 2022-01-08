/*
 * eeprom.h
 *
 *  Created on: Mar 13, 2021
 *      Author: woofer
 */

#ifndef INC_EEPROM_H_
#define INC_EEPROM_H_
#include "i2c.h"

HAL_StatusTypeDef writeEeprom(uint16_t memAddr,float *data, uint16_t len);
HAL_StatusTypeDef readEeprom(uint16_t memAddr,float *data, uint16_t len);

#endif /* INC_EEPROM_H_ */
