/*
 * lref_gen3.c
 *
 *  Created on: Dec 5, 2013
 *      Author: reboot
 */

#include "zonebuild_lref.h"

#include "common.h"
#include "config.h"

#include "zonebuild.h"
#include "str.h"
#include "lref.h"
#include "lref_gen.h"
#include "omfp.h"

#include <stdlib.h>
#include <errno.h>

void
dt_set_zone(__g_handle hdl)
{
  hdl->block_sz = INETO_SZ;
  hdl->d_memb = 1;
  //hdl->g_proc0 = gcb_gen3;
  hdl->g_proc1_lookup = ref_to_val_lk_zone;
  hdl->g_proc2 = ref_to_val_ptr_zone;
  hdl->g_proc3 = zone_format_block;
  hdl->g_proc3_batch = zone_format_block_batch;
  hdl->g_proc3_export = zone_format_block_exp;
  hdl->g_proc4 = g_omfp_norm;
  hdl->jm_offset = (size_t) &((__inet_obj) NULL)->fullpath;

}

int
zone_format_block(void *ptr, char *output)
{
  __inet_obj data = (__inet_obj) ptr;

  return NOTIFY("ZONE: %s\n", data->fullpath);

}

int
zone_format_block_batch(void *ptr, char *output)
{
  __inet_obj data = (__inet_obj) ptr;

  return NOTIFY("ZONE: %s\n", data->fullpath);

}

int
zone_format_block_exp(void *ptr, char *output)
{
  __inet_obj data = (__inet_obj) ptr;

  return NOTIFY("ZONE: %s\n", data->fullpath);

}

#define _MC_ZONE_PATH           "path"
#define _MC_ZONE_SIP            "startip"
#define _MC_ZONE_EIP            "endip"
#define _MC_ZONE_TREELVL        "treelevel"
#define _MC_ZONE_NSLVL          "nslevel"
#define _MC_ZONE_NSCOUNT        "nscount"
#define _MC_ZONE_NSERVER        "nserver"
#define _MC_ZONE_NSGLUE         "nsglue"
#define _MC_ZONE_NSGLUEIP       "nsglueip"
#define _MC_ZONE_PFXSIZE        "pfxsize"
#define _MC_ZONE_PFXMASK        "pfxmask"
#define _MC_ZONE_NLEVEL         "nlevel"
#define _MC_ZONE_RFC2317        "rfc2317"
#define _MC_ZONE_SERVER         "server"
#define _MC_ZONE_EMAIL          "email"
#define _MC_ZONE_HASGLUE        "hasglue"
#define _MC_ZONE_HASCHLD        "chcount"
#define _MC_ZONE_ISSHADOW       "isshadow"
#define _MC_ZONE_PARENTREF      "p:"

static int
ref_to_val_ptr_offset(char *match, size_t *offset, size_t max_size)
{
  char in_dummy[512];
  void *l_next_ref;

  char *s_ptr = l_mppd_shell_ex(match, in_dummy, sizeof(in_dummy),
      &l_next_ref,
      LMS_EX_L,
      LMS_EX_R, F_MPPD_SHX_TZERO);

  if (NULL == s_ptr || 0 == s_ptr[0])
    {
      return 1;
    }

  while (s_ptr[0] && s_ptr[0] != 0x5B)
    {
      s_ptr++;
    }

  if (s_ptr[0] != 0x5B)
    {
      return 1;
    }

  s_ptr++;

  errno = 0;

  *offset = strtoull(s_ptr, NULL, 10);

  if (errno == ERANGE || errno == EINVAL)
    {
      return 1;
    }

  if (*offset < 0 || *offset > max_size - 1)
    {
      return 2;
    }

  return 0;

}

