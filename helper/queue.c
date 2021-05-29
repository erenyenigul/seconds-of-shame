#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct
{
    int *elems;
    int in;
    int count;
    int max_s;
    pthread_mutex_t lock;
} queue_t;

void queue_init(queue_t **queue_, int size)
{
    queue_t *queue = (queue_t *)malloc(sizeof(queue_t));
    queue->elems = (int *)malloc(sizeof(int) * size);
    queue->lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    queue->max_s = size;
    queue->in = 0;
    queue->count = 0;

    *queue_ = queue;
}

int queue_push(queue_t *queue, int i)
{
    pthread_mutex_lock(&(queue->lock));

    int return_value;
    int max_s = queue->max_s;

    if (queue->count >= max_s)
    {
        return_value = -1;
    }
    else
    {
        queue->elems[(queue->in + queue->count) % max_s] = i;
        queue->count++;

        return_value = queue->count;
    }
    pthread_mutex_unlock(&(queue->lock));
    return return_value;
}

int queue_pop(queue_t *queue)
{
    pthread_mutex_lock(&(queue->lock));
    int max_s = queue->max_s;
    int return_value;

    if (queue->count <= 0)
        return_value = -1;
    else
    {
        return_value = queue->elems[queue->in % max_s];
        queue->in++;
        queue->count--;
    }

    pthread_mutex_unlock(&(queue->lock));
    return return_value;
}

int queue_size(queue_t *queue)
{
    int size = 0;
    pthread_mutex_lock(&(queue->lock));
    size = queue->count;
    pthread_mutex_unlock(&(queue->lock));
    return size;
}