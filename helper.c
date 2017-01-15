#include "helper.h"

void echoArray(INPUTTYPE *x, size_t size) {
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

void echoArrayExt(INPUTTYPE *x, size_t size, char* text) {
  printf("%s :", text);
  echoArray(x, size);
}

void merge_log(char* format, ...) {
	va_list argp;
	
	if(LOGGING_ACTIVE == 1) {
		va_start(argp, format);
		vfprintf(stdout, format, argp);
		va_end(argp);
		(void) fprintf(stdout, "\n");
	}
}

/**
 * Subtracts two timespec structures.
 * @see https://gist.github.com/diabloneo/9619917
 */
void timespec_diff(struct timespec *start, struct timespec *stop,
                   struct timespec *result)
{
    if ((stop->tv_nsec - start->tv_nsec) < 0) {
        result->tv_sec = stop->tv_sec - start->tv_sec - 1;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
    } else {
        result->tv_sec = stop->tv_sec - start->tv_sec;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec;
    }

    return;
}

void printTimeDiff(struct timespec starttime, struct timespec endtime) {
	struct timespec result;
	timespec_diff(&starttime, &endtime, &result);
	int sec = result.tv_sec;
	int nsec = result.tv_nsec;
	printf("Time: %ds %dns\n", sec, nsec);
}

static void usage(char* progname) {
	printf("Usage: %s [-r <size1> <size2>] | [-f <example-file>]\n", progname);
}

int handleArguments(int argc, char** argv, struct merge_sample *sample) {
	
	if(argc < 3) {
		usage(argv[0]);
	}
	
	// load file
	if(strcmp(argv[1], "-f")==0 && argc==3) {
		int loadErr = loadsample(argv[1], sample);
		if(loadErr != 0) {
			char* error;
			switch(loadErr) {
				case 1: error = "File could not be read!"; break;
				case 2: error = "Fileformat is wrong."; break;
				default: error = "An unknown error occured!"; break;
			}
			(void) fprintf(stderr, "Failed to load data: %s\n", error);
			return loadErr;
		}
		return 0;
	} else if(strcmp(argv[1], "-r")==0 && argc==4) {
		int arraySize1, arraySize2;
		
		arraySize1 = strtol(argv[2], NULL, 10);
		arraySize2 = strtol(argv[3], NULL, 10);
		
		if(arraySize1 <= 0 || arraySize2 <= 0) {
			fprintf(stderr, "Array Size must be bigger than zero!");
			return (-1);
		}
		
		srand(time(NULL));
		
		// allocate arrays
		sample->array1 = malloc(sizeof(int) * arraySize1);
		sample->array2 = malloc(sizeof(int) * arraySize2);
		sample->size1 = arraySize1;
		sample->size2 = arraySize2;
		
		// spread numbers over arrays
		int i, j, x;
		i = j = x = 0;
		while(i < arraySize1 && j < arraySize2) {
			if(rand() % 2 == 0) {
				sample->array1[i++] = x;
			} else {
				sample->array2[j++] = x;
			}
			++x;
		}
		while(i < arraySize1) {
			sample->array1[i++] = x++;
		}
		while(j < arraySize2) {
			sample->array2[j++] = x++;
		}
		
		return 0;
	} else {
		fprintf(stderr, "You need to specify either -r of -f!\n");
		fprintf(stderr, "%s %d", argv[1], argc);
		usage(argv[0]);
		return (-1);
	}
}

int checkSorted(INPUTTYPE* array, int size) {
	int i = 1;
	while(i < size) {
		if(array[i-1] > array[i]) {
			return 0;
		}
		i++;
	}
	return 1;
}

void mergeSeq(struct merge_sample *sample, INPUTTYPE *output, int range1, int range2, int shouldShiftInOutputArray) {
    struct merge_sample merge_payload;
    struct pair_of_coranks coranks1, coranks2;

    coranks1 = corank(range1, sample->array1, sample->size1, sample->array2,sample->size2);
    coranks2 = corank(range2, sample->array1, sample->size1, sample->array2,sample->size2);
    merge_log("[mergeSeq] corank1: (%d,%d)", coranks1.corank_A, coranks1.corank_B);
    merge_log("[mergeSeq] corank2: (%d,%d)", coranks2.corank_A, coranks2.corank_B);
    merge_log("[mergeSeq] shouldShift: %d)", shouldShiftInOutputArray);
    
    merge_payload.array1 = sample->array1 + coranks1.corank_A;
    merge_payload.size1 = coranks2.corank_A - coranks1.corank_A;
    merge_payload.array2 = sample->array2 + coranks1.corank_B;
    merge_payload.size2 = coranks2.corank_B - coranks1.corank_B;
    int offset = shouldShiftInOutputArray ? range1 : 0;
    merge(&merge_payload, output, offset, range2 - range1 + offset);
    
    merge_log("[mergeSeq] with (%d, %d) result :", range1, range2);
    if(LOGGING_ACTIVE) echoArray(output + offset, range2-range1);
}
