/*
 * common.h
 *
 *  Created on: May 29, 2015
 *      Author: reboot
 */

#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_

char debug_mode;

#include <stdio.h>
#include <string.h>
#include <errno.h>

#define FAULT(...) char err_b[1024]; fprintf(stderr, PACKAGE_NAME ": " __VA_ARGS__, \
    errno ? strerror_r(errno,err_b, sizeof(err_b)) : "" ); \
    errno ? errno = 0 : errno;
#define ERROR(...) fprintf(stderr, PACKAGE_NAME ": " __VA_ARGS__)
#define DEBUG(...) if ( debug_mode ) fprintf(stdout, PACKAGE_NAME ": " __VA_ARGS__)
#define NOTIFY(...) fprintf(stdout, PACKAGE_NAME ": " __VA_ARGS__)
#define ABORT(...) fprintf(stderr, PACKAGE_NAME ": " __VA_ARGS__); abort()

#ifdef __FreeBSD__
#include <sys/endian.h>
#else
#include <endian.h>
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
#elif __BYTE_ORDER == __BIG_ENDIAN
#else
#error
#endif

#if __x86_64__ || __ppc64__
#define _ARCH_STR        "x86_64"
#else
#define _ARCH_STR        "i686"
#endif

#define F_GH_PRINT                      ((uint64_t)1 << 52)

#include "fp_types.h"

typedef struct g_handle
{
  FILE *fh;
  char *file;
  int fd_out;
#ifdef HAVE_ZLIB_H
  char w_mode[6];
  gzFile gz_fh, gz_fh1;
#endif

  uint32_t block_sz;
  uint64_t flags;

  mda _match_rr;
  mda _accumulator;
  off_t max_results, max_hits;
  __g_ipcbm ifrh_l0, ifrh_l1;
  mda print_mech;
  mda post_print_mech;
  mda pre_print_mech;
  pmda act_mech;

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

  mda guid_stor;
  mda uuid_stor;

  const char *h_errstr_gz;
  char strerr_b[1024];
  void *v_b0;
  size_t v_b0_sz;

#ifdef _G_SSYS_NET
  void *pso_ref;
#endif
  char mv1_b[MAX_VAR_LEN];
} _g_handle, *__g_handle;

#endif /* SRC_COMMON_H_ */
