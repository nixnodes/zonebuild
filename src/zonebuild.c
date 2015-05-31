/*
 ============================================================================
 Name        : zonebuild.c
 Author      : siska
 Version     :
 Copyright   : Copyright (c) NixNodes
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <math.h>
#include <limits.h>

#include "common.h"
#include "config.h"

#include "signal_t.h"
#include "memory_t.h"
#include "x_f.h"
#include "str.h"
#include "fp_types.h"
#include "arg_proc.h"
#include "sort_hdr.h"
#include "exech.h"
#include "omfp.h"
#include "m_general.h"
#include "m_lom.h"
#include "m_string.h"
#include "zonebuild_lref.h"

#include "zonebuild.h"

regex_t pr_is_ipv4;

_stdh_go global_opt =
  { .load = NULL, .path = NULL, .root = NULL, .pfx_max_size = 32,
      .pfx_min_size = 8, .print_str = NULL, .handle =
        { 0 }, .pre_print_str = NULL, .post_print_str = NULL, .loc_serv_name =
      NULL, .loc_serv_name = "root.nic.dn42", .loc_email = "root.nic.dn42" };

int
o_zb_build(void *arg, int m, void *opt)
{
  char *mode = g_pg(arg, m);

  if ( NULL == mode)
    {
      return 1002;
    }

  if (!strncmp("rev", mode, 3))
    {
      global_opt.load = load_inetnum_data;
    }
  else
    {
      ERROR("o_zb_build: unknown mode: '%s'\n", mode);
      return 1010;
    }

  return 0;

}

int
o_zb_setroot(void *arg, int m, void *opt)
{
  global_opt.root = g_pg(arg, m);
  if ( NULL == global_opt.root)
    {
      return 2002;
    }

  return 0;
}

int
o_zb_setpath(void *arg, int m, void *opt)
{
  global_opt.path = g_pg(arg, m);
  if ( NULL == global_opt.path)
    {
      return 3002;
    }

  return 0;
}

int
o_zb_print_str(void *arg, int m, void *opt)
{
  global_opt.print_str = g_pg(arg, m);

  if ( NULL == global_opt.print_str)
    {
      return 4002;
    }

  global_opt.flags |= F_STDH_HAVE_PRINT;

  return 0;
}

int
o_zb_pre_print_str(void *arg, int m, void *opt)
{
  global_opt.pre_print_str = g_pg(arg, m);

  if ( NULL == global_opt.pre_print_str)
    {
      return 5002;
    }

  global_opt.flags |= F_STDH_HAVE_PRE_PRINT;

  return 0;
}

int
o_zb_post_print_str(void *arg, int m, void *opt)
{
  global_opt.post_print_str = g_pg(arg, m);

  if ( NULL == global_opt.post_print_str)
    {
      return 6002;
    }

  global_opt.flags |= F_STDH_HAVE_POST_PRINT;

  return 0;
}

int
o_zb_setlocsname(void *arg, int m, void *opt)
{
  global_opt.loc_serv_name = g_pg(arg, m);
  if ( NULL == global_opt.loc_serv_name)
    {
      return 7002;
    }

  return 0;
}

int
o_zb_setlocemail(void *arg, int m, void *opt)
{
  global_opt.loc_email = g_pg(arg, m);
  if ( NULL == global_opt.loc_email)
    {
      return 7002;
    }

  return 0;
}

static void
initialize_regexes()
{
  int r;

  if ((r = regcomp(&pr_is_ipv4, REG_IS_IPV4, REG_EXTENDED)))
    {
      ERROR("initialize_regexes: pr_is_ipv4: %s, [%d]\n", REG_IS_IPV4, r);
    }
}

static void
release_regexes()
{
  regfree(&pr_is_ipv4);
}

static uint8_t
get_ip4_class_from_size(uint8_t size)
{
  return ((32 - size) / 8);
}

static char*
convert_string_to_ipaddr(char *name, _ip_addr *ipdata)
{

  char *sptr = name, *ptr_tmp;

  int i;

  for (i = 3; (sptr[0] && sptr[0] != 0x5F); i--)
    {
      ptr_tmp = sptr;

      while (sptr[0] > 0x2F && sptr[0] < 0x40)
        {
          sptr++;
        }

      int c = atoi(ptr_tmp);

      if (c > UCHAR_MAX || c < 0)
        {
          ERROR("convert_string_to_ipaddr: '%s': IP octet out of range: '%d'\n",
              name, c);
          return NULL;
        }

      ipdata->data[i] = (uint8_t) c;

      if (sptr[0] != 0x2E)
        {
          break;
        }

      if (0 == i)
        {
          ERROR("convert_string_to_ipaddr: '%s': invalid IP addressn", name);
          return NULL;
        }

      sptr++;
    }

  return sptr;
}

static int
int_sort(pmda base, size_t off, int vb, uint32_t flags)
{

  int retval;
  _srd srd0;

  retval = preproc_sort_numeric(NULL, vb, NULL, flags, &srd0);

  if (retval)
    {
      ERROR("int_sort: preproc_sort_numeric failed: [%d]\n", retval);
      return 1;
    }

  srd0.off = (size_t) &((__inet_obj ) NULL)->pfx_size;

  if ((retval = g_qsort_exec(base, &srd0)))
    {
      ERROR("int_sort: g_qsort_exec failed: [%d]\n", retval);
      return 1;
    }

  return 0;
}

static int
commit_inetnum4_item(char *name, __inet_obj object)
{
  object->d_ip_start = (uint32_t*) object->ip_start.data;
  object->d_ip_end = (uint32_t*) object->ip_end.data;

  char *sptr, *size;

  if ( NULL == (sptr = convert_string_to_ipaddr(name, &object->ip_start)))
    {
      return 1;
    }

  if (sptr[0] != 0x5F)
    {
      ERROR(
          "commit_inetnum4_item: ip string does not terminate properly: '%s'\n",
          name);
      return 1;
    }

  //object->d_ip_start = *((uint32_t*)object->ip_start.data);

  sptr++;
  size = sptr;

  int c = atoi(size);

  if (c > 32 || c < 1)
    {
      ERROR("commit_inetnum_item: '%s': prefix size out of range: '%d'\n", name,
          c);
      return 1;
    }

  object->pfx_size = (uint8_t) c;
  object->pfx_class = get_ip4_class_from_size(object->pfx_size);

  /* get netmask */
  object->pfx_mask = (UINT_MAX << (32 - object->pfx_size));

  /* get last address in range */
  uint32_t *ip_end = ((uint32_t*) object->ip_end.data), *ip_start =
      ((uint32_t*) object->ip_start.data);

  *ip_end = *ip_start + ~(object->pfx_mask);

  if (object->pfx_size > (32 - 8))
    {
      object->flags |= F_INETNUM_RFC2317;
      object->rfc2317 = 1;
    }

  return 0;
}

