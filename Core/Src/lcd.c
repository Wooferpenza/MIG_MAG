/*
 * lcd.c
 *
 *  Created on: 10/06/2018
 *      Author: Olivier Van den Eede
 */

#include "lcd.h"
#include "mystring.h"
const uint8_t ROW_16[] = {0x00, 0x40, 0x10, 0x50};
const uint8_t ROW_20[] = {0x00, 0x40, 0x14, 0x54};
/*
Очередь данных
*/
#define QUEUE_SIZE 50
typedef struct {
	uint8_t data;   // Данные
	uint8_t rs;	// Тип команда/данные
	uint16_t delay; // задержка
}lcdData_t;
typedef struct{
lcdData_t queue[QUEUE_SIZE]; // Кольцевой буфер
uint8_t queueBegin; // Нчало элементов буфера
uint8_t queueEnd; // Конец элементов буфера
uint8_t queueCount; // Число элементов буфера
} queue_t;

queue_t mQueue;
static void push( queue_t *queue, lcdData_t value);
static lcdData_t pop(queue_t *queue);
uint8_t isCreate=0;
const uint16_t en_width=99;
const uint16_t en_cicle=99;
/************************************** Static declarations **************************************/

static void lcd_write_data(Lcd_HandleTypeDef * lcd, uint8_t data);
static void lcd_write_command(Lcd_HandleTypeDef * lcd, uint8_t command,uint16_t delay );

/************************************** Function definitions **************************************/

/**
 * Create new Lcd_HandleTypeDef and initialize the Lcd
 */
Lcd_HandleTypeDef Lcd_create(
		Lcd_PortType port[], Lcd_PinType pin[],
		Lcd_PortType rs_port, Lcd_PinType rs_pin,
		Lcd_PortType en_port, Lcd_PinType en_pin, TIM_HandleTypeDef *tmr)
{

	Lcd_HandleTypeDef lcd;
	lcd.tmr=tmr;
	lcd.en_pin = en_pin;
	lcd.en_port = en_port;

	lcd.rs_pin = rs_pin;
	lcd.rs_port = rs_port;

	lcd.data_pin = pin;
	lcd.data_port = port;
	Lcd_init(&lcd);
	isCreate=1;
	return lcd;
}

/**
 * Initialize 16x2-lcd without cursor
 */
void Lcd_init(Lcd_HandleTypeDef * lcd)
{
	mQueue.queueBegin=mQueue.queueEnd=mQueue.queueCount=0;
	lcd_write_command(lcd, 0x33,99);
	lcd_write_command(lcd, 0x32,99);
	lcd_write_command(lcd, FUNCTION_SET | OPT_N,FUNCTION_SET_DELAY);				// 4-bit mode

	lcd_write_command(lcd, CLEAR_DISPLAY,CLEAR_DISPLAY_DELAY);						// Clear screen
	lcd_write_command(lcd, DISPLAY_ON_OFF_CONTROL | OPT_D,DISPLAY_ON_OFF_CONTROL_DELAY);		// Lcd-on, cursor-off, no-blink
	lcd_write_command(lcd, ENTRY_MODE_SET | OPT_INC, ENTRY_MODE_SET_DELAY);			// Increment cursor
}

/**
 * Write a number on the current position
 */
void Lcd_int16(Lcd_HandleTypeDef * lcd, int16_t number)
{
	char buffer[7];
	uint16ToString(number,buffer);
	Lcd_string(lcd, buffer);
}

 /* Write a fix point number on the current position
 */
void Lcd_float(Lcd_HandleTypeDef * lcd, float number, int denominator)
{
 char buffer[12];
 floatToString(number, buffer);
 buffer[denominator+1]=0;
 Lcd_string(lcd, buffer);
}

/**
 * Write a string on the current position
 */
void Lcd_string(Lcd_HandleTypeDef * lcd, const char * string)
{
	for(uint8_t i = 0; i < strlen(string); i++)
	{
			lcd_write_data(lcd, string[i]);
	}
}

/**
 * Set the cursor position
 */
