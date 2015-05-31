/*
 * lref_gen3.c
 *
 *  Created on: Dec 5, 2013
 *      Author: reboot
 */

#include "zonebuild_lref.h"

#include "common.h"
#include "config.h"

#include "zonebuild.h"
#include "str.h"
#include "lref.h"
#include "lref_gen.h"
#include "omfp.h"

#include <errno.h>

void
dt_set_inetobj(__g_handle hdl)
{
  hdl->block_sz = INETO_SZ;
  hdl->d_memb = 1;
  //hdl->g_proc0 = gcb_gen3;
  hdl->g_proc1_lookup = ref_to_val_lk_zone;
  hdl->g_proc2 = ref_to_val_ptr_zone;
  hdl->g_proc3 = zone_format_block;
  hdl->g_proc3_batch = zone_format_block_batch;
  hdl->g_proc3_export = zone_format_block_exp;
  hdl->g_proc4 = g_omfp_norm;
  //hdl->ipc_key = IPC_KEY_GEN3LOG;
  hdl->jm_offset = (size_t) &((__inet_obj) NULL)->fullpath;

}

int
zone_format_block(void *ptr, char *output)
{
  __inet_obj data = (__inet_obj) ptr;

  return NOTIFY("ZONE: %s\n", data->fullpath);

}

int
zone_format_block_batch(void *ptr, char *output)
{
  __inet_obj data = (__inet_obj) ptr;

  return NOTIFY("ZONE: %s\n", data->fullpath);

}

int
zone_format_block_exp(void *ptr, char *output)
{
  __inet_obj data = (__inet_obj) ptr;

  return NOTIFY("ZONE: %s\n", data->fullpath);

}

#define _MC_ZONE_PATH           "path"
#define _MC_ZONE_SIP            "startip"
#define _MC_ZONE_EIP            "endip"
#define _MC_ZONE_TREELVL        "treelevel"
#define _MC_ZONE_NSLVL          "nslevel"
#define _MC_ZONE_NSCOUNT        "nscount"
#define _MC_ZONE_NSERVER        "nserver"
#define _MC_ZONE_NSGLUE         "nsglue"
#define _MC_ZONE_NSGLUEIP       "nsglueip"
#define _MC_ZONE_PFXSIZE        "pfxsize"
#define _MC_ZONE_PFXMASK        "pfxmask"
#define _MC_ZONE_NLEVEL         "nlevel"
#define _MC_ZONE_RFC2317        "rfc2317"
#define _MC_ZONE_SERVER         "server"
#define _MC_ZONE_EMAIL          "email"
#define _MC_ZONE_HASGLUE        "hasglue"

void *
ref_to_val_ptr_zone(void *arg, char *match, int *output)
{
  __inet_obj data = (__inet_obj) arg;
  if (!strncmp(match, _MC_ZONE_EIP, 5))
    {
      *output = ((int) sizeof(uint32_t));
      return &data->ip_end;
    }
  else if (!strncmp(match, _MC_ZONE_SIP, 7))
    {
      *output = ((int) sizeof(uint32_t));
      return &data->ip_start;
    }
  else if (!strncmp(match, _MC_ZONE_TREELVL, 9))
    {
      *output = ((int) sizeof(data->tree_level));
      return &data->tree_level;
    }
  else if (!strncmp(match, _MC_ZONE_NSLVL, 7))
    {
      *output = ((int) sizeof(data->ns_level));
      return &data->ns_level;
    }
  else if (!strncmp(match, _MC_ZONE_PFXSIZE, 7))
    {
      *output = ((int) sizeof(data->pfx_size));
      return &data->pfx_size;
    }
  else if (!strncmp(match, _MC_ZONE_PFXMASK, 7))
    {
      *output = ((int) sizeof(data->pfx_mask));
      return &data->pfx_mask;
    }
  else if (!strncmp(match, _MC_ZONE_NLEVEL, 6))
    {
      *output = ((int) sizeof(data->nrecurse_d));
      return &data->nrecurse_d;
    }
  else if (!strncmp(match, _MC_ZONE_NSGLUE, 6))
    {
      *output = ((int) sizeof(uint32_t));
      return &data->nserver_current.glue;
    }
  else if (!strncmp(match, _MC_ZONE_RFC2317, 7))
    {
      *output = ((int) sizeof(data->rfc2317));
      return &data->rfc2317;
    }
  else if (!strncmp(match, _MC_ZONE_NSCOUNT, 6))
    {
      *output = ((int) sizeof(data->nservers.offset));
      return &data->nservers.offset;
    }
  else if (!strncmp(match, _MC_ZONE_HASGLUE, 7))
    {
      *output = ((int) sizeof(data->hasglue));
      return &data->hasglue;
    }
  else
    {
      *output = 0;
    }

  return NULL;
}

