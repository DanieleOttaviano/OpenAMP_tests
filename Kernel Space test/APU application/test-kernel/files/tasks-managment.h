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
	uint32_t code;				// Code needed to require the Task (same on RPU)
	uint32_t stop_code;			// Code to stop the task
	uint32_t load;				// Task load 
};

//RPU tasks (code + load)
static struct RPU_Task Task_vector[RPU_N_TASK] = 
{
	{TASK_1, STOP_TASK_1, 34},		//Task 1
	{TASK_2, STOP_TASK_2, 25},		//Task 2
	{TASK_3, STOP_TASK_3, 17},		//Task 3
	{TASK_4, STOP_TASK_4, 20},		//Task 4
	{TASK_5, STOP_TASK_5, 10}		//Task 5
};

//Print the log on the screen
static int printscreen = 1;

int task_offloading(const uint32_t task_code, const char *data_payload, struct List *Lista, const int fd);

int stop_task(const uint32_t ID_Task, struct List *Lista, const int fd);

int shutdown_RPU(struct List *Lista, const int fd);