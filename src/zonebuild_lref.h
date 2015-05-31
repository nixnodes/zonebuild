/*
 * lref_gen3.h
 *
 *  Created on: Dec 5, 2013
 *      Author: reboot
 */

#ifndef LREF_GEN3_H_
#define LREF_GEN3_H_

#include "common.h"

#include "fp_types.h"

#include <stdint.h>
#include <inttypes.h>

__d_format_block zone_format_block, zone_format_block_batch,
    zone_format_block_exp;

__d_ref_to_pval ref_to_val_ptr_zone;

_d_rtv_lk ref_to_val_lk_zone;

void
dt_set_zone(__g_handle hdl);


#endif /* LREF_GEN3_H_ */
