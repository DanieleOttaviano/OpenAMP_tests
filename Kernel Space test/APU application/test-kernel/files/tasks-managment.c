#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include"list-test-kernel.h"
#include "tasks-managment.h"

//Number of requested tasks
static uint32_t n_Tasks = 0;
//self-incremental ID 
static uint32_t ID = 1;

int task_offloading(const uint32_t task_code, const char *data_payload, struct List *Lista, const int fd)
{
	int bytes_rcvd, bytes_sent, j;
	uint32_t load, U_lub, new_load, address;

	//Endpoint verification 
	if (fd < 0) {
		if(printscreen) printf("[APU]: Endpoint doesn't exist.\n");
		return -1;
	}

	//Task existence verification
	j = 0;
	while(Task_vector[j].code != task_code){
		j++;
		if(j == RPU_N_TASK){
			if(printscreen) printf("[APU]: Task code doesn't match! (0x%08x)\n",task_code);
			return -1;
		}		
	}

	//Payload Initialization to send e receive data
	struct _payload *i_payload = (struct _payload *)malloc(sizeof(struct _payload));
	struct _payload *r_payload = (struct _payload *)malloc(sizeof(struct _payload));
	if (i_payload == 0 || r_payload == 0) {
		if(printscreen) printf("[APU]: ERROR: Failed to allocate memory for payload.\n");
		return -1;
	}
	

	//Send command: RPU_LOAD to receive the current RPU load
	i_payload->command = RPU_LOAD;
	if(printscreen) printf("\n[APU]: Master: request for RPU current Load.\n");
	bytes_sent = write(fd, i_payload, sizeof(uint32_t));	//Only command is needed
	if (bytes_sent <= 0) {
		if(printscreen) printf("\r\n Error sending data");
		if(printscreen) printf(" .. \r\n");
		return -1;
	}	
	if(printscreen) printf("[APU]: sent : %d bytes\n", bytes_sent);
	if(printscreen) printf("[APU]: Master: wait for data.\n");
	bytes_rcvd = read(fd, r_payload, sizeof(r_payload));
	while (bytes_rcvd <= 0) {
		usleep(10000);
		bytes_rcvd = read(fd, r_payload, sizeof(r_payload));
	}
	if(printscreen) printf("\n[APU]: received load: ");
	if(printscreen) printf("%d%% (size: %d bytes)\r\n\n", r_payload->command, bytes_rcvd);
	load = r_payload->command;

	//Feasibility Test (Liu and Layland)
	n_Tasks++;											//Number of tasks
	U_lub = (100 * (n_Tasks*(pow(2,1./n_Tasks)-1)));	//Least Upper Bound
	new_load = load + Task_vector[j].load;				//RPU load with the new task
	
	if ( new_load >  U_lub){							//Liu and Layland test
		if(printscreen) printf("\n[APU]: UNSCHEDULABLE!\n");
		if(printscreen) printf("[APU]: RPU load: %d%%->%d%%\n",load, new_load);
		if(printscreen) printf("[APU]: U_lub (with N=%d tasks): %d%%\n",n_Tasks, U_lub);
		n_Tasks--;
		return -1;
	}
	else{
		if(printscreen) printf("\n[APU]: SCHEDULABLE!\n");
		if(printscreen) printf("[APU]: RPU load: %d%%->%d%%\n",load, new_load);
		if(printscreen) printf("[APU]: U_lub (with N=%d tasks): %d%%\n",n_Tasks, U_lub);
	}
	

	//Send command:task_code to require the offloading of a task
	i_payload->command = Task_vector[j].code;
	strncpy(i_payload->data, data_payload, MAX_DATA_LENGHT);
	if(printscreen) printf("\n[APU]: Master: Offloading task request.\n");
	bytes_sent = write(fd, i_payload, sizeof(uint32_t) + MAX_DATA_LENGHT*sizeof(char));
	if (bytes_sent <= 0) {
		if(printscreen) printf("\r\n [APU]: Error sending data");
		if(printscreen) printf(" .. \r\n");
		return -1;
	}	
	if(printscreen) printf("[APU]: sent : %d bytes\n", bytes_sent);
	if(printscreen) printf("[APU]: Master: Wait for address.\n");
	bytes_rcvd = read(fd, r_payload, sizeof(uint32_t) + MAX_DATA_LENGHT*sizeof(char));
	while (bytes_rcvd <= 0) {
		usleep(10000);
		bytes_rcvd = read(fd, r_payload, sizeof(uint32_t) + MAX_DATA_LENGHT*sizeof(char));
	}
	if(printscreen) printf("\n[APU]: received address: ");
	if(printscreen) printf("0x%08x (size: %d bytes)\r\n\n", r_payload->command, bytes_rcvd);
	address = r_payload->command;

	//to do.... CREO NUOVO END POINT PER IL TASK CREATO


	//Create a new instance of Requested_task and insert it in the list
	uint32_t id_vm = ID; 						//Da sostituire con un vero ID_VM
	push_list(Lista, ID, id_vm, address, task_code);
	ID++;

	//Clean the memory 
	free(i_payload);
	free(r_payload);

	return 0;
}

