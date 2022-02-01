/* This is a sample demonstration application that showcases usage of rpmsg
This application is meant to run on the remote CPU running baremetal code.
This application echoes back data that was sent to it by the master core. */

//Xilinx
#include "xil_printf.h"
#include "xtime_l.h"
//OpenAMP-Libmetal
#include <openamp/open_amp.h>
#include <metal/alloc.h>
#include "rsc_table.h"
#include "platform_info.h"
#include "rpmsg-echo.h"
//freeRTOS
#include "FreeRTOS.h"
#include "task.h"

#include "RPU_task.h"

//Commands and Tasks definitions
#define SHUTDOWN_MSG	0xEF56A55A
#define LOAD_MSG		0XFF56A55A

#define MAX_DATA_LENGHT 492

//Print end errors
#define LPRINTF(format, ...) xil_printf(format, ##__VA_ARGS__)
#define LPERROR(format, ...) LPRINTF("ERROR: " format, ##__VA_ARGS__)

//Time definitions
#define DELAY_1_SECOND		1000UL

//Static variable
static struct rpmsg_endpoint lept;		//Endpoint
static int shutdown_req = 0;			//Request to shutdown
static uint32_t load = 0;				//Current RPU load

/* External functions */
extern int init_system(void);
extern void cleanup_system(void);

//Print the log on the screen
static int printscreen = 1;

//Payload struct received from remote processor
struct _payload {
	uint32_t command;				//32 bit (4 bytes)
	char data[MAX_DATA_LENGHT];		//dati del payload  (492 bytes)
};

//Shared Variables
static struct _payload shared_variable;		//A shared variable for each channel is needed
static struct rpmsg_endpoint shared_ept;	//A Shared endpoint for each channel is needed

//TaskManager handle
static TaskHandle_t comm_task;

/*-----------------------------------------------------------------------------*
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------*/
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
				 uint32_t src, void *priv)
{
	//TickType_t Start;
	//TickType_t End;

	//Start = xTaskGetTickCount();

	//(void)priv;
	//(void)src;

	/*Just copy the received data in a shared resource to reduce the interruption interference*/
	struct _payload *payload = (struct _payload *)data;
	shared_variable = (*payload);
	shared_ept = *ept;

	//End = xTaskGetTickCount();
	//DEBUG: callback execution time
	//LPRINTF("\n [RPU]:CALLBACK awake\n");
	//LPRINTF(" [RPU]:CALLBACK start: %d\n", Start);
	//LPRINTF(" [RPU]:CALLBACK end:   %d\n", End);

	return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
	(void)ept;
	LPRINTF("[RPU]: unexpected Remote endpoint destroy\n");
	shutdown_req = 1;
}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
int TaskManager(struct rpmsg_device *rdev, void *priv)
{
	int ret,j;
	int task_load;					//load of the requested Task
	BaseType_t state;				//Starting Task state
	uint32_t ans;					//Answer to delete request
	uint32_t addr = 0;				//Auto-increment Address for channels

	/*Variables for TaskManager frequency*/
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 100;

	/*Shared variable initialization*/
	shared_variable.command = 0x00000000;

	/* Initialize RPMSG framework */
	if(printscreen) LPRINTF("[RPU]: TASK MANAGER-> Try to create rpmsg endpoint.\n");
	ret = rpmsg_create_ept(&lept, rdev, RPMSG_SERVICE_NAME,
				0, RPMSG_ADDR_ANY, rpmsg_endpoint_cb,
				rpmsg_service_unbind);
	if (ret) {
		LPERROR("[RPU]: TASK MANAGER-> Failed to create endpoint.\n");
		return -1;
	}
	if(printscreen) LPRINTF("[RPU]: TASK MANAGER-> Successfully created rpmsg endpoint.\n");

	/* Initialize the xLastWakeTime variable with the current time.*/
	xLastWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		platform_poll(priv);

		// Wait for the next cycle.
		vTaskDelayUntil( &xLastWakeTime, xFrequency);

		/* Perform action here.*/

		// Wait for messages from
		platform_poll(priv);

		if(shared_variable.command == 0x00000000){
			if(printscreen) LPRINTF("\n[RPU]: No Requests.\n");
		}

		/* On reception of a shutdown we signal the application to terminate */
		else if (shared_variable.command == SHUTDOWN_MSG) {
			if(printscreen) LPRINTF("\n[RPU]: shutdown message is received.\n");

			/*Shutdown the tasks*/
			j=0;
			while(j<RPU_N_TASK){
				if(Task_vector[j].running == 1){
					vTaskDelete(*Task_vector[j].Handler);						//Delete the Task
					load = load - Task_vector[j].load;							//Remove task load from RPU load
					Task_vector[j].running = 0;									//Task j is not more running
					if(printscreen) LPRINTF("[RPU]: %s deleted\n", Task_vector[j].name);
				}
				j++;
			}

			ans = 0x00000000;  											//All done!
			rpmsg_send(&shared_ept, &ans, sizeof(uint32_t));

			break;
		}

		else if(shutdown_req == 1){
			printf("\n[RPU]: Unexpected shutdown.\n");
			break;
		}

		/*Load Request*/
		else if(shared_variable.command == LOAD_MSG){
			if(printscreen) LPRINTF("\n[RPU]: Load request is received.\n");
			if(printscreen) LPRINTF("[RPU]: Current Load: %ld%%\n", load);
			if(printscreen) LPRINTF("[RPU]: Sending data...\n");
			rpmsg_send(&shared_ept, &load, sizeof(unsigned long));
		}

		/*Offloading task request*/
		else if (shared_variable.command < 0x0001000){
			if(printscreen) LPRINTF("\n[RPU]: Offload task request is received.\n");
			if(printscreen) LPRINTF("[RPU]: Received data: %s \n",shared_variable.data);	//Debug

			j=0;
			while(Task_vector[j].code != shared_variable.command && j<RPU_N_TASK){
				j++;
			}
			/*Task request doesn't match!*/
			if(j == RPU_N_TASK){
				if(printscreen) LPRINTF("[RPU]: The requested Task doesn't exist!\n");
			}
			/*Same Task request (to change.....)*/
			else if(Task_vector[j].running == 1){
				if(printscreen) LPRINTF("[RPU]: Task already running!\n");
			}
			/*Task request is correct*/
			else{
				addr = addr + 1;											//Address auto-increment

				task_load = Task_vector[j].load;
				load = load + task_load;

				state = xTaskCreate(Task_vector[j].function , ( const char * ) Task_vector[j].name,
						configMINIMAL_STACK_SIZE, NULL, Task_vector[j].priority , Task_vector[j].Handler);
				if (state != pdPASS) {
					LPERROR("[RPU]: cannot create task\n");
				}
				else{
					Task_vector[j].running = 1;
					if(printscreen) LPRINTF("[RPU]: %s started. (address = 0x%08x)\n",Task_vector[j].name ,addr);

					if(printscreen) LPRINTF("[RPU]: Sending address data...\n");
					rpmsg_send(&shared_ept, &addr, sizeof(uint32_t));
				}
			}
		}

		/*Delete task request*/
		else{
			if(printscreen) LPRINTF("\n[RPU]: Stop task request is received.\n");

			j=0;
			while(Task_vector[j].stop_code != shared_variable.command && j<RPU_N_TASK){
				j++;
			}

			/*Task request doesn't match!*/
			if(j == RPU_N_TASK){
				if(printscreen) LPRINTF("[RPU]: The requested Task doesn't exist!\n");
				ans = 0x00000001;											//Error in deleting the Task
			}
			/*Task request is correct*/
			else{
				vTaskDelete(*Task_vector[j].Handler);						//Delete the Task
				load = load - Task_vector[j].load;							//Remove task load from RPU load
				Task_vector[j].running = 0;									//Task j is not more running
				ans = 0x00000000;  											//All done!
				if(printscreen) LPRINTF("[RPU]: %s deleted\n", Task_vector[j].name);
			}

			if(printscreen) LPRINTF("[RPU]: Sending reply...\n");
			rpmsg_send(&shared_ept, &ans, sizeof(uint32_t));
		}
		//Reinitialize shared_variable command
		shared_variable.command = 0x00000000;
	}

	rpmsg_destroy_ept(&lept);

	return 0;
}

