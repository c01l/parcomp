#ifndef H_MERGE_HELPER
#define H_MERGE_HELPER
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include "def.h"
#include "loader.h"
#include "merge_sequential.h"
#include "corank.h"

#define LOGGING_ACTIVE (0)

void echoArray(INPUTTYPE *x, size_t size);

void echoArrayExt(INPUTTYPE *x, size_t size, char* text);

void merge_log(char* msg, ...);

void printTimeDiff(struct timespec starttime, struct timespec endtime);

int handleArguments(int argc, char** argv, struct merge_sample *sample);

int checkSorted(INPUTTYPE* array, int size);

void testIfSorted(INPUTTYPE* A, int size);

void mergeSeq(struct merge_sample *sample, INPUTTYPE *output, int range1, int range2, int shouldShiftInOutputArray);
#endif