int stop_task(const uint32_t ID_Task, struct List *Lista, const int fd){
	int bytes_rcvd, bytes_sent, j;
	uint32_t command, task_code;

	//Endpoint verification 
	if (fd < 0) {
		if(printscreen) printf("[APU]: Endpoint doesn't exist.\n");
		return -1;
	}

	j = 0;
	task_code = code_from_ID(Lista, ID_Task);
	if(task_code == 0){
		if(printscreen) printf("[APU]: ID_Task error during delete function\n");
		return -1;
	}

	//Task existence verification
	j = 0;
	while(Task_vector[j].code != task_code){
		j++;
		if(j == RPU_N_TASK){
			if(printscreen) printf("[APU]: can't stop the task, Task code doesn't match! (0x%08x)\n",task_code);
			return -1;
		}		
	}	

	//Payload Initialization to send e receive data
	struct _payload *i_payload = (struct _payload *)malloc(sizeof(struct _payload));
	struct _payload *r_payload = (struct _payload *)malloc(sizeof(struct _payload));
	if (i_payload == 0 || r_payload == 0) {
		if(printscreen) printf("[APU]: ERROR: Failed to allocate memory for payload.\n");
		return -1;
	}
	
	//Send command to stop the Task
	i_payload->command = Task_vector[j].stop_code;;
	if(printscreen) printf("\n[APU]: Master: request to stop task with ID:%d.\n",ID_Task);
	bytes_sent = write(fd, i_payload, sizeof(uint32_t));	//Only command is needed
	if (bytes_sent <= 0) {
		if(printscreen) printf("\r\n Error sending data");
		if(printscreen) printf(" .. \r\n");
		return -1;
	}	
	if(printscreen) printf("[APU]: sent : %d bytes\n", bytes_sent);
	if(printscreen) printf("[APU]: Master: wait for confirmation.\n");
	bytes_rcvd = read(fd, r_payload, sizeof(r_payload));
	while (bytes_rcvd <= 0) {
		usleep(10000);
		bytes_rcvd = read(fd, r_payload, sizeof(r_payload));
	}

	//0x00000000 -> all done
	//0x00000001 -> Error in deleting the Task
	if(r_payload->command == 0x00000000){
		if(printscreen) printf("\n[APU]: Confermation received: ");
		if(printscreen) printf("%d (size: %d bytes)\r\n\n", r_payload->command, bytes_rcvd);
	}
	else{
		if(printscreen) printf("\n[APU]: ERROR: cannot eliminate the Task");
		return -1;
	}
	
	//Delete the task from Requested_task vector
	remove_by_value_list(Lista, ID_Task);
	if(printscreen) printf("\n[APU]: Task with ID: %d has been removed from List\n",ID_Task);

	//Number of task decrese 
	n_Tasks--;

	//Clean the memory 
	free(i_payload);
	free(r_payload);

	return 0;
}

int shutdown_RPU(struct List *Lista, const int fd){
	int bytes_rcvd, bytes_sent;

	//Endpoint verification 
	if (fd < 0) {
		if(printscreen) printf("[APU]: Endpoint doesn't exist.\n");
		return -1;
	}

	//Payload Initialization to send e receive data
	struct _payload *i_payload = (struct _payload *)malloc(sizeof(struct _payload));
	struct _payload *r_payload = (struct _payload *)malloc(sizeof(struct _payload));
	if (i_payload == 0 || r_payload == 0) {
		if(printscreen) printf("[APU]: ERROR: Failed to allocate memory for payload.\n");
		return -1;
	}
	
	//Send command to stop the Task
	i_payload->command = SHUTDOWN_MSG;
	if(printscreen) printf("\n[APU]: Master: request to SHUTDOWN.\n");
	bytes_sent = write(fd, i_payload, sizeof(uint32_t));	//Only command is needed
	if (bytes_sent <= 0) {
		if(printscreen) printf("\r\n Error sending data");
		if(printscreen) printf(" .. \r\n");
		return -1;
	}	
	if(printscreen) printf("[APU]: sent : %d bytes\n", bytes_sent);
	if(printscreen) printf("[APU]: Master: wait for confirmation.\n");
	bytes_rcvd = read(fd, r_payload, sizeof(r_payload));
	while (bytes_rcvd <= 0) {
		usleep(10000);
		bytes_rcvd = read(fd, r_payload, sizeof(r_payload));
	}

	//0x00000000 -> all done
	//0x00000001 -> Error in deleting the Task
	if(r_payload->command == 0x00000000){
		if(printscreen) printf("\n[APU]: Confermation received: ");
		if(printscreen) printf("%d (size: %d bytes)\r\n\n", r_payload->command, bytes_rcvd);
	}
	else{
		if(printscreen) printf("\n[APU]: ERROR: cannot SHUTDOWN");
		return -1;
	}
	
	//Delete the task from Requested_task vector
	delete_list(&Lista);
	if(printscreen) printf("\n[APU]: The List has been cleaned\n");

	//Number of task decrese 
	n_Tasks = 0;

	//Clean the memory 
	free(i_payload);
	free(r_payload);

	return 0;
}