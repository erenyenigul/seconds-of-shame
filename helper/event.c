#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

typedef struct
{
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    int count;
} event_t;

//Event Related Functions
void event_init(event_t **event_ptr)
{
    event_t *event = (event_t *)malloc(sizeof(event_t));
    event->cond = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    event->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    event->count = 0;
    *event_ptr = event;
}

void wait_event(event_t *event)
{
    pthread_mutex_lock(&(event->mutex));
    event->count--;
    if (event->count < 0)
    {
        pthread_cond_wait(&(event->cond), &(event->mutex));
    }
    pthread_mutex_unlock(&(event->mutex));
}

void signal_event(event_t *event)
{
    pthread_mutex_lock(&(event->mutex));
    event->count++;
    pthread_cond_signal(&(event->cond));
    pthread_mutex_unlock(&(event->mutex));
}

int timed_wait_event(event_t *event, long ms)
{
    int result;

    struct timespec tm;
    struct timeval now;
    int rt;

    struct timeval tp;
    struct timespec timetoexpire;
    // When to expire is an absolute time, so get the current time and add
    // it to our delay time
    double seconds = ((double)ms) / 1000;
    gettimeofday(&tp, NULL);
    long new_nsec = tp.tv_usec * 1000 + (seconds - (long)seconds) * 1e9;
    timetoexpire.tv_sec = tp.tv_sec + (long)seconds + (new_nsec / (long)1e9);
    timetoexpire.tv_nsec = new_nsec % (long)1e9;

    pthread_mutex_lock(&(event->mutex));
    event->count--;
    if (event->count < 0)
    {
        result = pthread_cond_timedwait(&(event->cond), &(event->mutex), &timetoexpire);
        if (result != 0)
            event->count++;
    }
 
    pthread_mutex_unlock(&(event->mutex));

    return result;
}

void reset_event(event_t *event)
{
    pthread_mutex_lock(&(event->mutex));
    event->count = 0;
    pthread_mutex_unlock(&(event->mutex));
}

void broadcast_event(event_t *event, int n)
{
    pthread_mutex_lock(&(event->mutex));
    event->count += n;
    pthread_cond_broadcast(&(event->cond));
    pthread_mutex_unlock(&(event->mutex));
}