/*
 * msc_ring_store.c
 *
 *  Created on: 2023年9月23日
 *      Author: me
 */


#include "msc_ring_store.h"

#if defined(EFR32BG22C224F512GM32)
#define MSC_RING_STORE_BASE (280 * 1024)
#define MSC_RING_STORE_SIZE (200 * 1024)
#elif defined(EFR32BG22C112F352GM32)
#define MSC_RING_STORE_BASE (200 * 1024)
#define MSC_RING_STORE_SIZE (150 * 1024)
#else
#error "Unsupported device"
#endif

/* meta data map
 * 0:3   0x01234567
 * 4:7  0x89abcdef
 * 8:11 ring_start
 * 12:15 ring_end
 * 16:19 0x01234567
 * 20:23 0x89abcdef
*/

bool inited = false;
static uint32_t ring_start = 0;
static uint32_t ring_end = 0;
static uint8_t element_size = 0;
static uint16_t ring_store_numel = 0;


static uint8_t msc_ring_init(uint8_t ele_size) {
  CMU_ClockEnable(cmuClock_MSC, true);
  MSC_INIT();
  element_size = ele_size;
  ring_store_numel = (MSC_RING_STORE_SIZE / element_size);
  inited = true;
  return MSC_RING_ERR_SUCCESS;
}

static uint8_t msc_ring_append(uint8_t *data) {
  if (!inited) {
    return MSC_RING_ERR_NOT_INITED;
  }
  uint8_t *p_data = (uint8_t *)MSC_RING_STORE_BASE;
  uint16_t next = (ring_end + 1) % ring_store_numel;
  if (next == ring_start) {
    ring_start = (ring_start + 1) % ring_store_numel;
  }
  MSC_WriteWord(p_data + ring_end * element_size, data, element_size);
  ring_end = next;
  return MSC_RING_ERR_SUCCESS;
}

static uint8_t msc_ring_append_with_time(uint8_t *data) {
  uint8_t *data_with_time = (uint8_t *)malloc(element_size);
  uint32_t time = RTCC_CounterGet() >> 15;
  memcpy(data_with_time, &time, 4);
  memcpy(data_with_time + 4, data, element_size - 4);
  uint8_t ret = msc_ring_append(data_with_time);
  free(data_with_time);
  return ret;
}

static uint16_t mcs_ring_size() {
  if (!inited) {
    return 0;
  }
  if (ring_end == ring_start) {
    return 0;
  } else if (ring_end > ring_start) {
    return ring_end - ring_start;
  } else {
    return ring_store_numel - ring_start + ring_end;
  }
}

static uint8_t *mcs_ring_get(uint16_t index) {
  if (!inited) {
    return MSC_RING_ERR_NOT_INITED;
  }
  if (index >= mcs_ring_size()) {
    return MSC_RING_ERR_OUT_OF_RANGE;
  }
  uint16_t real_index = (ring_start + index) % ring_store_numel;
  uint8_t *p_data = (uint8_t *)MSC_RING_STORE_BASE;
  uint8_t *data = p_data + real_index * element_size;
  return data;
}