void *
ref_to_val_ptr_zone(void *arg, char *match, int *output)
{
  __inet_obj data = (__inet_obj) arg;

  int size;
  void *ptr;

  if (!strncmp(match, _MC_ZONE_EIP, 5))
    {
      size = ((int) sizeof(uint32_t));
      ptr = &data->ip_end;
    }
  else if (!strncmp(match, _MC_ZONE_SIP, 7))
    {
      size = ((int) sizeof(uint32_t));
      ptr = &data->ip_start;
    }
  else if (!strncmp(match, _MC_ZONE_TREELVL, 9))
    {
      size = ((int) sizeof(data->tree_level));
      ptr = &data->tree_level;
    }
  else if (!strncmp(match, _MC_ZONE_NSLVL, 7))
    {
      size = ((int) sizeof(data->ns_level));
      ptr = &data->ns_level;
    }
  else if (!strncmp(match, _MC_ZONE_PFXSIZE, 7))
    {
      size = ((int) sizeof(data->pfx_size));
      ptr = &data->pfx_size;
    }
  else if (!strncmp(match, _MC_ZONE_PFXMASK, 7))
    {
      size = ((int) sizeof(data->pfx_mask));
      ptr = &data->pfx_mask;
    }
  else if (!strncmp(match, _MC_ZONE_NLEVEL, 6))
    {
      size = ((int) sizeof(data->nrecurse_d));
      ptr = &data->nrecurse_d;
    }
  else if (!strncmp(match, _MC_ZONE_NSGLUE, 6))
    {
      size = ((int) sizeof(uint32_t));
      ptr = &data->nserver_current.glue;
    }
  else if (!strncmp(match, _MC_ZONE_RFC2317, 7))
    {
      size = ((int) sizeof(data->rfc2317));
      ptr = &data->rfc2317;
    }
  else if (!strncmp(match, _MC_ZONE_NSCOUNT, 6))
    {
      size = ((int) sizeof(data->nservers.offset));
      ptr = &data->nservers.offset;
    }
  else if (!strncmp(match, _MC_ZONE_HASGLUE, 5))
    {
      size = ((int) sizeof(data->has_glue));
      ptr = &data->has_glue;
    }
  else if (!strncmp(match, _MC_ZONE_HASCHLD, 5))
    {
      size = ((int) sizeof(data->child_objects.offset));
      ptr = &data->child_objects.offset;
    }
  else if (!strncmp(match, _MC_ZONE_ISSHADOW, 6))
    {
      size = ((int) sizeof(data->is_shadow));
      ptr = &data->is_shadow;
    }
  else
    {
      size = 0;
      ptr = NULL;
    }

  if ( size )
    {
      int retval;
      size_t offset;
      if ( 0 == (retval=ref_to_val_ptr_offset(match, &offset, size)) )
        {
          ptr = (void*)((size_t)ptr + offset);
          size = 1;
        }
      else if ( 2 == retval )
        {
          ptr = NULL;
          size = 0;
        }
    }

  *output = size;

  return ptr;
}

static char *
dt_rval_zone_path(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, ((__inet_obj) arg)->fullpath);
  return output;
}

static char *
dt_rval_zone_sip(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, *((__inet_obj) arg)->d_ip_start);
  return output;
}

static char *
dt_rval_zone_mask(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, ((__inet_obj) arg)->pfx_mask);
  return output;
}

static char *
dt_rval_zone_eip(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, *((__inet_obj) arg)->d_ip_end);
  return output;
}

static char *
dt_rval_zone_tl(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, ((__inet_obj) arg)->tree_level);
  return output;
}

static char *
dt_rval_zone_nslvl(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, ((__inet_obj) arg)->ns_level);
  return output;
}

static char *
dt_rval_zone_nserver(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, ((__inet_obj) arg)->nserver_current.host);
  return output;
}

static char *
dt_rval_zone_nsglue(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, ((__inet_obj) arg)->nserver_current.glue_str);
  return output;
}

static char *
dt_rval_zone_nsglue_ip(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, ((__inet_obj) arg)->nserver_current.glue);
  return output;
}

static char *
dt_rval_zone_nslevel(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, ((__inet_obj) arg)->nrecurse_d);
  return output;
}

static char *
dt_rval_zone_pfxsize(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, ((__inet_obj) arg)->pfx_size);
  return output;
}

static char *
dt_rval_zone_rfc2317(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, ((__inet_obj) arg)->rfc2317);
  return output;
}

static char *
dt_rval_zone_nscount(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, (uint64_t) ((__inet_obj) arg)->nservers.offset);
  return output;
}

static char *
dt_rval_zone_hasglue(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, (uint8_t) ((__inet_obj) arg)->has_glue);
  return output;
}

static char *
dt_rval_zone_server(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, ((__inet_obj) arg)->servername);
  return output;
}

static char *
dt_rval_zone_email(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc, ((__inet_obj) arg)->email);
  return output;
}

static char *
dt_rval_zone_hasch(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc,
      (uint64_t) (unsigned long long int) ((__inet_obj) arg)->child_objects.offset);
  return output;
}

static char *
dt_rval_zone_is_shadow(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  snprintf(output, max_size, ((__d_drt_h ) mppd)->direc,
      (unsigned long long int) ((__inet_obj) arg)->is_shadow);
  return output;
}