static int
validate_inetnum_block(char *name, __inet_obj object)
{
  if ((*object->d_ip_start & object->pfx_mask) != *object->d_ip_start)
    {
      ERROR("validate_inetnum_block: '%s': invalid prefix\n", name);
      return 1;
    }

  if (object->pfx_size > global_opt.pfx_max_size)
    {
      DEBUG(
          "validate_inetnum_block: '%s': prefix size exceeds pfx_max_size (%hhu > %hhu)\n",
          name, object->pfx_size, global_opt.pfx_max_size);
      return 1;
    }

  return 0;
}

static int
get_nservers(char *base, char *ptr, void *data, void *ns)
{
  if (strncmp("nserver:", base, 8))
    {
      return 1;
    }

  __inet_obj object = (__inet_obj) data;

  while (ptr[0] && (ptr[0] == 0x20 || ptr[0] == 0x9))
    {
      ptr++;
    }

  char *glue = NULL;

  if (ptr[0] != 0x0)
    {
      glue = ptr;
      while (ptr[0])
        {
          ptr++;
        }
      ptr[0] = 0x0;
    }

  size_t ns_len = strlen(ns);

  if (!ns_len || ns_len >= 8192)
    {
      ABORT(
          "get_nservers: critical error when parsing ns record: ns_len == %zu\n",
          ns_len);
    }

  __nserver nserver = md_alloc(&object->nservers, sizeof(_nserver));

  if ( NULL == nserver)
    {
      ABORT("get_nservers: md_alloc failed\n");
    }

  object->flags |= F_INETNUM_NS;
  nserver->host = malloc(ns_len + 2);
  memcpy(nserver->host, ns, ns_len + 1);

  if (NULL != glue)
    {
      if ( NULL == convert_string_to_ipaddr(glue, &nserver->glue))
        {
          ERROR(
              "get_nservers: convert_string_to_ipaddr failed on string: '%s'\n",
              glue);
          return 1;
        }

      snprintf(nserver->glue_str, sizeof(nserver->glue_str), "%s", glue);

      nserver->flags |= F_NS_HAVE_GLUE;
      object->flags |= F_INETNUM_NS_GLUE;
    }

  return 0;

}

