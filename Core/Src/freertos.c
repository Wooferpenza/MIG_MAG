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
#include "OneWire.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum
{
  IN_WireDown_Btn_Code = 0u,
  IN_WireUp_Btn_Code,
  IN_GasTest_Btn_Code,
  IN_Menu_Btn_Code,
  IN_Ok_Btn_Code,
  IN_Start_Btn_Code,
  Enc_Code
} Key_Code;
typedef enum
{
  Press = 0u, Release, Hold, Right, Left
} Key_State;
typedef struct
{
  Key_Code keyCode;
  Key_State keyState;
  int32_t count;
} keyInfo_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ON GPIO_PIN_SET
#define OFF GPIO_PIN_RESET

#define WELDING_RUN(state)	HAL_GPIO_WritePin(OUT_RUN_GPIO_Port, OUT_RUN_Pin, !state)
#define IS_WELDING_RUN		HAL_GPIO_ReadPin(OUT_RUN_GPIO_Port, OUT_RUN_Pin)

#define GAS_RUN(state)		HAL_GPIO_WritePin(OUT_GAS_GPIO_Port, OUT_GAS_Pin, state)
#define IS_GAS_RUN		HAL_GPIO_ReadPin(OUT_GAS_GPIO_Port, OUT_GAS_Pin)

#define WIRE_RUN(state)		HAL_GPIO_WritePin(OUT_Wire_Run_GPIO_Port, OUT_Wire_Run_Pin, state)
#define IS_WIRE_RUN		HAL_GPIO_ReadPin(OUT_Wire_Run_GPIO_Port, OUT_Wire_Run_Pin)

#define WIRE_DIR(state)		HAL_GPIO_WritePin(OUT_Wire_Dir_GPIO_Port, OUT_Wire_Dir_Pin, state)
#define IS_WIRE_DIR		HAL_GPIO_ReadPin(OUT_Wire_Dir_GPIO_Port, OUT_Wire_Dir_Pin)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
void valueEditRun(void);
volatile uint16_t adc[4];
volatile uint16_t adc0Filter=0;
volatile uint16_t adc1Filter=0;
volatile uint16_t adc2Filter=0;
volatile uint16_t adc3Filter=0;
Lcd_PortType ports[] = {LCD_D4_GPIO_Port, LCD_D5_GPIO_Port, LCD_D6_GPIO_Port, LCD_D7_GPIO_Port};
Lcd_PinType pins[] = {LCD_D4_Pin, LCD_D5_Pin, LCD_D6_Pin, LCD_D7_Pin};
Lcd_HandleTypeDef lcd;
//Параметры сварки
parameter_t U_Set={15.0,0.0,30.0,0.1,"U","V"};			// Уставка напряжения
parameter_t I_Set={215.0,0.0,215.0,1.0,"I","A"};		// Уставка тока
parameter_t V_Set={2.5,1.0,10.0,0.1,"V","m/min"};		// Уставка скорости подачи проволоки
parameter_t Wire_On={0.5,0.0,2.0,0.1,"Wire on","s"};	// Задержка подачи проволоки
parameter_t Welding_Off={0.5,0.0,2.0,0.1,"Welding off","s"};	// Задержка отключения источника
parameter_t Gas_Before={1.0,0.0,10.0,0.1,"Gas before","s"};		// Газ до сварки
parameter_t Gas_After={1.0,0.0,10.0,0.1,"Gas after","s"};		// Газ после сварки
parameter_t V_Manual={1.0,1.0,10.0,0.1,"V","m/min"};
parameter_t V_Manual_Marsh={1.0,1.0,10.0,0.1,"V","m/min"};
// Калибровочные коэффициенты
parameter_t U_Set_Code_Min={755.0,0.0,3300.0,1.0,"USetCodeMin",""};
parameter_t U_Set_Value_Min={15.0,0.0,30.0,0.1,"USetValueMin","V"};
parameter_t U_Set_Code_Max={1840.0,0.0,3300.0,1.0,"USetCodeMax",""};
parameter_t U_Set_Value_Max={30.0,0.0,30.0,0.1,"USetValueMax","V"};

parameter_t I_Set_Code_Min={1850.0,0.0,3300.0,1.0,"ISetCodeMin",""};
parameter_t I_Set_Value_Min={50.0,0.0,250.0,1.0,"ISetValueMin","A"};
parameter_t I_Set_Code_Max={0.0,0.0,3300.0,1.0,"ISetCodeMax",""};
parameter_t I_Set_Value_Max={215.0,0.0,250.0,1.0,"ISetValueMax","A"};

