/*
 * onewire.h
 *
 *  Version 1.0.1
 */

#ifndef ONEWIRE_H_
#define ONEWIRE_H_

// ��� ������ ����������� ����������� ��������� ������� OW_Init
// �� ������� ������������ ����� USART
#include "main.h"
#include "usart.h"
// ��������, �� ����� USART ��������� 1-wire




// ���� ����� �������� ���� FreeRTOS, �� �����������������
//#define OW_GIVE_TICK_RTOS

// ������ �������� ������� OW_Send
#define OW_SEND_RESET		1
#define OW_NO_RESET		2

// ������ �������� �������
#define OW_OK			1
#define OW_ERROR		2
#define OW_NO_DEVICE	3

#define OW_NO_READ		0xff

#define OW_READ_SLOT	0xff

uint8_t OW_Init();
uint8_t OW_Send(uint8_t sendReset, uint8_t *command, uint8_t cLen, uint8_t *data, uint8_t dLen, uint8_t readStart);
void OW_Measure();
int32_t OW_Read_Sensors(size_t n);
#endif /* ONEWIRE_H_ */
