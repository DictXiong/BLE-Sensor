/*
 * htu21d.c
 *
 *  Created on: 2023Äê8ÔÂ4ÈÕ
 *      Author: me
 */

#include <stddef.h>
#include <assert.h>
#include "i2cspm.h"

#include "htu21d.h"

/* src: https://github.com/hepingood/htu21d/blob/master/src/driver_htu21d.c */
#define HTU21D_ADDRESS        0x80        /**< iic device address */
#define HTU21D_COMMAND_TRIG_TEMP_HOLD_MASTER        0xE3        /**< trigger temperature measurement with hold master command */
#define HTU21D_COMMAND_TRIG_HUMI_HOLD_MASTER        0xE5        /**< trigger humidity measurement with hold master command */
#define HTU21D_COMMAND_TRIG_TEMP_NO_HOLD_MASTER     0xF3        /**< trigger temperature measurement command */
#define HTU21D_COMMAND_TRIG_HUMI_NO_HOLD_MASTER     0xF5        /**< trigger humidity measurement command */
#define HTU21D_COMMAND_WRITE_USER_REGISTER          0xE6        /**< write user register command */
#define HTU21D_COMMAND_READ_USER_REGISTER           0xE7        /**< read user register command */
#define HTU21D_COMMAND_SOFT_RESET                   0xFE        /**< soft reset command */

#define HTU21D_USER_REGISTER_EXPECTED_RESOLUTION    0x80        /**< mask for user register resolution */
#define HTU21D_USER_REGISTER_END_OF_BATTERY         0x40        /**< mask for user register end of battery */

static uint8_t htu21d_crc(uint16_t value, uint32_t crc) {
    uint32_t polynom = 0x988000U;
    uint32_t msb     = 0x800000U;
    uint32_t mask    = 0xFF8000U;
    uint32_t result  = (uint32_t)value << 8;

    while (msb != 0x80)                                                       /* check the msb */
    {
        if ((result & msb) != 0)                                              /* check the result */
        {
            result = ((result ^ polynom) & mask) | (result & (~mask));        /* get the new result */
        }
        msb >>= 1;                                                            /* right shift 1 */
        mask >>= 1;                                                           /* right shift 1 */
        polynom >>= 1;                                                        /* right shift 1 */
    }
    if (result == crc)                                                        /* check the result */
    {
        return 0;                                                             /* success return 0 */
    }
    else
    {
        return 1;                                                             /* return error */
    }
}


uint8_t htu21d_is_online(I2C_TypeDef *i2c) {
  uint8_t userreg;
  if (htu21d_read_userreg(i2c, &userreg) == i2cTransferDone) {
    return 1;
  }
  return 0;
}


int8_t htu21d_read_userreg(I2C_TypeDef *i2c, uint8_t *userreg) {
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t i2c_write_data[1];
  uint8_t i2c_read_data[1];

  seq.addr = HTU21D_ADDRESS;
  seq.flags = I2C_FLAG_WRITE_READ;
  i2c_write_data[0] = HTU21D_COMMAND_READ_USER_REGISTER;
  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len = 1;
  seq.buf[1].data = i2c_read_data;
  seq.buf[1].len = 1;

  ret = I2CSPM_Transfer(i2c, &seq);
  if (ret == i2cTransferDone) {
    *userreg = i2c_read_data[0];
  }
  return (int8_t) ret;
}

int8_t htu21d_write_userreg(I2C_TypeDef *i2c, uint8_t userreg) {
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t i2c_write_data[2];

  seq.addr = HTU21D_ADDRESS;
  seq.flags = I2C_FLAG_WRITE;
  i2c_write_data[0] = HTU21D_COMMAND_WRITE_USER_REGISTER;
  i2c_write_data[1] = userreg;
  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len = 2;

  ret = I2CSPM_Transfer(i2c, &seq);
  return (int8_t) ret;
}

int8_t htu21d_init(I2C_TypeDef *i2c) {
  uint8_t userreg;
  while(htu21d_read_userreg(i2c, &userreg) != i2cTransferDone);
  userreg |= HTU21D_USER_REGISTER_EXPECTED_RESOLUTION;
  while(htu21d_write_userreg(i2c, userreg) != i2cTransferDone);
  while(htu21d_read_userreg(i2c, &userreg) != i2cTransferDone);
  assert(userreg & HTU21D_USER_REGISTER_EXPECTED_RESOLUTION);
  return i2cTransferDone;
}

int8_t htu21d_is_end_of_battery(I2C_TypeDef *i2c, uint8_t *end_of_battery) {
  uint8_t userreg;
  I2C_TransferReturn_TypeDef ret;
  ret = htu21d_read_userreg(i2c, &userreg);
  if (ret == i2cTransferDone) {
    *end_of_battery = (userreg & HTU21D_USER_REGISTER_END_OF_BATTERY) == HTU21D_USER_REGISTER_END_OF_BATTERY;
  }
  return (int8_t) ret;
}

int8_t htu21d_read_humidity(I2C_TypeDef *i2c, uint16_t *humidity) {
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t i2c_write_data[1];
  uint8_t i2c_read_data[3];

  seq.addr = HTU21D_ADDRESS;
  seq.flags = I2C_FLAG_WRITE_READ;
  i2c_write_data[0] = HTU21D_COMMAND_TRIG_HUMI_HOLD_MASTER;
  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len = 1;
  seq.buf[1].data = i2c_read_data;
  seq.buf[1].len = 3;

  ret = I2CSPM_Transfer(i2c, &seq);
  if (ret != i2cTransferDone) {
    return (int8_t) ret;
  }
  uint16_t data;
  data = ((uint16_t)i2c_read_data[0] << 8) | i2c_read_data[1];
  if (htu21d_crc(data, i2c_read_data[2]) != 0) {
    return 110;
  }
  *humidity = data;

  return (int8_t) i2cTransferDone;
}


int8_t htu21d_read_temperature(I2C_TypeDef *i2c, uint16_t *temperature) {
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t i2c_write_data[1];
  uint8_t i2c_read_data[3];

  seq.addr = HTU21D_ADDRESS;
  seq.flags = I2C_FLAG_WRITE_READ;
  i2c_write_data[0] = HTU21D_COMMAND_TRIG_TEMP_HOLD_MASTER;
  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len = 1;
  seq.buf[1].data = i2c_read_data;
  seq.buf[1].len = 3;

  ret = I2CSPM_Transfer(i2c, &seq);
  if (ret != i2cTransferDone) {
    return (int8_t) ret;
  }
  uint16_t data;
  data = ((uint16_t)i2c_read_data[0] << 8) | i2c_read_data[1];
  if (htu21d_crc(data, i2c_read_data[2]) != 0) {
	return 110;
  }
  *temperature = data;

  return (int8_t) i2cTransferDone;
}