void Lcd_cursor(Lcd_HandleTypeDef * lcd, uint8_t row, uint8_t col)
{
	#ifdef LCD20xN
	lcd_write_command(lcd, SET_DDRAM_ADDR + ROW_20[row] + col);
	#endif

	#ifdef LCD16xN
	lcd_write_command(lcd, SET_DDRAM_ADDR + ROW_16[row] + col,SET_DDRAM_ADDR_DELAY);
	#endif
}

/**
 * Clear the screen
 */
void Lcd_clear(Lcd_HandleTypeDef * lcd)
{
	lcd_write_command(lcd, CLEAR_DISPLAY,CLEAR_DISPLAY_DELAY);
}

void Lcd_define_char(Lcd_HandleTypeDef * lcd, uint8_t code, uint8_t bitmap[]){
	lcd_write_command(lcd, SETCGRAM_ADDR + (code << 3),SETCGRAM_ADDR_DELAY);
	for(uint8_t i=0;i<8;++i){
		lcd_write_data(lcd, bitmap[i]);
	}

}


/************************************** Static function definition **************************************/

/**
 * Write a byte to the command register
 */
void lcd_write_command(Lcd_HandleTypeDef * lcd, uint8_t command,uint16_t delay)
{
	HAL_TIM_Base_Stop_IT(lcd->tmr);
	lcdData_t value={command, LCD_COMMAND_REG,delay};
	push(&mQueue, value);
	if (isCreate)
	HAL_TIM_Base_Start_IT(lcd->tmr);
}

 /* Write a byte to the data register
 */
void lcd_write_data(Lcd_HandleTypeDef * lcd, uint8_t data)
{
	HAL_TIM_Base_Stop_IT(lcd->tmr);
	lcdData_t value={data,LCD_DATA_REG,99};
	push(&mQueue, value);
	if (isCreate)
	HAL_TIM_Base_Start_IT(lcd->tmr);
}

/**
 *
 *
 */
void Lcd_Timer_Callback (Lcd_HandleTypeDef *lcd)
{
  static uint8_t state = 0;
  static lcdData_t data;
  switch (state)
    {
    case 0:
      if (mQueue.queueCount == 0)
	{
	  HAL_TIM_Base_Stop_IT (lcd->tmr);
	  return;
	}
      data = pop (&mQueue);
      HAL_GPIO_WritePin (lcd->rs_port, lcd->rs_pin, data.rs);
      for (uint8_t i = 0; i < 4; i++)
	{
	  HAL_GPIO_WritePin (lcd->data_port[i], lcd->data_pin[i], (data.data >> (i+4)) & 0x01);
	}
      HAL_GPIO_WritePin (lcd->en_port, lcd->en_pin, 1);
      __HAL_TIM_SetAutoreload(lcd->tmr,en_width);
      state = 1;
      break;
    case 1:
      HAL_GPIO_WritePin (lcd->en_port, lcd->en_pin, 0);
      __HAL_TIM_SetAutoreload(lcd->tmr,en_cicle);
      state = 2;
      break;
    case 2:
      for (uint8_t i = 0; i < 4; i++)
	{
	  HAL_GPIO_WritePin (lcd->data_port[i], lcd->data_pin[i], (data.data >> i) & 0x01);
	}
      HAL_GPIO_WritePin (lcd->en_port, lcd->en_pin, 1);
      __HAL_TIM_SetAutoreload(lcd->tmr,en_width);
      state = 3;
      break;
    case 3:
      HAL_GPIO_WritePin (lcd->en_port, lcd->en_pin, 0);
      __HAL_TIM_SetAutoreload(lcd->tmr,data.delay);
      state = 0;
      break;
    }
  return;
}

static void push(queue_t *queue,lcdData_t value)
{
	if (queue->queueCount<QUEUE_SIZE)
	{
		queue->queue[queue->queueEnd]=value;
		queue->queueCount++;
		queue->queueEnd++;
		if (queue->queueEnd==QUEUE_SIZE)
		{
			queue->queueEnd=0;
		}
	}
}
static lcdData_t pop(queue_t *queue)
{
	lcdData_t value;
	if (queue->queueCount>0)
	{
		value=queue->queue[queue->queueBegin];
		queue->queueCount--;
		queue->queueBegin++;
		if (queue->queueBegin==QUEUE_SIZE)
		{
			queue->queueBegin=0;
		}
	}
	return value;
}
