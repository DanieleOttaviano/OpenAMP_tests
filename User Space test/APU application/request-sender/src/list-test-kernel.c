#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
 
#include "list-test-kernel.h"

/*LIST FUNCTIONS*/
struct List * init_list(){
	struct List *Lista = (struct List *) malloc(sizeof(struct List));
	Lista->head = NULL;
	Lista->tail = NULL;
	Lista->size = 0;

	return Lista;
}

void push_list(struct List *Lista, uint32_t ID_TASK, uint32_t ID_VM, uint32_t addr, uint32_t code){
    if(Lista == NULL){
        printf("Attempting to read a deallocating list\n");
        return;
    }

    if(Lista->size == 0)
    {
        Lista->head = (node_t *) malloc(sizeof(node_t));
        Lista->head->ID_task = ID_TASK;
        Lista->head->ID_VM = ID_VM;
        Lista->head->channel_address = addr;
        Lista->head->code =  code;
        Lista->head->next = NULL; 
        Lista->tail = Lista->head;

        Lista->size = 1;
    }
    else{
        Lista->tail->next = (node_t *) malloc(sizeof(node_t));
        Lista->tail = Lista->tail->next;

        Lista->tail->ID_task = ID_TASK;
        Lista->tail->ID_VM = ID_VM;
        Lista->tail->channel_address = addr;
        Lista->tail->code = code;
        Lista->tail->next = NULL;

        Lista->size++;
    }
}

int pop_list(struct List *Lista) {
    if(Lista == NULL){
        printf("Attempting to read a deallocating list\n");
        return -1;
    }

    int retval = -1;
    node_t * next_node = NULL;

    if (Lista->head == NULL) {
        return -1;
    }

    next_node = Lista->head->next;
    retval = Lista->head->ID_task;
    free(Lista->head);
    Lista->head = next_node;

    Lista->size--;

    return retval;
}

void print_list(struct List *Lista) {
    if(Lista == NULL){
        printf("Attempting to read a deallocating list\n");
        return;
    }

    node_t * current = Lista->head;

    while (current != NULL) {
        printf("ID_TASK: %d\t", current->ID_task);
        printf("ID_VM:   %d\t", current->ID_VM);
        printf("Address: %d\t", current->channel_address);
        printf("Code:    %d\n", current->code);
        current = current->next;
    }

    printf("List size = %d\n\n", Lista->size);
}

int remove_by_value_list(struct List *Lista, uint32_t ID_TASK) {
    if(Lista == NULL){
        printf("Attempting to read a deallocating list\n");
        return -1;
    }
    node_t *previous, *current;

    if (Lista->head == NULL) {
        return -1;
    }

    if (Lista->head->ID_task == ID_TASK) {
        return pop_list(Lista);
    }

    previous = Lista->head;
    current = Lista->head->next;
    while (current) {
        if (current->ID_task == ID_TASK) {
            previous->next = current->next;
            free(current);
            Lista->size--;
            return ID_TASK;
        }

        previous = current;
        current  = current->next;
    }
    return -1;
}

void delete_list(struct List **Lista) {
    if(Lista == NULL){
        printf("Attempting to read a deallocating list\n");
        return;
    }
    while ((*Lista)->size > 0) {
        pop_list(*Lista);
        //print_list(*Lista); 
    }
    (*Lista)->head = NULL;
    (*Lista)->tail = NULL;
    (*Lista)->size = 0;

    free(*Lista);

    *Lista = NULL;
}

uint32_t code_from_ID(struct List *Lista, uint32_t ID_Task){
    if(Lista == NULL){
        printf("Attempting to read a deallocating list\n");
        return -1;
    }

	node_t * current = Lista->head;
	
	while(current->ID_task != ID_Task && current != NULL){
		current = current->next;
	}

	if(current->ID_task == ID_Task){
		return current->code;
	}
	else{
		return 0;
	}
}