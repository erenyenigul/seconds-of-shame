#include "atomic.c"

void atomic_init(atomic_t **atomic);
void atomic_set(atomic_t *atomic, int i);
int atomic_get(atomic_t *atomic);
int atomic_cond_set(atomic_t *atomic, int cond, int value);
void atomic_cond_signal_event(atomic_t *atomic, int cond, event_t *event);
void atomic_increment(atomic_t *atomic);