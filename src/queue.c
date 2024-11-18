#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */
        if(q && proc)
        {
        	if(!q->size)
        	{
        		if(!q->head)
        		q->head = (struct pcbList*)malloc(sizeof(struct pcbList));
        		q->head->proc = proc;
        		q->head->next = q->head->prev = NULL;
        		q->tail = q->head;
        	}
        	else
        	{
        		struct pcbList* temp = (struct pcbList*)malloc(sizeof(struct pcbList));
        		temp->proc = proc;
        		temp->next = temp->prev = NULL;
        		q->tail->next = temp;
        		temp->prev = q->tail;
        		q->tail = temp;
        	}
        	q->size++;
        }
}

struct pcb_t * dequeue(struct queue_t * q) {
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */
         if(q&& !empty(q) && q->head)
         {
         	struct pcb_t* result = q->head->proc;
         	if(q->head->next)
         	{
         		q->head = q->head->next;
         		free(q->head->prev);
         		q->head->prev = NULL;
         	}
         	else
         	{
         		free(q->head);
         		q->head = q->tail = NULL;
         	}
         	q->size--;
         	return result;
         }
         	
	return NULL;
}
