/*
 * sort.c
 *
 *  Created on: Dec 4, 2013
 *      Author: reboot
 */

#include "sort_hdr.h"

#include "m_comp.h"
#include <arg_proc.h>
#include "gv_off.h"
#include <lc_oper.h>
#include <l_error.h>
#include <str.h>

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

mda _md_gsort =
  { 0 };
char *g_sort_field = NULL;

uint32_t g_sort_flags = 0;

static int
gs_t_is_lower(void *s, void *d, void * t_ptr)
{
  __p_srd psrd = (__p_srd) t_ptr;
  g_t64_ptr_p t64_ptr = (g_t64_ptr_p) psrd->g_t_ptr_c;
  return (t64_ptr(s, psrd->off) < t64_ptr(d, psrd->off));
}

static int
gs_t_is_higher(void *s, void *d, void * t_ptr)
{
  __p_srd psrd = (__p_srd) t_ptr;
  g_t64_ptr_p t64_ptr = (g_t64_ptr_p) psrd->g_t_ptr_c;
  return (t64_ptr(s, psrd->off) > t64_ptr(d, psrd->off));
}

static int
gs_ts_is_lower(void *s, void *d, void * t_ptr)
{
  __p_srd psrd = (__p_srd) t_ptr;
  g_ts64_ptr_p ts64_ptr = (g_ts64_ptr_p) psrd->g_t_ptr_c;
  return (ts64_ptr(s, psrd->off) < ts64_ptr(d, psrd->off));
}

static int
gs_ts_is_higher(void *s, void *d, void * t_ptr)
{
  __p_srd psrd = (__p_srd) t_ptr;
  g_ts64_ptr_p ts64_ptr = (g_ts64_ptr_p) psrd->g_t_ptr_c;
  return (ts64_ptr(s, psrd->off) > ts64_ptr(d, psrd->off));
}

static int
gs_tf_is_lower(void *s, void *d, void * t_ptr)
{
  __p_srd psrd = (__p_srd) t_ptr;
  g_tf_ptr_p tf_ptr = (g_tf_ptr_p) psrd->g_t_ptr_c;
  return (tf_ptr(s, psrd->off) < tf_ptr(d, psrd->off));
}

static int
gs_tf_is_higher(void *s, void *d, void * t_ptr)
{
  __p_srd psrd = (__p_srd) t_ptr;
  g_tf_ptr_p tf_ptr = (g_tf_ptr_p) psrd->g_t_ptr_c;
  return (tf_ptr(s, psrd->off) > tf_ptr(d, psrd->off));
}

static int
gs_s_is_lower(void *s, void *d, void * t_ptr)
{
  __p_srd psrd = (__p_srd) t_ptr;
  __g_proc_v sb_ref = (__g_proc_v) psrd->g_t_ptr_c;
  return (strcasecmp(((char*) sb_ref(s, NULL, psrd->m_buf1, sizeof(psrd->m_buf1), (void*)&psrd->mppd)),
          ((char*) sb_ref(d, NULL, psrd->m_buf2, sizeof(psrd->m_buf2), (void*)&psrd->mppd))) < 0);
}

static int
gs_s_is_higher(void *s, void *d, void * t_ptr)
{
  __p_srd psrd = (__p_srd) t_ptr;
  __g_proc_v sb_ref = (__g_proc_v) psrd->g_t_ptr_c;
  return (strcasecmp(((char*) sb_ref(s, NULL, psrd->m_buf1, sizeof(psrd->m_buf1), (void*)&psrd->mppd)),
          ((char*) sb_ref(d, NULL, psrd->m_buf2, sizeof(psrd->m_buf2), (void*)&psrd->mppd))) > 0);
}

static void
g_swap_p(void **x, void **y)
{
  void *z = *x;
  *x = *y;
  *y = z;
}

#define G_SWAP_P(x, y) void *z = *x;*x = *y;*y = z;

static int
g_insertion_sort_exec(pmda m_ptr, __p_srd psrd)
{
  g_setjmp(0, "g_insertion_sort_exec", NULL, NULL);
  int ret = 0;
  void **ref_arr = calloc(m_ptr->offset + 1, sizeof(void*));

  if (md_md_to_array(m_ptr, ref_arr))
    {
      ret = 1;
      goto cl_end;
    }

  int64_t j, i, length = (int64_t) m_ptr->offset;

  for (i = 1; i < length; i++)
    {
      j = i;

      while (j > 0 && !psrd->m_op(ref_arr[j], ref_arr[j - 1], psrd))
        {
          g_swap_p(&ref_arr[j], &ref_arr[j - 1]);
          j--;
        }
    }

  if (md_array_to_md(ref_arr, m_ptr))
    {
      ret = 2;
    }

  cl_end: ;

  free(ref_arr);

  return ret;
}

