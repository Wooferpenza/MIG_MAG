/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "MicroMenu.h"
#include "stdbool.h"
#include "math.h"
#include "tim.h"
#include "adc.h"
#include "dma.h"
#include "parameter.h"
#include "eeprom.h"
#include "dsp.h"
#include "PLC.h"
#include "onewire.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ON GPIO_PIN_SET
#define OFF GPIO_PIN_RESET

#define WELDING_RUN(state) HAL_GPIO_WritePin(OUT_RUN_GPIO_Port, OUT_RUN_Pin, !state)
#define IS_WELDING_RUN !HAL_GPIO_ReadPin(OUT_RUN_GPIO_Port, OUT_RUN_Pin)

#define GAS_RUN(state) HAL_GPIO_WritePin(OUT_GAS_GPIO_Port, OUT_GAS_Pin, (GPIO_PinState)state)
#define IS_GAS_RUN HAL_GPIO_ReadPin(OUT_GAS_GPIO_Port, OUT_GAS_Pin)

#define WIRE_RUN(state) HAL_GPIO_WritePin(OUT_Wire_Run_GPIO_Port, OUT_Wire_Run_Pin, state)
#define IS_WIRE_RUN HAL_GPIO_ReadPin(OUT_Wire_Run_GPIO_Port, OUT_Wire_Run_Pin)

#define WIRE_DIR(state) HAL_GPIO_WritePin(OUT_Wire_Dir_GPIO_Port, OUT_Wire_Dir_Pin, state)
#define IS_WIRE_DIR HAL_GPIO_ReadPin(OUT_Wire_Dir_GPIO_Port, OUT_Wire_Dir_Pin)
#define WIREDOWN 0
#define WIREUP 1
#define GASTEST 2
#define MENU 3
#define OK 4
#define START 5
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
void valueEditRun(void);
GPIO_PinState key[6]; // состояние кнопок
M_Type keyPLC[6];
M_Type keyMenu[6];
volatile uint16_t adc[4];
volatile uint16_t adc0Filter = 0;
volatile uint16_t adc1Filter = 0;
volatile uint16_t adc2Filter = 0;
volatile uint16_t adc3Filter = 0;
int32_t temperature=0;
Lcd_PortType ports[] = {LCD_D4_GPIO_Port, LCD_D5_GPIO_Port, LCD_D6_GPIO_Port, LCD_D7_GPIO_Port};
Lcd_PinType pins[] = {LCD_D4_Pin, LCD_D5_Pin, LCD_D6_Pin, LCD_D7_Pin};
Lcd_HandleTypeDef lcd;
// Диапазоны параметров
const range_t Urange = {0.0, 30.0};
const range_t Irange = {0.0, 215.0};
const range_t Vrange = {1.0, 10.0};
const range_t Trange = {0.0, 10.0};
const range_t SetCoderange = {0.0, 3300};
const range_t PresCoderange = {0.0, 4095};
//Параметры сварки
parameter_t U_Set = {15.0, &Urange, 0.1, "U", "V"};				   // Уставка напряжения
parameter_t I_Set = {215.0, &Irange, 1.0, "I", "A"};			   // Уставка тока
parameter_t V_Set = {2.5, &Vrange, 0.1, "V", "mm/min"};			   // Уставка скорости подачи проволоки
parameter_t Wire_On = {0.5, &Trange, 0.1, "Wire on", "s"};		   // Задержка подачи проволоки
parameter_t Welding_Off = {0.5, &Trange, 0.1, "Welding off", "s"}; // Задержка отключения источника
parameter_t Gas_Before = {1.0, &Trange, 0.1, "Gas before", "s"};   // Газ до сварки
parameter_t Gas_After = {1.0, &Trange, 0.1, "Gas after", "s"};	   // Газ после сварки
parameter_t V_Manual = {1.0, &Vrange, 0.1, "V", "mm/min"};		   // Проволока вручную
parameter_t V_Manual_Marsh = {1.0, &Vrange, 0.1, "V", "mm/min"};
// Измеренные значения
float U_Present=0;
float I_Present=0;
// Калибровочные коэффициенты
parameter_t U_Set_Code_Min = {755.0, &SetCoderange, 1.0, "USetCodeMin", ""};
parameter_t U_Set_Value_Min = {15.0, &Urange, 0.1, "USetValueMin", "V"};
parameter_t U_Set_Code_Max = {1840.0, &SetCoderange, 1.0, "USetCodeMax", ""};
parameter_t U_Set_Value_Max = {30.0, &Urange, 0.1, "USetValueMax", "V"};

