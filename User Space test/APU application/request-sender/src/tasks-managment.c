#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <openamp/open_amp.h>
#include <metal/alloc.h>

#include"list-test-kernel.h"
#include "tasks-managment.h"

/*Print definition*/
#define LPRINTF(format, ...) printf(format, ##__VA_ARGS__)
#define LPERROR(format, ...) LPRINTF("ERROR: " format, ##__VA_ARGS__)

//Number of requested tasks
static uint32_t n_Tasks = 0;

int rpu_load_request(const int task_index,struct rpmsg_endpoint lept)
{
	//int bytes_rcvd, bytes_sent;
	int ret;
	int j;
	uint32_t task_code;

	//Task existence verification
	j = 0;
	while(Task_vector[j].index != task_index){
		j++;
		if(j == RPU_N_TASK){
			if(printscreen) LPRINTF("[APU]: Task index doesn't match! (%d)\n",task_index);
			return -1;
		}		
	}
	task_code = Task_vector[j].code;

	//Payload Initialization to send e receive data
	struct _payload *i_payload = (struct _payload *)malloc(sizeof(struct _payload));
	if (i_payload == 0) {
		if(printscreen) LPRINTF("[APU]: ERROR: Failed to allocate memory for payload.\n");
		return -1;
	}
	

	//Send command: RPU_LOAD to receive the current RPU load
	i_payload->command = RPU_LOAD;
	if(printscreen) LPRINTF("\n[APU]: Master: request for RPU current Load.\n");
	ret = rpmsg_send(&lept, i_payload,sizeof(uint32_t)); //maybe *(i_payload->command)
	if (ret < 0) {
		LPERROR("Failed to send data...\r\n");
		return -1;
	}

	//Clean the memory
	free(i_payload);

	return 0;
}


int feasibility_check(const int task_index, uint32_t load){
	int j;
	uint32_t U_lub, new_load;

	//Task existence verification
	j = 0;
	while(Task_vector[j].index != task_index){
		j++;
		if(j == RPU_N_TASK){
			if(printscreen) LPRINTF("[APU]: Task index doesn't match! (%d)\n",task_index);
			return -1;
		}
	}

	//Feasibility Test (Liu and Layland)
	n_Tasks++;											//Number of tasks
	U_lub = (100 * (n_Tasks*(pow(2,1./n_Tasks)-1)));	//Least Upper Bound
	new_load = load + Task_vector[j].load;				//RPU load with the new task

	if ( new_load >  U_lub){							//Liu and Layland test
		if(printscreen) printf("\n[APU]: UNSCHEDULABLE!\n");
		if(printscreen) printf("[APU]: RPU load: %d%%->%d%%\n",load, new_load);
		if(printscreen) printf("[APU]: U_lub (with N=%d tasks): %d%%\n",n_Tasks, U_lub);
		n_Tasks--;
		return 0;
	}
	else{
		if(printscreen) printf("\n[APU]: SCHEDULABLE!\n");
		if(printscreen) printf("[APU]: RPU load: %d%%->%d%%\n",load, new_load);
		if(printscreen) printf("[APU]: U_lub (with N=%d tasks): %d%%\n",n_Tasks, U_lub);
		return 1;
	}
}

int task_offloading(const int task_index, const char *data_payload, struct rpmsg_endpoint lept){
	int ret;
	int j;
	uint32_t task_code;

	//Task existence verification
	j = 0;
	while(Task_vector[j].index != task_index){
		j++;
		if(j == RPU_N_TASK){
			if(printscreen) LPRINTF("[APU]: Task index doesn't match! (%d)\n",task_index);
			return -1;
		}
	}
	task_code = Task_vector[j].code;

	//Payload Initialization to send e receive data
	struct _payload *i_payload = (struct _payload *)malloc(sizeof(struct _payload));
	if (i_payload == 0) {
		if(printscreen) LPRINTF("[APU]: ERROR: Failed to allocate memory for payload.\n");
		return -1;
	}


	//Send command:task_code to require the offloading of a task
	i_payload->command = Task_vector[j].code;
	strncpy(i_payload->data, data_payload, MAX_DATA_LENGHT);
	if(printscreen) LPRINTF("\n[APU]: Master: Offloading task request.\n");
	ret = rpmsg_send(&lept, i_payload,sizeof(uint32_t) + MAX_DATA_LENGHT*sizeof(char));
	if (ret < 0) {
		LPERROR("Failed to send data...\r\n");
		return -1;
	}

	//Clean the memory
	free(i_payload);

	return 0;
}

int stop_task(const int task_index, struct List *Lista, struct rpmsg_endpoint lept){
	int j,ret;
	uint32_t task_code;

	// Verify the presence of the task in the List
	j = 0;
	task_code = code_from_ID(Lista, task_index);
	if(task_code == 0){
		if(printscreen) printf("[APU]: ID_Task error during delete function\n");
		return -1;
	}

	//Task existence verification
	j = 0;
	while(Task_vector[j].index != task_index){
		j++;
		if(j == RPU_N_TASK){
			if(printscreen) LPRINTF("[APU]: Task index doesn't match! (%d)\n",task_index);
			return -1;
		}
	}
	task_code = Task_vector[j].code;

	//Payload Initialization to send e receive data
	struct _payload *i_payload = (struct _payload *)malloc(sizeof(struct _payload));
	if (i_payload == 0) {
		if(printscreen) printf("[APU]: ERROR: Failed to allocate memory for payload.\n");
		return -1;
	}

	//Send command to stop the Task
	i_payload->command = Task_vector[j].stop_code;
	if(printscreen) LPRINTF("\n[APU]: Master: request to stop task:%d.\n",task_index);
	ret = rpmsg_send(&lept, i_payload,sizeof(uint32_t));
	if (ret < 0) {
		LPERROR("Failed to send data...\r\n");
		return -1;
	}

	//Number of task decrease
	n_Tasks--;

	//Clean the memory
	free(i_payload);

	return 0;

}

int shutdown_RPU(struct rpmsg_endpoint lept){
	int ret = 0;

	//Payload Initialization to send e receive data
	struct _payload *i_payload = (struct _payload *)malloc(sizeof(struct _payload));
	if (i_payload == 0) {
		if(printscreen) printf("[APU]: ERROR: Failed to allocate memory for payload.\n");
		return -1;
	}
	
	//Send command to stop the Task
	i_payload->command = SHUTDOWN_MSG;
	if(printscreen) LPRINTF("\n[APU]: Master: request to SHUTDOWN.\n");
	ret = rpmsg_send(&lept, i_payload,sizeof(uint32_t));
	if (ret < 0) {
		LPERROR("Failed to send data...\r\n");
		return -1;
	}

	//Number of task decrese
	n_Tasks = 0;

	//Clean the memory 
	free(i_payload);

	return 0;
}
