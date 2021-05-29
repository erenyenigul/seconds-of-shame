#include "util.c"

bool roll_dice(float win_probability);
int validate_program_parameters(int parameter_count, char **parameter_list);
void tprintf(char *format, ...);
int uniform_random(int lower_bound, int upper_bound);