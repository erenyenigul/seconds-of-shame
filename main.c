#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include "pthread_sleep.c"
#include <stdbool.h>
#include <semaphore.h>
#define _XOPEN_SOURCE 600 /* Or higher */
#define _POSIX_C_SOURCE 200112L /* Or higher */
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

bool is_round_over = false;
sem_t speak;
pthread_barrier_t round_end;

int validate_program_parameters(int parameter_count, char **parameter_list);
void commentator_main(void* str);
void moderator_main(void* str);
bool roll_dice(float win_probability);

int main(int argc, char *argv[]) {
	sem_init(&speak, 0, 1);
	pthread_barrier_init(&round_end);

	// Validating the input.
	if (!validate_program_parameters(argc, argv)) return -1;
	
	pthread_t commentators[N];
	pthread_t moderator;
	
	for(int i=0; i < N; i++){
		pthread_create( &commentators[i], NULL, &commentator_main, (void*) "hey");
	}
	pthread_create( &moderator, NULL, &moderator_main, (void*) "hey");
	
	pthread_exit(NULL);
}

void commentator_main(void* str){
	char* string = (char*) str;
	
	bool will_speak = roll_dice(P);

	
}

void moderator_main(void* str){
	char* string = (char*) str;
	printf("MOD : %s\n", string);
}

int validate_program_parameters(int parameter_count, char **parameter_list) {

	// Checking if parameter count is valid.
	if (parameter_count != 11) {
		printf("\nseconds-of-shame: Please enter exactly 5 parameter and their values.\n\n");
		return 0;
	}

	// Assigning values to programSettings array for further use.
	// Terminating the program if invalid parameter is given.
	for(int i = 1; i < parameter_count; i += 2) {
		if (!strcmp(parameter_list[i], "-n")) program_settings[0] = N = atof(parameter_list[i+1]);
		else if (!strcmp(parameter_list[i], "-p")) program_settings[1]= P = atof(parameter_list[i+1]);
		else if (!strcmp(parameter_list[i], "-q")) program_settings[2] = Q =atof(parameter_list[i+1]);
		else if (!strcmp(parameter_list[i], "-t")) program_settings[3] = T =atof(parameter_list[i+1]);
		else if (!strcmp(parameter_list[i], "-b")) program_settings[4] = B = atof(parameter_list[i+1]);
		else {
			printf("\nseconds-of-shame: Illegal parameter(s).\n\n");
			return 0;
		}
	}

	// If inputs are valid, returning 1.
	return 1;
}

bool roll_dice(float win_probability){
	float dice = (float) rand()/ RAND_MAX;
	return dice<=win_probability;
}