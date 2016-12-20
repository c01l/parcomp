#ifndef H_MERGE_SEQUENTIAL
#define H_MERGE_SEQUENTIAL
#include "def.h"
#include "loader.h"


#define COUNT(x) (sizeof(x) / sizeof(x[0]))


/**
 * @brief Merges the two sorted arrays from the sample parameter to one huge sorted array
 * @param sample the sample set
 * @param output the array where the final data gets put
 */
void merge(struct merge_sample *sample, INPUTTYPE *output, int start_index, int end_index);

#endif
