#include "queue.h"
#include "lib/types.h"

void queue_init(Queue *q) {
    q->head = NULL;
    q->tail = NULL;
    q->count = 0;
}

void queue_add(Queue *q, QueueNode *node) {
    if (q->tail)
        q->tail->next = node;
    else
        q->head = node;

    q->count += 1;
    node->next = NULL;
    q->tail = node;
}

QueueNode *queue_poll(Queue *q) {
    QueueNode *head = q->head;
    if (!head)
        return NULL;

    QueueNode *next = head->next;
    q->head = next;
    if (!next)
        q->tail = NULL;

    q->count -= 1;
    return head;
}