static int
g_selection_sort_exec(pmda m_ptr, __p_srd psrd)
{
  g_setjmp(0, "g_selection_sort_exec", NULL, NULL);
  int ret = 0;
  void **ref_arr = calloc(m_ptr->offset + 1, sizeof(void*));

  if (md_md_to_array(m_ptr, ref_arr))
    {
      ret = 1;
      goto cl_end;
    }

  off_t i = m_ptr->offset;
  for (i = 0; i < m_ptr->offset - 1; ++i)
    {
      off_t j, min = i;

      for (j = i + 1; j < m_ptr->offset; ++j)
        {

          if (!psrd->m_op(ref_arr[j], ref_arr[min], psrd))
            {
              min = j;
            }
        }

      g_swap_p(&ref_arr[i], &ref_arr[min]);
    }

  if (md_array_to_md(ref_arr, m_ptr))
    {
      ret = 2;
    }

  cl_end: ;

  free(ref_arr);

  return ret;
}

void
g_qsort(void **arr, int64_t left, int64_t right, __p_srd psrd)
{
  int64_t i = left, j = right;

  void *pivot = arr[(left + right) / 2];

  while (i <= j)
    {
      while (psrd->m_op_opp(arr[i], pivot, psrd))
        i++;
      while (psrd->m_op(arr[j], pivot, psrd))
        j--;

      if (i <= j)
        {
          g_swap_p(&arr[i], &arr[j]);
          i++;
          j--;
        }
    };

  if (left < j)
    g_qsort(arr, left, j, psrd);

  if (i < right)
    g_qsort(arr, i, right, psrd);
}

int
g_qsort_exec(pmda m_ptr, __p_srd psrd)
{
  int ret = 0;
  void **ref_arr = calloc(m_ptr->offset + 1, sizeof(void*));

  if (md_md_to_array(m_ptr, ref_arr))
    {
      ret = 1;
      goto cl_end;
    }

  g_qsort(ref_arr, 0, (int64_t) m_ptr->offset - 1, psrd);

  if (md_array_to_md(ref_arr, m_ptr))
    {
      ret = 2;
    }

  cl_end: ;

  free(ref_arr);

  return ret;
}

static void
g_heap_siftdown(void **arr, int64_t start, int64_t end, __p_srd psrd)
{
  int64_t root, child;

  root = start;
  while (2 * root + 1 < end)
    {
      child = 2 * root + 1;
      if ((child + 1 < end) && (!psrd->m_op(arr[child], arr[child + 1], psrd)))
        ++child;
      if (!psrd->m_op(arr[root], arr[child], psrd))
        {
          g_swap_p(&arr[child], &arr[root]);
          root = child;
        }
      else
        return;
    }
}

void
g_heapsort(void **ref_arr, int64_t left, int64_t right, __p_srd psrd)
{
  int64_t start, end;

  for (start = (right - 2) / 2; start >= 0; --start)
    g_heap_siftdown(ref_arr, start, right, psrd);

  for (end = (right - 1); end; --end)
    {
      g_swap_p(&ref_arr[end], &ref_arr[0]);
      g_heap_siftdown(ref_arr, 0, end, psrd);
    }
}

static int
g_heapsort_exec(pmda m_ptr, __p_srd psrd)
{
  int ret = 0;
  void **ref_arr = calloc(m_ptr->offset + 1, sizeof(void*));

  if (md_md_to_array(m_ptr, ref_arr))
    {
      ret = 1;
      goto cl_end;
    }

  g_heapsort(ref_arr, 0, (int64_t) m_ptr->offset, psrd);

  if (md_array_to_md(ref_arr, m_ptr))
    {
      ret = 2;
    }

  cl_end: ;

  free(ref_arr);

  return ret;
}

static int
g_swapsort_exec(pmda m_ptr, __p_srd psrd)
{

  p_md_obj ptr, ptr_n;

  uint32_t ml_f = 0;

  for (;;)
    {
      ml_f ^= F_INT_GSORT_LOOP_DID_SORT;
      ptr = md_first(m_ptr);
      while (ptr && ptr->next)
        {
          ptr_n = (p_md_obj) ptr->next;

          if (psrd->m_op(ptr->ptr, ptr_n->ptr, psrd))
            {
              ptr = md_swap_s(m_ptr, ptr, ptr_n);
              if (!(ml_f & F_INT_GSORT_LOOP_DID_SORT))
                {
                  ml_f |= F_INT_GSORT_LOOP_DID_SORT;
                }
            }
          else
            {
              ptr = ptr->next;
            }
        }

      if (!(ml_f & F_INT_GSORT_LOOP_DID_SORT))
        {
          break;
        }

    }

  if ((psrd->flags & F_SORT_RESETPOS))
    {
      m_ptr->pos = m_ptr->r_pos = md_first(m_ptr);
    }

  return 0;
}

