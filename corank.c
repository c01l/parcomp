#include "corank.h"

int min(int a, int b) {
  return a < b ? a : b;
}

int max(int a, int b) {
  return a > b ? a : b;
}


/**
 * @brief calculates the coranks of A and B to the index i
 * @returns a struct pair_of_coranks
 */
struct pair_of_coranks corank(int i, INPUTTYPE *A, int m, INPUTTYPE *B, int n) {
  int j = min(i, m);
  int k = (i - j);
  int j_low = max (0, (i - n));
  int k_low = 0;
  int counter = 0;
  struct pair_of_coranks coranks;

  // return value:

  
  while(counter < (m + n)) {
    if( j>0  &&  k<n  &&  A[j-1]>B[k] ) {
      // j was too big
      k_low = k;
      int diff = j-j_low;
      int delta = diff/2 + (diff&1);
      j = j - delta;
      k = k + delta;
    } else if ( k>0  &&  j<m  &&  B[k-1]>=A[j]  ) {
      // j was too small
      j_low = j; 
      int diff = k-k_low;
      int delta = diff/2 + (diff&1);
      k = k - delta;
      j = j + delta;
    } else {
      // all conditions are fulfilled
      coranks.corank_A = j;
      coranks.corank_B = k;
    }
    counter++;
  }
  return coranks;
}
