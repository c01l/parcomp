#include <stdlib.h>
#include <time.h>
#include "def.h"
#include "loader.h"

#define COUNT(x) (sizeof(x) / sizeof(x[0]))

/**
 * @brief Merges the two sorted arrays from the sample parameter to one huge sorted array
 * @param sample the sample set
 * @param output the array where the final data gets put
 */
void merge(struct merge_sample *sample, INPUTTYPE *output);

static void echoArray(INPUTTYPE *x, size_t size) {
	int i;
	for(i = 0; i < size; ++i) {
		char out[20];
		INPUTTYPE_PRINT(out, 20, x[i]);
		(void) fprintf(stdout, "%s", out);
		
		if(i != size - 1) {
			(void) fprintf(stdout, ", ");
		}
	}
	(void) fprintf(stdout, "\n");
}

int main(int argc, char** args) {
	
	if (argc != 2) {
		(void) fprintf(stdout, "Usage: %s <samplefile>\n", args[0]);
		exit(1);
	}
	
	struct merge_sample sampleset;
	
	int loadErr = loadsample(args[1], &sampleset);
	if(loadErr != 0) {
		char* error;
		switch(loadErr) {
			case 1: error = "File could not be read!"; break;
			case 2: error = "Fileformat is wrong."; break;
			default: error = "An unknown error occured!"; break;
		}
		(void) fprintf(stderr, "Failed to load data: %s\n", error);
		exit(1);
	}
	
	// show input
	(void) fprintf(stdout, "Array 1:\n");
	echoArray(sampleset.array1, sampleset.size1);
	(void) fprintf(stdout, "\nArray 2:\n");
	echoArray(sampleset.array2, sampleset.size2);
	(void) fprintf(stdout, "\n");
	
	// register output
	int n = sampleset.size1 + sampleset.size2;
	INPUTTYPE *output = malloc(sizeof(INPUTTYPE) * n); // TODO CHECKME reuse memory?
	
	// setup benchmark
	struct timespec starttime, endtime;
	
	clock_gettime(CLOCK_REALTIME, &starttime);
	merge(&sampleset, output);
	clock_gettime(CLOCK_REALTIME, &endtime);
	
	// calculate results
	int sec = endtime.tv_sec - starttime.tv_sec;
	int nsec = endtime.tv_nsec - starttime.tv_nsec;
	
	// print results
	(void) fprintf(stdout, "Final Array: ");
	echoArray(output, n);
	
	freesample(&sampleset);
	
	(void) fprintf(stdout, "Time: %ds %dns\n", sec, nsec);
	
	fprintf(stdout, "Process finished!\n");
	
	return 0;
}


void merge(struct merge_sample *sample, INPUTTYPE *output) {
	int x = 0;
	int i = 0, j = 0;
	while(i < sample->size1 && j < sample->size2) {
		INPUTTYPE v1 = sample->array1[i];
		INPUTTYPE v2 = sample->array2[j];
		
		if(v1 < v2) {
			output[x] = v1;
			++i;
		} else {
			output[x] = v2;
			++j;
		}
		++x;
	}
	
	while(i < sample->size1) {
		output[x++] = sample->array1[i++];
	}
	
	while(j < sample->size2) {
		output[x++] = sample->array2[j++];
	}
}