parameter_t I_Set_Code_Min = {1850.0, &SetCoderange, 1.0, "ISetCodeMin", ""};
parameter_t I_Set_Value_Min = {50.0, &Irange, 1.0, "ISetValueMin", "A"};
parameter_t I_Set_Code_Max = {0.0, &SetCoderange, 1.0, "ISetCodeMax", ""};
parameter_t I_Set_Value_Max = {215.0, &Irange, 1.0, "ISetValueMax", "A"};

parameter_t V_Set_Code_Min = {800.0, &SetCoderange, 1.0, "VSetCodeMin", ""};
parameter_t V_Set_Value_Min = {2.2, &Vrange, 0.1, "VSetValueMin", "mm/min"};
parameter_t V_Set_Code_Max = {3300.0, &SetCoderange, 1.0, "VSetCodeMax", ""};
parameter_t V_Set_Value_Max = {8.7, &Vrange, 0.1, "VSetValueMax", "mm/min"};

parameter_t U_Present_Code_Min = {0.0, &PresCoderange, 1.0, "UPresentCodeMin", ""};
parameter_t U_Present_Value_Min = {0.0, &Urange, 0.1, "UPresentValMin", "V"};
parameter_t U_Present_Code_Max = {0.0, &PresCoderange, 1.0, "UPresentCodeMax", ""};
parameter_t U_Present_Value_Max = {0.0, &Urange, 0.1, "UPresentValMax", "V"};

parameter_t I_Present_Code_Min = {0.0, &PresCoderange, 1.0, "IPresentCodeMin", ""};
parameter_t I_Present_Value_Min = {0.0, &Irange, 1.0, "IPresentValMin", "A"};
parameter_t I_Present_Code_Max = {0.0, &PresCoderange, 1.0, "IPresentCodeMax", ""};
parameter_t I_Present_Value_Max = {0.0, &Irange, 1.0, "IPresentValMax", "A"};

parameter_t *pVar = &U_Set;
parameter_t *pEditValue;
//Параметры для сохранения
float *const Parameters[6][4] = {
	{&Gas_Before.value, &Gas_After.value, &Wire_On.value, &Welding_Off.value},
	{&U_Set_Code_Min.value, &U_Set_Value_Min.value, &U_Set_Code_Max.value, &U_Set_Value_Max.value},
	{&I_Set_Code_Min.value, &I_Set_Value_Min.value, &I_Set_Code_Max.value, &I_Set_Value_Max.value},
	{&V_Set_Code_Min.value, &V_Set_Value_Min.value, &V_Set_Code_Max.value, &V_Set_Value_Max.value},
	{&U_Present_Code_Min.value, &U_Present_Value_Min.value, &U_Present_Code_Max.value, &U_Present_Value_Max.value},
	{&I_Present_Code_Min.value, &I_Present_Value_Min.value, &I_Present_Code_Max.value, &I_Present_Value_Max.value}};

//----------MENU------------------
//        Name,		Next,		Previous,	Parent,		Child,		SelectFunc,	EnterFunc,		Text
//		  �?мя,		Следующ,	Предыдущ,	Верхн,		Вложен,		Функц выбор,
MENU_ITEM(Menu_1, Menu_2, NULL_MENU, NULL_MENU, Menu_1_1, NULL, NULL, NULL, "Wire");

