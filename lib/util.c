#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

bool roll_dice(float win_probability)
{
    float dice = (float)rand() / RAND_MAX;
    return dice < win_probability;
}

int uniform_random(int lower_bound, int upper_bound)
{
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
    sprintf(time_added_format, "[%02d:%02d:%02d:%03d]", current_time->tm_hour, current_time->tm_min, current_time->tm_sec, milli);
    strncat(time_added_format, format, len);

    va_list args;
    va_start(args, format);
    vprintf(time_added_format, args);
    va_end(args);
}
