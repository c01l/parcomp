#ifndef H_LOADER
#define H_LOADER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "def.h"

struct merge_sample {
	INPUTTYPE *array1, *array2;
	int size1, size2;
};

/**
 * @brief Loads an sampleset to a provided address.
 * @param filename The file you want to loadsample
 * @param out The sample structure containing the sample data
 * @returns errorcode
 * 			- 0: No error
 * 			- 1: File could not be read
 * 			- 2: Fileformat is wrong
 */
int loadsample(char* filename, struct merge_sample *out);

/**
 * @brief frees the sample storage
 */
void freesample(struct merge_sample *sample);

#endif