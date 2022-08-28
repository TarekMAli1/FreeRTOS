/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"
#include "queue.h"
/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"


/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )


/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
int systimeTime;
float cpu_load;
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/
pinState_t B_Read;
TaskHandle_t Button_1_Monitor_Handler  = NULL;
TaskHandle_t Button_2_Monitor_Handler  = NULL;
TaskHandle_t Periodic_Transmitter_Handler  = NULL;
TaskHandle_t Uart_Receiver_Handler  = NULL;
TaskHandle_t Load_1_Simulation_Handler  = NULL;
TaskHandle_t Load_2_Simulation_Handler  = NULL;

volatile pinState_t Button_1_OldValue; 
volatile pinState_t Button_1_NewValue;						

pinState_t Button_2_OldValue ;
pinState_t Button_2_NewValue;

static QueueHandle_t UART_Queue ;


int Load_1_switchin_time =0 , load_1_switchout_time =0,load_1_ExecutionTime =0 ;
int Load_2_switchin_time =0 , load_2_switchout_time =0,load_2_ExecutionTime =0 ;
int UART_switchin_time =0 , UART_switchout_time =0,UART_ExecutionTime =0 ;
int But_1_switchin_time =0 , But_1_switchout_time =0, But_1_ExecutionTime =0 ;
int But_2_switchin_time =0 , But_2_switchout_time =0,But_2_ExecutionTime =0 ;
int Periodic_Transmitter_switchin_time =0 , Periodic_Transmitter_switchout_time =0,Periodic_Transmitter_ExecutionTime =0 ;


void Uart_Receiver ( void * pvParameters )
{
			TickType_t xLastWakeTime;
     // Initialise the xLastWakeTime variable with the current time.
     xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag(NULL ,(void *)1 );
	for (;;){
		char QValue[10];
		int i=0;
		GPIO_write(PORT_0, PIN2, PIN_IS_HIGH);	
		for( i = 0;i<5;i++){
			xQueueReceive( UART_Queue,
										&( QValue ),
										( TickType_t) 0 );
			 vSerialPutString(QValue,10);
		}
		GPIO_write(PORT_0, PIN2, PIN_IS_LOW);
			vTaskDelayUntil( &xLastWakeTime, 20 );
		}
}
void Button_1_Monitor ( void * pvParameters )
{
	char Result[10]="";
	
	 TickType_t xLastWakeTime;
		Button_1_OldValue =GPIO_read(PORT_0,PIN0);
     // Initialise the xLastWakeTime variable with the current time.
     xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag(NULL ,(void *)2);
	for (;;){
			GPIO_write(PORT_0, PIN3, PIN_IS_HIGH);
			Button_1_NewValue =GPIO_read(PORT_0,PIN0);
			if ((Button_1_OldValue!=Button_1_NewValue)&&(Button_1_OldValue==PIN_IS_LOW))
			{
				Button_1_OldValue=Button_1_NewValue;
				strcpy(Result,"B1rising");
			}	
			else if((Button_1_OldValue!=Button_1_NewValue)&&(Button_1_OldValue==PIN_IS_HIGH)) {
				strcpy(Result,"B1falling");
			}
			else{
				strcpy(Result,"B1nochange");
			}
			xQueueSend( UART_Queue,
								( void * ) &Result,
								( TickType_t ) 0	);
			GPIO_write(PORT_0, PIN3, PIN_IS_LOW);
			vTaskDelayUntil( &xLastWakeTime, 50 );
		}
}
void Button_2_Monitor ( void * pvParameters )
{
	   TickType_t xLastWakeTime;
     // Initialise the xLastWakeTime variable with the current time.
     xLastWakeTime = xTaskGetTickCount();
		Button_2_OldValue =GPIO_read(PORT_0,PIN1);
	vTaskSetApplicationTaskTag(NULL ,(void *)3);
	for (;;){
		char Result[10]="";
		GPIO_write(PORT_0, PIN4, PIN_IS_HIGH);
			Button_2_NewValue =GPIO_read(PORT_0,PIN1);
			if ((Button_2_OldValue!=Button_2_NewValue)&&(Button_2_OldValue==PIN_IS_LOW))
			{
				Button_2_OldValue=Button_2_NewValue;
				strcpy(Result, "B2Rising");
			}
			else if((Button_2_OldValue!=Button_2_NewValue)&&(Button_2_OldValue==PIN_IS_HIGH)) {
				strcpy(Result,"B2Falling");
			}
			else{
				strcpy(Result,"B2nochange");
			}
			xQueueSend( UART_Queue,
								( void * ) &Result,
								( TickType_t ) 0	);
								GPIO_write(PORT_0, PIN4, PIN_IS_LOW);
								vTaskDelayUntil( &xLastWakeTime, 50 );
		}
}
void Periodic_Transmitter ( void * pvParameters )
{
			TickType_t xLastWakeTime;
     // Initialise the xLastWakeTime variable with the current time.
     xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag(NULL ,(void *)4 );
	for (;;){
		char Result[10]="any";
			GPIO_write(PORT_0, PIN5, PIN_IS_HIGH);
			xQueueSend( UART_Queue,
								( void * ) &Result,
								( TickType_t ) 0	);
			GPIO_write(PORT_0, PIN5, PIN_IS_LOW);
			vTaskDelayUntil( &xLastWakeTime, 100 );
		}
}