parameter_t V_Set_Code_Min={800.0,0.0,3300.0,1.0,"VSetCodeMin",""};
parameter_t V_Set_Value_Min={2.2,0.0,10.0,0.1,"VSetValueMin","m/min"};
parameter_t V_Set_Code_Max={3300.0,0.0,3300.0,1.0,"VSetCodeMax",""};
parameter_t V_Set_Value_Max={8.7,0.0,10.0,0.1,"VSetValueMax","m/min"};

parameter_t U_Present_Code_Min={0.0,0.0,4096.0,1.0,"UPresentCodeMin",""};
parameter_t U_Present_Value_Min={0.0,0.0,30.0,0.1,"UPresentValMin","V"};
parameter_t U_Present_Code_Max={0.0,0.0,4096.0,1.0,"UPresentCodeMax",""};
parameter_t U_Present_Value_Max={0.0,0.0,30.0,0.1,"UPresentValMax","V"};

parameter_t I_Present_Code_Min={0.0,0.0,4096.0,1.0,"IPresentCodeMin",""};
parameter_t I_Present_Value_Min={0.0,0.0,250.0,1.0,"IPresentValMin","A"};
parameter_t I_Present_Code_Max={0.0,0.0,4096.0,1.0,"IPresentCodeMax",""};
parameter_t I_Present_Value_Max={0.0,0.0,250.0,1.0,"IPresentValMax","A"};

parameter_t *pVar=&U_Set;
parameter_t *pEditValue;
extern float Temp[MAXDEVICES_ON_THE_BUS];
//Параметры для сохранения

float* Par1[]={&Gas_Before.value,&Gas_After.value,&Wire_On.value,&Welding_Off.value};
float* Par2[]={&U_Set_Code_Min.value,&U_Set_Value_Min.value,&U_Set_Code_Max.value,&U_Set_Value_Max.value};
float* Par3[]={&I_Set_Code_Min.value,&I_Set_Value_Min.value,&I_Set_Code_Max.value,&I_Set_Value_Max.value};
float* Par4[]={&V_Set_Code_Min.value,&V_Set_Value_Min.value,&V_Set_Code_Max.value,&V_Set_Value_Max.value};
float* Par5[]={&U_Present_Code_Min.value,&U_Present_Value_Min.value,&U_Present_Code_Max.value,&U_Present_Value_Max.value};
float* Par6[]={&I_Present_Code_Min.value,&I_Present_Value_Min.value,&I_Present_Code_Max.value,&I_Present_Value_Max.value};
//----------MENU------------------
//        Name,		Next,		Previous,	Parent,		Child,		SelectFunc,	EnterFunc,		Text
//		  �?мя,		Следующ,	Предыдущ,	Верхн,		Вложен,		Функц выбор,
MENU_ITEM(Menu_1,	Menu_2,		NULL_MENU,	NULL_MENU,	Menu_1_1,	NULL,		NULL,		NULL,	"Wire");

MENU_ITEM(Menu_1_1,	Menu_1_2,	Menu_1_4,	Menu_1, 	NULL_MENU,	NULL,		valueEditRun,		&Wire_On,	"Wire on time");
MENU_ITEM(Menu_1_2,	Menu_1_3,	Menu_1_1,	Menu_1, 	NULL_MENU,	NULL,		valueEditRun,		&Welding_Off,	"Welding off time");
MENU_ITEM(Menu_1_3,	Menu_1_4,	Menu_1_2,	Menu_1, 	NULL_MENU,	NULL,		valueEditRun,		&V_Manual,	"Manual speed");
MENU_ITEM(Menu_1_4,	Menu_1_1,	Menu_1_3,	Menu_1, 	NULL_MENU,	NULL,		valueEditRun,		&V_Manual_Marsh,	"Marsh speed");


MENU_ITEM(Menu_2,	Menu_3,		Menu_1,		NULL_MENU,	Menu_2_1,	NULL,		NULL,		NULL,	"Gas");

MENU_ITEM(Menu_2_1,	Menu_2_2,	NULL_MENU,	Menu_2, 	NULL_MENU,	NULL,		valueEditRun,	&Gas_Before,	"Before");
MENU_ITEM(Menu_2_2,	NULL_MENU,	Menu_2_1,	Menu_2, 	NULL_MENU,	NULL,		valueEditRun,	&Gas_After,	"After");


MENU_ITEM(Menu_3,	NULL_MENU,	Menu_2,		NULL_MENU,	Menu_3_1,	NULL,		NULL,		NULL,	"Calibration");

