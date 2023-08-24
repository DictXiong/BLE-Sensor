/*
 * htu21d.h
 *
 *      Author: me@beardic.cn
 */

#ifndef HARDWARE_KIT_COMMON_DRIVERS_HTU21D_H_
#define HARDWARE_KIT_COMMON_DRIVERS_HTU21D_H_

#include "i2cspm.h"

uint8_t htu21d_is_online(I2C_TypeDef *i2c);
int8_t htu21d_read_userreg(I2C_TypeDef *i2c, uint8_t *userreg);
int8_t htu21d_write_userreg(I2C_TypeDef *i2c, uint8_t userreg);
int8_t htu21d_init(I2C_TypeDef *i2c);
int8_t htu21d_is_end_of_battery(I2C_TypeDef *i2c, uint8_t *end_of_battery);
int8_t htu21d_read_humidity(I2C_TypeDef *i2c, uint16_t *humidity);
int8_t htu21d_read_temperature(I2C_TypeDef *i2c, uint16_t *temperature);


#endif /* HARDWARE_KIT_COMMON_DRIVERS_HTU21D_H_ */
