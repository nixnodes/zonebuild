/*
 * zonebuild.h
 *
 *  Created on: May 28, 2015
 *      Author: reboot
 */

#ifndef ZONEBUILD_H_
#define ZONEBUILD_H_ 1

#include <regex.h>

#define REG_IS_IPV4             ""
regex_t pr_is_ipv4;

#define F_INETNUM_OK_DATA       ((uint32_t)1 << 1)
#define F_INETNUM_OK_PREFIX     ((uint32_t)1 << 2)

#define F_INETNUM_OK            (F_INETNUM_OK_DATA|F_INETNUM_OK_PREFIX)

#define F_INETNUM_ROOT          ((uint32_t)1 << 3)

#include <stdint.h>
#include <sys/stat.h>

#include "memory_t.h"

typedef struct ___ip_addr
{
  uint8_t data[16];
} _ip_addr;

typedef struct ___inetnum_object
{
  uint32_t flags;
  struct stat st;
  uint32_t *d_ip_start, *d_ip_end;
  struct ___ip_addr ip_start, ip_end;
  uint32_t pfx_mask;
  uint8_t pfx_size;
  mda child_objects;
  pmda parent;
} _inet_obj, *__inet_obj;

typedef struct ___def_ophdr
{
  pmda base;
  __inet_obj root;
  void *arg;
} _def_ophdr, *__def_ophdr;

#include "fp_types.h"

typedef int
_load_inetnum_data(char *path, __def_ophdr option_header, __d_cvp callback);
typedef int
(*__load_inetnum_data)(char *path, __def_ophdr option_header, __d_cvp callback);

_load_inetnum_data load_inetnum_data;

int
o_zb_build(void *arg, int m, void *opt);
int
o_zb_setroot(void *arg, int m, void *opt);
int
o_zb_setpath(void *arg, int m, void *opt);

_d_cvp load_inetnum4_item;


typedef struct ___stdh_gopt
{
  __load_inetnum_data load;
  char *path;
  char *root;
} _stdh_go, *__stdh_go;

_stdh_go global_op;

#endif
