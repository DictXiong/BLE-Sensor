/*
 * bmp280.c
 *
 *  Created on: 2023Äê8ÔÂ5ÈÕ
 *      Author: me
 */
#include <stddef.h>
#include "i2cspm.h"

#include "bmp280.h"

/*! @name I2C addresses */
#define BMP2_I2C_ADDR                                 UINT8_C(0xEC)

/*! @name Register addresses */
#define BMP2_REG_CHIP_ID                              UINT8_C(0xD0)
#define BMP2_REG_SOFT_RESET                           UINT8_C(0xE0)
#define BMP2_REG_STATUS                               UINT8_C(0xF3)
#define BMP2_REG_CTRL_MEAS                            UINT8_C(0xF4)
#define BMP2_REG_CONFIG                               UINT8_C(0xF5)
#define BMP2_REG_PRES_MSB                             UINT8_C(0xF7)
#define BMP2_REG_PRES_LSB                             UINT8_C(0xF8)
#define BMP2_REG_PRES_XLSB                            UINT8_C(0xF9)
#define BMP2_REG_TEMP_MSB                             UINT8_C(0xFA)
#define BMP2_REG_TEMP_LSB                             UINT8_C(0xFB)
#define BMP2_REG_TEMP_XLSB                            UINT8_C(0xFC)
#define BMP2_REG_DIG_T1_LSB                           UINT8_C(0x88)

/*! @name Calibration parameters' relative position */
#define BMP2_DIG_T1_LSB_POS                           UINT8_C(0)
#define BMP2_DIG_T1_MSB_POS                           UINT8_C(1)
#define BMP2_DIG_T2_LSB_POS                           UINT8_C(2)
#define BMP2_DIG_T2_MSB_POS                           UINT8_C(3)
#define BMP2_DIG_T3_LSB_POS                           UINT8_C(4)
#define BMP2_DIG_T3_MSB_POS                           UINT8_C(5)
#define BMP2_DIG_P1_LSB_POS                           UINT8_C(6)
#define BMP2_DIG_P1_MSB_POS                           UINT8_C(7)
#define BMP2_DIG_P2_LSB_POS                           UINT8_C(8)
#define BMP2_DIG_P2_MSB_POS                           UINT8_C(9)
#define BMP2_DIG_P3_LSB_POS                           UINT8_C(10)
#define BMP2_DIG_P3_MSB_POS                           UINT8_C(11)
#define BMP2_DIG_P4_LSB_POS                           UINT8_C(12)
#define BMP2_DIG_P4_MSB_POS                           UINT8_C(13)
#define BMP2_DIG_P5_LSB_POS                           UINT8_C(14)
#define BMP2_DIG_P5_MSB_POS                           UINT8_C(15)
#define BMP2_DIG_P6_LSB_POS                           UINT8_C(16)
#define BMP2_DIG_P6_MSB_POS                           UINT8_C(17)
#define BMP2_DIG_P7_LSB_POS                           UINT8_C(18)
#define BMP2_DIG_P7_MSB_POS                           UINT8_C(19)
#define BMP2_DIG_P8_LSB_POS                           UINT8_C(20)
#define BMP2_DIG_P8_MSB_POS                           UINT8_C(21)
#define BMP2_DIG_P9_LSB_POS                           UINT8_C(22)
#define BMP2_DIG_P9_MSB_POS                           UINT8_C(23)
#define BMP2_DIG_P10_POS                              UINT8_C(24)
#define BMP2_CALIB_DATA_SIZE                          UINT8_C(25)

#define BMP2_MSBLSB_TO_U16(msb, lsb)                  (((uint16_t)msb << 8) | ((uint16_t)lsb))
/*! Calibration parameter of temperature data */
uint16_t dig_t1;
int16_t dig_t2;
int16_t dig_t3;
/*! Calibration parameter of pressure data */
uint16_t dig_p1;
int16_t dig_p2;
int16_t dig_p3;
int16_t dig_p4;
int16_t dig_p5;
int16_t dig_p6;
int16_t dig_p7;
int16_t dig_p8;
int16_t dig_p9;
int8_t dig_p10;


int8_t bmp280_read_data(I2C_TypeDef *i2c, uint8_t *data_raw, uint8_t addr, uint8_t len) {
  uint8_t txbuf[1] = {addr};
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;

  seq.addr = BMP2_I2C_ADDR;
  seq.flags = I2C_FLAG_WRITE_READ;
  seq.buf[0].data = txbuf;
  seq.buf[0].len = 1;
  seq.buf[1].data = data_raw;
  seq.buf[1].len = len;

  ret = I2CSPM_Transfer(i2c, &seq);
  return ret;
}

int8_t bmp280_write_byte(I2C_TypeDef *i2c, uint8_t addr, uint8_t data) {
  uint8_t txbuf[2] = {addr, data};
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;

  seq.addr = BMP2_I2C_ADDR;
  seq.flags = I2C_FLAG_WRITE;
  seq.buf[0].data = txbuf;
  seq.buf[0].len = 2;

  ret = I2CSPM_Transfer(i2c, &seq);
  return ret;
}

uint8_t bmp280_is_online(I2C_TypeDef *i2c) {
  uint8_t id;
  if (bmp280_read_data(i2c, &id, BMP2_REG_CHIP_ID, 1) == i2cTransferDone) {
    return 1;
  }
  return 0;
}