static int
parse_zone_records(char *name, __inet_obj object, __d_cvp3 callback)
{
  FILE *fh = fopen(object->fullpath, "r");

  if ( NULL == fh)
    {
      FAULT("parse_zone_file: fopen: %s: %s\n", object->fullpath);
      return 1;
    }

  md_init(&object->nservers, 8);

  char line_buffer[8192], *ptr = line_buffer;

  while (NULL != (ptr = fgets(line_buffer, sizeof(line_buffer), fh)))
    {
      char *base_ptr = ptr;

      char *t_ptr = ptr;
      while (t_ptr[0] && t_ptr[0] != 0xA)
        {
          t_ptr++;
        }
      t_ptr[0] = 0x0;

      ptr = (ptr + 8);

      while (ptr[0] && (ptr[0] == 0x20 || ptr[0] == 0x9))
        {
          ptr++;
        }

      if (ptr[0] == 0x0)
        {
          ERROR("parse_zone_file: %s: missing record\n", name);
          continue;
        }

      int c = 0;

      char *ns = ptr;

      while (ptr[0] && !(ptr[0] == 0x20 || ptr[0] == 0x9))
        {
          c++;
          ptr++;
        }

      if (0 == c)
        {
          ERROR("parse_zone_file: %s: corrupted record in zone file\n", name);
          continue;
        }

      ptr[0] = 0x0;
      ptr++;

      if (callback(base_ptr, ptr, (void*) object, ns))
        {
          continue;
        }

    }

  if (ferror(fh))
    {
      FAULT("parse_zone_file: fgets: %s: %s\n", object->fullpath);
    }

  if (EOF == fclose(fh))
    {
      FAULT("parse_zone_file: fclose: %s: %s\n", object->fullpath);
      return 1;
    }

  return 0;
}

int
load_inetnum4_item(char *name, void *data)
{
  __def_ophdr option_header = (__def_ophdr) data;
  __inet_obj object = (__inet_obj) option_header->arg0;

  if (commit_inetnum4_item(name, object))
    {
      return 1;
    }

  object->flags |= F_INETNUM_OK_DATA;

  if (validate_inetnum_block(name, object))
    {
      return 0;
    }

  object->flags |= F_INETNUM_OK_PREFIX;

  if (parse_zone_records(name, object, get_nservers))
    {
      return 1;
    }

  if (!strncmp(global_opt.root, name, strlen(global_opt.root)))
    {
      if (NULL != option_header->root)
        {
          ABORT("load_inetnum4_item: critical: multiple roots found: '%s'\n", name);
        }
      else
        {
          object->flags |= F_INETNUM_ROOT;
          option_header->root = object;
          DEBUG("found root entry: '%s'\n", name);
        }
    }

  IP_PRINT(&object->ip_start,st_b0);
  IP_PRINT(&object->ip_end,st_b1);
  IP_PRINT((__ip_addr)&object->pfx_mask,st_b2);

  DEBUG(
      "load_inetnum4_item: %s - %s (%u) | [%s] - %u %u - %u, class %hhu\n",
      st_b0, st_b1, object->pfx_size, st_b2, *object->d_ip_start, ~object->pfx_mask,
      (object->flags & F_INETNUM_ROOT), object->pfx_class);

  return 0;
}

