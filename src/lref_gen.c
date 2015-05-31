/*
 * lref_dirlog.c
 *
 *  Created on: Dec 4, 2013
 *      Author: reboot
 */

#include "lref_gen.h"

#include "m_comp.h"
#include "lref.h"
#include "str.h"
#include "x_f.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

char *l_av_st[L_AV_MAX] =
  { 0 };

int
ref_to_val_generic(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  if (!strncmp(match, "procid", 6))
    {
      snprintf(output, max_size, "%d", getpid());
    }
  else if (!strncmp(match, "curtime", 7))
    {
      snprintf(output, max_size, "%d", (int32_t) time(NULL));
    }

  else if (!strncmp(match, "exe", 3) && strlen(match) == 3)
    {
      if (self_get_path(output))
        {
          output[0x0] = 0;
        }
    }
  else
    {
      return 1;
    }
  return 0;
}

char *
dt_rval_generic_procid(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, getpid());
  return output;
}

char *
dt_rval_generic_ipc(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc,
      ((__d_drt_h ) mppd)->hdl->ipc_key);
  return output;
}

char *
dt_rval_generic_curtime(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, (int32_t) time(NULL));
  return output;
}

char *
dt_rval_gg_int(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc,
      (unsigned long long int) glob_ui64_stor[((__d_drt_h) mppd)->uc_1]);

  return output;
}

char *
dt_rval_gg_sint(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc,
      (long long int) glob_si64_stor[((__d_drt_h) mppd)->uc_1]);

  return output;
}

char *
dt_rval_gg_float(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc,
      glob_float_stor[((__d_drt_h) mppd)->uc_1]);

  return output;
}

char *
dt_rval_generic_exe(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  if (self_get_path(output))
    {
      output[0x0] = 0;
    }
  return output;
}

static char *
dt_rval_generic_noop(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  return "";
}

static char *
dt_rval_generic_lav(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  char *ptr = l_av_st[((__d_drt_h ) mppd)->v_ui0];

  if (NULL != ptr)
    {
      snprintf(output, max_size, ((__d_drt_h ) mppd)->direc,
          l_av_st[((__d_drt_h ) mppd)->v_ui0]);
    }
  else
    {
      output[0] = 0x0;
    }
  return output;
}

static void *
rt_af_gen_lav(void *arg, char *match, char *output, size_t max_size, void *mppd)
{
  while (is_ascii_numeric(match[0]) && 0 != match[0])
    {
      match++;
    }

  if (match[0] == 0x0)
    {
      ERROR("rt_af_gen_lav: invalid 'arg' variable name: '%s'\n", match);
      return NULL;
    }

  errno = 0;
  uint32_t lav_idx = (uint32_t) strtoul(match, NULL, 10);

  if (errno == EINVAL || errno == ERANGE)
    {
      ERROR("rt_af_gen_lav: could not get index: '%s'\n", match);
      return NULL;
    }

  uint32_t max_index = sizeof(l_av_st) / sizeof(void*);

  if (lav_idx > max_index)
    {
      ERROR("rt_af_gen_lav: index out of range: %u, max: %u\n", lav_idx,
          max_index);
      return NULL;
    }

  __d_drt_h _mppd = (__d_drt_h) mppd;

  _mppd->v_ui0 = lav_idx;

  return as_ref_to_val_lk(match, dt_rval_generic_lav, (__d_drt_h ) mppd, "%s");
}

char *
dt_rval_generic_newline(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  return "\n";
}

char *
dt_rval_generic_cr(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  return "\r";
}

char *
dt_rval_generic_tab(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  return "\t";
}

#define MSG_GENERIC_BS         0x3A

void *
ref_to_val_lk_generic(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  if (!strncmp(match, "procid", 6))
    {
      return as_ref_to_val_lk(match, dt_rval_generic_procid, (__d_drt_h ) mppd,
          "%d");
    }
  else if (!strncmp(match, "ipc", 3))
    {
      return as_ref_to_val_lk(match, dt_rval_generic_ipc, (__d_drt_h ) mppd,
          "%X");
    }
  else if (!strncmp(match, "curtime", 7))
    {
      return as_ref_to_val_lk(match, dt_rval_generic_curtime, (__d_drt_h ) mppd,
          "%d");
    }

  else if (match[0] == 0x3A)
    {
      switch (match[1])
        {
      case 0x6E:
        return as_ref_to_val_lk(match, dt_rval_generic_newline,
            (__d_drt_h ) mppd, "%s");
      case 0x74:
        return as_ref_to_val_lk(match, dt_rval_generic_tab, (__d_drt_h ) mppd,
            "%s");
      case 0x72:
        return as_ref_to_val_lk(match, dt_rval_generic_cr, (__d_drt_h ) mppd,
            "%s");
      case 0x52:
        return as_ref_to_val_lk(match, dt_rval_generic_cr, (__d_drt_h ) mppd,
            "%s");
      case 0x4E:
        return as_ref_to_val_lk(match, dt_rval_generic_newline,
            (__d_drt_h ) mppd, "%s");
      case 0x54:
        return as_ref_to_val_lk(match, dt_rval_generic_tab, (__d_drt_h ) mppd,
            "%s");
        }

    }
  else if (!strncmp(match, "exe", 3) && (l_mppd_gvlen(match) == 3))
    {
      return as_ref_to_val_lk(match, dt_rval_generic_exe, (__d_drt_h ) mppd,
          "%s");
    }
  else if (!strncmp(match, "arg", 3))
    {
      return rt_af_gen_lav(arg, match, output, max_size, (__d_drt_h ) mppd);
    }
  else if (!strncmp(match, "noop", 4))
    {
      return as_ref_to_val_lk(match, dt_rval_generic_noop, (__d_drt_h ) mppd,
      NULL);
    }
  else if (match[0] == 0x3F)
    {
      return ref_to_val_af(arg, match, output, max_size, (__d_drt_h ) mppd);
    }

  return NULL;
}
