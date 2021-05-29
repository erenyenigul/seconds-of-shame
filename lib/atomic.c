#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct
{
    pthread_mutex_t mutex;
    int value;
} atomic_t;

void atomic_init(atomic_t **atomic)
{
    *atomic = (atomic_t *)malloc(sizeof(atomic_t));
    (*atomic)->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    (*atomic)->value = 0;
}

void atomic_set(atomic_t *atomic, int i)
{
    pthread_mutex_lock(&(atomic->mutex));
    atomic->value = i;
    pthread_mutex_unlock(&(atomic->mutex));
}

int atomic_get(atomic_t *atomic)
{
    int result;
    pthread_mutex_lock(&(atomic->mutex));
    result = atomic->value;
    pthread_mutex_unlock(&(atomic->mutex));
    return result;
}

int atomic_cond_set(atomic_t *atomic, int cond, int value)
{
    int result = 0;
    pthread_mutex_lock(&(atomic->mutex));
    if (atomic->value == cond)
    {
        atomic->value = value;
        result = 1;
    }
    pthread_mutex_unlock(&(atomic->mutex));
    return result;
}

void atomic_cond_signal_event(atomic_t *atomic, int cond, event_t *event)
{
    pthread_mutex_lock(&(atomic->mutex));
    if (atomic->value == cond)
    {
        signal_event(event);
    }
    pthread_mutex_unlock(&(atomic->mutex));
}

void atomic_increment(atomic_t *atomic)
{
    pthread_mutex_lock(&(atomic->mutex));
    atomic->value++;
    pthread_mutex_unlock(&(atomic->mutex));
}