MENU_ITEM(Menu_1_1, Menu_1_2, Menu_1_4, Menu_1, NULL_MENU, NULL, valueEditRun, &Wire_On, "Wire on time");
MENU_ITEM(Menu_1_2, Menu_1_3, Menu_1_1, Menu_1, NULL_MENU, NULL, valueEditRun, &Welding_Off, "Welding off time");
MENU_ITEM(Menu_1_3, Menu_1_4, Menu_1_2, Menu_1, NULL_MENU, NULL, valueEditRun, &V_Manual, "Manual speed");
MENU_ITEM(Menu_1_4, Menu_1_1, Menu_1_3, Menu_1, NULL_MENU, NULL, valueEditRun, &V_Manual_Marsh, "Marsh speed");

MENU_ITEM(Menu_2, Menu_3, Menu_1, NULL_MENU, Menu_2_1, NULL, NULL, NULL, "Gas");

MENU_ITEM(Menu_2_1, Menu_2_2, NULL_MENU, Menu_2, NULL_MENU, NULL, valueEditRun, &Gas_Before, "Before");
MENU_ITEM(Menu_2_2, NULL_MENU, Menu_2_1, Menu_2, NULL_MENU, NULL, valueEditRun, &Gas_After, "After");

MENU_ITEM(Menu_3, NULL_MENU, Menu_2, NULL_MENU, Menu_3_1, NULL, NULL, NULL, "Calibration");

MENU_ITEM(Menu_3_1, Menu_3_2, NULL_MENU, Menu_3, NULL_MENU, NULL, valueEditRun, &U_Set_Code_Min, "USetCodeMin");
MENU_ITEM(Menu_3_2, Menu_3_3, Menu_3_1, Menu_3, NULL_MENU, NULL, valueEditRun, &U_Set_Value_Min, "USetValueMin");
MENU_ITEM(Menu_3_3, Menu_3_4, Menu_3_2, Menu_3, NULL_MENU, NULL, valueEditRun, &U_Set_Code_Max, "USetCodeMax");
MENU_ITEM(Menu_3_4, Menu_3_5, Menu_3_3, Menu_3, NULL_MENU, NULL, valueEditRun, &U_Set_Value_Max, "USetValueMax");
MENU_ITEM(Menu_3_5, Menu_3_6, Menu_3_4, Menu_3, NULL_MENU, NULL, valueEditRun, &I_Set_Code_Min, "ISetCodeMin");
MENU_ITEM(Menu_3_6, Menu_3_7, Menu_3_5, Menu_3, NULL_MENU, NULL, valueEditRun, &I_Set_Value_Min, "ISetValueMin");
MENU_ITEM(Menu_3_7, Menu_3_8, Menu_3_6, Menu_3, NULL_MENU, NULL, valueEditRun, &I_Set_Code_Max, "ISetCodeMax");
MENU_ITEM(Menu_3_8, Menu_3_9, Menu_3_7, Menu_3, NULL_MENU, NULL, valueEditRun, &I_Set_Value_Max, "ISetValueMax");
MENU_ITEM(Menu_3_9, Menu_3_10, Menu_3_8, Menu_3, NULL_MENU, NULL, valueEditRun, &V_Set_Code_Min, "VSetCodeMin");
MENU_ITEM(Menu_3_10, Menu_3_11, Menu_3_9, Menu_3, NULL_MENU, NULL, valueEditRun, &V_Set_Value_Min, "VSetValueMin");
MENU_ITEM(Menu_3_11, Menu_3_12, Menu_3_10, Menu_3, NULL_MENU, NULL, valueEditRun, &V_Set_Code_Max, "VSetCodeMax");
MENU_ITEM(Menu_3_12, Menu_3_13, Menu_3_11, Menu_3, NULL_MENU, NULL, valueEditRun, &V_Set_Value_Max, "VSetValueMax");
MENU_ITEM(Menu_3_13, Menu_3_14, Menu_3_12, Menu_3, NULL_MENU, NULL, valueEditRun, &U_Present_Code_Min, "UPresentCodeMin");
MENU_ITEM(Menu_3_14, Menu_3_15, Menu_3_13, Menu_3, NULL_MENU, NULL, valueEditRun, &U_Present_Value_Min, "UPresentValueMin");
MENU_ITEM(Menu_3_15, Menu_3_16, Menu_3_14, Menu_3, NULL_MENU, NULL, valueEditRun, &U_Present_Code_Max, "UPresentCodeMax");
MENU_ITEM(Menu_3_16, Menu_3_17, Menu_3_15, Menu_3, NULL_MENU, NULL, valueEditRun, &U_Present_Value_Max, "UPresentValueMax");
MENU_ITEM(Menu_3_17, Menu_3_18, Menu_3_16, Menu_3, NULL_MENU, NULL, valueEditRun, &I_Present_Code_Min, "IPresentCodeMin");
MENU_ITEM(Menu_3_18, Menu_3_19, Menu_3_17, Menu_3, NULL_MENU, NULL, valueEditRun, &I_Present_Value_Min, "IPresentValueMin");
MENU_ITEM(Menu_3_19, Menu_3_20, Menu_3_18, Menu_3, NULL_MENU, NULL, valueEditRun, &I_Present_Code_Max, "IPresentCodeMax");
MENU_ITEM(Menu_3_20, NULL_MENU, Menu_3_19, Menu_3, NULL_MENU, NULL, valueEditRun, &I_Present_Value_Max, "IPresentValueMax");
//------------END MENU---------------
// extern float Temp[MAXDEVICES_ON_THE_BUS];
/* USER CODE END Variables */
osThreadId displayTaskHandle;
uint32_t displayTaskBuffer[ 128 ];
osStaticThreadDef_t displayTaskControlBlock;
osThreadId controlTaskHandle;
uint32_t controTaskBuffer[ 128 ];
osStaticThreadDef_t controTaskControlBlock;
osThreadId keyScanTaskHandle;
uint32_t keyScanTaskBuffer[ 64 ];
osStaticThreadDef_t keyScanTaskControlBlock;
osThreadId menuControlHandle;
uint32_t menuControlBuffer[ 128 ];
osStaticThreadDef_t menuControlControlBlock;
osThreadId editValueHandle;
uint32_t editValueBuffer[ 64 ];
osStaticThreadDef_t editValueControlBlock;
osMutexId uSetMutexHandle;
osStaticMutexDef_t uSetMutexControlBlock;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
int32_t encGetCount(TIM_HandleTypeDef *tmr);
void saveSettings(void);
void loadSettings(void);
void menuDisplayUpdate(void);
/* USER CODE END FunctionPrototypes */