int8_t bmp280_prepare_calib(I2C_TypeDef *i2c) {
  uint8_t temp[25];
  while(bmp280_read_data(i2c, temp, BMP2_REG_DIG_T1_LSB, BMP2_CALIB_DATA_SIZE) != 0);
  dig_t1 = (uint16_t) (BMP2_MSBLSB_TO_U16(temp[BMP2_DIG_T1_MSB_POS], temp[BMP2_DIG_T1_LSB_POS]));
  dig_t2 = (int16_t) (BMP2_MSBLSB_TO_U16(temp[BMP2_DIG_T2_MSB_POS], temp[BMP2_DIG_T2_LSB_POS]));
  dig_t3 = (int16_t) (BMP2_MSBLSB_TO_U16(temp[BMP2_DIG_T3_MSB_POS], temp[BMP2_DIG_T3_LSB_POS]));
  dig_p1 = (uint16_t) (BMP2_MSBLSB_TO_U16(temp[BMP2_DIG_P1_MSB_POS], temp[BMP2_DIG_P1_LSB_POS]));
  dig_p2 = (int16_t) (BMP2_MSBLSB_TO_U16(temp[BMP2_DIG_P2_MSB_POS], temp[BMP2_DIG_P2_LSB_POS]));
  dig_p3 = (int16_t) (BMP2_MSBLSB_TO_U16(temp[BMP2_DIG_P3_MSB_POS], temp[BMP2_DIG_P3_LSB_POS]));
  dig_p4 = (int16_t) (BMP2_MSBLSB_TO_U16(temp[BMP2_DIG_P4_MSB_POS], temp[BMP2_DIG_P4_LSB_POS]));
  dig_p5 = (int16_t) (BMP2_MSBLSB_TO_U16(temp[BMP2_DIG_P5_MSB_POS], temp[BMP2_DIG_P5_LSB_POS]));
  dig_p6 = (int16_t) (BMP2_MSBLSB_TO_U16(temp[BMP2_DIG_P6_MSB_POS], temp[BMP2_DIG_P6_LSB_POS]));
  dig_p7 = (int16_t) (BMP2_MSBLSB_TO_U16(temp[BMP2_DIG_P7_MSB_POS], temp[BMP2_DIG_P7_LSB_POS]));
  dig_p8 = (int16_t) (BMP2_MSBLSB_TO_U16(temp[BMP2_DIG_P8_MSB_POS], temp[BMP2_DIG_P8_LSB_POS]));
  dig_p9 = (int16_t) (BMP2_MSBLSB_TO_U16(temp[BMP2_DIG_P9_MSB_POS], temp[BMP2_DIG_P9_LSB_POS]));
  dig_p10 = (int8_t) ((uint8_t)(temp[BMP2_DIG_P10_POS]));
  return i2cTransferDone;
}

int8_t bmp280_init(I2C_TypeDef *i2c) {
  while(bmp280_prepare_calib(i2c) != 0);
  while(bmp280_write_byte(i2c, BMP2_REG_CTRL_MEAS, 0x25) != 0);
  while(bmp280_write_byte(i2c, BMP2_REG_CONFIG, 0x00) != 0);
  return i2cTransferDone;
}

int8_t bmp280_request_measure(I2C_TypeDef *i2c) {
  return bmp280_write_byte(i2c, BMP2_REG_CTRL_MEAS, 0x25);
}

int8_t bmp280_read_measurements(I2C_TypeDef *i2c, int16_t *temperature, uint32_t *pressure) {
  uint8_t data_raw[6];
  I2C_TransferReturn_TypeDef ret;
  ret = bmp280_read_data(i2c, data_raw, BMP2_REG_PRES_MSB, 6);
  if (ret != i2cTransferDone) {
    return ret;
  }
  int32_t adc_P = (int32_t) (((uint32_t) data_raw[0] << 12) | ((uint32_t) data_raw[1] << 4) | ((uint32_t) data_raw[2] >> 4));
  int32_t adc_T = (int32_t) (((uint32_t) data_raw[3] << 12) | ((uint32_t) data_raw[4] << 4) | ((uint32_t) data_raw[5] >> 4));
  int32_t t_fine;
  {
    int32_t var1, var2;
    var1 = ((((adc_T>>3) - ((int32_t)dig_t1<<1))) * ((int32_t)dig_t2)) >> 11;
    var2 = (((((adc_T>>4) - ((int32_t)dig_t1)) * ((adc_T>>4) - ((int32_t)dig_t1))) >> 12) * ((int32_t)dig_t3)) >> 14;
    t_fine = var1 + var2;
    *temperature = (int16_t)((t_fine * 5 + 128) >> 8);
  }
  {
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_p6;
    var2 = var2 + ((var1*(int64_t)dig_p5)<<17);
    var2 = var2 + (((int64_t)dig_p4)<<35);
    var1 = ((var1 * var1 * (int64_t)dig_p3)>>8) + ((var1 * (int64_t)dig_p2)<<12);
    var1 = (((((int64_t)1)<<47)+var1))*((int64_t)dig_p1)>>33;
    if (var1 == 0) {
      *pressure = 0; // avoid exception caused by division by zero
    } else {
      p = 1048576-adc_P;
      p = (((p<<31)-var2)*3125)/var1;
      var1 = (((int64_t)dig_p9) * (p>>13) * (p>>13)) >> 25;
      var2 = (((int64_t)dig_p8) * p) >> 19;
      p = ((p + var1 + var2) >> 8) + (((int64_t)dig_p7)<<4);
      *pressure = (uint32_t)p;
    }
  }
  return ret;
}