int
preproc_sort_numeric(__g_handle hdl, int vbs, char *field, uint32_t flags,
    __p_srd psrd)
{
  void *g_fh_f = NULL, *g_fh_s = NULL, *g_fh_f_opp = NULL, *g_fh_s_opp = NULL;

  switch (flags & F_SORT_ORDER)
    {
  case F_SORT_DESC:
    psrd->m_op = gs_t_is_lower;
    psrd->m_op_opp = gs_t_is_higher;
    g_fh_f = gs_tf_is_lower;
    g_fh_f_opp = gs_tf_is_higher;
    g_fh_s = gs_ts_is_lower;
    g_fh_s_opp = gs_ts_is_higher;
    break;
  case F_SORT_ASC:
    psrd->m_op = gs_t_is_higher;
    psrd->m_op_opp = gs_t_is_lower;
    g_fh_f = gs_tf_is_higher;
    g_fh_f_opp = gs_tf_is_lower;
    g_fh_s = gs_ts_is_higher;
    g_fh_s_opp = gs_ts_is_lower;
    break;
  default:
    ;
    return 3;
    }

  int vb = 0;

  if ( NULL != hdl)
    {
      psrd->off = (size_t) hdl->g_proc2(hdl->_x_ref, field, &vb);
    }
  else
    {
      vb = vbs;
    }

  if (0 == vb)
    {
      return 9;
    }

  switch (vb)
    {
  case -32:
    psrd->m_op = g_fh_f;
    psrd->m_op_opp = g_fh_f_opp;
    psrd->g_t_ptr_c = g_tf_ptr;
    break;
  case 1:
    psrd->g_t_ptr_c = g_t8_ptr;
    break;
  case 2:
    psrd->g_t_ptr_c = g_t16_ptr;
    break;
  case 4:
    psrd->g_t_ptr_c = g_t32_ptr;
    break;
  case 8:
    psrd->g_t_ptr_c = g_t64_ptr;
    break;
  case -2:
    psrd->g_t_ptr_c = g_ts8_ptr;
    break;
  case -3:
    psrd->g_t_ptr_c = g_ts16_ptr;
    break;
  case -5:
    psrd->g_t_ptr_c = g_ts32_ptr;
    break;
  case -9:
    psrd->g_t_ptr_c = g_ts64_ptr;
    break;
  default:
    return 14;
    }

  if (vb < 0 && vb > -10)
    {
      psrd->m_op = g_fh_s;
      psrd->m_op_opp = g_fh_s_opp;
    }

  return 0;
}

int
g_sort_string(__g_handle hdl, char *field, uint32_t flags, __p_srd psrd)
{
  g_setjmp(0, "g_sort_string", NULL, NULL);
  switch (flags & F_SORT_ORDER)
    {
  case F_SORT_DESC:
    psrd->m_op = gs_s_is_lower;
    psrd->m_op_opp = gs_s_is_higher;
    break;
  case F_SORT_ASC:
    psrd->m_op = gs_s_is_higher;
    psrd->m_op_opp = gs_s_is_lower;
    break;
  default:
    ;
    return 3;
    }

  psrd->g_t_ptr_c = hdl->g_proc1_lookup(hdl->_x_ref, field, psrd->m_buf1,
      sizeof(psrd->m_buf1), (void*) &psrd->mppd);

  if (NULL == psrd->g_t_ptr_c)
    {
      return 7;
    }

  return 0;
}

int
g_check_is_data_numeric(__g_handle hdl, char *field)
{
  int dummy_vb = 0;

  hdl->g_proc2(hdl->_x_ref, field, &dummy_vb);

  return !(0 != dummy_vb);
}

int
g_sort(__g_handle hdl, char *field, uint32_t flags)
{
  g_setjmp(0, "g_sort", NULL, NULL);

  g_xsort_exec_p g_s_ex;

  _srd srd =
    { 0 };
  pmda m_ptr = &hdl->buffer;

  if (!hdl)
    {
      return 1;
    }

  if (!hdl->g_proc2)
    {
      return 2;
    }

  switch (flags & F_SORT_METHOD_ALL)
    {
  case F_SORT_METHOD_SWAP:
    g_s_ex = g_swapsort_exec;
    break;
  case F_SORT_METHOD_HEAP:
    g_s_ex = g_heapsort_exec;
    break;
  case F_SORT_METHOD_Q:
    g_s_ex = g_qsort_exec;
    break;
  case F_SORT_METHOD_INSSORT:
    g_s_ex = g_insertion_sort_exec;
    break;
  case F_SORT_METHOD_SELECT:
    g_s_ex = g_selection_sort_exec;
    break;
  default:
    g_s_ex = g_qsort_exec;
    }

  int ret;

  if (!(flags & F_SORT_TYPE))
    {
      if (!g_check_is_data_numeric(hdl, field))
        {
          flags |= F_SORT_NUMERIC;
        }
      else
        {
          flags |= F_SORT_STRING;
        }
    }

  switch (flags & F_SORT_TYPE)
    {
  case F_SORT_NUMERIC:
    ;
    ret = preproc_sort_numeric(hdl, 0, field, flags, &srd);
    break;
  case F_SORT_STRING:
    ;
    ret = g_sort_string(hdl, field, flags, &srd);
    break;
  default:
    return 11;
    }

  if (0 != ret)
    {
      return ret;
    }

  return g_s_ex(m_ptr, &srd);

}

void
g_invert_sort_order(uint32_t *flags)
{
  switch (*flags & F_SORT_ORDER)
    {
  case F_SORT_ASC:
    *flags ^= F_SORT_ASC;
    *flags |= F_SORT_DESC;
    break;
  case F_SORT_DESC:
    *flags ^= F_SORT_DESC;
    *flags |= F_SORT_ASC;
    break;
    }
}
