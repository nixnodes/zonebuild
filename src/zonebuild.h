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

#define F_INETNUM_ROOT          ((uint32_t)1 << 10)
#define F_INETNUM_NS            ((uint32_t)1 << 11)
#define F_INETNUM_NS_GLUE       ((uint32_t)1 << 12)
#define F_INETNUM_RFC2317       ((uint32_t)1 << 14)
#define F_INETNUM_FORCE_PROC    ((uint32_t)1 << 15)

#define F_INETNUM_MISC_00       ((uint32_t)1 << 20)

#define F_GH_POST_PRINT         ((uint64_t)1 << 46)
#define F_GH_PRE_PRINT          ((uint64_t)1 << 47)

#include <stdint.h>
#include <sys/stat.h>

#include "memory_t.h"

#define IP_PRINT(ipaddr, var) char var[128]; snprintf(var, sizeof(var), "%hhu.%hhu.%hhu.%hhu", \
(ipaddr)->data[3], (ipaddr)->data[2], (ipaddr)->data[1], (ipaddr)->data[0])

#define PREFIX_PRINT(ipaddr) char st_b0[128]; snprintf(st_b0, sizeof(st_b0), "%hhu.%hhu.%hhu.%hhu", \
(ipaddr)->data[3], (ipaddr)->data[2], (ipaddr)->data[1], (ipaddr)->data[0])

typedef struct ___ip_addr
{
  uint8_t data[16];
} _ip_addr, *__ip_addr;

#define F_NS_HAVE_GLUE          ((uint32_t)1 << 1)

typedef struct ___nameservers
{
  char *host;
  _ip_addr glue;
  uint32_t flags;
  char glue_str[128];
} _nserver, *__nserver;

#define NET_CLASS_A             (uint8_t) 1
#define NET_CLASS_B             (uint8_t) 2
#define NET_CLASS_C             (uint8_t) 3

#pragma pack(push, 4)

/* Class reference
 *      1 - C  /24
 *      2 - B  /16
 *      3 - A  /8
 */

typedef struct ___inetnum_object
{
  struct stat st;
  uint32_t *d_ip_start, *d_ip_end;
  _ip_addr ip_start, ip_end;
  uint8_t pfx_size, pfx_class, rfc2317, has_glue, is_shadow;
  mda child_objects;
  pmda parent;
  void *parent_link;
  char *fullpath, *servername, *email;
  mda nservers;
  uint32_t flags, tree_level, ns_level, nrecurse_d, pfx_mask;
  _nserver nserver_current;
} _inet_obj, *__inet_obj;

#pragma pack(pop)

#define INETO_SZ                           sizeof(_inet_obj)

struct ___def_ophdr_ufield
{
  uint32_t level, ns_level;
};

typedef struct ___def_ophdr
{
  pmda base;
  __inet_obj root;
  void *arg0, *ptr;
  char *sarg0;
  struct ___def_ophdr_ufield ufield;
} _def_ophdr, *__def_ophdr;

#include "fp_types.h"

typedef int
_load_inetnum_data(char *path, __def_ophdr option_header, __d_cvp callback);
typedef int
(*__load_inetnum_data)(char *path, __def_ophdr option_header, __d_cvp callback);

typedef int
(*__d_zb_c0)(__inet_obj object, __def_ophdr option_header, void* arg);
typedef int
_d_zb_c0(__inet_obj object, __def_ophdr option_header, void* arg);

_load_inetnum_data load_inetnum_data;

int
o_zb_build(void *arg, int m, void *opt);
int
o_zb_setroot(void *arg, int m, void *opt);
int
o_zb_setpath(void *arg, int m, void *opt);
int
o_zb_print_str(void *arg, int m, void *opt);
int
o_zb_pre_print_str(void *arg, int m, void *opt);
int
o_zb_post_print_str(void *arg, int m, void *opt);
int
o_zb_setlocsname(void *arg, int m, void *opt);
int
o_zb_setlocemail(void *arg, int m, void *opt);

_d_cvp load_inetnum4_item;

#define F_STDH_HAVE_PRINT               ((uint32_t)1 << 1)
#define F_STDH_HAVE_PRE_PRINT           ((uint32_t)1 << 2)
#define F_STDH_HAVE_POST_PRINT          ((uint32_t)1 << 3)

#define F_STDH_HAVE_PRINT_ANY           (F_STDH_HAVE_PRINT|F_STDH_HAVE_PRE_PRINT|F_STDH_HAVE_POST_PRINT)

typedef struct ___stdh_gopt
{
  __load_inetnum_data load;
  char *path;
  char *root;
  uint8_t pfx_min_size;
  uint8_t pfx_max_size;
  char *print_str;
  char *pre_print_str;
  char *post_print_str;
  char *loc_serv_name;
  char *loc_email;
  _g_handle handle;
  uint32_t flags;
} _stdh_go, *__stdh_go;

_stdh_go global_op;

typedef int
(*_chf)(p_md_obj pos, void *data, void *arg);

typedef struct ___ch_funct
{
  _chf call;
  uint32_t flags;
} _ch_funct, *__ch_funct;

#define CH_PROC_NEXT(pos) { p_md_obj next = pos->next; if (next) { return ((__ch_funct) next->ptr)->call(next, data, arg); }  }

#define CH_PROC_ITEM(object) { \
  if ( !g_bmatch((void*)object,&global_opt.handle,(object)->parent) ) { \
              global_opt.handle.g_proc4(&global_opt.handle, object, NULL); } \
              (object)->flags |= F_INETNUM_MISC_00; \
}

#endif
