/*
 * eeprom.c
 *
 *  Created on: Mar 13, 2021
 *      Author: woofer
 */
#include "eeprom.h"
#include "crc.h"
uint16_t devAddr = (0x50 << 1);
HAL_StatusTypeDef status;

HAL_StatusTypeDef writeEeprom(uint16_t memAddr, float *data,uint16_t len)
{
	uint32_t CRCVal = HAL_CRC_Calculate(&hcrc, (uint32_t*) data, len>>2);
	HAL_I2C_Mem_Write(&hi2c2, devAddr, memAddr, I2C_MEMADD_SIZE_8BIT,(uint8_t*) data, len, HAL_MAX_DELAY);
	HAL_Delay(50);
	HAL_I2C_Mem_Write(&hi2c2, devAddr,memAddr+len, I2C_MEMADD_SIZE_8BIT, (uint8_t*) &CRCVal, sizeof(CRCVal) , HAL_MAX_DELAY);
	HAL_Delay(50);
	return status;
}

HAL_StatusTypeDef readEeprom(uint16_t memAddr, float *data,uint16_t len)
{
	uint32_t CRCVal;
	float *dat=data;
	for (;;)
	{ // wait...
		status = HAL_I2C_IsDeviceReady(&hi2c2, devAddr, 1, HAL_MAX_DELAY);
		//if (status == HAL_OK)
			break;
	}

	HAL_I2C_Mem_Read(&hi2c2, devAddr, memAddr, I2C_MEMADD_SIZE_8BIT,(uint8_t*) data, len, HAL_MAX_DELAY);
	for (;;)
		{ // wait...
			status = HAL_I2C_IsDeviceReady(&hi2c2, devAddr, 1, HAL_MAX_DELAY);
			//if (status == HAL_OK)
				break;
		}

	HAL_I2C_Mem_Read(&hi2c2, devAddr, memAddr+len, I2C_MEMADD_SIZE_8BIT, (uint8_t*) &CRCVal, sizeof(CRCVal) , HAL_MAX_DELAY);
	uint32_t CRCValRead = HAL_CRC_Calculate(&hcrc, (uint32_t*) dat,len>>2);
 	if (CRCVal == CRCValRead)
	{
		return HAL_OK;
	}
	else
	{
		return HAL_ERROR;
	}
}
