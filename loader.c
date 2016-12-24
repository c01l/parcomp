#include "loader.h"
#define LOADER_BUFFSIZE (1024)

/**
 * @brief Removes all whitespaces of an string
 * @returns a buffer new string without the whitespaces
 */
static char* trimString(char* str);

/**
 * @brief thells if a character is a whitepsace
 * @returns 1 if it is a whitespace, else 0
 */
static int isWhitespace(char c);

int loadsample(char* filename, struct merge_sample *out) {
	
	FILE* file = fopen(filename, "r");
	if(file == NULL) { return 1; }
	
	char buff[LOADER_BUFFSIZE];
	
	int mode = 0;
	int pos = 0;
	
	INPUTTYPE x;
	
	// read file
	while(fgets(buff, LOADER_BUFFSIZE, file) != NULL) {
		// fprintf(stdout, "Read: %s", buff);
		char* line = trimString(buff);
		// fprintf(stdout, "Trimmed: %s\n", line);
		if(*line == '#' || *line == 0) {
			// comment/empty line -> skip
			free(line);
		} else {
			// use line
			// fprintf(stdout, "Parse: %s\n", line);
			
			char* token = strtok(line, ",");
			while(token != NULL) {
				switch(mode) {
					case 0: // looking for size of the first array
						out->size1 = strtol(token, NULL, 10);
						out->array1 = malloc(sizeof(INPUTTYPE) * out->size1);
						pos = 0;
						++mode;
						break;
					case 1: // looking for content to put in the first array
						x = PARSE(token);
						out->array1[pos] = x;
						++pos;
						if(pos == out->size1) {
							++mode;
						}
						break;
					case 2: // looking for size of the second array
						out->size2 = strtol(token, NULL, 10);
						out->array2 = malloc(sizeof(INPUTTYPE) * out->size2);
						pos = 0;
						++mode;
						break;
					case 3: // looking for content of the second array
						x = PARSE(token);
						out->array2[pos] = x;
						++pos;
						if(pos == out->size2) {
							++mode;
						}
						break;
					default:
						++mode;
				}
				
				token = strtok(NULL, ",");
			}
		}
	}
	
	if(mode != 4) {
		return 2;
	}
	
	return fclose(file);
}

void freesample(struct merge_sample *sample) {
	free(sample->array1);
	free(sample->array2);
}

char* trimString(char* str) {
	char *x,*y;
	
	// count non-whitespaces
	int count = 0;
	for(x = str; (*x)!=0; ++x) {
		if(!isWhitespace(*x)) {
			++count;
		}
	}
	
	// increment for null-byte in the end
	++count;
	
	char* ret = malloc(sizeof(char) * count);
	for(x = str,y = ret; (*x)!=0; ++x) {
		if(!isWhitespace(*x)) {
			*y = *x;
			++y;
		}
	}
	*y = 0;
	
	return ret;
}

int isWhitespace(char c) {
	if(c == ' ' || c == '\t' || c == '\n' || c == '\r') {
		return 1;
	}
	return 0;
}
