#include <stdlib.h>
#include "def.h"
#include "loader.h"
#include "corank.h"

char* check(struct pair_of_coranks actual, int expected_corank_of_A, int expected_corank_of_B){
  int checkA = actual.corank_A == expected_corank_of_A;
  int checkB = actual.corank_B == expected_corank_of_B;
  if(!checkA) {
    printf("Expected corank of A: %i, actual: %i\n",
        expected_corank_of_A, actual.corank_A);
  }
  if(!checkB) {
    printf("Expected corank of B: %i, actual: %i\n",
        expected_corank_of_B, actual.corank_B);
  }
  return (checkA && checkB) ? "⇒ passed" : "⇒ failed";
}

char* test1() {
  int A [] = {1};
  int B [] = {3};
  struct pair_of_coranks result1 = corank(1, A, 1,  B, 1);
  struct pair_of_coranks result2 = corank(2, A, 1,  B, 1);
  check(result2, 1, 1);
  return check(result1, 1, 0);
}

char* test2() {
  int A [] = {1,2,3,4};
  int B [] = {1,2,3,4};
  struct pair_of_coranks result = corank(4, A, 4,  B, 4);
  return check(result, 2, 2);
}

char* test3() {
  int A [] = {1, 1, 2, 2, 3, 4, 5, 6, 6, 8, 9, 11, 16, 20, 21};
  int B [] = {1, 2, 3, 3, 3, 3, 3, 3, 4, 5, 6,  7, 18, 24, 24};
  struct pair_of_coranks result = corank(10, A, 25,  B, 25);
  return check(result, 5, 5);
}

char* test4() {
  int A [] = {1, 1, 2, 2, 3, 4, 5, 6, 6, 8, 9, 11, 16, 20, 21};
  int B [] = {1, 2, 3, 3, 3, 3, 3, 3, 4, 5, 6,  7, 18, 24, 24};
  struct pair_of_coranks result = corank(6, A, 25,  B, 25);
  return check(result, 4, 2);
}

int main(int argc, char** args) {
  printf("Starting Tests for corank\n");
  printf("Test1: %s\n", test1());
  printf("Test2: %s\n", test2());
  printf("Test3: %s\n", test3());
  printf("Test4: %s\n", test4());
  return 0;
}