MENU_ITEM(Menu_3_1,	Menu_3_2,	NULL_MENU,	Menu_3,		NULL_MENU,	NULL,		valueEditRun,	&U_Set_Code_Min,	"USetCodeMin");
MENU_ITEM(Menu_3_2,	Menu_3_3,	Menu_3_1,	Menu_3,		NULL_MENU,	NULL,		valueEditRun,	&U_Set_Value_Min,	"USetValueMin");
MENU_ITEM(Menu_3_3,	Menu_3_4,	Menu_3_2,	Menu_3,		NULL_MENU,	NULL,		valueEditRun,	&U_Set_Code_Max,	"USetCodeMax");
MENU_ITEM(Menu_3_4,	Menu_3_5,	Menu_3_3,	Menu_3,		NULL_MENU,	NULL,		valueEditRun,	&U_Set_Value_Max,	"USetValueMax");
MENU_ITEM(Menu_3_5,	Menu_3_6,	Menu_3_4,	Menu_3,		NULL_MENU,	NULL,		valueEditRun,	&I_Set_Code_Min,	"ISetCodeMin");
MENU_ITEM(Menu_3_6,	Menu_3_7,	Menu_3_5,	Menu_3,		NULL_MENU,	NULL,		valueEditRun,	&I_Set_Value_Min,	"ISetValueMin");
MENU_ITEM(Menu_3_7,	Menu_3_8,	Menu_3_6,	Menu_3,		NULL_MENU,	NULL,		valueEditRun,	&I_Set_Code_Max,	"ISetCodeMax");
MENU_ITEM(Menu_3_8,	Menu_3_9,	Menu_3_7,	Menu_3,		NULL_MENU,	NULL,		valueEditRun,	&I_Set_Value_Max,	"ISetValueMax");
MENU_ITEM(Menu_3_9,	Menu_3_10,	Menu_3_8,	Menu_3,		NULL_MENU,	NULL,		valueEditRun,	&V_Set_Code_Min,	"VSetCodeMin");
MENU_ITEM(Menu_3_10,Menu_3_11,	Menu_3_9,	Menu_3,		NULL_MENU,	NULL,		valueEditRun,	&V_Set_Value_Min,	"VSetValueMin");
MENU_ITEM(Menu_3_11,Menu_3_12,	Menu_3_10,	Menu_3,		NULL_MENU,	NULL,		valueEditRun,	&V_Set_Code_Max,	"VSetCodeMax");
MENU_ITEM(Menu_3_12,Menu_3_13,	Menu_3_11,	Menu_3,		NULL_MENU,	NULL,		valueEditRun,	&V_Set_Value_Max,	"VSetValueMax");
MENU_ITEM(Menu_3_13,Menu_3_14,	Menu_3_12,	Menu_3,		NULL_MENU,	NULL,		valueEditRun,	&U_Present_Code_Min,	"UPresentCodeMin");
MENU_ITEM(Menu_3_14,Menu_3_15,	Menu_3_13,	Menu_3,		NULL_MENU,	NULL,		valueEditRun,	&U_Present_Value_Min,	"UPresentValueMin");
MENU_ITEM(Menu_3_15,Menu_3_16,	Menu_3_14,	Menu_3,		NULL_MENU,	NULL,		valueEditRun,	&U_Present_Code_Max,	"UPresentCodeMax");
MENU_ITEM(Menu_3_16,Menu_3_17,	Menu_3_15,	Menu_3,		NULL_MENU,	NULL,		valueEditRun,	&U_Present_Value_Max,	"UPresentValueMax");
MENU_ITEM(Menu_3_17,Menu_3_18,	Menu_3_16,	Menu_3,		NULL_MENU,	NULL,		valueEditRun,	&I_Present_Code_Min,	"IPresentCodeMin");
MENU_ITEM(Menu_3_18,Menu_3_19,	Menu_3_17,	Menu_3,		NULL_MENU,	NULL,		valueEditRun,	&I_Present_Value_Min,	"IPresentValueMin");
MENU_ITEM(Menu_3_19,Menu_3_20,	Menu_3_18,	Menu_3,		NULL_MENU,	NULL,		valueEditRun,	&I_Present_Code_Max,	"IPresentCodeMax");
MENU_ITEM(Menu_3_20,NULL_MENU,	Menu_3_19,	Menu_3,		NULL_MENU,	NULL,		valueEditRun,	&I_Present_Value_Max,	"IPresentValueMax");
//------------END MENU---------------
//extern float Temp[MAXDEVICES_ON_THE_BUS];
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
osMessageQId keyQueueHandle;
uint8_t keyQueueBuffer[ 5 * sizeof( keyInfo_t ) ];
osStaticMessageQDef_t keyQueueControlBlock;
osMutexId uSetMutexHandle;
osStaticMutexDef_t uSetMutexControlBlock;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
int32_t encGetStep(TIM_HandleTypeDef *tmr);

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

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
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
     lcd = Lcd_create(ports, pins, LCD_RS_GPIO_Port, LCD_RS_Pin, LCD_E_GPIO_Port, LCD_E_Pin,&htim4);
     HAL_ADCEx_Calibration_Start(&hadc1);
     HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc, 4);
     HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_1);
     HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_2);
     HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_3);
     HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_4);
     HAL_TIM_Base_Start_IT(&htim4);
     HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_1);
     get_ROMid();
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

  /* Create the queue(s) */
  /* definition and creation of keyQueue */
  osMessageQStaticDef(keyQueue, 5, keyInfo_t, keyQueueBuffer, &keyQueueControlBlock);
  keyQueueHandle = osMessageCreate(osMessageQ(keyQueue), NULL);

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
  osThreadSuspend(menuControlHandle);
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
    float uSet=0;
    float vSet=0;
    float iSet=0;
    float uPresentDisplay=0;
    float iPresentDisplay=0;
  for(;;)
  {
	    osMutexWait(uSetMutexHandle, osWaitForever);
        	uSet=U_Set.value;  vSet=V_Set.value; iSet=I_Set.value;
        osMutexRelease(uSetMutexHandle);
        if (!HAL_GPIO_ReadPin (IN_Start_Btn_GPIO_Port, IN_Start_Btn_Pin))
        {
        	float uPresent=calibration((float)adc2Filter,U_Present_Code_Min.value,U_Present_Value_Min.value,U_Present_Code_Max.value,U_Present_Value_Max.value);
        	uPresentDisplay=RCfilter(uPresent, uPresentDisplay, 0.95);
        	float iPresent=calibration((float)adc3Filter,I_Present_Code_Min.value,I_Present_Value_Min.value,I_Present_Code_Max.value,I_Present_Value_Max.value);
        	iPresentDisplay=RCfilter(iPresent, iPresentDisplay, 0.95);
        }

        	Lcd_clear (&lcd);
            Lcd_cursor (&lcd, 0, 1);
            Lcd_string (&lcd, "U");
            Lcd_float (&lcd, uSet,3);
            Lcd_cursor (&lcd, 0, 8);
            if (pVar == &I_Set)
      	{
      	  Lcd_string (&lcd, "I");
      	  Lcd_float (&lcd, iSet,3);
      	}
            else
      	{
            Lcd_string (&lcd, "V");
            Lcd_float (&lcd, vSet,3);
      	}

            Lcd_cursor (&lcd, 1, 2);
            Lcd_float (&lcd, uPresentDisplay, 3);

            Lcd_cursor (&lcd, 1, 9);
            Lcd_float (&lcd, iPresentDisplay, 3);

            if (pVar == &U_Set)
      	{
      	  Lcd_cursor (&lcd, 0, 0);
      	}
            else
      	{
      	  Lcd_cursor (&lcd, 0, 7);
      	}
            Lcd_string (&lcd, ">");

                 uint16_t USetCode=(uint16_t)calibration(uSet, U_Set_Value_Min.value, U_Set_Code_Min.value, U_Set_Value_Max.value, U_Set_Code_Max.value);
				 USetCode=rangeLimitInt(USetCode, 0, 3300);
                 __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, USetCode);

                 uint16_t ISetCode=(uint16_t)calibration(iSet, I_Set_Value_Min.value, I_Set_Code_Min.value, I_Set_Value_Max.value, I_Set_Code_Max.value);
                 ISetCode=rangeLimitInt(ISetCode, 0, 3300);
                 __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, ISetCode);

                 uint16_t VSetCode=(uint16_t)calibration(vSet, V_Set_Value_Min.value, V_Set_Code_Min.value, V_Set_Value_Max.value, V_Set_Code_Max.value);
                 VSetCode=rangeLimitInt(VSetCode, 0, 3300);
                 __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, VSetCode);
            get_Temperature();
            osDelay (100);
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

	 HAL_GPIO_WritePin (OUT_LCD_LED_GPIO_Port, OUT_LCD_LED_Pin, GPIO_PIN_SET);
	 keyInfo_t key;
	bool cicle = false;
	bool okBtn = false;
	bool menuBtn = false;
	bool startBtn = false;
	bool gasBtn = false;
	bool wireUpBtn = false;
	bool wireDownBtn = false;
	uint8_t keyCount = 0;
	float readPar[]={0.0,0.0,0.0,0.0};
	if (readEeprom(0,readPar,sizeof(readPar) )==HAL_OK)
		    {
		            Lcd_clear (&lcd);
		      		Lcd_cursor (&lcd, 0, 5);
		      		Lcd_string(&lcd, "READ OK1");
				Gas_Before.value=readPar[0];
		    	Gas_After.value=readPar[1];
		    	Wire_On.value=readPar[2];
		    	Welding_Off.value=readPar[3];
		    	osDelay(100);
		    }

	if (readEeprom(32,readPar,sizeof(readPar) )==HAL_OK)
			    {
			            Lcd_clear (&lcd);
			      		Lcd_cursor (&lcd, 0, 5);
			      		Lcd_string(&lcd, "READ OK2");
			    U_Set_Code_Min.value=readPar[0];
			    U_Set_Value_Min.value=readPar[1];
			    U_Set_Code_Max.value=readPar[2];
			    U_Set_Value_Max.value=readPar[3];
			    osDelay(100);
			    }
	if (readEeprom(64,readPar,sizeof(readPar) )==HAL_OK)
			    {
			            Lcd_clear (&lcd);
			      		Lcd_cursor (&lcd, 0, 5);
			      		Lcd_string(&lcd, "READ OK3");
			      		I_Set_Code_Min.value=readPar[0];
			      		I_Set_Value_Min.value=readPar[1];
			      		I_Set_Code_Max.value=readPar[2];
			      		I_Set_Value_Max.value=readPar[3];
				    	osDelay(100);
			    }

	if (readEeprom(96,readPar,sizeof(readPar) )==HAL_OK)
				    {
				            Lcd_clear (&lcd);
				      		Lcd_cursor (&lcd, 0, 5);
				      		Lcd_string(&lcd, "READ OK4");
				      		V_Set_Code_Min.value=readPar[0];
				      		V_Set_Value_Min.value=readPar[1];
				      		V_Set_Code_Max.value=readPar[2];
				      		V_Set_Value_Max.value=readPar[3];
					    	osDelay(100);
				    }
	if (readEeprom(128,readPar,sizeof(readPar) )==HAL_OK)
				    {
				            Lcd_clear (&lcd);
				      		Lcd_cursor (&lcd, 0, 5);
				      		Lcd_string(&lcd, "READ OK5");
				      		U_Present_Code_Min.value=readPar[0];
				      		U_Present_Value_Min.value=readPar[1];
				      		U_Present_Code_Max.value=readPar[2];
				      		U_Present_Value_Max.value=readPar[3];
					    	osDelay(100);
				    }
	if (readEeprom(160,readPar,sizeof(readPar) )==HAL_OK)
				    {
				            Lcd_clear (&lcd);
				      		Lcd_cursor (&lcd, 0, 5);
				      		Lcd_string(&lcd, "READ OK6");
				      		I_Present_Code_Min.value=readPar[0];
				      		I_Present_Value_Min.value=readPar[1];
				      		I_Present_Code_Max.value=readPar[2];
				      		I_Present_Value_Max.value=readPar[3];
					    	osDelay(100);
				    }

	osThreadResume(displayTaskHandle);
	/* Infinite loop */
	for (;;)
	{
		xQueueReceive(keyQueueHandle, &key, portMAX_DELAY);
		switch (key.keyCode)
		{
		case IN_Ok_Btn_Code:
			if (key.keyState == Press)
			{
				okBtn = true;
				if (pVar == &U_Set)
					pVar = &V_Set;
				else if (pVar == &V_Set)
					pVar = &I_Set;
				else if (pVar == &I_Set)
					pVar = &U_Set;
			}
			if (key.keyState == Release)
			{
				okBtn = false;
			}

			if (key.keyState == Hold)
			{
				Lcd_init(&lcd);
			}
			break;
		case Enc_Code:
			osMutexWait(uSetMutexHandle, osWaitForever);
			inc(pVar, key.count,1.0);
			osMutexRelease(uSetMutexHandle);
			break;
		case IN_Start_Btn_Code:
			if (key.keyState == Press)
			{
				startBtn = true;
				cicle = true;
				GAS_RUN(ON);
				osDelay((uint32_t) (Gas_Before.value * 1000.0));
				WELDING_RUN(ON);
				osDelay((uint32_t) (Wire_On.value * 1000.0));
				WIRE_RUN(ON);
				WIRE_DIR(ON);
			}
			else if (key.keyState == Release)
			{
				WIRE_RUN(OFF);
				WIRE_DIR(OFF);
				osDelay((uint32_t) (Welding_Off.value * 1000.0));
				WELDING_RUN(OFF);
				osDelay((uint32_t) (Gas_After.value * 1000.0));
				GAS_RUN(OFF);
				startBtn = false;
				cicle = false;
			}
			break;

		case IN_GasTest_Btn_Code:
			if (key.keyState == Press)
			{
				gasBtn = true;
				GAS_RUN(ON);
			}
			else if (key.keyState == Release)
			{
				gasBtn = false;
				GAS_RUN(OFF);
			}
			break;

		case IN_WireDown_Btn_Code:
			if (key.keyState == Press)
			{
				wireDownBtn = true;
				WIRE_RUN(ON);
				WIRE_DIR(ON);
			}
			else if (key.keyState == Release)
			{
				wireDownBtn = false;
				WIRE_RUN(OFF);
				WIRE_DIR(OFF);
			}
			break;

		case IN_WireUp_Btn_Code:
			if (key.keyState == Press)
			{
				wireUpBtn = true;
				WIRE_RUN(ON);

			}
			else if (key.keyState == Release)
			{
				wireUpBtn = false;
				WIRE_RUN(OFF);

			}
			break;

		case IN_Menu_Btn_Code:
			if (key.keyState == Press)
			{
				Menu_Navigate(&Menu_1);
				xQueueReset(keyQueueHandle);
				taskENTER_CRITICAL();
				osThreadSuspend(displayTaskHandle);
				osThreadResume(menuControlHandle);
				osThreadSuspend(controlTaskHandle);
				taskEXIT_CRITICAL();

			}
			break;

		default:
			break;
		}
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
    GPIO_PinState oldPinState[6];
    GPIO_PinState pinState[6];
    uint_fast32_t count[6]={};
    oldPinState[IN_WireDown_Btn_Code] = HAL_GPIO_ReadPin (IN_WireDown_Btn_GPIO_Port, IN_WireDown_Btn_Pin);
    oldPinState[1] = HAL_GPIO_ReadPin (IN_WireUp_Btn_GPIO_Port,   IN_WireUp_Btn_Pin);
    oldPinState[2] = HAL_GPIO_ReadPin (IN_GasTest_Btn_GPIO_Port,  IN_GasTest_Btn_Pin);
    oldPinState[3] = HAL_GPIO_ReadPin (IN_Menu_Btn_GPIO_Port, IN_Menu_Btn_Pin);
    oldPinState[4] = HAL_GPIO_ReadPin (IN_Ok_Btn_GPIO_Port, IN_Ok_Btn_Pin);
    oldPinState[5] = HAL_GPIO_ReadPin (IN_Start_Btn_GPIO_Port, IN_Start_Btn_Pin);
    keyInfo_t key;
  /* Infinite loop */
  for(;;)
  {
            pinState[IN_WireDown_Btn_Code] = HAL_GPIO_ReadPin (IN_WireDown_Btn_GPIO_Port, IN_WireDown_Btn_Pin);
            pinState[1] = HAL_GPIO_ReadPin (IN_WireUp_Btn_GPIO_Port, IN_WireUp_Btn_Pin);
            pinState[2] = HAL_GPIO_ReadPin (IN_GasTest_Btn_GPIO_Port, IN_GasTest_Btn_Pin);
            pinState[3] = HAL_GPIO_ReadPin (IN_Menu_Btn_GPIO_Port, IN_Menu_Btn_Pin);
            pinState[4] = HAL_GPIO_ReadPin (IN_Ok_Btn_GPIO_Port, IN_Ok_Btn_Pin);
            pinState[5] = HAL_GPIO_ReadPin (IN_Start_Btn_GPIO_Port, IN_Start_Btn_Pin);

            if (oldPinState[IN_WireDown_Btn_Code] > pinState[IN_WireDown_Btn_Code])
      	{
      	  key.keyCode = IN_WireDown_Btn_Code;
      	  key.keyState = Press;
      	  xQueueSend(keyQueueHandle, &key, portMAX_DELAY);
       	}
            if (oldPinState[IN_WireDown_Btn_Code] < pinState[IN_WireDown_Btn_Code])
      	{
      	  key.keyCode = IN_WireDown_Btn_Code;
      	  key.keyState = Release;
      	  xQueueSend(keyQueueHandle, &key, portMAX_DELAY);
      	}
            if (oldPinState[1] > pinState[1])
      	{
      	  key.keyCode = IN_WireUp_Btn_Code;
      	  key.keyState = Press;
      	  xQueueSend(keyQueueHandle, &key, portMAX_DELAY);
      	}
            if (oldPinState[1] < pinState[1])
      	{
      	  key.keyCode = IN_WireUp_Btn_Code;
      	  key.keyState = Release;
      	  xQueueSend(keyQueueHandle, &key, portMAX_DELAY);
      	}
            if (oldPinState[2] > pinState[2])
      	{
      	  key.keyCode = IN_GasTest_Btn_Code;
      	  key.keyState = Press;
      	  xQueueSend(keyQueueHandle, &key, portMAX_DELAY);
      	}
            if (oldPinState[2] < pinState[2])
      	{
      	  key.keyCode = IN_GasTest_Btn_Code;
      	  key.keyState = Release;
      	  xQueueSend(keyQueueHandle, &key, portMAX_DELAY);
      	}
            if (oldPinState[3] > pinState[3])
      	{
      	  key.keyCode = IN_Menu_Btn_Code;
      	  key.keyState = Press;
      	  xQueueSend(keyQueueHandle, &key, portMAX_DELAY);
      	}
            if (oldPinState[3] < pinState[3])
      	{
      	  key.keyCode = IN_Menu_Btn_Code;
      	  key.keyState = Release;
      	  xQueueSend(keyQueueHandle, &key, portMAX_DELAY);
      	}
            if (oldPinState[4] > pinState[4])
      	{
      	  key.keyCode = IN_Ok_Btn_Code;
      	  key.keyState = Press;
      	  xQueueSend(keyQueueHandle, &key, portMAX_DELAY);
      	}
            if (oldPinState[4] < pinState[4])
      	{
      	  key.keyCode = IN_Ok_Btn_Code;
      	  key.keyState = Release;
      	  xQueueSend(keyQueueHandle, &key, portMAX_DELAY);
      	}
            if (!pinState[4])
              {
        	count[4]++;
        	if (count[4]==50)
        	  {
        	    key.keyCode = IN_Ok_Btn_Code;
        	    key.keyState = Hold;
        	    xQueueSend(keyQueueHandle, &key, portMAX_DELAY);
        	  }
              }
            else
              {
        	count[4]=0;
              }

            if (oldPinState[5] > pinState[5])
      	{
      	  key.keyCode = IN_Start_Btn_Code;
      	  key.keyState = Press;
      	  xQueueSend(keyQueueHandle, &key, portMAX_DELAY);
      	}
            if (oldPinState[5] < pinState[5])
      	{
      	  key.keyCode = IN_Start_Btn_Code;
      	  key.keyState = Release;
      	  xQueueSend(keyQueueHandle, &key, portMAX_DELAY);
      	}
            for (int i = 0; i < 6; i++)
      	{
      	  oldPinState[i] = pinState[i];
      	}
            int32_t encStep = encGetStep (&htim2);
            if (encStep)
      	{
      	  key.keyCode = Enc_Code;
      	  if (encStep > 0)
      	    key.keyState = Right;
      	  else
      	    key.keyState = Left;
      	  key.count = encStep;
      	  xQueueSend(keyQueueHandle, &key, portMAX_DELAY);
      	}
            osDelay (100);
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
  keyInfo_t key;
  Menu_Item_t *currMemuItem;
  /* Infinite loop */
  for(;;)
  {
      xQueueReceive (keyQueueHandle, &key, portMAX_DELAY);
            switch (key.keyCode)
      	{
      	case Enc_Code:
      	  if (key.keyState == Right&&(Menu_GetCurrentMenu()->Next!=&NULL_MENU))
      	    Menu_Navigate (MENU_NEXT);
      	  else if (key.keyState == Left&&(Menu_GetCurrentMenu()->Previous!=&NULL_MENU))
      	    Menu_Navigate (MENU_PREVIOUS);
      	  break;
      	case IN_Ok_Btn_Code:
      	  if (key.keyState == Press&&(Menu_GetCurrentMenu()->Child!=&NULL_MENU))
      	    Menu_Navigate (MENU_CHILD);
      	  else if (key.keyState == Press&&(Menu_GetCurrentMenu()->Child==&NULL_MENU))
      	    Menu_EnterCurrentItem();
      	  break;
      	case IN_Menu_Btn_Code:
      	  if (key.keyState == Press)
      	    {
      	      if (Menu_GetCurrentMenu ()->Parent == &NULL_MENU)
      		{
      		Lcd_clear (&lcd);
      		Lcd_cursor (&lcd, 0, 5);
      		Lcd_string(&lcd, "SAVE");
      		float writePar[]={0.0,0.0,0.0,0.0};
      		for (int i=0;i<4;i++)
      		{
      			writePar[i]=*Par1[i];
      		}
      		writeEeprom(0,writePar,sizeof(writePar));
      		osDelay(100);
      		for (int i=0;i<4;i++)
      		      		{
      		      			writePar[i]=*Par2[i];
      		      		}
      		writeEeprom(32,writePar,sizeof(writePar));
      		osDelay(100);
      		for (int i=0;i<4;i++)
      		      		{
      		      			writePar[i]=*Par3[i];
      		      		}
      		writeEeprom(64,writePar,sizeof(writePar));
      		osDelay(100);
      		for (int i=0;i<4;i++)
      		      		{
      		      			writePar[i]=*Par4[i];
      		      		}
      		writeEeprom(96,writePar,sizeof(writePar));
      		osDelay(100);
      		for (int i=0;i<4;i++)
      		      		{
      		      			writePar[i]=*Par5[i];
      		      		}
      		writeEeprom(128,writePar,sizeof(writePar));
      		osDelay(100);
      		for (int i=0;i<4;i++)
      		      		{
      		      			writePar[i]=*Par6[i];
      		      		}
      		writeEeprom(160,writePar,sizeof(writePar));
      		osDelay(100);
      		taskENTER_CRITICAL();
      		xQueueReset(keyQueueHandle);
      		osThreadResume (displayTaskHandle);
     		osThreadResume (controlTaskHandle);
      		osThreadSuspend (menuControlHandle);
      		taskEXIT_CRITICAL();
      		}
      	      else
      		{
      		  Menu_Navigate (MENU_PARENT);
      		}
      	    }
      	default:
      	  break;
      	}
                currMemuItem=Menu_GetCurrentMenu();
                Lcd_clear (&lcd);
                Lcd_cursor (&lcd, 0, 5);
                if (currMemuItem->Parent==&NULL_MENU)    Lcd_string (&lcd, "MENU");
                else Lcd_string (&lcd, currMemuItem->Parent->Text);
                Lcd_cursor (&lcd, 1, 1);
                Lcd_string (&lcd, currMemuItem->Text);
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
  keyInfo_t key;
  float stepFactor=1.0;
  /* Infinite loop */
  for(;;)
  {

      xQueueReceive (keyQueueHandle, &key, portMAX_DELAY);
                  switch (key.keyCode)
            	{
            	case IN_Ok_Btn_Code:
            	  if (key.keyState == Press)
            	    {
            	  Lcd_clear (&lcd);
            	  Lcd_cursor (&lcd, 0, 5);
            	  Lcd_string (&lcd, "Ok");
            	  osDelay(1000);
            	  xQueueReset(keyQueueHandle);
            	  osThreadResume (menuControlHandle);
            	  osThreadSuspend(editValueHandle);
            	    }
            	  break;
            	case Enc_Code:
            	  inc(pEditValue,key.count,stepFactor);
            	  break;
            	case IN_Menu_Btn_Code:
            		if (key.keyState == Press)
            		   {
            		stepFactor=10.0;
            		   }
            		if (key.keyState == Release)
            		            		   {
            		            		stepFactor=1.0;
            		            		   }
            	break;
            	default:;
            	}
                      Lcd_clear (&lcd);
            	      Lcd_cursor (&lcd, 0, 1);
            	      Lcd_string (&lcd, pEditValue->name);
            	      Lcd_cursor (&lcd, 1, 1);
            	      Lcd_float (&lcd, pEditValue->value,4);
            	      Lcd_string (&lcd, pEditValue->unit);
            osDelay(100);
  }
  /* USER CODE END StartEditValue */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void valueEditRun(void)
{
  pEditValue=Menu_GetCurrentMenu()->pointer;
  osThreadResume (editValueHandle);
  osThreadSuspend (menuControlHandle);
}

int32_t encGetStep(TIM_HandleTypeDef *tmr)
{
   int16_t count = __HAL_TIM_GetCounter(tmr);
   __HAL_TIM_SetCounter(tmr,0);
   return count;
}
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
static uint16_t adc0[8]={};
static uint16_t adc1[8]={};
static uint16_t adc2[8]={};
static uint16_t adc3[8]={};
static uint8_t pointer=0;
adc0[pointer]=adc[0];
adc1[pointer]=adc[1];
adc2[pointer]=adc[2];
adc3[pointer]=adc[3];
if(pointer++>=8) pointer=0;
adc0Filter=0;
adc1Filter=0;
adc2Filter=0;
adc3Filter=0;
for(int i=0;i<8;i++)
  {
    adc0Filter+=adc0[i];
    adc1Filter+=adc1[i];
    adc2Filter+=adc2[i];
    adc3Filter+=adc3[i];
  }
adc0Filter>>=3;
adc1Filter>>=3;
adc2Filter>>=3;
adc3Filter>>=3;
}
/* USER CODE END Application */

