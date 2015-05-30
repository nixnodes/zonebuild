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

#include "signal_t.h"
#include "memory_t.h"
#include "x_f.h"
#include "str.h"
#include "fp_types.h"
#include "arg_proc.h"

#include "zonebuild.h"

#include "common.h"
#include "config.h"

#include "sort_hdr.h"

#include <math.h>

regex_t pr_is_ipv4;

_stdh_go global_opt =
  { .load = NULL, .path = NULL, .root = NULL, .pfx_max_size = 32,
      .pfx_min_size = 8 };

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
      FAULT("o_zb_build: unknown mode: '%s' %s\n", mode);
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
          FAULT(
              "convert_string_to_ipaddr: '%s': IP octet out of range: '%d' %s\n",
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
          FAULT("convert_string_to_ipaddr: '%s': invalid IP address %s\n", name);
          return NULL;
        }

      sptr++;
    }

  return sptr;
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
      FAULT(
          "commit_inetnum4_item: ip string does not terminate properly: '%s' %s\n",
          name);
      return 1;
    }

  //object->d_ip_start = *((uint32_t*)object->ip_start.data);

  sptr++;
  size = sptr;

  int c = atoi(size);

  if (c > 32 || c < 1)
    {
      FAULT("commit_inetnum_item: '%s': prefix size out of range: '%d'%s\n",
          name, c);
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
          NOTIFY("found root entry: '%s'\n", name);
        }
    }

  uint8_t *mres = (uint8_t*) &object->pfx_mask;

  DEBUG(
      "load_inetnum4_item: %hhu.%hhu.%hhu.%hhu - %hhu.%hhu.%hhu.%hhu (%u) | [%hhu.%hhu.%hhu.%hhu] - %u %u - %u, class %hhu\n",
      object->ip_start.data[3], object->ip_start.data[2],
      object->ip_start.data[1], object->ip_start.data[0],
      object->ip_end.data[3], object->ip_end.data[2], object->ip_end.data[1],
      object->ip_end.data[0], object->pfx_size, mres[3], mres[2], mres[1],
      mres[0], *object->d_ip_start, ~object->pfx_mask, (object->flags & F_INETNUM_ROOT), object->pfx_class);

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
              NOTIFY("load_inetnum_data: F_INETNUM_OK not satisfied\n");
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

  IP_PRINT((&object->ip_start));

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

      IP_PRINT((&ptr_object->ip_start));

      if ( ptr_object == parent )
        {
          ERROR("link_hierarchy_tree: %s refusing to link %s/%hhu back to parent [%u]\n",
              b_level, st_b0, ptr_object->pfx_size,
              option_header.ufield.level);
          goto e_loop;
        }
      if ( ptr_object == object )
        {
          ERROR("link_hierarchy_tree: %s refusing to link %s/%hhu back to self [%u]\n",
              b_level,st_b0, ptr_object->pfx_size,
              option_header.ufield.level);
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

  DEBUG("link_hierarchy_tree: %s exiting %s/%hhu [%u] [%u]\n", b_level, st_b0,
      object->pfx_size, option_header.ufield.level, object->pfx_class);

  option_header.ufield.level--;

  free(b_level);

  return 0;
}

static int
walk_test(__inet_obj object, __def_ophdr option_header, void* arg)
{
  int b_count = option_header->ufield.level * 4;

  char *b_level = malloc(b_count +1);

  memset(b_level, 0x2D, b_count);

  b_level[b_count] = 0x0;

  printf("|%s: %s\n", b_level, basename(object->fullpath));

  free(b_level);
  return 0;
}

static int
walk_zone_tree(__inet_obj object, __def_ophdr option_header, __d_zb_c0 callback,
    uint32_t flags)
{
  option_header->ufield.level++;
  if (callback(object, option_header, NULL))
    {
      option_header->ufield.level--;
      return 1;
    }

  p_md_obj ptr = md_first(&object->child_objects);

  while (ptr)
    {
      if (walk_zone_tree((__inet_obj ) ptr->ptr, option_header, callback,
          flags))
        {
          option_header->ufield.level--;
          return 2;
        }
      ptr = ptr->next;
    }

  option_header->ufield.level--;

  return 0;
}

static int
init(void)
{
  _def_ophdr option_header =
    { 0 };

  mda base =
    { 0 };


  check_glob_opts();

  md_init_le(&base, 65535);

  option_header.base = &base;

  global_opt.load(global_opt.path, &option_header, load_inetnum4_item);

  int retval;

  if ( NULL == option_header.root)
    {
      ERROR("main: NULL option_header.root\n");
      retval = 2;
      goto cleanup;
    }

  NOTIFY("init: loaded %llu records\n", (unsigned long long int )
  option_header.base->offset);

  _srd srd0;

  retval = preproc_sort_numeric(NULL, 1, NULL, F_SORT_ASC, &srd0);

  if (retval)
    {
      ERROR("init: preproc_sort_numeric failed: [%d]\n", retval);
      retval = 1;
      goto cleanup;
    }

  srd0.off = (size_t) &((__inet_obj ) NULL)->pfx_size;

  if ((retval = g_qsort_exec(option_header.base, &srd0)))
    {
      ERROR("init: g_qsort_exec failed: [%d]\n", retval);
      retval = 1;
      goto cleanup;
    }

  if (link_hierarchy_tree(option_header, option_header.root, NULL))
    {
      ERROR("main: link_hierarchy_tree failed: '%s'\n", global_opt.root);
      retval = 1;
      goto cleanup;
    }

  NOTIFY("init: inetnum hierarchy tree created\n");

  walk_zone_tree(option_header.root, &option_header, walk_test, 0);

  cleanup: ;

  md_g_free_l(&base);

  return 0;
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
