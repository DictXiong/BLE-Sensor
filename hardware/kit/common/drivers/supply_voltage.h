/*
 * supply_voltage.h
 *
 *      Author: me@beardic.cn
 */

#ifndef HARDWARE_KIT_COMMON_DRIVERS_SUPPLY_VOLTAGE_H_
#define HARDWARE_KIT_COMMON_DRIVERS_SUPPLY_VOLTAGE_H_

#include "stdint.h"

void request_supply_voltage();
uint16_t read_supply_voltage();


#endif /* HARDWARE_KIT_COMMON_DRIVERS_SUPPLY_VOLTAGE_H_ */