void StartDisplayTask(void const * argument);
void StartControlTask(void const * argument);
void StartKeyScanTask(void const * argument);
void StartMenuControl(void const * argument);
void StartEditValue(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
	*ppxIdleTaskStackBuffer = &xIdleStack[0];
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
	/* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	HAL_Delay(50);
	lcd = Lcd_create(ports, pins, LCD_RS_GPIO_Port, LCD_RS_Pin, LCD_E_GPIO_Port, LCD_E_Pin, &htim4);
	HAL_ADCEx_Calibration_Start(&hadc1);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&adc, 4);
	HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_4);
	HAL_TIM_Base_Start_IT(&htim4);
	HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_1);
	HAL_Delay(10);
   
  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* definition and creation of uSetMutex */
  osMutexStaticDef(uSetMutex, &uSetMutexControlBlock);
  uSetMutexHandle = osMutexCreate(osMutex(uSetMutex));

  /* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of displayTask */
  osThreadStaticDef(displayTask, StartDisplayTask, osPriorityNormal, 0, 128, displayTaskBuffer, &displayTaskControlBlock);
  displayTaskHandle = osThreadCreate(osThread(displayTask), NULL);

  /* definition and creation of controlTask */
  osThreadStaticDef(controlTask, StartControlTask, osPriorityNormal, 0, 128, controTaskBuffer, &controTaskControlBlock);
  controlTaskHandle = osThreadCreate(osThread(controlTask), NULL);

  /* definition and creation of keyScanTask */
  osThreadStaticDef(keyScanTask, StartKeyScanTask, osPriorityIdle, 0, 64, keyScanTaskBuffer, &keyScanTaskControlBlock);
  keyScanTaskHandle = osThreadCreate(osThread(keyScanTask), NULL);

  /* definition and creation of menuControl */
  osThreadStaticDef(menuControl, StartMenuControl, osPriorityIdle, 0, 128, menuControlBuffer, &menuControlControlBlock);
  menuControlHandle = osThreadCreate(osThread(menuControl), NULL);

  /* definition and creation of editValue */
  osThreadStaticDef(editValue, StartEditValue, osPriorityNormal, 0, 64, editValueBuffer, &editValueControlBlock);
  editValueHandle = osThreadCreate(osThread(editValue), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	osThreadSuspend(displayTaskHandle);
//	osThreadSuspend(menuControlHandle);
	osThreadSuspend(editValueHandle);
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDisplayTask */
/**
 * @brief  Function implementing the displayTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDisplayTask */
void StartDisplayTask(void const * argument)
{
  /* USER CODE BEGIN StartDisplayTask */
	/* Infinite loop */
	float uSet = 0;
	float vSet = 0;
	float iSet = 0;
	float uPresentDisplay = 0;
	float iPresentDisplay = 0;
	for (;;)
	{
		
		osMutexWait(uSetMutexHandle, osWaitForever);
		uSet = U_Set.value;
		vSet = V_Set.value;
		iSet = I_Set.value;
		if (IS_WIRE_RUN && IS_WELDING_RUN)
		{
			uPresentDisplay = U_Present;
			iPresentDisplay = I_Present;
		}
		osMutexRelease(uSetMutexHandle);

		Lcd_clear(&lcd);
		Lcd_cursor(&lcd, 0, 1);
		Lcd_string(&lcd, "U");
		Lcd_float(&lcd, uSet, 1);
		Lcd_cursor(&lcd, 0, 8);
		if (pVar == &I_Set)
		{
			Lcd_string(&lcd, "I");
			Lcd_float(&lcd,temperature,0);
			//Lcd_float(&lcd, iSet, 0);
		}
		else
		{
			Lcd_string(&lcd, "V");
			Lcd_float(&lcd, vSet, 1);
		}

		Lcd_cursor(&lcd, 1, 2);
		Lcd_float(&lcd, uPresentDisplay, 1);

		Lcd_cursor(&lcd, 1, 9);
		Lcd_float(&lcd, iPresentDisplay, 0);

		if (pVar == &U_Set)
		{
			Lcd_cursor(&lcd, 0, 0);
		}
		else
		{
			Lcd_cursor(&lcd, 0, 7);
		}
		Lcd_string(&lcd, ">");

		Lcd_cursor(&lcd, 0, 13);
		if (IS_GAS_RUN)
		{
			Lcd_string(&lcd, "G");
		}
		else
		{
			Lcd_string(&lcd, " ");
		}
		Lcd_cursor(&lcd, 0, 15);
		if (IS_WELDING_RUN)
		{
			Lcd_string(&lcd, "A");
		}
		else
		{
			Lcd_string(&lcd, " ");
		}
		Lcd_cursor(&lcd, 1, 13);
		if (IS_WIRE_RUN)
		{
			Lcd_string(&lcd, "W");
			if (IS_WIRE_DIR)
			{
				Lcd_string(&lcd, "U");
			}
			else
			{
				Lcd_string(&lcd, "D");
			}
		}
		else
		{
			Lcd_string(&lcd, "  ");
		}

		osDelay(100);
	}
  /* USER CODE END StartDisplayTask */
}

/* USER CODE BEGIN Header_StartControlTask */
/**
 * @brief Function implementing the controlTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartControlTask */
void StartControlTask(void const * argument)
{
  /* USER CODE BEGIN StartControlTask */

	HAL_GPIO_WritePin(OUT_LCD_LED_GPIO_Port, OUT_LCD_LED_Pin, GPIO_PIN_SET);
	Lcd_cursor(&lcd, 0, 5);
	Lcd_string(&lcd, "LOAD");
	loadSettings();
	osDelay(400);
	Lcd_clear(&lcd);

	osThreadResume(displayTaskHandle);
	uint16_t state=0;
	M_Type cicle, gasCicle, weldingCicle, wireCicle, menuMode, menuModefront, editMode;
	T_Type gasBefore_T, gasAfter_T, wireOn_T, weldingOff_T;
	/* Infinite loop */
	for (;;)
	{
		for (size_t i = 0; i < 6; i++)
		{
			keyPLC[i].state = key[i];
		}
		if (LDI(menuMode) && LDI(editMode))
		{
			if (LDP(keyPLC[OK]))
			{
				if (pVar == &U_Set)
					pVar = &V_Set;
				else if (pVar == &V_Set)
					pVar = &I_Set;
				else if (pVar == &I_Set)
					pVar = &U_Set;
			}
			int32_t encoderCount = encGetCount(&htim2);
			float uPresent = calibration((float)adc2Filter, U_Present_Code_Min.value, U_Present_Value_Min.value, U_Present_Code_Max.value, U_Present_Value_Max.value);
			float iPresent = calibration((float)adc3Filter, I_Present_Code_Min.value, I_Present_Value_Min.value, I_Present_Code_Max.value, I_Present_Value_Max.value);
			osMutexWait(uSetMutexHandle, osWaitForever);
			inc(pVar, encoderCount, 1.0);
			U_Present = RCfilter(uPresent, U_Present, 0.95);
			I_Present = RCfilter(iPresent, I_Present, 0.95);
			osMutexRelease(uSetMutexHandle);

			TMR(&gasAfter_T, LDI(weldingCicle), Gas_After.value * 1000.0);
			gasCicle = OUT((LD(keyPLC[START]) || LD(gasCicle)) && (LD(keyPLC[START]) || !gasAfter_T.curr));
			TMR(&gasBefore_T, LD(keyPLC[START]) || LD(weldingCicle), Gas_Before.value * 1000.0);
			weldingCicle = OUT(gasBefore_T.curr && !weldingOff_T.curr);
			TMR(&wireOn_T, LD(keyPLC[START]) && LD(weldingCicle), Wire_On.value * 1000);
			wireCicle = OUT(wireOn_T.curr);
			TMR(&weldingOff_T, LDI(keyPLC[START]) && LDI(wireCicle), Welding_Off.value * 1000.0);
			cicle = OUT(LD(gasCicle));

			GAS_RUN((LD(keyPLC[GASTEST]) && LDI(cicle)) || LD(gasCicle));
			WIRE_RUN(((LD(keyPLC[WIREDOWN]) || LD(keyPLC[WIREUP])) && LDI(cicle)) || LD(wireCicle));
			WIRE_DIR((LD(keyPLC[WIREDOWN]) && LDI(cicle)) || LD(wireCicle));
			WELDING_RUN(LD(weldingCicle));
			
			float uPresent1 = calibration((float)adc2Filter, U_Present_Code_Min.value, U_Present_Value_Min.value, U_Present_Code_Max.value, U_Present_Value_Max.value);
			U_Present = RCfilter(uPresent1, U_Present, 0.95);
			float iPresent1 = calibration((float)adc3Filter, I_Present_Code_Min.value, I_Present_Value_Min.value, I_Present_Code_Max.value, I_Present_Value_Max.value);
			I_Present = RCfilter(iPresent1, I_Present, 0.95);

			uint16_t USetCode = (uint16_t)calibration(U_Set.value, U_Set_Value_Min.value, U_Set_Code_Min.value, U_Set_Value_Max.value, U_Set_Code_Max.value);
			USetCode = rangeLimitInt(USetCode, SetCoderange.min, SetCoderange.max);
		//	if (state==0)
		//	{
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, USetCode);
		//	state=1;
		//	}
		//	else 
		//	{
		//		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 0);
		//		state=0;
		//	}
			uint16_t ISetCode = (uint16_t)calibration(I_Set.value, I_Set_Value_Min.value, I_Set_Code_Min.value, I_Set_Value_Max.value, I_Set_Code_Max.value);
			ISetCode = rangeLimitInt(ISetCode, SetCoderange.min, SetCoderange.max);
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, ISetCode);

			uint16_t VSetCode = (uint16_t)calibration(V_Set.value, V_Set_Value_Min.value, V_Set_Code_Min.value, V_Set_Value_Max.value, V_Set_Code_Max.value);
			VSetCode = rangeLimitInt(VSetCode, SetCoderange.min, SetCoderange.max);

			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, VSetCode);

			if (LDP(keyPLC[MENU]))
			{
				menuMode.state = true;
				Menu_Navigate(&Menu_1);
				osThreadSuspend(displayTaskHandle);
			}
		}
		menuModefront.state = (LD(menuMode) && LDI(editMode));
		if (LD(menuMode) && LDI(editMode))
		{

			if (LDP(menuModefront))
				menuDisplayUpdate();
			int32_t encoderCount = encGetCount(&htim2);
			if (encoderCount > 0 && (Menu_GetCurrentMenu()->Next != &NULL_MENU))
			{
				Menu_Navigate(MENU_NEXT);
				menuDisplayUpdate();
			}
			if (encoderCount < 0 && (Menu_GetCurrentMenu()->Previous != &NULL_MENU))
			{
				Menu_Navigate(MENU_PREVIOUS);
				menuDisplayUpdate();
			}
			if (LDP(keyPLC[OK]))
			{
				if ((Menu_GetCurrentMenu()->Child != &NULL_MENU))
				{
					Menu_Navigate(MENU_CHILD);
					menuDisplayUpdate();
				}
				else
				{
					Menu_EnterCurrentItem();
					menuMode.state = false;
					editMode.state = true;
				}
			}
			if (!LDP(menuMode) && LDP(keyPLC[MENU]))
			{
				if (Menu_GetCurrentMenu()->Parent != &NULL_MENU)
				{
					Menu_Navigate(MENU_PARENT);
					menuDisplayUpdate();
				}
				else
				{
					Lcd_clear(&lcd);
					Lcd_cursor(&lcd, 0, 5);
					Lcd_string(&lcd, "SAVE");
					saveSettings();
					osDelay(400);
					Lcd_clear(&lcd);
					menuMode.state = false;
					osThreadResume(displayTaskHandle);
				}
			}
		}
		if (LD(editMode) && LDI(menuMode))
		{
			float stepFactor;
			parameter_t value;
			if (LDP(editMode))
			{
				value = *pEditValue;
			}
			if (!LDP(editMode) && LDP(keyPLC[OK]))
			{
				Lcd_clear(&lcd);
				Lcd_cursor(&lcd, 0, 5);
				Lcd_string(&lcd, "OK");
				*pEditValue = value;
				menuMode.state = true;
				editMode.state = false;
				osDelay(1000);
			}
			if (!LDP(editMode) && LDP(keyPLC[MENU]))
			{
				Lcd_clear(&lcd);
				Lcd_cursor(&lcd, 0, 5);
				Lcd_string(&lcd, "CANCEL");
				//*pEditValue=value;
				menuMode.state = true;
				editMode.state = false;
				osDelay(1000);
			}
			if (LD(keyPLC[GASTEST]))
			{
				stepFactor = 10.0;
			}
			else
			{
				stepFactor = 1.0;
			}
			int32_t encoderCount = encGetCount(&htim2);
			if (encoderCount != 0 || LDP(editMode))
			{
				inc(&value, encoderCount, stepFactor);
				Lcd_clear(&lcd);
				Lcd_cursor(&lcd, 0, 1);
				Lcd_string(&lcd, value.name);
				Lcd_cursor(&lcd, 1, 1);
				Lcd_float(&lcd, value.value, 4);
				Lcd_string(&lcd, value.unit);
			}
		}

		for (size_t i = 0; i < 6; i++)
		{
			keyPLC[i].oldState = keyPLC[i].state;
		}
		menuMode.oldState = menuMode.state;
		menuModefront.oldState = menuModefront.state;
		editMode.oldState = editMode.state;
		cicle.oldState = cicle.state;
		gasCicle.oldState = gasCicle.state;
		wireCicle.oldState = wireCicle.state;
		weldingCicle.oldState = weldingCicle.state;
		osDelay(10);
	}
  /* USER CODE END StartControlTask */
}