int
load_inetnum_data(char *path, __def_ophdr option_header, __d_cvp callback)
{
  int retval = 0;

  DIR *dp = opendir(path);

  if ( NULL == dp)
    {
      FAULT("load_inetnum_data: '%s': %s\n", path);
      return 1;
    }

  errno = 0;

  struct dirent *pde;

  while ((pde = readdir(dp)))
    {
      if (pde->d_name[0] == 0x2E
          && (pde->d_name[1] == 0x2E || pde->d_name[1] == 0x0))
        {
          continue;
        }

      char *st_b0 = malloc(PATH_MAX);
      snprintf(st_b0, PATH_MAX, "%s/%s", path, pde->d_name);

      __inet_obj object = md_alloc_le(option_header->base, sizeof(_inet_obj), 0,
      NULL);

      if ( NULL == object)
        {
          FAULT("load_inetnum_data: '%s': md_alloc_le failed%s\n", pde->d_name);
          abort();
        }

      object->parent = option_header->base;
      object->fullpath = st_b0;

      if (-1 == lstat(st_b0, &object->st))
        {
          FAULT("load_inetnum_data: lstat: '%s': %s\n", st_b0);
          md_unlink_le(option_header->base, option_header->base->pos);
          continue;
        }

      if (S_IFREG == (object->st.st_mode & S_IFMT))
        {
          option_header->arg0 = (void*) object;

          if (callback(pde->d_name, (void*) option_header))
            {
              errno = 0;
              retval = 2;
              break;
            }

          if (!(object->flags & F_INETNUM_OK))
            {
              ERROR("load_inetnum_data: F_INETNUM_OK not satisfied\n");
              md_unlink_le(option_header->base, option_header->base->pos);
            }
        }
      else
        {
          //fprintf(stderr, "load_inetnum_data: '%s': not a regular file\n", st_b0);
        }

      errno = 0;
    }

  if ( errno)
    {
      FAULT("load_inetnum_data: readdir: %s\n");
      retval = errno;
    }

  if (-1 == closedir(dp))
    {
      FAULT("load_inetnum_data: closedir: %s\n");
      retval = errno;
    }

  return retval;
}

static void
check_glob_opts()
{
  if ( NULL == global_opt.path)
    {
      ERROR("main: missing path to data files\n");
      exit(1);
    }
  if ( NULL == global_opt.root)
    {
      ERROR("main: missing pointer to root zone\n");
      exit(1);
    }
}

static int
link_hierarchy_tree(_def_ophdr option_header, __inet_obj object,
    __inet_obj parent)
{

  md_init(&object->child_objects, 32);
  object->child_objects.flags |= F_MDA_REFPTR;

  option_header.ufield.level++;

  int b_count = option_header.ufield.level;

  char *b_level = malloc(b_count + 1);

  memset(b_level, 0x2D, b_count);

  b_level[b_count] = 0x0;

  IP_PRINT(&object->ip_start, st_b0);

  DEBUG("link_hierarchy_tree: %s entering %s/%hhu [%u] [%u]\n", b_level, st_b0,
      object->pfx_size, option_header.ufield.level, object->pfx_class);

  p_md_obj ptr =
  NULL != option_header.ptr ? option_header.ptr : object->parent->first;

  while (ptr)
    {
      __inet_obj ptr_object = (__inet_obj) ptr->ptr;

      if ( NULL != ptr_object->parent_link )
        {
          goto e_loop;
        }

      if ( ptr_object->pfx_size <= object->pfx_size ||
          !((*ptr_object->d_ip_start >= *object->d_ip_start) &&
              (*ptr_object->d_ip_end <= *object->d_ip_end)) )
        {
          goto e_loop;
        }

      IP_PRINT(&ptr_object->ip_start, st_b0);

      if ( ptr_object == parent )
        {
          goto e_loop;
        }
      if ( ptr_object == object )
        {
          goto e_loop;
        }

      DEBUG("link_hierarchy_tree: %s * linking:  %s/%hhu [%u] [%u]\n",b_level,st_b0,
          ptr_object->pfx_size, option_header.ufield.level, ptr_object->pfx_class);

      object->child_objects.lref_ptr = ptr_object;
      ptr_object = md_alloc(&object->child_objects, 0);

      ptr_object->parent_link = (void*)object;

      option_header.ptr = ptr;
      link_hierarchy_tree(option_header, ptr_object, object);

      e_loop:;

      ptr = ptr->next;
    }

  size_t offset = (size_t) &((__inet_obj) NULL)->pfx_size;

  int_sort(&object->child_objects, offset, 1, F_SORT_DESC);

  DEBUG("link_hierarchy_tree: %s exiting %s/%hhu [%u] [%u]\n", b_level, st_b0,
      object->pfx_size, option_header.ufield.level, object->pfx_class);

  option_header.ufield.level--;

  free(b_level);

  return 0;
}

