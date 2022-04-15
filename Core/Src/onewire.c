/*
 * onewire.c
 *
 *  Created on: 13.02.2012
 *      Author: di
 */

#include "onewire.h"

#define OW_DMA_CH_RX DMA1_Channel5
#define OW_DMA_CH_TX DMA1_Channel4
//#define OW_DMA_FLAG		DMA1_FLAG_TC5

// ����� ��� ������/�������� �� 1-wire
uint8_t ow_buf[8];

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
	if (HAL_HalfDuplex_Init(&huart1) != HAL_OK)
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
	if (HAL_HalfDuplex_Init(&huart1) != HAL_OK)
	{
		Error_Handler();
	}
	uint8_t chTX = 0xD0;
	uint8_t chRX = 0x0;
	HAL_UART_Transmit(&huart1, &chTX, sizeof(chTX), 10);
	HAL_UART_Receive(&huart1, &chRX, sizeof(chRX), 10);

	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_HalfDuplex_Init(&huart1) != HAL_OK)
	{
		Error_Handler();
	}

	if ((chRX != chTX)||1)
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

		DMA_InitTypeDef DMA_InitStructure;

		// DMA �� ������
		// DMA_DeInit(OW_DMA_CH_RX);
		// DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (USART2->DR);
		// DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ow_buf;
		// DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
		// DMA_InitStructure.DMA_BufferSize = 8;
		// DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		// DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		// DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		// DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		// DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
		// DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
		// DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
		// DMA_Init(OW_DMA_CH_RX, &DMA_InitStructure);

		// DMA �� ������
		// DMA_DeInit(OW_DMA_CH_TX);
		// DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (USART2->DR);
		// DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ow_buf;
		// DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
		// DMA_InitStructure.DMA_BufferSize = 8;
		// DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		// DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		// DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		// DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		// DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
		// DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
		// DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
		// DMA_Init(OW_DMA_CH_TX, &DMA_InitStructure);

		// ����� ����� ��������
		// USART_ClearFlag(OW_USART, USART_FLAG_RXNE | USART_FLAG_TC | USART_FLAG_TXE);
		// USART_DMACmd(OW_USART, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);
		// DMA_Cmd(OW_DMA_CH_RX, ENABLE);
		// DMA_Cmd(OW_DMA_CH_TX, ENABLE);

		// ����, ���� �� ������ 8 ����
// 		while (DMA_GetFlagStatus(OW_DMA_FLAG) == RESET)
// 		{
// #ifdef OW_GIVE_TICK_RTOS
// 			taskYIELD();
// #endif
// 		}

		// ��������� DMA
		// DMA_Cmd(OW_DMA_CH_TX, DISABLE);
		// DMA_Cmd(OW_DMA_CH_RX, DISABLE);
		// USART_DMACmd(OW_USART, USART_DMAReq_Tx | USART_DMAReq_Rx, DISABLE);

		// ���� ����������� ������ ����-�� ����� - ������� �� � �����
		HAL_UART_Transmit(&huart1,ow_buf,sizeof(ow_buf),1);
	//	HAL_UART_Receive(&huart1,ow_buf,sizeof(ow_buf),100);
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
