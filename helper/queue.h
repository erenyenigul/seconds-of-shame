#include "queue.c"

void queue_init(queue_t **queue_, int size);
int queue_push(queue_t *queue, int i);
int queue_pop(queue_t *queue);
int queue_size(queue_t *queue);