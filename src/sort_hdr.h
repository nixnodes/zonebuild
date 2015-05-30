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

#include <lc_oper.h>
#include <m_comp.h>

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



typedef struct g_handle
{
  FILE *fh;
#ifdef HAVE_ZLIB_H
  char w_mode[6];
  gzFile gz_fh, gz_fh1;
#endif
  off_t offset, bw, br, total_sz;
  off_t rw, t_rw;
  uint32_t block_sz;
  uint64_t flags, status;
  mda buffer, w_buffer;
  mda _match_rr;
  mda _accumulator;
  off_t max_results, max_hits;
  __g_ipcbm ifrh_l0, ifrh_l1;

  mda print_mech;
  mda post_print_mech;
  mda pre_print_mech;
  pmda act_mech;
  void *data;

  mode_t st_mode;
  key_t ipc_key;
  int shmid;
  struct shmid_ds ipcbuf;
  int
  (*g_proc0)(void *, char *, char *);
  __g_proc_v g_proc1_ps;
  __d_ref_to_pv g_proc2;
  __g_proc_v g_proc1_lookup;
  _d_proc3 g_proc3, g_proc3_batch, g_proc3_export, g_proc3_extra;
  _d_gcb_pp_hook gcb_post_proc;
  _d_omfp g_proc4, g_proc4_pr, g_proc4_po;
  __d_is_wb w_d, w_d_pr, w_d_po;
  size_t j_offset, jm_offset;
  int d_memb;
  void *_x_ref;
  int shmcflags;
  int shmatflags;
  mda guid_stor;
  mda uuid_stor;
  int hd_errno;
  int h_errno_gz;
  const char *h_errstr_gz;
  char strerr_b[1024];
  void *v_b0;
  size_t v_b0_sz;
  __d_wpid_cb execv_wpid_fp;
#ifdef _G_SSYS_NET
  void *pso_ref;
#endif
} _g_handle, *__g_handle;

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
preproc_sort_numeric(__g_handle hdl, int vbs, char *field, uint32_t flags, __p_srd psrd);
int
g_sort_string(__g_handle hdl, char *field, uint32_t flags, __p_srd psrd);
int
g_qsort_exec(pmda m_ptr, __p_srd psrd);

typedef void
(*__d_sort_p)(void **arr, int64_t left, int64_t right, __p_srd psrd);

#endif /* SORT_HDR_H_ */
