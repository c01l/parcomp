#ifndef H_MERGE_HELPER
#define H_MERGE_HELPER
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include "def.h"
#include "loader.h"

#define LOGGING_ACTIVE (0)

void echoArray(INPUTTYPE *x, size_t size);

void merge_log(char* msg, ...);

void printTimeDiff(struct timespec starttime, struct timespec endtime);

int handleArguments(int argc, char** argv, struct merge_sample *sample);

#endif
