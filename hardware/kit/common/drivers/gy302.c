/*
 * gy302.c
 *
 *  Created on: 2023Äê8ÔÂ5ÈÕ
 *      Author: me
 */
#include <stddef.h>
#include "i2cspm.h"

#include "gy302.h"


#define GY302_ADDRESS                  0x46
#define GY302_COMMAND_POWER_ON         0x01
#define GY302_COMMAND_RESET            0x07
#define GY302_COMMAND_OTH_READ         0x20

uint8_t gy302_is_online(I2C_TypeDef *i2c) {
  if (gy302_command(i2c, GY302_COMMAND_POWER_ON) == i2cTransferDone) {
    return 1;
  }
  return 0;
}

int8_t gy302_command(I2C_TypeDef *i2c, uint8_t command) {
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t i2c_write_data[1];

  seq.addr = GY302_ADDRESS;
  seq.flags = I2C_FLAG_WRITE;
  i2c_write_data[0] = command;
  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len = 1;

  ret = I2CSPM_Transfer(i2c, &seq);
  return (int8_t) ret;
}

int8_t gy302_request_measure(I2C_TypeDef *i2c) {
  return gy302_command(i2c, GY302_COMMAND_OTH_READ);
}

int8_t gy302_read_lx(I2C_TypeDef *i2c, uint16_t *lx) {
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t i2c_read_data[2];
  seq.addr = GY302_ADDRESS;
  seq.flags = I2C_FLAG_READ;
  seq.buf[0].data = i2c_read_data;
  seq.buf[0].len = 2;

  ret = I2CSPM_Transfer(i2c, &seq);
  if (ret == i2cTransferDone) {
    *lx = ((uint16_t)i2c_read_data[0] << 8) | i2c_read_data[1];
  }
  
  return (int8_t) ret;
}
