#include <stdlib.h>
#include <time.h>
#include "def.h"
#include "loader.h"
#include "merge_sequential.h"
#include "helper.h"

// start_index = index in output array where the first item is located
// end_index = index in output array where the last item is located? TODO: better docu :)
int loggen = 0;
void merge(struct merge_sample *sample, INPUTTYPE *output, int start_index, int end_index) {
        if(loggen) {
          char* a = (char*) malloc (20 * sizeof(char));
          char* b = (char*) malloc (20 * sizeof(char));
          sprintf(a, "A (length = %d)", sample->size1);
          sprintf(b, "B (length = %d)", sample->size2);
          merge_log("Merge arrays:");
          echoArrayExt(sample->array1, sample->size1, a);
          echoArrayExt(sample->array2, sample->size2, b);
          free(a);
          free(b);
          merge_log("starting at %d", start_index);
          merge_log("...\n");
        }
	int x = start_index; // TODO checkme optimizable by "output = output + start_index"? possible to remove index parameters?
	int i = 0, j = 0;
	while(i < sample->size1 && j < sample->size2) {
                if(x > end_index) {
                  printf("index (%d) to big (>%d).(startindex = %d). Stop! \n", x, end_index, start_index);
                  break;
                }
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
                if(loggen) printf("output[%d] = array1[%d]\t(%d)\n", x, i, sample->array1[i]);
		output[x++] = sample->array1[i++];
	}
	
	while(j < sample->size2) {
                if(loggen) printf("output[%d] = array2[%d]\t(%d)\n", x, j, sample->array2[j]);
		output[x++] = sample->array2[j++];
	}
}
