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

#include <math.h>

regex_t pr_is_ipv4;

_stdh_go global_opt =
  { 0 };

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

static int
commit_inetnum4_item(char *name, __inet_obj object)
{
  object->d_ip_start = (uint32_t*) object->ip_start.data;
  object->d_ip_end = (uint32_t*) object->ip_end.data;

  char *sptr = name, *ptr_tmp, *size;

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
          FAULT("commit_inetnum_item: '%s': IP octet out of range: '%d' %s\n",
              name, c);
          return 1;
        }

      object->ip_start.data[i] = (uint8_t) c;

      if (sptr[0] != 0x2E)
        {
          break;
        }

      if (0 == i)
        {
          FAULT("commit_inetnum_item: '%s': invalid IP address %s\n", name);
          return 1;
        }

      sptr++;
    }

  if (sptr[0] != 0x5F)
    {
      FAULT("commit_inetnum_item: invalid inetnum file: '%s' %s\n", name);
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

  /* get netmask */
  object->pfx_mask = (UINT_MAX << (32 - c));

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
      FAULT("validate_inetnum_block: '%s': invalid prefix %s\n", name);
      return 1;
    }
  return 0;
}

int
load_inetnum4_item(char *name, void *data)
{
  __def_ophdr option_header = (__def_ophdr) data;
  __inet_obj object = (__inet_obj) option_header->arg;

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

  if (!strncmp(global_opt.root, name, strlen(global_opt.root)))
    {
      if (NULL != option_header->root)
        {
          DEBUG("load_inetnum4_item: multiple roots, ignoring: %s\n", name);
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
      "load_inetnum4_item: %hhu.%hhu.%hhu.%hhu - %hhu.%hhu.%hhu.%hhu (%u) | [%hhu.%hhu.%hhu.%hhu] - %u %u - %u\n",
      object->ip_start.data[3], object->ip_start.data[2],
      object->ip_start.data[1], object->ip_start.data[0],
      object->ip_end.data[3], object->ip_end.data[2], object->ip_end.data[1],
      object->ip_end.data[0], object->pfx_size, mres[3], mres[2], mres[1],
      mres[0], *object->d_ip_start, ~object->pfx_mask, (object->flags & F_INETNUM_ROOT));

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

      if (-1 == lstat(st_b0, &object->st))
        {
          FAULT("load_inetnum_data: lstat: '%s': %s\n", st_b0);
          md_unlink_le(option_header->base, option_header->base->pos);
          continue;
        }

      if (S_IFREG == (object->st.st_mode & S_IFMT))
        {
          option_header->arg = (void*) object;

          if (callback(pde->d_name, (void*) option_header))
            {
              errno = 0;
              retval = 2;
              break;
            }

          if (!(object->flags & F_INETNUM_OK))
            {
              DEBUG("load_inetnum_data: F_INETNUM_OK not satisfied\n");
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
link_hierarchy_tree(__inet_obj object)
{
  md_init_le(&object->child_objects, 8192);
  object->child_objects.flags |= F_MDA_REFPTR;

  p_md_obj ptr = object->parent->first;

  while (ptr)
    {
      __inet_obj ptr_object = (__inet_obj) ptr->ptr;

      if ( (ptr_object->d_ip_start >= object->d_ip_start) &&
          (ptr_object->d_ip_end <= object->d_ip_end) )
        {
          ptr_object = md_alloc_le(&object->child_objects, 0, 0, ptr_object);
        }

      ptr = ptr->next;
    }

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

  check_glob_opts();

  if ( NULL == global_opt.load)
    {
      ERROR("main: NULL global_opt.load\n");
      return 1;
    }

  mda base =
    { 0 };

  md_init_le(&base, 65535);

  initialize_regexes();

  _def_ophdr option_header =
    { 0 };

  retval = 0;

  option_header.base = &base;

  global_opt.load(global_opt.path, &option_header, load_inetnum4_item);

  if ( NULL == option_header.root)
    {
      ERROR("main: could not find root: '%s'\n", global_opt.root);
      retval = 2;
      goto _exit;
    }

  if (link_hierarchy_tree(option_header.root))
    {
      ERROR("main: link_hierarchy_tree failed: '%s'\n", global_opt.root);
      retval = 2;
      goto _exit;
    }

  _exit: ;

  release_regexes();

  md_g_free_l(&base);

  return retval;
}
