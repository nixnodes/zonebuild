/*
 * m_string.c
 *
 *  Created on: Dec 4, 2013
 *      Author: reboot
 */

#include "m_string.h"
#include "memory_t.h"

#include "common.h"
#include "m_general.h"
#include "m_lom.h"
#include "lc_oper.h"
#include "arg_proc.h"
#include "l_error.h"
#include "arg_opts.h"

#include <regex.h>

uint8_t l_sfo;

int
g_cprg(void *arg, int m, int match_i_m, int fn_flags, int regex_flags,
    uint32_t flags, void *opt)
{
  char *buffer = g_pg(arg, m);

  if (!buffer)
    {
      return 1113;
    }

  /*size_t a_i = strlen(buffer);

   if (!a_i)
   {
   return 0;
   }

   if (a_i > MAX_CPRG_STRING)
   {
   return 1114;
   }*/

  __g_match pgm = g_global_register_match();

  if (!pgm)
    {
      return 1114;
    }

  pgm->data = buffer;

  pgm->match_i_m = match_i_m;
  pgm->flags = flags;

  pgm->g_oper_ptr = g_oper_and;
  pgm->flags |= F_GM_NAND;

  switch (flags & F_GM_TYPES)
    {
  case F_GM_ISREGEX:
    pgm->regex_flags = regex_flags | REG_NOSUB;

    break;
  case F_GM_ISMATCH:

    break;
  case F_GM_ISFNAME:
    pgm->fname_flags = fn_flags;

    break;
    }

  //bzero(&_match_rr_l, sizeof(_match_rr_l));
  _match_rr_l.ptr = (void *) pgm;
  //_match_rr_l.flags = F_LM_CPRG;

  if ( NULL != ar_find(&ar_vref, AR_VRP_OPT_TARGET_FD))
    {
      pgm->flags |= F_GM_TFD;
      ar_remove(&ar_vref, AR_VRP_OPT_TARGET_FD);
    }

  __ar_vrp ptr;

  if ( NULL != (ptr = ar_find(&ar_vref, AR_VRP_OPT_TARGET_LOOKUP)))
    {
      pgm->field = (char*) ptr->arg;
      ar_remove(&ar_vref, AR_VRP_OPT_TARGET_LOOKUP);
    }

  ar_remove(&ar_vref, AR_VRP_OPT_NEGATE_MATCH);

  l_sfo = L_STFO_FILTER;

  return 0;
}

int
g_commit_strm_regex(__g_handle hdl, char *field, char *m, int reg_i_m,
    int regex_flags, uint32_t flags)
{
  md_init(&hdl->_match_rr, 32);

  int r = 0;

  __g_match pgm = md_alloc(&hdl->_match_rr, sizeof(_g_match));

  if (!pgm)
    {
      r = 3401;
      goto end;
    }

  pgm->match_i_m = reg_i_m;
  pgm->regex_flags = regex_flags | REG_NOSUB;
  pgm->flags = flags;

  pgm->g_oper_ptr = g_oper_and;
  pgm->flags |= F_GM_NAND;

  pgm->dtr.hdl = hdl;

  if (!(hdl->g_proc1_lookup
      && (pgm->pmstr_cb = hdl->g_proc1_lookup(hdl->_x_ref, field, hdl->mv1_b,
      MAX_VAR_LEN, &pgm->dtr))))
    {
      r = 3402;
      goto end;
    }
  pgm->field = field;

  int re;
  if ((re = regcomp(&pgm->preg, m, pgm->regex_flags)))
    {
      r = 3403;
      goto end;
    }

  pgm->match = m;

  end:

  if (r)
    {
      md_unlink(&hdl->_match_rr, hdl->_match_rr.pos);
    }
  else
    {
      l_sfo = L_STFO_FILTER;
    }

  return r;
}

static void *
dt_rval_default_generic(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  return (void*) (arg + ((__d_drt_h) mppd)->hdl->jm_offset);
}

