/*
 * exech.h
 *
 *  Created on: Dec 5, 2013
 *      Author: reboot
 */

#ifndef EXECH_H_
#define EXECH_H_

#include "common.h"

#include "memory_t.h"

#include "m_comp.h"

#define MAX_EXEC_VAR_NAME_LEN   64

#define M_EXECH_DCNBT   "could not build exec string, output too large\n"


typedef struct ___d_exec_ch
{
  char *st_ptr;
  size_t len;
  __g_proc_v callback;
  _d_drt_h dtr;
} _d_exec_ch, *__d_exec_ch;

int
g_compile_exech(pmda mech, __g_handle hdl, char *instr);
char *
g_exech_build_string(void *d_ptr, pmda mech, __g_handle hdl, char *outstr,
    size_t maxlen);
int
g_op_load_print_mech(__g_handle hdl, pmda print_mech, char *print_ptr,
    size_t len);

#endif /* EXECH_H_ */
