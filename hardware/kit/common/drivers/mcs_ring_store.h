/*
 * mcs_ring_store.h
 *
 *  Created on: 2023年9月23日
 *      Author: me
 */

#ifndef HARDWARE_KIT_COMMON_DRIVERS_MCS_RING_STORE_H_
#define HARDWARE_KIT_COMMON_DRIVERS_MCS_RING_STORE_H_

#include "stdint.h"
#include "em_msc.h"

#define MSC_RING_ERR_SUCCESS 0
#define MSC_RING_ERR 1
#define MSC_RING_ERR_NOT_INITED 2

static uint8_t msc_ring_init(uint8_t element_size);
static uint8_t msc_ring_append(uint8_t *data);
static uint8_t msc_ring_append_with_time(uint8_t *data);
static uint16_t mcs_ring_size();
static uint8_t *mcs_ring_get(uint16_t index);


#endif /* HARDWARE_KIT_COMMON_DRIVERS_MCS_RING_STORE_H_ */