void Load_1_Simulation ( void * pvParameters )
{
			TickType_t xLastWakeTime;
     // Initialise the xLastWakeTime variable with the current time.
     xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag(NULL ,(void *)5 );
	for (;;){
			int i;
			GPIO_write(PORT_0, PIN6, PIN_IS_HIGH);
			for(i=0;i<34500;i++){}
			GPIO_write(PORT_0, PIN6, PIN_IS_LOW);
					vTaskDelayUntil( &xLastWakeTime, 10 );
		}
}
void Load_2_Simulation ( void * pvParameters )
{
			TickType_t xLastWakeTime;
     // Initialise the xLastWakeTime variable with the current time.
     xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag(NULL ,(void *)6);
		for (;;){
			int i;
			GPIO_write(PORT_0, PIN7, PIN_IS_HIGH);
			for(i=0;i<81000;i++){}
			GPIO_write(PORT_0, PIN7, PIN_IS_LOW);
			vTaskDelayUntil( &xLastWakeTime, 100 );
		}
}

void vApplicationIdleHook( void ){
	GPIO_write(PORT_0,PIN8,PIN_IS_HIGH);
	GPIO_write(PORT_0,PIN8,PIN_IS_LOW);
}
/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
  {
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();
	UART_Queue = xQueueCreate( 5, sizeof( char ) );
    /* Create Tasks here */

    /* Create the task, storing the handle. */
    /*xTaskCreate(
                    Button_Task,       // Function that implements the task. 
                    "but",          // Text name for the task. 
                    100,      // Stack size in words, not bytes. 
                    ( void * ) 0,    // Parameter passed into the task. 
                    1,// Priority at which the task is created. 
                    &ButtonHandler );      //Used to pass out the created task's handle. 
										*/
		
		
		xTaskCreatePeriodic(
                    Periodic_Transmitter,       /* Function that implements the task. */
                    "Periodic_Transmitter",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &Periodic_Transmitter_Handler,
										100);
		xTaskCreatePeriodic(
                    Load_2_Simulation,       /* Function that implements the task. */
                    "Load_2_Simulation",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &Load_2_Simulation_Handler,
										100);
		
		xTaskCreatePeriodic(
                    Button_1_Monitor,       /* Function that implements the task. */
                    "Button_1_Monitor",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &Button_1_Monitor_Handler,
										50);      /* Used to pass out the created task's handle. */

		xTaskCreatePeriodic(
                    Button_2_Monitor,       /* Function that implements the task. */
                    "Button_2_Monitor",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &Button_2_Monitor_Handler,
										50); 
		xTaskCreatePeriodic(
                    Uart_Receiver,       /* Function that implements the task. */
                    "Uart_Receiver",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &Uart_Receiver_Handler,
										20);
		
		xTaskCreatePeriodic(
                    Load_1_Simulation,       /* Function that implements the task. */
                    "Load_1_Simulation",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                   &Load_1_Simulation_Handler,
										10);
		
	/* Now all the tasks have been started - start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo applicati	on projects then ensure Supervisor mode is used here. */
	vTaskStartScheduler();

	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

/* Function to reset timer 1 */
void timer1Reset(void)
{
	T1TCR |= 0x2;
	T1TCR &= ~0x2;
}

/* Function to initialize and start timer 1 */
 void configTimer1(void)
{
	T1PR = 1000;
	T1TCR |= 0x1;
}

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();
	
	/* Config trace timer 1 and read T1TC to get current tick */
	configTimer1();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/