/* USER CODE BEGIN Header_StartKeyScanTask */
/**
 * @brief Function implementing the keyScanTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartKeyScanTask */
void StartKeyScanTask(void const * argument)
{
  /* USER CODE BEGIN StartKeyScanTask */

	/* Infinite loop */
	for (;;)
	{
		key[WIREDOWN] = !HAL_GPIO_ReadPin(IN_WireDown_Btn_GPIO_Port, IN_WireDown_Btn_Pin);
		key[WIREUP] = !HAL_GPIO_ReadPin(IN_WireDown_Btn_GPIO_Port, IN_WireUp_Btn_Pin);
		key[GASTEST] = !HAL_GPIO_ReadPin(IN_GasTest_Btn_GPIO_Port, IN_GasTest_Btn_Pin);
		key[MENU] = !HAL_GPIO_ReadPin(IN_Menu_Btn_GPIO_Port, IN_Menu_Btn_Pin);
		key[OK] = !HAL_GPIO_ReadPin(IN_Ok_Btn_GPIO_Port, IN_Ok_Btn_Pin);
		key[START] = !HAL_GPIO_ReadPin(IN_Start_Btn_GPIO_Port, IN_Start_Btn_Pin);
		osDelay(70);
	}
  /* USER CODE END StartKeyScanTask */
}

