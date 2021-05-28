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

// Declaring global variables for the further use.
int N, Q, T;
float P, B; 

// Color codes.
char boldRed[20] = "\033[1m\033[31m";
char boldGreen[20] = "\033[1m\033[32m";
char boldCyan[20] = "\033[1m\033[36m";
char boldBlue[20] = "\033[1m\033[34m";
char white[20] = "\033[m\033[1m\033[37m";
char boldYellow[20] = "\033[7m\033[1m\033[33m";

// Creating queue struct.
typedef struct
{
	int *elems;
	int in;
	int count;
	int max_s;
} queue_t;

// Creating event struct.
typedef struct
{
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	int count;
} event_t;

typedef struct{
	pthread_mutex_t mutex;
	int value;
} atomic_t;

// Declearing function prototypes.
bool roll_dice(float win_probability);
int validate_program_parameters(int parameter_count, char **parameter_list);
void tprintf(char *format, ...);
int uniform_random(int lower_bound, int upper_bound);

void commentator_main(void *id_);
void commentator_round(void *id_);
void moderator_main();
void moderator_round();
void news_reporter_main();

void queue_init();
int queue_push(int i);
int queue_pop();
int queue_size();

void event_init(event_t **event_ptr);
void wait_event(event_t *event);
void signal_event(event_t *event);
void broadcast_event(event_t *event, int n);
void reset_event(event_t *event);
int timed_wait_event(event_t *event, long ms);

void atomic_init(atomic_t **atomic);
void atomic_set(atomic_t *atomic, int i);
int atomic_get(atomic_t *atomic);
int atomic_cond_set(atomic_t *atomic, int cond, int value);
void atomic_cond_signal_event(atomic_t *atomic, int cond, event_t *event);
void atomic_increment(atomic_t *atomic);

//Global queue, initialized in main()
queue_t *queue;

//Global events
event_t *all_ready;
event_t *all_decided;
event_t *question_asked;
event_t *commentator_done;
event_t *next_round;
event_t *breaking_news_start;
event_t *breaking_news_end;

//Atomic ints
atomic_t *turn;
atomic_t *num_decided;
atomic_t *num_ready;
atomic_t *is_breaking_news;

pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;

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
	event_init(&breaking_news_start);
	event_init(&breaking_news_end);

	atomic_init(&turn);
	atomic_set(turn, -1);
	atomic_init(&num_decided);
	atomic_init(&num_ready);
	atomic_init(&is_breaking_news);

	pthread_t commentators[N];
	pthread_t moderator;
	pthread_t news_reporter;

	for (int i = 0; i < N; i++)
	{
		pthread_create(&commentators[i], NULL, (void *) commentator_main, (void *) i);
	}
	pthread_create(&moderator, NULL, (void *) moderator_main, NULL);
	pthread_create(&news_reporter, NULL, (void *) news_reporter_main, NULL);

	while(!is_last_round){
		pthread_sleep(1);
		if(roll_dice(B)){
			broadcast_event(breaking_news_start, 2);
			wait_event(breaking_news_end);
		}
	}

	pthread_exit(NULL);
}

//Thread Related Functions

//What do I need:

// Commentators should stop commenting after breaking news start (wait event)
// Moderator should check if breaking news before starting turns of commentators 

// atomic int for determining if breaking news is ongoing
// events: for cond wait 
// 	used by: commentator that was speaking

void news_reporter_main(){
	while(!is_last_round){
		wait_event(breaking_news_start);
		atomic_set(is_breaking_news, 1);
		tprintf(" %sBreaking news!%s\n", boldYellow, white);
		pthread_sleep(5);
		tprintf(" %sBreaking news ends!%s\n", boldYellow, white);
		atomic_set(is_breaking_news, 0);
		broadcast_event(breaking_news_end, 2);
	}
}

