#include "queue.h"
#include "sched.h"
#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>

static struct queue_t ready_queue;
static struct queue_t run_queue;
static pthread_mutex_t queue_lock;

#ifdef MLQ_SCHED
static struct queue_t mlq_ready_queue[MAX_PRIO];

#endif

int queue_empty(void) {
#ifdef MLQ_SCHED
	unsigned long prio;
	for (prio = 0; prio < MAX_PRIO; prio++)
		if (!empty(&mlq_ready_queue[prio]))
			return -1;
#endif
	return (empty(&ready_queue) && empty(&run_queue));
}

void init_scheduler(void) {
#ifdef MLQ_SCHED
	int i;
	for (i = 0; i < MAX_PRIO; i++)
		mlq_ready_queue[i].size = 0;
#endif
	ready_queue.size = 0;
	run_queue.size = 0;
	pthread_mutex_init(&queue_lock, NULL);
}

#ifdef MLQ_SCHED
struct pcb_t *get_mlq_proc(void) {
	pthread_mutex_lock(&queue_lock);
	struct pcb_t *proc = NULL;
	for (int i = 0; i < MAX_PRIO - 1; i++) {

		if (mlq_ready_queue[i].size > 0) {
			proc = dequeue(&mlq_ready_queue[i]);
			if (empty(&mlq_ready_queue[i])) {
				i++;
				if (i >= MAX_PRIO - 1)
					i = 1;
			}
			break;
		}
	}
	pthread_mutex_unlock(&queue_lock);
	return proc;
}
void put_mlq_proc(struct pcb_t *proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_mlq_proc(struct pcb_t *proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}
#endif

// Các hàm wrapper cho MLQ scheduling
struct pcb_t *get_proc(void) {
#ifdef MLQ_SCHED
	return get_mlq_proc();
#else
	// Nếu không sử dụng MLQ, có thể triển khai lập lịch khác ở đây
	return NULL;
#endif
}

void put_proc(struct pcb_t *proc) {
#ifdef MLQ_SCHED
	put_mlq_proc(proc);
#endif
}

void add_proc(struct pcb_t *proc) {
#ifdef MLQ_SCHED
	add_mlq_proc(proc);
#endif
}