static char *
dt_rval_zone_pref(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  __inet_obj object = (__inet_obj) arg;

  if ( NULL != object->parent_link)
    {
      __d_drt_h _mppd = (__d_drt_h ) mppd;
      char *p_b0 = _mppd->fp_rval1(object->parent_link, match, _mppd->tp_b0, sizeof(_mppd->tp_b0),
          _mppd->mppd_next);

      if ( NULL != p_b0)
        {
          snprintf(output, max_size, ((__d_drt_h ) mppd)->direc,
              p_b0);
        }
      else
        {
          output[0] = 0x0;
        }
    }
  else
    {
      output[0] = 0x0;
    }

  return output;

}

static char *
rt_af__zone_pref(void *arg, char *match, char *output, size_t max_size,
    void *_mppd)
{
  __d_drt_h mppd = (__d_drt_h) _mppd;

  mppd->mppd_next = l_mppd_create_copy(mppd);

  mppd->fp_rval1 = mppd->hdl->g_proc1_lookup(arg, match, output, max_size,
      mppd->mppd_next);

  if (NULL == mppd->fp_rval1)
    {
      ERROR("dt_rval_zone_pref: could not resolve: '%s'\n", match);
      return NULL;
    }

  return as_ref_to_val_lk(match, dt_rval_zone_pref, (__d_drt_h) mppd, "%s");
}

void *
ref_to_val_lk_zone(void *arg, char *match, char *output, size_t max_size,
    void *mppd)
{
  PROC_SH_EX(match)

  void *ptr;
  if ((ptr = ref_to_val_lk_generic(arg, match, output, max_size, mppd)))
    {
      return ptr;
    }
  else if (!strncmp(match, _MC_ZONE_PATH, 4))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_path, (__d_drt_h) mppd, "%s");
    }
  else if (!strncmp(match, _MC_ZONE_SIP, 7))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_sip, (__d_drt_h) mppd, "%u");
    }
  else if (!strncmp(match, _MC_ZONE_EIP, 5))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_eip, (__d_drt_h) mppd, "%u");
    }
  else if (!strncmp(match, _MC_ZONE_TREELVL, 9))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_tl, (__d_drt_h) mppd, "%u");
    }
  else if (!strncmp(match, _MC_ZONE_NSLVL, 7))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_nslvl, (__d_drt_h) mppd, "%u");
    }
  else if (!strncmp(match, _MC_ZONE_NSERVER, 7))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_nserver, (__d_drt_h) mppd, "%s");
    }
  else if (!strncmp(match, _MC_ZONE_NSGLUEIP, 8))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_nsglue_ip, (__d_drt_h) mppd, "%u");
    }
  else if (!strncmp(match, _MC_ZONE_NSGLUE, 6))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_nsglue, (__d_drt_h) mppd, "%s");
    }
  else if (!strncmp(match, _MC_ZONE_NLEVEL, 6))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_nslevel, (__d_drt_h) mppd, "%u");
    }
  else if (!strncmp(match, _MC_ZONE_PFXSIZE, 7))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_pfxsize, (__d_drt_h) mppd, "%hhu");
    }
  else if (!strncmp(match, _MC_ZONE_PFXMASK, 7))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_mask, (__d_drt_h) mppd, "%u");
    }
  else if (!strncmp(match, _MC_ZONE_RFC2317, 7))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_rfc2317, (__d_drt_h) mppd, "%hhu");
    }
  else if (!strncmp(match, _MC_ZONE_NSCOUNT, 6))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_nscount, (__d_drt_h) mppd, "%llu");
    }
  else if (!strncmp(match, _MC_ZONE_SERVER, 6))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_server, (__d_drt_h) mppd, "%s");
    }
  else if (!strncmp(match, _MC_ZONE_EMAIL, 5))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_email, (__d_drt_h) mppd, "%s");
    }
  else if (!strncmp(match, _MC_ZONE_HASGLUE, 7))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_hasglue, (__d_drt_h) mppd, "%hhu");
    }
  else if (!strncmp(match, _MC_ZONE_HASCHLD, 5))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_hasch, (__d_drt_h) mppd, "%llu");
    }
  else if (!strncmp(match, _MC_ZONE_ISSHADOW, 6))
    {
      return as_ref_to_val_lk(match, dt_rval_zone_is_shadow, (__d_drt_h) mppd, "%hhu");
    }
  else if (!strncmp(match, _MC_ZONE_PARENTREF, 2))
    {
      return rt_af__zone_pref(arg, &match[2], output, max_size,mppd);
    }

  return NULL;
}

