/*
 * arg_opts.c
 *
 *  Created on: Dec 5, 2013
 *      Author: reboot
 */

#include "arg_opts.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <fnmatch.h>

#include "arg_proc.h"

#include "common.h"

static int
o_hello(void *arg, int m, void *opt)
{
  puts(PACKAGE_NAME "-" PACKAGE_VERSION "_" _ARCH_STR);
  exit(0);
}

static int
o_debug(void *arg, int m, void *opt)
{
  debug_mode = 1;

  return 0;
}

static int
opt_g_negate(void *arg, int m, void *opt)
{
  ar_add(&ar_vref, AR_VRP_OPT_NEGATE_MATCH, -1, NULL);

  ar_vref.flags |= F_MDA_ST_MISC00;

  return 0;
}

#include "zonebuild.h"
#include "m_general.h"
#include "m_lom.h"
#include "m_string.h"

_gg_opt gg_f_ref[] =
  {
    { .id = 0x0001, .on = "--version", .ac = 0, .op = o_hello },
    { .id = 0x0002, .on = "-build", .ac = 1, .op = o_zb_build },
    { .id = 0x0003, .on = "--root", .ac = 1, .op = o_zb_setroot },
    { .id = 0x0004, .on = "--path", .ac = 1, .op = o_zb_setpath },
    { .id = 0x0005, .on = "--debug", .ac = 0, .op = o_debug },
    { .id = 0x0006, .on = "-print", .ac = 1, .op = o_zb_print_str },
    { .id = 0x0007, .on = "-lom", .ac = 1, .op = opt_g_d_lom_match },
    { .id = 0x1000, .on = "!", .ac = 0, .op = opt_g_negate },
    { .id = 0x2002, .on = "-and", .ac = 0, .op = opt_g_operator_and },
    { .id = 0x2003, .on = "-or", .ac = 0, .op = opt_g_operator_or },
    { .id = 0x0004, .on = "(", .ac = 0, .op = opt_g_m_raise_level },
    { .id = 0x0005, .on = ")", .ac = 0, .op = opt_g_m_lower_level },
    { .id = 0x1001, .on = "-l:", .ac = 1, .op = opt_g_lookup },
    { .id = 0x0468, .on = "-regexi", .ac = 1, .op = opt_g_d_regexi },
    { .id = 0x046A, .on = "-regex", .ac = 1, .op = opt_g_d_regex },
    { .id = 0x0441, .on = "-match", .ac = 1, .op = opt_g_d_match },
    { .id = 0x1441, .on = "-name", .ac = 1, .op = opt_g_d_fname },
    { .id = 0x1443, .on = "-namei", .ac = 1, .op = opt_g_d_fnamei },
    { .id = 0x1501, .on = "-preprint", .ac = 1, .op = o_zb_pre_print_str },
    { .id = 0x1502, .on = "-postprint", .ac = 1, .op = o_zb_post_print_str },
    { .id = 0x1502, .on = "--server", .ac = 1, .op = o_zb_setlocsname },
    { .id = 0x1502, .on = "--email", .ac = 1, .op = o_zb_setlocemail },
    { 0x0 } };

