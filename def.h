#ifndef H_DEF
#define H_DEF

#include <stdlib.h>

#define INPUTTYPE int
#define PARSE(x) strtol((x), NULL, 10)
#define INPUTTYPE_PRINT(str_buff, buff_len, x) snprintf(str_buff, buff_len, "%d", x) 

#endif