static char *
dt_rval_zone_path(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, ((__inet_obj) arg)->fullpath);
  return output;
}

static char *
dt_rval_zone_sip(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, *((__inet_obj) arg)->d_ip_start);
  return output;
}

static char *
dt_rval_zone_mask(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, ((__inet_obj) arg)->pfx_mask);
  return output;
}

static char *
dt_rval_zone_eip(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, *((__inet_obj) arg)->d_ip_end);
  return output;
}

static char *
dt_rval_zone_tl(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, ((__inet_obj) arg)->tree_level);
  return output;
}

static char *
dt_rval_zone_nslvl(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, ((__inet_obj) arg)->ns_level);
  return output;
}

static char *
dt_rval_zone_nserver(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{

  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, ((__inet_obj) arg)->nserver_current.host);

  return output;
}

static char *
dt_rval_zone_nsglue(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, ((__inet_obj) arg)->nserver_current.glue_str);
  return output;
}

static char *
dt_rval_zone_nsglue_ip(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, ((__inet_obj) arg)->nserver_current.glue);
  return output;
}

static char *
dt_rval_zone_nslevel(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, ((__inet_obj) arg)->nrecurse_d);
  return output;
}

static char *
dt_rval_zone_pfxsize(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, ((__inet_obj) arg)->pfx_size);
  return output;
}

static char *
dt_rval_zone_rfc2317(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, ((__inet_obj) arg)->rfc2317);
  return output;
}

static char *
dt_rval_zone_nscount(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, (uint64_t) ((__inet_obj) arg)->nservers.offset);
  return output;
}

static char *
dt_rval_zone_hasglue(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, (uint8_t) ((__inet_obj) arg)->hasglue);
  return output;
}

static char *
dt_rval_zone_server(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, ((__inet_obj) arg)->servername);
  return output;
}

static char *
dt_rval_zone_email(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, ((__inet_obj) arg)->email);
  return output;
}

void *
ref_to_val_lk_zone(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  PROC_SH_EX(match)

  void *ptr;
  if ((ptr = ref_to_val_lk_generic(arg, match, output, max_size, mppd)))
    {
      return ptr;
    }
  else if (!strncmp(match, _MC_ZONE_PATH, 4))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_path, (__d_drt_h) mppd, "%s");
    }
  else if (!strncmp(match, _MC_ZONE_SIP, 7))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_sip, (__d_drt_h) mppd, "%u");
    }
  else if (!strncmp(match, _MC_ZONE_EIP, 5))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_eip, (__d_drt_h) mppd, "%u");
    }
  else if (!strncmp(match, _MC_ZONE_TREELVL, 9))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_tl, (__d_drt_h) mppd, "%u");
    }
  else if (!strncmp(match, _MC_ZONE_NSLVL, 7))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_nslvl, (__d_drt_h) mppd, "%u");
    }
  else if (!strncmp(match, _MC_ZONE_NSERVER, 7))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_nserver, (__d_drt_h) mppd, "%s");
    }
  else if (!strncmp(match, _MC_ZONE_NSGLUEIP, 8))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_nsglue_ip, (__d_drt_h) mppd, "%u");
    }
  else if (!strncmp(match, _MC_ZONE_NSGLUE, 6))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_nsglue, (__d_drt_h) mppd, "%s");
    }
  else if (!strncmp(match, _MC_ZONE_NLEVEL, 6))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_nslevel, (__d_drt_h) mppd, "%u");
    }
  else if (!strncmp(match, _MC_ZONE_PFXSIZE, 7))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_pfxsize, (__d_drt_h) mppd, "%hhu");
    }
  else if (!strncmp(match, _MC_ZONE_PFXMASK, 7))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_mask, (__d_drt_h) mppd, "%u");
    }
  else if (!strncmp(match, _MC_ZONE_RFC2317, 7))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_rfc2317, (__d_drt_h) mppd, "%hhu");
    }
  else if (!strncmp(match, _MC_ZONE_NSCOUNT, 6))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_nscount, (__d_drt_h) mppd, "%llu");
    }
  else if (!strncmp(match, _MC_ZONE_SERVER, 6))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_server, (__d_drt_h) mppd, "%s");
    }
  else if (!strncmp(match, _MC_ZONE_EMAIL, 5))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_email, (__d_drt_h) mppd, "%s");
    }
  else if (!strncmp(match, _MC_ZONE_HASGLUE, 7))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_hasglue, (__d_drt_h) mppd, "%hhu");
    }

  return NULL;
}

