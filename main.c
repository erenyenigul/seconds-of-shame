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
float program_settings[MAX_PARAMETER_SIZE];

typedef struct{
	int *elems;
	int in;
	int count;
	int max_s;
}queue_t;

typedef struct {
	pthread_cond_t cond;
	pthread_mutex_t mutex;
}event_t;

int validate_program_parameters(int parameter_count, char **parameter_list);
void commentator_main(void *str);
void moderator_main(void *str);
bool roll_dice(float win_probability);
void queue_init();
int queue_push(int i);
int queue_pop();
event_t *create_event();
void wait_event(event_t *event);
void signal_event(event_t *event);
void broadcast_event(event_t *event);


//Global queue, initialized in main()
queue_t *queue;

//Global events
event_t *all_decided;
event_t *question_asked;
event_t *commentator_done;
event_t *next_round;

//Global locks
pthread_mutex_t decided_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_lock   = PTHREAD_MUTEX_INITIALIZER;

int num_decided = 0;

int main(int argc, char *argv[])
{
	// Validating the input.
	if (!validate_program_parameters(argc, argv))
		return -1;

	queue_init();

	pthread_t commentators[N];
	pthread_t moderator;

	for (int i = 0; i < N; i++)
	{
		pthread_create(&commentators[i], NULL, &commentator_main, (void *)"hey");
	}
	pthread_create(&moderator, NULL, &moderator_main, (void *)"hey");


	pthread_exit(NULL);
}


//Thread Related Functions

void commentator_main(void *str)
{
	char *string = (char *)str;

	bool will_speak = roll_dice(P);
}

void moderator_main(void *str)
{	
	printf("lol i am the mod");
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
			program_settings[0] = N = atof(parameter_list[i + 1]);
		else if (!strcmp(parameter_list[i], "-p"))
			program_settings[1] = P = atof(parameter_list[i + 1]);
		else if (!strcmp(parameter_list[i], "-q"))
			program_settings[2] = Q = atof(parameter_list[i + 1]);
		else if (!strcmp(parameter_list[i], "-t"))
			program_settings[3] = T = atof(parameter_list[i + 1]);
		else if (!strcmp(parameter_list[i], "-b"))
			program_settings[4] = B = atof(parameter_list[i + 1]);
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
	return dice <= win_probability;
}


//Queue Related Functions
//push and pop uses mutex for synching the adding/removing process.

void queue_init(){
	queue = (queue_t*) malloc(sizeof(queue_t));
	queue->elems = (int*) malloc(sizeof(int) * N);
	
	queue->max_s = N;
	queue->in=0;
	queue->count=0;
}

int queue_push(int i){
	pthread_mutex_lock(&queue_lock);

	int return_value;
	int max_s = queue->max_s;

	if(queue->count>=max_s){
		return_value = -1;
	}else{
		printf("i %d in: %d count: %d\n", i, queue->in, queue->count);
		queue->elems[(queue->in + queue->count)%max_s] = i;
		queue->count++;
		
		return_value = 1;
	}

	pthread_mutex_unlock(&queue_lock);
	return return_value;
}

int queue_pop(){
	pthread_mutex_lock(&queue_lock);
	int max_s = queue->max_s;
	int return_value;

	if(queue->count==0) 
		return_value = -1;
	else{
 	   	return_value = queue->elems[queue->in%max_s];
		queue->in++;
		queue->count--;
	}
	
	pthread_mutex_unlock(&queue_lock);
	return return_value;
}


//Event Related Functions

event_t *create_event(){
	event_t *event = (event_t*) malloc(sizeof(event_t));
	event->cond = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
	event->mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;

	return event;
}

void wait_event(event_t *event){
	pthread_mutex_lock(&(event->mutex));
	pthread_cond_wait(&(event->cond), &(event->mutex));
	pthread_mutex_unlock(&(event->mutex));
}

void signal_event(event_t *event){
	pthread_mutex_lock(&(event->mutex));
	pthread_cond_signal(&(event->cond));
	pthread_mutex_unlock(&(event->mutex));
}

void broadcast_event(event_t *event){
	pthread_mutex_lock(&(event->mutex));
	pthread_cond_broadcast(&(event->cond));
	pthread_mutex_unlock(&(event->mutex));
}