//int main2(void);
/*-----------------------------------------------------------------------------*
 *  Processing Task
 *-----------------------------------------------------------------------------*/
static void processing(void *unused_arg)
{
	void *platform;
	struct rpmsg_device *rpdev;
	const TickType_t x1second = pdMS_TO_TICKS( DELAY_1_SECOND );


	LPRINTF("[RPU]: Starting application...\n");
	/* Initialize platform */
	if (platform_init(NULL, NULL, &platform)) {
		LPERROR("[RPU]: Failed to initialize platform.\n");
	} else {
		//main2();
		rpdev = platform_create_rpmsg_vdev(platform, 0,
										VIRTIO_DEV_SLAVE,
										NULL, NULL);
		if (!rpdev){
			LPERROR("[RPU]: Failed to create rpmsg virtio device.\n");
		} else {
			TaskManager(rpdev, platform);
			platform_release_rpmsg_vdev(rpdev);
		}
	}
	while (1){
		vTaskDelay( x1second );
		LPRINTF(".");
	}

	LPRINTF("[RPU]: Stopping application...\n");
	platform_cleanup(platform);	//why commented?

	/* Terminate this task */
	vTaskDelete(NULL);
}

/*-----------------------------------------------------------------------------*
 *  Application entry point
 *-----------------------------------------------------------------------------*/
int main(void)
{
	BaseType_t stat;

	/* COMMENTARE SE NON SI USA IL DEBUGGER (attach on running target) DI SDK! */
	//while (debug_var == 123) ;

	/* Create the tasks */
	stat = xTaskCreate(processing, ( const char * ) "HW2",
				1024, NULL, 2, &comm_task);
	LPRINTF("[RPU]: vTaskStartScheduler...\n");
	if (stat != pdPASS) {
		LPERROR("[RPU]: cannot create task\n");
	} else {
		/* Start running FreeRTOS tasks */
		vTaskStartScheduler();
	}

	/* Will not get here, unless a call is made to vTaskEndScheduler() */
	while (1) ;

	/* suppress compilation warnings*/
	return 0;
}
