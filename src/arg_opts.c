/*
 * arg_opts.c
 *
 *  Created on: Dec 5, 2013
 *      Author: reboot
 */

#include "arg_opts.h"

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <fnmatch.h>

#include "arg_proc.h"

static int
o_hello(void *arg, int m, void *opt)
{
  printf("hello\n");

  return 0;
}

#include "zonebuild.h"



_gg_opt gg_f_ref[] =
  {
    { .id = 0x0001, .on = "--version", .ac = 0, .op = o_hello },
    { .id = 0x0002, .on = "--build", .ac = 1, .op = o_zb_build },
    { .id = 0x0002, .on = "--root", .ac = 1, .op = o_zb_setroot },
    { .id = 0x0002, .on = "--path", .ac = 1, .op = o_zb_setpath },
    { 0x0 } };