static int
ch_zone_nservers_1(p_md_obj pos, void *data, void *arg)
{
  __inet_obj object = (__inet_obj) data;

  p_md_obj ptr = md_first(&object->nservers);

  object->nrecurse_d = 0;

  p_md_obj next = pos->next;

  while (ptr)
    {
      __nserver ns_ptr = (__nserver) ptr->ptr;
      object->nserver_current = *ns_ptr;
      if (ns_ptr->flags & F_NS_HAVE_GLUE)
        {
          object->hasglue = 1;
        }
      else
        {
          object->hasglue = 0;
        }

      if (next)
        {
          ((__ch_funct) next->ptr)->call(next, data, arg);
        }
      else
        {
          CH_PROC_ITEM(object);
        }

      object->nrecurse_d++;

      ptr =ptr->next;
    }

  return 0;
}

static __inet_obj
find_block_by_address(pmda base, __inet_obj block, __inet_obj parent)
{
  p_md_obj ptr = md_first(base);

  while (ptr)
    {
      __inet_obj object = (__inet_obj) ptr->ptr;

      if ( *object->d_ip_start == *block->d_ip_start &&
          object->pfx_size == block->pfx_size && block != parent)
        {
          return object;
        }

      ptr = ptr->next;
    }
  return NULL;
}

static int
ch_zone4_breakdown_0(p_md_obj pos, void *data, void *arg)
{
  __inet_obj object = (__inet_obj) data;

  if ( 0 == object->nservers.offset )
    {
      CH_PROC_ITEM(object);
      return 0;
    }

  uint8_t class_size = ((( 32 - object->pfx_size ) / 8) * 8);

  _inet_obj dummy = *object;
  dummy.d_ip_start = (uint32_t*)&dummy.ip_start;
  dummy.d_ip_end = (uint32_t*)&dummy.ip_end;

  if ( 0 == class_size)
    {
      class_size = 24;
    }
  else
    {
      dummy.pfx_size = 32 - class_size;
    }

  dummy.flags |= F_INETNUM_FORCE_PROC;

  uint32_t inc_f = 0x1;

  int i = class_size -1;
  while (i--)
    {
      inc_f <<= 1;
      inc_f |= 0x1;
    }
  inc_f++;

  p_md_obj next = pos->next;

  while ( *dummy.d_ip_start < *dummy.d_ip_end - 1 )
    {
      __inet_obj block;
      if ( NULL != (block=find_block_by_address(object->parent, &dummy, object)) )
        {
          if ( block->nservers.offset > 0 )
            {
              dummy.nservers = block->nservers;
            }
        }

      if (next)
        {
          ((__ch_funct) next->ptr)->call(next, &dummy, arg);
        }
      else
        {
          CH_PROC_ITEM(&dummy);
        }

      *dummy.d_ip_start += inc_f;

    }

  return 0;
}

static __ch_funct
register_ch_funct(pmda base, _chf function)
{
  if ( NULL == function)
    {
      return NULL;
    }

  __ch_funct ptr = md_alloc_le(base, sizeof(_ch_funct), 0, NULL);

  if ( NULL == ptr)
    {
      ABORT("register_ch_funct: md_alloc_le failed\n");
    }

  ptr->call = function;

  return ptr;
}