/* USER CODE BEGIN Header_StartMenuControl */
/**
 * @brief Function implementing the menuControl thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartMenuControl */
void StartMenuControl(void const * argument)
{
  /* USER CODE BEGIN StartMenuControl */

	/* Infinite loop */
	for (;;)
	{
		OW_Measure();
		osDelay(1000);
		temperature=OW_Read_Sensors(0);
		osDelay(1000);
	}
  /* USER CODE END StartMenuControl */
}

/* USER CODE BEGIN Header_StartEditValue */
/**
 * @brief Function implementing the editValue thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartEditValue */
void StartEditValue(void const * argument)
{
  /* USER CODE BEGIN StartEditValue */

	/* Infinite loop */
	for (;;)
	{
		osDelay(100);
	}
  /* USER CODE END StartEditValue */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void valueEditRun(void)
{
	pEditValue = Menu_GetCurrentMenu()->pointer;
}
void saveSettings(void)
{
	float writePar[] = {0.0, 0.0, 0.0, 0.0};
	for (size_t i = 0; i < 6; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			writePar[j] = *Parameters[i][j];
		}
		writeEeprom(i * 32, writePar, sizeof(writePar));
		osDelay(100);
	}
}
void loadSettings(void)
{
	float readPar[] = {0.0, 0.0, 0.0, 0.0};
	for (size_t i = 0; i < 6; i++)
	{
		if (readEeprom(i * 32, readPar, sizeof(readPar)) == HAL_OK)
		{
			for (size_t j = 0; j < 4; j++)
			{
				*Parameters[i][j] = readPar[j];
			}
			osDelay(100);
		}
	}
}
void menuDisplayUpdate(void)
{
	Menu_Item_t *currMemuItem = Menu_GetCurrentMenu();
	Lcd_clear(&lcd);
	Lcd_cursor(&lcd, 0, 5);
	if (currMemuItem->Parent == &NULL_MENU)
		Lcd_string(&lcd, "MENU");
	else
		Lcd_string(&lcd, currMemuItem->Parent->Text);
	Lcd_cursor(&lcd, 1, 1);
	Lcd_string(&lcd, currMemuItem->Text);
}

int32_t encGetCount(TIM_HandleTypeDef *tmr)
{
	int16_t count = __HAL_TIM_GetCounter(tmr);
	__HAL_TIM_SetCounter(tmr, 0);
	return count;
}
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
	static uint16_t adctmp[4] = {0};
	static uint8_t pointer = 0;
	if (hadc->Instance == ADC1)
	{
		for (size_t i = 0; i < 4; i++)
		{
			adctmp[i] += adc[i];
		}
		if (pointer++ >= 8)
		{
			pointer = 0;
			adc0Filter = adctmp[0] >> 3;
			adc1Filter = adctmp[1] >> 3;
			adc2Filter = adctmp[2] >> 3;
			adc3Filter = adctmp[3] >> 3;
			for (size_t i = 0; i < 4; i++)
			{
				adctmp[i] = 0;
			}
		}
	}
}
/* USER CODE END Application */

