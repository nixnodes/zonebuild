/*
 * sort_hdr.h
 *
 *  Created on: Dec 4, 2013
 *      Author: reboot
 */

#ifndef SORT_HDR_H_
#define SORT_HDR_H_

#include "memory_t.h"

#include <stdio.h>

#define F_SORT_DESC                     ((uint32_t)1 << 1)
#define F_SORT_ASC                      ((uint32_t)1 << 2)
#define F_SORT_RESETPOS                 ((uint32_t)1 << 3)
#define F_SORT_NUMERIC                  ((uint32_t)1 << 4)
#define F_SORT_STRING                   ((uint32_t)1 << 5)
#define F_SORT_F_DATE                   ((uint32_t)1 << 6)

#define F_SORT_ORDER                    (F_SORT_DESC|F_SORT_ASC)
#define F_SORT_TYPE                     (F_SORT_NUMERIC|F_SORT_STRING)
#define F_SORT_TYPE_F                   (F_SORT_STRING|F_SORT_F_DATE)

#define F_SORT_METHOD_SWAP              ((uint32_t)1 << 11)
#define F_SORT_METHOD_HEAP              ((uint32_t)1 << 12)
#define F_SORT_METHOD_Q                 ((uint32_t)1 << 13)
#define F_SORT_METHOD_INSSORT           ((uint32_t)1 << 14)
#define F_SORT_METHOD_SELECT            ((uint32_t)1 << 15)

#define F_SORT_METHOD_ALL               (F_SORT_METHOD_SWAP|F_SORT_METHOD_HEAP|F_SORT_METHOD_Q| \
                                          F_SORT_METHOD_INSSORT|F_SORT_METHOD_SELECT)

#define MAX_SORT_LOOPS                  MAX_uint64_t

#define F_INT_GSORT_LOOP_DID_SORT       ((uint32_t)1 << 1)
#define F_INT_GSORT_DID_SORT            ((uint32_t)1 << 2)

mda _md_gsort;
char *g_sort_field;

uint32_t g_sort_flags;

#include "lc_oper.h"
#include "m_comp.h"

#include "common.h"

typedef struct g_sref_data
{
  void *g_t_ptr_c;
  gs_cmp_p m_op, m_op_opp;
  uint32_t flags;
  size_t off;
  char m_buf1[4096], m_buf2[4096];
  _d_drt_h mppd;
  void *sp_0;
} _srd, *__p_srd;

typedef int
(*g_xsort_exec_p)(pmda m_ptr, __p_srd psrd);

int
do_sort(__g_handle hdl, char *field, uint32_t flags);
int
opt_g_sort(void *arg, int m, void *opt);
void
g_invert_sort_order(uint32_t *flags);
int
g_sort(__g_handle hdl, char *field, uint32_t flags);
int
g_check_is_data_numeric(__g_handle hdl, char *field);
void
g_heapsort(void **ref_arr, int64_t offset, int64_t dummy, __p_srd psrd);
void
g_qsort(void **arr, int64_t left, int64_t right, __p_srd psrd);
int
preproc_sort_numeric(__g_handle hdl, int vbs, char *field, uint32_t flags,
    __p_srd psrd);
int
g_sort_string(__g_handle hdl, char *field, uint32_t flags, __p_srd psrd);
int
g_qsort_exec(pmda m_ptr, __p_srd psrd);

typedef void
(*__d_sort_p)(void **arr, int64_t left, int64_t right, __p_srd psrd);

#endif /* SORT_HDR_H_ */
