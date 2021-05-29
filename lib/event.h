#include "event.c"

void event_init(event_t **event_ptr);
void wait_event(event_t *event);
void signal_event(event_t *event);
void broadcast_event(event_t *event, int n);
void reset_event(event_t *event);
int timed_wait_event(event_t *event, long ms);