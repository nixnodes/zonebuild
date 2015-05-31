#include "common.h"

#define MAX_GLOB_STOR_AR_COUNT          255

uint64_t glob_ui64_stor[MAX_GLOB_STOR_AR_COUNT];
int64_t glob_si64_stor[MAX_GLOB_STOR_AR_COUNT];
float glob_float_stor[MAX_GLOB_STOR_AR_COUNT];
int64_t glob_curtime;

void *
g_get_glob_ptr(__g_handle hdl, char *field, int * output);