mda chf_data =
  { 0 };

static int
walk_test(__inet_obj object, __def_ophdr option_header, void* arg)
{

  object->tree_level = option_header->ufield.level;
  object->ns_level = option_header->ufield.ns_level;

  if (object->flags & F_INETNUM_MISC_00)
    {
      DEBUG("%s: already processed\n", object->fullpath);
      return 0;
    }

  if (global_opt.handle.flags & F_GH_PRINT)
    {

      __ch_funct chf = (__ch_funct ) chf_data.first->ptr;

      chf->call(chf_data.first, object, option_header);

    }
  else
    {
      int b_count = option_header->ufield.level * 4;
      char *b_level = malloc(b_count + 1);

      memset(b_level, 0x2D, b_count);
      b_level[b_count] = 0x0;

      printf("|%s: %s\n", b_level, basename(object->fullpath));

      free(b_level);
    }

  return 0;
}

static int
walk_zone_tree(__inet_obj object, __def_ophdr option_header, __d_zb_c0 callback,
    uint32_t flags)
{
  int retval = 0;

  option_header->ufield.level++;

  if (object->nservers.offset > 0)
    {
      option_header->ufield.ns_level++;
    }

  if (callback(object, option_header, NULL))
    {
      retval = 1;
      goto finish;
    }

  p_md_obj ptr = md_first(&object->child_objects);

  while (ptr)
    {
      if (walk_zone_tree((__inet_obj ) ptr->ptr, option_header, callback,
          flags))
        {
          retval = 2;
          goto finish;
        }
      ptr = ptr->next;
    }

  finish: ;

  option_header->ufield.level--;

  if (object->nservers.offset > 0)
    {
      option_header->ufield.ns_level--;
    }

  return retval;
}

static int
g_h_deepcp_mrr(void *source, void *dest, void *d_ptr)
{

  __g_match src_pmd = (__g_match) source;
  if ( src_pmd->flags & F_GM_IS_MOBJ)
    {
      pmda t_md = calloc(1, sizeof(mda));

      md_copy(src_pmd->next, t_md, sizeof(_g_match), g_h_deepcp_mrr);

      __g_match dest_pmd = (__g_match) d_ptr;

      dest_pmd->next = t_md;
    }

  return 0;
}

static int
preproc(void)
{
  int r;
  _g_handle *hdl = &global_opt.handle;

  dt_set_zone(hdl);

  hdl->v_b0 = malloc(65535);
  hdl->v_b0_sz = 65535;

  if (!(hdl->flags & F_GH_HASMATCHES) && _match_rr.count > 0)
    {
      if ((r = md_copy(&_match_rr, &hdl->_match_rr, sizeof(_g_match),
          g_h_deepcp_mrr)))
        {
          ERROR("load_print_params: %s: md_copy(_match_rr, handle) failed\n",
              hdl->file);
          return 2000;
        }
      if (hdl->_match_rr.offset)
        {
          DEBUG("load_print_params: %s: commit %llu matches to handle\n",
              hdl->file, (unsigned long long int ) hdl->_match_rr.offset);

          hdl->flags |= F_GH_HASMATCHES;
        }

    }

  if (g_load_strm(hdl))
    {
      return 1;
    }

  if (g_load_lom(hdl))
    {
      return 1;
    }

  hdl->ifrh_l1 = g_ipcbm;

  if (global_opt.flags & F_STDH_HAVE_PRINT_ANY)
    {
      hdl->fd_out = fileno(stdout);
      hdl->g_proc4 = g_omfp_eassemble;
      hdl->w_d = g_omfp_write;
      hdl->act_mech = &hdl->print_mech;

      hdl->flags |= F_GH_PRINT;

    }

  if (global_opt.flags & F_STDH_HAVE_PRINT)
    {
      if (g_op_load_print_mech(hdl, &hdl->print_mech, global_opt.print_str,
          hdl->v_b0_sz))
        {
          ERROR("main: g_op_load_print_mech (print_mech) failed: '%s'\n",
              global_opt.root);
          return 1;
        }
    }

  return 0;
}

