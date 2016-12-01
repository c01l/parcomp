#ifndef H_CORANK
#define H_CORANK

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "def.h"
#include "logger.h"

struct pair_of_coranks {
  int corank_A;
  int corank_B;
};


struct pair_of_coranks corank(int i, INPUTTYPE *A, int m, INPUTTYPE *B, int n);
#endif