void commentator_round(void *id_)
{
	int id = (int) id_;
	
	atomic_increment(num_ready);
	
	wait_event(question_asked);

	bool will_answer = roll_dice(P);
	
	if(will_answer){
		//queue_push() operation is secured with lock. The lock is implemented in the function
		//queue_push also prints that commentator generates an answer
		queue_push(id);
	}
	
	atomic_increment(num_decided);
	atomic_cond_signal_event(num_decided, N, all_decided);

	if(will_answer){
		while(atomic_get(turn)!=id);
		
		long sleep_amount = uniform_random(1000, T*1000);
		tprintf(" %sCommentator #%d’s turn to speak for %.3f seconds!%s\n", boldCyan, id, ((float)sleep_amount)/1000, white);
		
		int result = timed_wait_event(breaking_news_start, sleep_amount);
		if(result)
			tprintf(" Commentator #%d finished speaking.\n", id);
		else
			tprintf(" Commentator #%d is cut short due to breaking news.\n", id);

		signal_event(commentator_done);
	}

	wait_event(next_round);
}

void moderator_round(int round_num)
{	
	while(atomic_get(num_ready)!=N);
    
	tprintf(" %sModerator asked the question %d!%s\n", boldRed, round_num, white);
	broadcast_event(question_asked, N);
	
	wait_event(all_decided);

	int commentator_id;
	
	while((commentator_id = queue_pop())!=-1){
		if(atomic_get(is_breaking_news))
			wait_event(breaking_news_end);
		atomic_set(turn, commentator_id);
		wait_event(commentator_done);
	}
	if (atomic_get(is_breaking_news))
		wait_event(breaking_news_end);

	atomic_set(num_decided, 0);
	atomic_set(num_ready, 0);
	atomic_set(turn, -1);

	tprintf(" %sEnd of the round %d.%s\n", boldBlue, round_num, white);
	broadcast_event(next_round, N);
}

void moderator_main(){
	for(int i=0;i<Q;i++){
		moderator_round(i+1);
		if(i+1==Q) 
			is_last_round = true;
	}
	tprintf(" %sEnd of the game.\n", boldBlue);

	exit(0);
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

int uniform_random(int lower_bound, int upper_bound){
	int random_num = (((int)rand()) % (upper_bound - lower_bound)) + lower_bound;
	return random_num;
}

void tprintf(char *format, ...)
{
	struct timeval ms_time;
	gettimeofday(&ms_time, NULL);
	int milli = ms_time.tv_usec / 1000;
	
	time_t s, val = 1;
	struct tm *current_time;
	s = time(NULL);
	current_time = localtime(&s);

	int len = strlen(format) + 50;
	char time_added_format[len];
	sprintf(time_added_format, "<%02d:%02d:%02d:%03d>", current_time->tm_hour, current_time->tm_min, current_time->tm_sec, milli);
	strncat(time_added_format, format, len);

	va_list args;
	va_start(args, format);
	vprintf(time_added_format, args);
	va_end(args);
}

//Queue Related Functions
//push, pop and size use mutex for synching the adding/removing process.

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

		return_value = queue->count;
	}
	tprintf(" %sCommentator #%d generates an answer. Position in queue: %d!%s\n", boldGreen, i, return_value-1, white);

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

int queue_size(){
	int size = 0;
	pthread_mutex_lock(&queue_lock);
	size= queue->count;
	pthread_mutex_unlock(&queue_lock);
	return size;
}

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
	if(event->count<0){
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

int timed_wait_event(event_t *event, long ms){
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
	if(event->count<=0){
		result = pthread_cond_timedwait(&(event->cond), &(event->mutex), &timetoexpire);
		if (result == 0)
			event->count--;
	}else{
		event->count--;
	}
	pthread_mutex_unlock(&(event->mutex));

	return result;
}

void reset_event(event_t *event){
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

void atomic_init(atomic_t ** atomic)
{
	*atomic = (atomic_t *) malloc(sizeof(atomic_t));
	(*atomic)->mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
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
	if(atomic->value == cond){
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