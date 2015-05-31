/*
 * omfp.c
 *
 *  Created on: Dec 4, 2013
 *      Author: reboot
 */

#include "omfp.h"

#include "exech.h"

#include <stdlib.h>
#include <unistd.h>

uint32_t g_omfp_sto = 0, g_omfp_suto = 0;
int fd_out;

int
(*int_printf)(const char *__restrict __format, ...) = printf;

#define g_p_print(hdl) { \
    void *t_ptr = calloc(1, hdl->block_sz); \
    hdl->g_proc4_l((void*) hdl, t_ptr, NULL); \
    free(t_ptr); \
}

void
g_do_ppprint(__g_handle hdl, uint64_t t_flags, pmda p_mech, _d_omfp g_proc)
{
  if (hdl->flags & t_flags)
    {
      void *s_act_mech = hdl->act_mech;
      hdl->act_mech = p_mech;
      void *t_ptr = calloc(1, hdl->block_sz);
      g_proc((void*) hdl, t_ptr, NULL);
      free(t_ptr);
      hdl->act_mech = (pmda) s_act_mech;
    }
}

int
g_omfp_write(int fd, char *buffer, size_t max_size, void *arg)
{
  return write(fd, buffer, max_size);
}

int
g_omfp_write_nl(int fd, char *buffer, size_t max_size, void *arg)
{
  if (-1 == write(fd, buffer, max_size))
    {
      return -1;
    }

  return write(fd, "\n", 1);
}

void
g_omfp_norm(void *hdl, void *ptr, char *sbuffer)
{
  ((__g_handle ) hdl)->g_proc3(ptr, sbuffer);
}

void
g_omfp_raw(void * hdl, void *ptr, char *sbuffer)
{

#ifdef HAVE_ZLIB_H
  if (((__g_handle ) hdl)->flags & F_GH_IO_GZIP)
    {
      gzwrite(((__g_handle ) hdl)->gz_fh1, ptr, ((__g_handle ) hdl)->block_sz);
    }
  else
    {
      write(fd_out, ptr, ((__g_handle ) hdl)->block_sz);
    }
#else
  write(fd_out, ptr, ((__g_handle ) hdl)->block_sz);
#endif
}

void
g_omfp_ocomp(void * hdl, void *ptr, char *sbuffer)
{
  ((__g_handle ) hdl)->g_proc3(ptr, (void*) sbuffer);
}

#include <errno.h>

void
g_omfp_eassemble(void *hdl, void *ptr, char *sbuffer)
{
  char *s_ptr;
  if (!(s_ptr = g_exech_build_string(ptr, ((__g_handle ) hdl)->act_mech,
      (__g_handle) hdl, (char*)((__g_handle ) hdl)->v_b0, ((__g_handle ) hdl)->v_b0_sz)))
    {
      ERROR("g_omfp_eassemble: could not assemble print string\n");
      //gfl |= F_OPT_KILL_GLOBAL;
      return;
    }

  int ret = ((__g_handle) hdl)->w_d(((__g_handle) hdl)->fd_out, (char*)((__g_handle ) hdl)->v_b0,
  strlen((char*)((__g_handle ) hdl)->v_b0), (void*)sbuffer);

  if (ret == -1)
    {
      FAULT("g_omfp_eassemble: write failed: %s\n");
      //gfl |= F_OPT_KILL_GLOBAL;
      return;
    }

}

