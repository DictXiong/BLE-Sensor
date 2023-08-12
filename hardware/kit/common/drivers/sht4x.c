/*
 * sht4x.c
 *
 *  Created on: 2023Äê8ÔÂ5ÈÕ
 *      Author: me
 */
#include <stddef.h>
#include "i2cspm.h"

#include "sht4x.h"

#define SHT4X_ADDRESS 0x88
#define SHT4X_CMD_MEASURE_HPM 0xFD
#define SHT4X_CMD_MEASURE_LPM 0xE0
#define SHT4X_CMD_READ_SERIAL 0x89

static uint8_t sht4x_crc(uint16_t value, uint8_t crc);
static uint8_t sht4x_crc(uint16_t value, uint8_t crc) {
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

int8_t sht4x_read_serial(I2C_TypeDef *i2c, uint32_t* serial) {
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t i2c_write_data[1];
  uint8_t i2c_read_data[6];

  seq.addr = SHT4X_ADDRESS;
  seq.flags = I2C_FLAG_WRITE_READ;
  i2c_write_data[0] = SHT4X_CMD_READ_SERIAL;
  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len = 1;
  seq.buf[1].data = i2c_read_data;
  seq.buf[1].len = 6;

  ret = I2CSPM_Transfer(i2c, &seq);
  if (ret == i2cTransferDone) {
    uint16_t tmp;
    tmp = (i2c_read_data[0] << 8) | i2c_read_data[1];
    if (sht4x_crc(tmp, i2c_read_data[2])) return STATUS_CRC_ERR;
    serial = tmp << 16;
    tmp = (i2c_read_data[3] << 8) | i2c_read_data[4];
    if (sht4x_crc(tmp, i2c_read_data[5])) return STATUS_CRC_ERR;
    serial = serial | tmp;
  }
  return (int8_t) ret;
}

uint8_t sht4x_is_online(I2C_TypeDef *i2c) {
  uint32_t serial;
  return sht4x_read_serial(i2c, &serial) == 0;
}

int8_t sht4x_request_measure(I2C_TypeDef *i2c) {
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t i2c_write_data[1];

  seq.addr = SHT4X_ADDRESS;
  seq.flags = I2C_FLAG_WRITE;
  i2c_write_data[0] = SHT4X_CMD_MEASURE_HPM;
  seq.buf[0].data = i2c_write_data;
  seq.buf[0].len = 1;

  ret = I2CSPM_Transfer(i2c, &seq);
  return (int8_t) ret;
}

int8_t sht4x_read_measurements(I2C_TypeDef *i2c, uint16_t *temperature, uint16_t *humidity)
{
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t i2c_read_data[6];
  seq.addr = SHT4X_ADDRESS;
  seq.flags = I2C_FLAG_READ;
  seq.buf[0].data = i2c_read_data;
  seq.buf[0].len = 6;

  ret = I2CSPM_Transfer(i2c, &seq);
  if (ret == i2cTransferDone) {
    *temperature = (i2c_read_data[0] << 8) | i2c_read_data[1];
    *humidity = (i2c_read_data[3] << 8) | i2c_read_data[4];
    if (sht4x_crc(temperature, i2c_read_data[2])) return STATUS_CRC_ERR;
    if (sht4x_crc(humidity, i2c_read_data[5])) return STATUS_CRC_ERR;
  }

  return (int8_t) ret;
}
