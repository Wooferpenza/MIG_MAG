/*
 * onewire.c
 *
 *  Created on: 13.02.2012
 *      Author: di
 */

#include "onewire.h"
#include "FreeRTOS.h"
#include "task.h"
#define OW_DMA_CH_RX DMA1_Channel5
#define OW_DMA_CH_TX DMA1_Channel4
//#define OW_DMA_FLAG		DMA1_FLAG_TC5

// ����� ��� ������/�������� �� 1-wire
uint8_t ow_buf[8];
const uint8_t ow_sensorsID[3][12]={
	{"\x55\x28\x61\x64\x12\x33\xA7\x60\x04\xbe\xff\xff"},
	{"\x55\x28\x61\x64\x12\x33\xAD\x21\xFB\xbe\xff\xff"},
	{"\x55\x28\x61\x64\x12\x33\x93\x27\xD7\xbe\xff\xff"}
};

#define OW_0 0x00
#define OW_1 0xff
#define OW_R_1 0xff

//-----------------------------------------------------------------------------
// ������� ����������� ���� ���� � ������, ��� �������� ����� USART
// ow_byte - ����, ������� ���� �������������
// ow_bits - ������ �� �����, �������� �� ����� 8 ����
//-----------------------------------------------------------------------------
void OW_toBits(uint8_t ow_byte, uint8_t *ow_bits)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		if (ow_byte & 0x01)
		{
			*ow_bits = OW_1;
		}
		else
		{
			*ow_bits = OW_0;
		}
		ow_bits++;
		ow_byte = ow_byte >> 1;
	}
}

//-----------------------------------------------------------------------------
// �������� �������������� - �� ����, ��� �������� ����� USART ����� ���������� ����
// ow_bits - ������ �� �����, �������� �� ����� 8 ����
//-----------------------------------------------------------------------------
uint8_t OW_toByte(uint8_t *ow_bits)
{
	uint8_t ow_byte, i;
	ow_byte = 0;
	for (i = 0; i < 8; i++)
	{
		ow_byte = ow_byte >> 1;
		if (*ow_bits == OW_R_1)
		{
			ow_byte |= 0x80;
		}
		ow_bits++;
	}

	return ow_byte;
}

//-----------------------------------------------------------------------------
// �������������� USART � DMA
//-----------------------------------------------------------------------------
uint8_t OW_Init()
{
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
		Error_Handler();
	}
	return OW_OK;
}

//-----------------------------------------------------------------------------
// ������������ ����� � �������� �� ������� ��������� �� ����
//-----------------------------------------------------------------------------
uint8_t OW_Reset()
{
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 9600;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
		Error_Handler();
	}
	uint8_t chTX = 0xF0;
	uint8_t chRX = 0x0;
	huart1.Instance->DR = chTX;
	HAL_Delay(1);
	chRX=huart1.Instance->DR;
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
		Error_Handler();
	}

	if ((chRX != chTX))
	{
		return OW_OK;
	}

	return OW_NO_DEVICE;
}

//-----------------------------------------------------------------------------
// ��������� ������� � ����� 1-wire
// sendReset - �������� RESET � ������ �������.
// 		OW_SEND_RESET ��� OW_NO_RESET
// command - ������ ����, ���������� � ����. ���� ����� ������ - ���������� OW_READ_SLOTH
// cLen - ����� ������ ������, ������� ���� ��������� � ����
// data - ���� ��������� ������, �� ������ �� ����� ��� ������
// dLen - ����� ������ ��� ������. ����������� �� ����� ���� �����
// readStart - � ������ ������� �������� �������� ������ (���������� � 0)
//		����� ������� OW_NO_READ, ����� ����� �� �������� data � dLen
//-----------------------------------------------------------------------------
uint8_t OW_Send(uint8_t sendReset, uint8_t *command, uint8_t cLen,
				uint8_t *data, uint8_t dLen, uint8_t readStart)
{

	// ���� ��������� ����� - ���������� � ��������� �� ������� ���������
	if (sendReset == OW_SEND_RESET)
	{
		if (OW_Reset() == OW_NO_DEVICE)
		{
			return OW_NO_DEVICE;
		}
	}

	while (cLen > 0)
	{

		OW_toBits(*command, ow_buf);
		command++;
		cLen--;

		// ���� ����������� ������ ����-�� ����� - ������� �� � �����
		HAL_UART_Receive_DMA(&huart1,ow_buf,sizeof(ow_buf));
		HAL_UART_Transmit_DMA(&huart1, ow_buf, sizeof(ow_buf));
		while (HAL_UART_GetState(&huart1) != HAL_UART_STATE_READY)
		{
			// #ifdef OW_GIVE_TICK_RTOS
			 			taskYIELD();
			// #endif
		}
		
			if (readStart == 0 && dLen > 0)
			{
				*data = OW_toByte(ow_buf);
				data++;
				dLen--;
			}
			else
			{
				if (readStart != OW_NO_READ)
				{
					readStart--;
				}
			}
	}

	return OW_OK;
}

void OW_Measure()
{
	OW_Send(OW_SEND_RESET, "\xcc\x44",2,NULL,0,OW_NO_READ);
}
int32_t OW_Read_Sensors(size_t n)
{
	uint8_t buf[2];
	int32_t value=0;
	OW_Send(OW_SEND_RESET, ow_sensorsID[n], 12, buf, 2, 10);
	value=(buf[1]<<8)+buf[0];
	return value/16;
}