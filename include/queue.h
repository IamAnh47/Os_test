#ifndef QUEUE_H
#define QUEUE_H

#include "common.h"

#define MAX_QUEUE_SIZE 10

struct queue_t {
    struct pcb_t *proc[MAX_QUEUE_SIZE];
    int size;
    struct pcbList *head; // Con trỏ tới nút đầu tiên trong danh sách
    struct pcbList *tail; // Con trỏ tới nút cuối cùng trong danh sách
};
struct pcbList {
    struct pcb_t *proc; // Con trỏ tới quy trình (process)
    struct pcbList *next; // Con trỏ tới nút tiếp theo trong danh sách
    struct pcbList *prev; // Con trỏ tới nút trước đó trong danh sách
};


void enqueue(struct queue_t *q, struct pcb_t *proc);
struct pcb_t *dequeue(struct queue_t *q);
int empty(struct queue_t *q);

#endif

