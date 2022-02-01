//LIST: it contains all the requested tasks running on RPU 

typedef struct Requested_task{
	uint32_t ID_task;					//unique ID
	uint32_t ID_VM;						//VM how requested the TASK	
	uint32_t channel_address;			//address for the communication
	uint32_t code;						//code to request the task (identify the requested task)
    struct Requested_task *next;		
}node_t;

struct List{
    node_t * head;						//Head of the List
    node_t * tail;						//Tail of the List
    int size;    
};

/*LIST INTERFACES*/

struct List * init_list();

void push_list(struct List *Lista, uint32_t ID_TASK, uint32_t ID_VM, uint32_t addr, uint32_t code);

int pop_list(struct List *Lista);

void print_list(struct List *Lista);

int remove_by_value_list(struct List *Lista, uint32_t ID_TASK);

void delete_list(struct List **Lista);

uint32_t code_from_ID(struct List *Lista, uint32_t ID_Task);
