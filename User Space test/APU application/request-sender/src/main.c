/*Include Section*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openamp/open_amp.h>
#include <metal/alloc.h>

//#include <linux/rpmsg.h>
//#include <sys/ioctl.h>
//#include <time.h>
//#include <fcntl.h>
//#include <dirent.h>
//#include <errno.h>
//#include <stdint.h>

/*Include in src directory*/
#include "platform_info.h"
#include "main.h"
#include "list-test-kernel.h"
#include "tasks-managment.h"

/*Print definition*/
#define LPRINTF(format, ...) printf(format, ##__VA_ARGS__)
#define LPERROR(format, ...) LPRINTF("ERROR: " format, ##__VA_ARGS__)

/* Globals */
static struct rpmsg_endpoint lept;
static int ept_deleted = 0;

//Shared Variables
static struct _payload r_payload;			//A shared variable that has the data received from RPU

//self-incremental ID
static uint32_t ID = 1;

/*Clean for scanf check*/
int clean_stdin()
{
    while (getchar()!='\n');
    return 1;
}

/*-----------------------------------------------------------------------------*
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------*/
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
			     uint32_t src, void *priv)
{
	(void)ept;
	(void)src;
	(void)priv;

	//Save the data received in r_payload
	struct _payload *payload = (struct _payload *)data;
	r_payload = (*payload);


	return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
	(void)ept;
	rpmsg_destroy_ept(&lept);
	LPRINTF("Service is destroyed\r\n");
	ept_deleted = 1;
}

