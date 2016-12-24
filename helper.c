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

void merge_log(char* format, ...) {
	va_list argp;
	
	if(LOGGING_ACTIVE == 1) {
		va_start(argp, format);
		vfprintf(stdout, format, argp);
		va_end(argp);
		(void) fprintf(stdout, "\n");
	}
}

void printTimeDiff(struct timespec starttime, struct timespec endtime) {
	int sec = endtime.tv_sec - starttime.tv_sec;
	int nsec = endtime.tv_nsec - starttime.tv_nsec;
	printf("Time: %us %uns\n", sec, nsec);
}