#include "../include/voicemeeter_bypass.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

void make_log(const char *format, ...)
{
    // add \n if not present
    if (strlen(format) == 0 || format[strlen(format) - 1] != '\n')
    {
        char *new_format = malloc(strlen(format) + 2);
        strcpy(new_format, format);
        strcat(new_format, "\n");
        format = new_format;
    }

    // append current time
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char time_str[30] = { 0 };
    sprintf(time_str, "[%04d-%02d-%02d %02d:%02d:%02d] ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    char *new_format = malloc(strlen(format) + 30);
    strcpy(new_format, time_str);
    strcat(new_format, format);
    format = new_format;

    // print to console
    va_list args;
    va_start(args, format);
    vprintf(format, args); // printf
    va_end(args);

    FILE *f = fopen("log.txt", "a");
    if (f)
    {
        // print to file
        va_start(args, format);
        vfprintf(f, format, args); // fprintf
        va_end(args);
        fclose(f);
    }
    else
    {
        #ifdef VOICEMEETER_BYPASS_DEBUG
        printf("Failed to open log file.\n");
        #endif
    }

}