static int
g_proc_strm(__g_handle hdl, pmda match_rr, int *loaded)
{
  p_md_obj ptr = md_first(match_rr);
  __g_match _m_ptr;

  while (ptr)
    {
      _m_ptr = (__g_match) ptr->ptr;
      if ( _m_ptr->flags & F_GM_TYPES_STR )
        {
          if (NULL != _m_ptr->field)
            {
              if (NULL == hdl->g_proc1_lookup )
                {
                  ERROR("g_proc_strm: %s: g_proc_strm: no lookup table exists for this log (report this): [%s]\n", hdl->file, _m_ptr->field);
                  return 11;
                }
              _m_ptr->dtr.hdl = hdl;
              if ( NULL == (_m_ptr->pmstr_cb = hdl->g_proc1_lookup(hdl->_x_ref, _m_ptr->field, hdl->mv1_b, MAX_VAR_LEN, &_m_ptr->dtr)) )
                {
                  ERROR("g_proc_strm: %s: g_proc_strm: field lookup failed: '%s'\n", hdl->file, _m_ptr->field);
                  return 12;
                }

            }
          else
            {
              _m_ptr->dtr.hdl = hdl;
              _m_ptr->pmstr_cb = dt_rval_default_generic;
            }

          _m_ptr->match = _m_ptr->data;

          if (_m_ptr->flags & F_GM_ISREGEX)
            {
              int re;
              if ((re = regcomp(&_m_ptr->preg, _m_ptr->match, _m_ptr->regex_flags)))
                {
                  ERROR("g_proc_strm: %s: [%d]: regex compilation failed : %s\n", hdl->file, re, _m_ptr->match);
                  return 3;
                }
            }

          *loaded += 1;
        }
      else if (_m_ptr->flags & F_GM_IS_MOBJ)
        {
          int rt;
          if ((rt = g_proc_strm(hdl, _m_ptr->next, loaded)))
            {
              return rt;
            }
        }
      ptr = ptr->next;
    }

  return 0;
}

int
g_load_strm(__g_handle hdl)
{
  g_setjmp(0, "g_load_strm", NULL, NULL);

  if ((hdl->flags & F_GH_HASSTRM))
    {
      return 0;
    }

  int loaded = 0;
  int rt;
  if ((rt = g_proc_strm(hdl, &hdl->_match_rr, &loaded)))
    {
      return rt;
    }

  if (loaded > 0)
    {
      hdl->flags |= F_GH_HASSTRM;
      DEBUG("g_load_strm: %s: pre-processed %d string match filters\n",
          hdl->file, loaded);

      return 0;
    }
  else
    {
      if (0 == rt)
        {
          return 0;
        }
      else
        {
          DEBUG(
              "g_load_strm: %s: [%d] string matches specified, but none were processed\n",
              hdl->file, rt);
          return 1;
        }
    }

}

static int
regex_determine_negated(void)
{
  if ( NULL != ar_find(&ar_vref, AR_VRP_OPT_NEGATE_MATCH))
    {
      return REG_NOMATCH;
    }
  else
    {
      return 0;
    }
}

int
opt_g_d_regex(void *arg, int m, void *opt)
{
  return g_cprg(arg, m, regex_determine_negated(), 0, 0, F_GM_ISREGEX, opt);
}

int
opt_g_d_regexi(void *arg, int m, void *opt)
{
  return g_cprg(arg, m, regex_determine_negated(), 0, REG_ICASE, F_GM_ISREGEX,
      opt);
}

int
opt_g_d_match(void *arg, int m, void *opt)
{
  return g_cprg(arg, m, default_determine_negated(), 0, 0, F_GM_ISMATCH, opt);
}

int
opt_g_d_fname(void *arg, int m, void *opt)
{
  return g_cprg(arg, m, default_determine_negated(), 0, 0, F_GM_ISFNAME, opt);
}

int
opt_g_d_fnamei(void *arg, int m, void *opt)
{
#if defined FNM_CASEFOLD
  return g_cprg(arg, m, fname_determine_negated(), FNM_CASEFOLD, 0,
      F_GM_ISFNAME, opt);
#else
  return 9910;
#endif
}