static int
init(void)
{

  mda base =
    { 0 };
  _def_ophdr option_header =
    { 0 };

  check_glob_opts();

  md_init_le(&base, 65535);

  option_header.base = &base;

  global_opt.load(global_opt.path, &option_header, load_inetnum4_item);

  int retval = 0;

  if ( NULL == option_header.root)
    {
      ERROR("main: root zone '%s' not found\n", global_opt.root);
      retval = 2;
      goto cleanup;
    }

  if (option_header.root->nservers.offset == 0)
    {
      ERROR("main: root zone '%s' contains no valid 'nserver' records\n",
          global_opt.root);
      retval = 2;
      goto cleanup;
    }

  if ( NULL != global_opt.loc_serv_name)
    {
      option_header.root->servername = global_opt.loc_serv_name;
    }

  if ( NULL != global_opt.loc_email)
    {
      option_header.root->email = global_opt.loc_email;
    }

  DEBUG("init: loaded %llu records\n",
      (unsigned long long int )option_header.base->offset);

  size_t offset = (size_t) &((__inet_obj) NULL)->pfx_size;

  if (int_sort(option_header.base, offset, 1, F_SORT_DESC))
    {
      retval = 1;
      goto cleanup;
    }

  if (link_hierarchy_tree(option_header, option_header.root, NULL))
    {
      ERROR("main: link_hierarchy_tree failed: '%s'\n", global_opt.root);
      retval = 1;
      goto cleanup;
    }

  DEBUG("init: inetnum hierarchy tree created\n");

  if (preproc())
    {
      ERROR("main: preproc failed: '%s'\n", global_opt.root);
      retval = 1;
      goto cleanup;
    }

  if ( NULL != global_opt.pre_print_str)
    {
      if (g_op_load_print_mech(&global_opt.handle,
          &global_opt.handle.pre_print_mech, global_opt.pre_print_str,
          strlen(global_opt.pre_print_str) + 1))
        {
          ERROR("main: g_op_load_print_mech (pre_print_mech) failed: '%s'\n",
              global_opt.root);
          retval = 1;
          goto cleanup;
        }
      else
        {
          pmda p_act_mech = global_opt.handle.act_mech;
          global_opt.handle.act_mech = &global_opt.handle.pre_print_mech;
          global_opt.handle.g_proc4(&global_opt.handle, option_header.root,
          NULL);
          global_opt.handle.act_mech = p_act_mech;
        }
    }

  md_init_le(&chf_data, 16);

  register_ch_funct(&chf_data, ch_zone4_breakdown_0);
  register_ch_funct(&chf_data, ch_zone_nservers_1);

  walk_zone_tree(option_header.root, &option_header, walk_test, 0);

  if ( NULL != global_opt.post_print_str)
    {
      if (g_op_load_print_mech(&global_opt.handle,
          &global_opt.handle.post_print_mech, global_opt.post_print_str,
          strlen(global_opt.post_print_str) + 1))
        {
          ERROR("main: g_op_load_print_mech (post_print_mech) failed: '%s'\n",
              global_opt.root);
          retval = 1;
          goto cleanup;
        }
      else
        {
          global_opt.handle.act_mech = &global_opt.handle.post_print_mech;
          global_opt.handle.g_proc4(&global_opt.handle, option_header.root,
          NULL);
        }

    }

  cleanup: ;

  md_g_free_l(&base);

  return retval;
}

int
main(int argc, char *argv[])
{
  int retval;

  if ((retval = setup_sighandlers()))
    {
      ERROR("main: could not setup signal handlers [%d]\n", retval);
      abort();
    }

  retval = parse_args(argc, argv, gg_f_ref, NULL, 0);

  if (retval == -2 || retval == -1)
    {
      return retval;
    }

  if ( NULL == global_opt.load)
    {
      ERROR("main: no --build option selected\n");
      return 1;
    }

  initialize_regexes();

  retval = init();

  release_regexes();

  return retval;
}
