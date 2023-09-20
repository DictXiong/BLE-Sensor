/*
 * gy302.h
 *
 *      Author: me@beardic.cn
 */

#ifndef HARDWARE_KIT_COMMON_DRIVERS_GY302_H_
#define HARDWARE_KIT_COMMON_DRIVERS_GY302_H_

#include "i2cspm.h"

uint8_t gy302_is_online(I2C_TypeDef *i2c);
int8_t gy302_command(I2C_TypeDef *i2c, uint8_t command);
int8_t gy302_request_measure(I2C_TypeDef *i2c);
int8_t gy302_read_lx(I2C_TypeDef *i2c, uint16_t *lx);


#endif /* HARDWARE_KIT_COMMON_DRIVERS_GY302_H_ */
