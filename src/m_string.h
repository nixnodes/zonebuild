/*
 * m_string.h
 *
 *  Created on: Dec 5, 2013
 *      Author: reboot
 */

#ifndef M_STRING_H_
#define M_STRING_H_

#define MAX_CPRG_STRING         4096

#include "common.h"

#include "fp_types.h"

int
g_load_strm(__g_handle hdl);
int
g_cprg(void *arg, int m, int match_i_m, int reg_i_m, int regex_flags,
    uint32_t flags, void *opt);
int
g_commit_strm_regex(__g_handle hdl, char *field, char *m, int reg_i_m,
    int regex_flags, uint32_t flags);

#define L_STFO_FILTER                   0x2

#define F_GH_HASSTRM                    ((uint64_t)1 << 33)

uint8_t l_sfo;

int
opt_g_d_regex(void *arg, int m, void *opt);
int
opt_g_d_regexi(void *arg, int m, void *opt);
int
opt_g_d_match(void *arg, int m, void *opt);
int
opt_g_d_fname(void *arg, int m, void *opt);
int
opt_g_d_fnamei(void *arg, int m, void *opt);
#endif /* M_STRING_H_ */
