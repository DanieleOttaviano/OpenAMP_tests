//freeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
//OpenAMP-Libmetal
#include <openamp/open_amp.h>
#include <metal/alloc.h>
#include "rsc_table.h"
#include "platform_info.h"
#include "main.h"
//Xilinx
#include "xil_printf.h"

#include "RPU_task.h"

void activeWaiting(const int ms){
	// This implementation doesn't consider the case in which the task is preempted.  
	TickType_t time = xTaskGetTickCount();
	while( xTaskGetTickCount() < time + pdMS_TO_TICKS(ms));
}

void periodicTask1(void *unused_arg)
{
	 LPRINTF("[RPU]: Periodic Task 1 alive...\n");
	 TickType_t xLastWakeTime;
	 TickType_t start;
	 TickType_t restart_time;
	 TickType_t end;
	 const TickType_t xFrequency = 15;
	 int first = 1;

	     // Initialize the xLastWakeTime variable with the current time.
	     xLastWakeTime = xTaskGetTickCount();

	     for( ;; )
	     {
	         // Wait for the next cycle.
	         vTaskDelayUntil( &xLastWakeTime, xFrequency );

	         // Perform action here.
	         restart_time = xTaskGetTickCount() - start;
	         start = xTaskGetTickCount();

	         activeWaiting(2*5);

	         end = xTaskGetTickCount();
	         //LPRINTF("[RPU]: Periodic Task 1 execution time: %d\n", end-start);
	         //Se il tempo di restart + l'esecuzione del task è maggiore di due volte il periodo -> DEADLINE superata
	         if(((restart_time + (end-start)) > 2*xFrequency) && first==0 )	LPRINTF("[RPU]: Periodic Task 1 DEADLINE EXCEEDED");
	         if(first==1)first = 0;
	     }
}

void periodicTask2(void *unused_arg)
{
	 LPRINTF("[RPU]: Periodic Task 2 alive...\n");
	 TickType_t xLastWakeTime;
	 TickType_t start;
	 TickType_t restart_time;
	 TickType_t end;
	 const TickType_t xFrequency = 40;
	 int first = 1;

	     // Initialize the xLastWakeTime variable with the current time.
	     xLastWakeTime = xTaskGetTickCount();

	     for( ;; )
	     {
	         // Wait for the next cycle.
	         vTaskDelayUntil( &xLastWakeTime, xFrequency );

	         // Perform action here.
	         restart_time = xTaskGetTickCount() - start;
	         start = xTaskGetTickCount();

	         activeWaiting(2*10);

	         end = xTaskGetTickCount();
	         //LPRINTF("[RPU]: Periodic Task 2 execution time: %d\n", end-start);
	         //LPRINTF("[RPU]: Periodic Task 2 execution time: %d\n", end-start);
	         //Se il tempo di restart + l'esecuzione del task è maggiore di due volte il periodo -> DEADLINE superata
	         if(((restart_time + (end-start)) > 2*xFrequency) && first==0 )	LPRINTF("[RPU]: Periodic Task 2 DEADLINE EXCEEDED");
	         if(first==1)first = 0;
	     }
}

void periodicTask3(void *unused_arg)
{
	 LPRINTF("[RPU]: Periodic Task 3 alive...\n");
	 TickType_t xLastWakeTime;
	 TickType_t start;
	 TickType_t restart_time;
	 TickType_t end;
	 const TickType_t xFrequency = 90;
	 int first = 1;

	     // Initialize the xLastWakeTime variable with the current time.
	     xLastWakeTime = xTaskGetTickCount();

	     for( ;; )
	     {
	         // Wait for the next cycle.
	         vTaskDelayUntil( &xLastWakeTime, xFrequency );

	         // Perform action here.
	         restart_time = xTaskGetTickCount() - start;
	         start = xTaskGetTickCount();

	         activeWaiting(2*15);

	         end = xTaskGetTickCount();
	         //LPRINTF("[RPU]: Periodic Task 3 execution time: %d\n", end-start);
	         //Se il tempo di restart + l'esecuzione del task è maggiore di due volte il periodo -> DEADLINE superata
	         if(((restart_time + (end-start)) > 2*xFrequency) && first==0 )	LPRINTF("[RPU]: Periodic Task 3 DEADLINE EXCEEDED");
	         if(first==1)first = 0;
	     }
}

void periodicTask4(void *unused_arg)
{
	 LPRINTF("[RPU]: Periodic Task 4 alive...\n");
	 TickType_t xLastWakeTime;
	 TickType_t start;
	 TickType_t restart_time;
	 TickType_t end;
	 const TickType_t xFrequency = 100;
	 int first = 1;

	     // Initialize the xLastWakeTime variable with the current time.
	     xLastWakeTime = xTaskGetTickCount();

	     for( ;; )
	     {
	         // Wait for the next cycle.
	         vTaskDelayUntil( &xLastWakeTime, xFrequency );

	         // Perform action here.
	         restart_time = xTaskGetTickCount() - start;
	         start = xTaskGetTickCount();

	         activeWaiting(2*20);

	         end = xTaskGetTickCount();
	         //LPRINTF("[RPU]: Periodic Task 4 execution time: %d\n", end-start);
	         //Se il tempo di restart + l'esecuzione del task è maggiore di due volte il periodo -> DEADLINE superata
	         if(((restart_time + (end-start)) > 2*xFrequency) && first==0 )	LPRINTF("[RPU]: Periodic Task 4 DEADLINE EXCEEDED");
	         if(first==1)first = 0;
	     }
}

void periodicTask5(void *unused_arg)
{
	 LPRINTF("[RPU]: Periodic Task 5 alive...\n");
	 TickType_t xLastWakeTime;
	 TickType_t start;
	 TickType_t restart_time;
	 TickType_t end;
	 const TickType_t xFrequency = 300;
	 int first = 1;

	     // Initialize the xLastWakeTime variable with the current time.
	     xLastWakeTime = xTaskGetTickCount();

	     for( ;; )
	     {
	         // Wait for the next cycle.
	         vTaskDelayUntil( &xLastWakeTime, xFrequency );

	         // Perform action here.
	         restart_time = xTaskGetTickCount() - start;
	         start = xTaskGetTickCount();

	         activeWaiting(2*30);

	         end = xTaskGetTickCount();
	         //LPRINTF("[RPU]: Periodic Task 5 execution time: %d\n", end-start);
	         //Se il tempo di restart + l'esecuzione del task è maggiore di due volte il periodo -> DEADLINE superata
	         if(((restart_time + (end-start)) > 2*xFrequency) && first==0 )	LPRINTF("[RPU]: Periodic Task 5 DEADLINE EXCEEDED");
	         if(first==1)first = 0;
	     }
}

