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
