/*
 * sht4x.h
 *
 *      Author: me@beardic.cn
 */

#ifndef HARDWARE_KIT_COMMON_DRIVERS_SHT4X_H_
#define HARDWARE_KIT_COMMON_DRIVERS_SHT4X_H_

#include "i2cspm.h"

#define STATUS_CRC_ERR 110

int8_t sht4x_read_serial(I2C_TypeDef *i2c, uint32_t* serial);
uint8_t sht4x_is_online(I2C_TypeDef *i2c);
int8_t sht4x_request_measure(I2C_TypeDef *i2c);
int8_t sht4x_read_measurements(I2C_TypeDef *i2c, uint16_t* temperature, uint16_t* humidity);

#endif /* HARDWARE_KIT_COMMON_DRIVERS_SHT4X_H_ */
