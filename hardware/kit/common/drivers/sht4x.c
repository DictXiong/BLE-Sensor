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

static bool sht4x_crc(uint16_t value, uint8_t crc);
static uint8_t sht4x_crc_table[] = {
  0,   49,  98,  83,  196, 245, 166, 151, 185, 136, 219, 234, 125, 76,  31,  46,
  67,  114, 33,  16,  135, 182, 229, 212, 250, 203, 152, 169, 62,  15,  92,  109,
  134, 183, 228, 213, 66,  115, 32,  17,  63,  14,  93,  108, 251, 202, 153, 168,
  197, 244, 167, 150, 1,   48,  99,  82,  124, 77,  30,  47,  184, 137, 218, 235,
  61,  12,  95,  110, 249, 200, 155, 170, 132, 181, 230, 215, 64,  113, 34,  19,
  126, 79,  28,  45,  186, 139, 216, 233, 199, 246, 165, 148, 3,   50,  97,  80,
  187, 138, 217, 232, 127, 78,  29,  44,  2,   51,  96,  81,  198, 247, 164, 149,
  248, 201, 154, 171, 60,  13,  94,  111, 65,  112, 35,  18,  133, 180, 231, 214,
  122, 75,  24,  41,  190, 143, 220, 237, 195, 242, 161, 144, 7,   54,  101, 84,
  57,  8,   91,  106, 253, 204, 159, 174, 128, 177, 226, 211, 68,  117, 38,  23,
  252, 205, 158, 175, 56,  9,   90,  107, 69,  116, 39,  22,  129, 176, 227, 210,
  191, 142, 221, 236, 123, 74,  25,  40,  6,   55,  100, 85,  194, 243, 160, 145,
  71,  118, 37,  20,  131, 178, 225, 208, 254, 207, 156, 173, 58,  11,  88,  105,
  4,   53,  102, 87,  192, 241, 162, 147, 189, 140, 223, 238, 121, 72,  27,  42,
  193, 240, 163, 146, 5,   52,  103, 86,  120, 73,  26,  43,  188, 141, 222, 239,
  130, 179, 224, 209, 70,  119, 36,  21,  59,  10,  89,  104, 255, 206, 157, 172
};
static bool sht4x_crc(uint16_t value, uint8_t crc) {
  uint8_t result = 0xff;
  result = sht4x_crc_table[(uint8_t)(value >> 8) ^ result];
  result = sht4x_crc_table[(uint8_t)(value) ^ result];
  return result == crc;
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
    if (!sht4x_crc(tmp, i2c_read_data[2])) return STATUS_CRC_ERR;
    *serial = tmp << 16;
    tmp = (i2c_read_data[3] << 8) | i2c_read_data[4];
    if (!sht4x_crc(tmp, i2c_read_data[5])) return STATUS_CRC_ERR;
    *serial |= tmp;
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
    if (!sht4x_crc(*temperature, i2c_read_data[2])) return STATUS_CRC_ERR;
    if (!sht4x_crc(*humidity, i2c_read_data[5])) return STATUS_CRC_ERR;
  }

  return (int8_t) ret;
}
