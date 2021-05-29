#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdbool.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdarg.h>
#include "pthread_sleep.c"
#include "lib/util.h"
#include "lib/event.h"
#include "lib/atomic.h"
#include "lib/queue.h"

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

void commentator_main(void *id_);
void commentator_round(void *id_);
void moderator_main();
void moderator_round();
void news_reporter_main();

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

bool is_last_round = false;

int main(int argc, char *argv[])
{
	// Validating the input.
	if (!validate_program_parameters(argc, argv))
		return -1;

	queue_init(&queue, N);
	
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
	exit(0);
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
		int size = queue_push(queue, id);
		tprintf(" %sCommentator #%d generates an answer. Position in queue: %d!%s\n", boldGreen, id,  size-1, white);
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
	
	while((commentator_id = queue_pop(queue))!=-1){
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