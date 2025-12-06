#ifndef QUEUE_H_
#define QUEUE_H_

#include "types.h"

typedef struct QueueNode_ QueueNode;
struct QueueNode_ {
    QueueNode *next;
};

typedef struct {
    QueueNode *head;
    QueueNode *tail;
    u32 count;
} Queue;

void queue_init(Queue *q);
void queue_add(Queue *q, QueueNode *node);
QueueNode *queue_poll(Queue *q);

#endif // QUEUE_H_
