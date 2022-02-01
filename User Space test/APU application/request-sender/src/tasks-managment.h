//Commands and Tasks define
#define RPU_LOAD	    0XFF56A55A
#define SHUTDOWN_MSG    0xEF56A55A
//Task definition
#define TASK_1		0X00000001
#define STOP_TASK_1	0X00010000
#define TASK_2 		0x00000002
#define STOP_TASK_2	0X00020000
#define TASK_3		0X00000003
#define STOP_TASK_3	0X00030000
#define TASK_4 		0x00000004
#define STOP_TASK_4	0X00040000
#define TASK_5 		0x00000005
#define STOP_TASK_5	0X00050000

//Max lenght of Data Buffer  
#define MAX_DATA_LENGHT 492     //512 - 16(header) - 4(command) 

//Number of existing Task on RPU
#define RPU_N_TASK 5   

//The buffer dimension is 512 bytes, but only 496 bytes are really available (OpenAMP limitation)
struct _payload {
	uint32_t command;				//32 bit (4 bytes)
	char data[MAX_DATA_LENGHT];		//payload data  (492 byte)
};

/*Tasks Managment*/
struct RPU_Task{
	int 	 index;				// Index of the task (a simple integer)
	uint32_t code;				// Code needed to require the Task (same on RPU)
	uint32_t stop_code;			// Code to stop the task
	uint32_t load;				// Task load 
};

//RPU tasks (code + load)
static struct RPU_Task Task_vector[RPU_N_TASK] = 
{
	{1,TASK_1, STOP_TASK_1, 34},		//Task 1
	{2,TASK_2, STOP_TASK_2, 25},		//Task 2
	{3,TASK_3, STOP_TASK_3, 17},		//Task 3
	{4,TASK_4, STOP_TASK_4, 20},		//Task 4
	{5,TASK_5, STOP_TASK_5, 10}		    //Task 5
};

//Print the log on the screen
static int printscreen = 1;

int rpu_load_request(const int task_index, struct rpmsg_endpoint lept);

int feasibility_check(const int task_index, uint32_t load);

int task_offloading(const int task_index, const char *data_payload, struct rpmsg_endpoint lept);

int stop_task(const int task_index, struct List *Lista, struct rpmsg_endpoint lept);

int shutdown_RPU(struct rpmsg_endpoint lept);