static void rpmsg_name_service_bind_cb(struct rpmsg_device *rdev,
				       const char *name, uint32_t dest)
{
	LPRINTF("new endpoint notification is received.\r\n");
	if (strcmp(name, RPMSG_SERVICE_NAME))
		LPERROR("Unexpected name service %s.\r\n", name);
	else
		(void)rpmsg_create_ept(&lept, rdev, RPMSG_SERVICE_NAME,
				       RPMSG_ADDR_ANY, dest,
				       rpmsg_endpoint_cb,
				       rpmsg_service_unbind);

}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
int app (struct rpmsg_device *rdev, void *priv)
{
    /*Variables*/
	int ret = 0;
	int index = 0;
    int task_index = 0;
    char c;
    uint32_t load, address;

    /*List for current task informations*/
    struct List *Lista = init_list();

	LPRINTF("******TEST START******\n");

	/* Create RPMsg endpoint */
	LPRINTF("Try to create endpoint ...\n");
	ret = rpmsg_create_ept(&lept, rdev, RPMSG_SERVICE_NAME,
			       RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
			       rpmsg_endpoint_cb, rpmsg_service_unbind);
	if (ret) {
		LPERROR("Failed to create RPMsg endpoint.\r\n");
		//metal_free_memory(i_payload);
		return ret;
	}
	while (!is_rpmsg_ept_ready(&lept))
		platform_poll(priv);
	LPRINTF("RPMSG endpoint is binded with remote.\r\n");

	/*List Initialization to do...*/


	while(1){
		/*Menu*/
		LPRINTF("Test for RPU communication, choose the option:\n");
		LPRINTF("1 - Require Task on RPU\n");
		LPRINTF("2 - Stop Task in RPU\n");
		LPRINTF("3 - End test\n");
        index = 0;
        do{
            printf("\nEnter an integer from 1 to 3: ");
        } while (((scanf("%d%c", &index, &c)!=2 || c!='\n') && clean_stdin()) || index<1 || index>3);

		LPRINTF("\nYou choose %d\n",index);

        switch (index)
        {
        case 1:
            /*REQUIRE TASK PROCEDURE*/
            LPRINTF("Which task do you require?%d\n",task_index);
            do{
                printf("\nEnter the task (1 to %d): ",(int)RPU_N_TASK);
            } while (((scanf("%d%c", &task_index, &c)!=2 || c!='\n') && clean_stdin()) || task_index<1 || task_index>RPU_N_TASK);

            /*Require RPU load*/
            rpu_load_request(task_index,lept);
            LPRINTF("Data Sent to RPU, wait for answer\r\n");

            //Wait for answer
            platform_poll(priv);
            load = r_payload.command;
        	LPRINTF("\n[APU]: received load: %d%%\r\n\n", load);

        	//Feasibility Check
        	if(feasibility_check(task_index,load) == 0){
        		break;
        	}
        	else{
        		// Task request
        		ret = task_offloading(task_index,"MESSAGGIO PROVA",lept);
        		if(ret<0){
        			LPRINTF("[APU] Error in Task offloading\n");
        		}
        		if(printscreen) LPRINTF("[APU]: Master: Wait for address.\n");

        		//Wait for address
        		platform_poll(priv);
        		address = r_payload.command;
        		if(printscreen) printf("\n[APU]: received address: 0x%08x\r\n\n", r_payload.command);


        		//to do.... CREO NUOVO END POINT PER IL TASK CREATO


        		//Create a new instance of Requested_task and insert it in the list
        		uint32_t id_vm = ID; 						//Da sostituire con un vero ID_VM
        		push_list(Lista, ID, id_vm, address, task_index);
        		ID++;
        	}
            break;

        case 2:
            /*stop TASK*/
            LPRINTF("Which task do you want to stop?%d\n",task_index);
            do{
                printf("\nEnter the task (1 to %d): ",(int)RPU_N_TASK);
            } while (((scanf("%d%c", &task_index, &c)!=2 || c!='\n') && clean_stdin()) || task_index<1 || task_index>RPU_N_TASK);

            /*Stop request*/
            stop_task(task_index, Lista, lept);
        	LPRINTF("[APU]: Master: wait for confirmation.\n");

        	/*Wait for Confirmation*/
        		//0x00000000 -> all done
        		//0x00000001 -> Error in deleting the Task
        	platform_poll(priv);
        	if(r_payload.command == 0x00000000){
        		LPRINTF("\n[APU]: Confirmation received: %d \r\n\n", r_payload.command);
        	}
        	else{
        		LPRINTF("\n[APU]: ERROR: cannot eliminate the Task");
        		break;
        	}

        	//Delete the task from Requested_task vector
        	remove_by_value_list(Lista, task_index);
        	LPRINTF("\n[APU]: Task with INDEX: %d has been removed from List\n",task_index);

            break;
        case 3:
        	/* End RPU*/
        	LPRINTF("Send request to stop RPU\n");

            /*Request to stop RPU */
        	shutdown_RPU(lept);

        	/*Wait for Confirmation*/
				//0x00000000 -> all done
				//0x00000001 -> Error in deleting the Task
			platform_poll(priv);
        	if(r_payload.command == 0x00000000){
        		if(printscreen) LPRINTF("\n[APU]: Confermation received: %d \r\n\n", r_payload.command);
        	}
        	else{
        		if(printscreen) LPRINTF("\n[APU]: ERROR: cannot SHUTDOWN");
        		return -1;
        	}

        	//Delete the task from Requested_task vector
        	delete_list(&Lista);
        	if(printscreen) LPRINTF("\n[APU]: The List has been cleaned\n");

            break;

        default:
            LPRINTF("You should not be here.\n");
            break;
        }


	}

	LPRINTF("**********************************\r\n");
	LPRINTF(" Test Finished \r\n");
	LPRINTF("**********************************\r\n");
	/* Destroy the RPMsg endpoint */
	rpmsg_destroy_ept(&lept);
	LPRINTF("Quitting application ...\r\n");

	//metal_free_memory(i_payload);
	return 0;
}

int main(int argc, char *argv[])
{
	void *platform;
	struct rpmsg_device *rpdev;
	int ret;

	/* Initialize platform with error control*/
	ret = platform_init(argc, argv, &platform);
	if (ret) {
		LPERROR("Failed to initialize platform.\r\n");
		ret = -1;
	} else {
		rpdev = platform_create_rpmsg_vdev(platform, 0,
						  VIRTIO_DEV_MASTER,
						  NULL,
						  rpmsg_name_service_bind_cb);
		if (!rpdev) {
			LPERROR("Failed to create rpmsg virtio device.\r\n");
			ret = -1;
		} else {
			app(rpdev, platform);
			platform_release_rpmsg_vdev(rpdev, platform);
			ret = 0;
		}
	}

	LPRINTF("Stopping application...\r\n");
	platform_cleanup(platform);

	return ret;
}

