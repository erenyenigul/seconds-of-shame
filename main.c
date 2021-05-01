#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include "pthread_sleep.c"
#include <stdbool.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdarg.h>

// Setting max parameter size as a global variable.
const int MAX_PARAMETER_SIZE = 5;

// Each index will save the related setting as following:
// Index 0 - n parameter's value.
// Index 1 - p parameter's value.
// Index 2 - q parameter's value.
// Index 3 - t parameter's value.
// Index 4 - b parameter's value.
int N, Q, T;
float P, B;

typedef struct
{
	int *elems;
	int in;
	int count;
	int max_s;
} queue_t;

typedef struct
{
	pthread_cond_t cond;
	pthread_mutex_t mutex;
} event_t;

bool roll_dice(float win_probability);
int validate_program_parameters(int parameter_count, char **parameter_list);
void tprintf(char *format, ...);

void commentator_main(void *id_);
void commentator_round(void *id_);
void moderator_main();
void moderator_round();

void queue_init();
int queue_push(int i);
int queue_pop();

void event_init(event_t **event_ptr);
void wait_event(event_t *event);
void signal_event(event_t *event);
void broadcast_event(event_t *event);

//Global queue, initialized in main()
queue_t *queue;

//Global events
event_t *all_ready;
event_t *all_decided;
event_t *question_asked;
event_t *commentator_done;
event_t *next_round;

//Global locks
pthread_mutex_t decided_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t turn_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ready_lock = PTHREAD_MUTEX_INITIALIZER;

int num_decided = 0;
int num_ready = 0;
int turn = -1;

bool is_last_round = false;

int main(int argc, char *argv[])
{
	// Validating the input.
	if (!validate_program_parameters(argc, argv))
		return -1;

	queue_init();

	event_init(&all_ready);
	event_init(&all_decided);
	event_init(&question_asked);
	event_init(&commentator_done);
	event_init(&next_round);

	pthread_t commentators[N];
	pthread_t moderator;

	for (int i = 0; i < N; i++)
	{
		pthread_create(&commentators[i], NULL, &commentator_main, (void *)i);
	}
	pthread_create(&moderator, NULL, &moderator_main, NULL);

	pthread_exit(NULL);
	
}

//Thread Related Functions

void commentator_round(void *id_)
{
	int id = (int) id_;
	
	pthread_mutex_lock(&ready_lock);
	num_ready++;
	if(num_ready>=N){
		pthread_mutex_unlock(&ready_lock);
		signal_event(all_ready);
	}else{
		pthread_mutex_unlock(&ready_lock);
	}
		
	wait_event(question_asked);
	bool will_answer = roll_dice(P);
	
	if(will_answer){
		//queue_push() operation is secured with lock. The lock is implemented in the function
	 	queue_push(id);
	}
	
	pthread_mutex_lock(&decided_lock);
	num_decided++;
	//printf("%d\n", num_decided);
	if(num_decided >= N){
		pthread_mutex_unlock(&decided_lock);
		signal_event(all_decided);
	}else{
		pthread_mutex_unlock(&decided_lock);
	}

	if(will_answer){
		while(turn!=id);
		tprintf("Commentator #%d speaks\n", id);
		signal_event(commentator_done);
	}

	wait_event(next_round);
}

void moderator_round(int round_num)
{	
	while(num_ready<N);

	tprintf("Moderator asked the question %d\n", round_num);
	broadcast_event(question_asked);
	
	wait_event(all_decided);

	int commentator_id;
	while((commentator_id = queue_pop())!=-1){
	 	pthread_mutex_lock(&turn_lock);
		turn = commentator_id;
		pthread_mutex_unlock(&turn_lock);

		wait_event(commentator_done);
		tprintf("Commentator #%d finished speaking\n", commentator_id);
	}

	num_decided = 0;
	pthread_mutex_lock(&ready_lock);
	num_ready = 0;
	pthread_mutex_unlock(&ready_lock);
	turn = -1;
	tprintf("End of round %d\n", round_num);
	broadcast_event(next_round);
}

void moderator_main(){
	for(int i=0;i<Q;i++){
		moderator_round(i+1);
		if(i+1==Q) 
			is_last_round = true;
	}
	tprintf("End game");
}

void commentator_main(void *id_){
	while(!is_last_round)
		commentator_round(id_);
}

//Misc Functions

int validate_program_parameters(int parameter_count, char **parameter_list)
{

	// Checking if parameter count is valid.
	if (parameter_count != 11)
	{
		printf("\nseconds-of-shame: Please enter exactly 5 parameter and their values.\n\n");
		return 0;
	}

	// Assigning values to programSettings array for further use.
	// Terminating the program if invalid parameter is given.
	for (int i = 1; i < parameter_count; i += 2)
	{
		if (!strcmp(parameter_list[i], "-n"))
			N = atof(parameter_list[i + 1]);
		else if (!strcmp(parameter_list[i], "-p"))
			P = atof(parameter_list[i + 1]);
		else if (!strcmp(parameter_list[i], "-q"))
			Q = atof(parameter_list[i + 1]);
		else if (!strcmp(parameter_list[i], "-t"))
			T = atof(parameter_list[i + 1]);
		else if (!strcmp(parameter_list[i], "-b"))
			B = atof(parameter_list[i + 1]);
		else
		{
			printf("\nseconds-of-shame: Illegal parameter(s).\n\n");
			return 0;
		}
	}

	// If inputs are valid, returning 1.
	return 1;
}

bool roll_dice(float win_probability)
{
	float dice = (float)rand() / RAND_MAX;
	return dice < win_probability;
}

void tprintf(char *format, ...)
{
	time_t s, val = 1;
	struct tm *current_time;
	s = time(NULL);
	current_time = localtime(&s);

	int len = strlen(format) + 11;
	char time_added_format[len];
	sprintf(time_added_format, "%02d:%02d:%02d>", current_time->tm_hour, current_time->tm_min, current_time->tm_sec);
	strncat(time_added_format, format, len);

	va_list args;
	va_start(args, format);
	vprintf(time_added_format, args);
	va_end(args);
}

//Queue Related Functions
//push and pop uses mutex for synching the adding/removing process.

void queue_init()
{
	queue = (queue_t *)malloc(sizeof(queue_t));
	queue->elems = (int *)malloc(sizeof(int) * N);

	queue->max_s = N;
	queue->in = 0;
	queue->count = 0;
}

int queue_push(int i)
{
	pthread_mutex_lock(&queue_lock);

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

		return_value = 1;
	}

	pthread_mutex_unlock(&queue_lock);
	return return_value;
}

int queue_pop()
{
	pthread_mutex_lock(&queue_lock);
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

	pthread_mutex_unlock(&queue_lock);
	return return_value;
}

//Event Related Functions

void event_init(event_t **event_ptr)
{
	event_t *event = (event_t *)malloc(sizeof(event_t));
	event->cond = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
	event->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

	*event_ptr = event;
}

void wait_event(event_t *event)
{
	pthread_mutex_lock(&(event->mutex));
	pthread_cond_wait(&(event->cond), &(event->mutex));
	pthread_mutex_unlock(&(event->mutex));
}

void signal_event(event_t *event)
{
	pthread_mutex_lock(&(event->mutex));
	pthread_cond_signal(&(event->cond));
	pthread_mutex_unlock(&(event->mutex));
}

void broadcast_event(event_t *event)
{
	pthread_mutex_lock(&(event->mutex));
	pthread_cond_broadcast(&(event->cond));
	pthread_mutex_unlock(&(event->mutex));
}