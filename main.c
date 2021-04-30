#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include "pthread_sleep.c"

// Setting max parameter size as a global variable.
const int MAX_PARAMETER_SIZE = 5;

// Each index will save the related setting as following:
// Index 0 - n parameter's value.
// Index 1 - p parameter's value.
// Index 2 - q parameter's value.
// Index 3 - t parameter's value.
// Index 4 - b parameter's value.
float programSettings[MAX_PARAMETER_SIZE];

int validateProgramParameters(int parameterCount, char **parameterList) {

	// Checking if parameter count is valid.
	if (parameterCount != 11) {
		printf("\nseconds-of-shame: Please enter exactly 5 parameter and their values.\n\n");
		return 0;
	}

	// Assigning values to programSettings array for further use.
	// Terminating the program if invalid parameter is given.
	for(int i = 1; i < parameterCount; i += 2) {
		if (!strcmp(parameterList[i], "-n")) programSettings[0] = atof(parameterList[i+1]);
		else if (!strcmp(parameterList[i], "-p")) programSettings[1] = atof(parameterList[i+1]);
		else if (!strcmp(parameterList[i], "-q")) programSettings[2] = atof(parameterList[i+1]);
		else if (!strcmp(parameterList[i], "-t")) programSettings[3] = atof(parameterList[i+1]);
		else if (!strcmp(parameterList[i], "-b")) programSettings[4] = atof(parameterList[i+1]);
		else {
			printf("\nseconds-of-shame: Illegal parameter(s).\n\n");
			return 0;
		}
	}

	// If inputs are valid, returning 1.
	return 1;
}

int main(int argc, char *argv[]) {

	// Validating the input.
	if (!validateProgramParameters(argc, argv)) return -1;
	
	// Start code.

}