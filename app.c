/***************************************************************************//**
 * @file app.c
 * @brief Silicon Labs Empty Example Project
 *
 * This example demonstrates the bare minimum needed for a Blue Gecko C application
 * that allows Over-the-Air Device Firmware Upgrading (OTA DFU). The application
 * starts advertising after boot and restarts advertising after a connection is closed.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"
#include "i2cspm.h"
#include "supply_voltage.h"
#include "htu21d.h"
#include "sht4x.h"
#include "bmp280.h"
#include "gy302.h"

#include "app.h"

/* Print boot message */
static void bootMessage(struct gecko_msg_system_boot_evt_t *bootevt);

/* Timer handles */
#define TIMER_HANDLE_REQ_MEAS 0
#define TIMER_HANDLE_READ_MEAS 1

/* Flag for indicating DFU Reset must be performed */
static uint8_t boot_to_dfu = 0;
uint8_t is_htu21d_online = 0;
uint8_t is_sht4x_online = 0;
uint8_t is_bmp280_online = 0;
uint8_t is_gy302_online = 0;
uint8_t is_end_of_battery = 0;
uint8_t supply_voltage_count = 0;

/* report data
 * 0     0x12
 * 1     device_status
 * 2:3   supply voltage (*1000)
 * 4:5   htu21d temperature (raw)
 * 6:7   htu21d humidity (raw)
 * 8:11  bmp280 pressure (*25600)
 * 12:13 bmp280 temperature (*100)
 * 14:15 gy302 light (*1.2)
 * 16    0x23
 */
uint8_t report_data[17];


void requestData() {
  if (is_sht4x_online) sht4x_request_measure(I2C0);
  if (is_bmp280_online) bmp280_request_measure(I2C0);
  if (is_gy302_online) gy302_request_measure(I2C0);
  printLog("----- request data -----\r\n");
}


void updateData() {
  report_data[0] = 0x12;
  report_data[16] = 0x23;
  uint8_t device_status = 0;
  if (true) {
    printLog("operating sht4x ...\r\n");

    uint32_t serial;

    int8_t ret;

    ret = sht4x_read_serial(I2C0, &serial);
    printLog("serial: %d %lu\r\n", ret, serial);


    uint16_t temperature, humidity;
    ret = sht4x_read_measurements(I2C0, &temperature, &humidity);
    if (ret == 0) {
      report_data[4] = temperature;
      report_data[5] = temperature >> 8;
      report_data[6] = humidity;
      report_data[7] = humidity >> 8;
      device_status |= (1 << 4);
    }
    printLog("temperature: %d %d\r\n", ret, ((21875 * (int32_t)temperature) >> 13) - 45000);
    printLog("humidity: %d %d\r\n", ret, ((15625 * (int32_t)humidity) >> 13) - 6000);
  }
  // TODO change to else-if
  if (is_htu21d_online) {
    printLog("operating htu21d ...\r\n");
    int8_t ret;
    uint16_t data;
    uint8_t end_of_battery;
    ret = htu21d_read_temperature(I2C0, &data);
    if (ret == 0) {
      report_data[4] = data;
      report_data[5] = data >> 8;
      device_status |= (1 << 7);
    }
    printLog("temperature: %d %d\r\n", ret, (int16_t)data * 17572 / 65536 - 4685);
    ret = htu21d_read_humidity(I2C0, &data);
    if (ret == 0) {
      report_data[6] = data;
      report_data[7] = data >> 8;
      device_status |= (1 << 7);
    }
    printLog("humidity: %d %u\r\n", ret, (data) * 1250 / 65536 - 60);
    ret = htu21d_is_end_of_battery(I2C0, &end_of_battery);
    if (ret == 0 && end_of_battery) is_end_of_battery = 1;
  }
  if (is_bmp280_online) {
    printLog("operating bmp280 ...\r\n");
    int8_t ret;
    int16_t temperature;
    uint32_t pressure;
    ret = bmp280_read_measurements(I2C0, &temperature, &pressure);
    if (ret == 0) {
      report_data[8] = pressure;
      report_data[9] = pressure >> 8;
      report_data[10] = pressure >> 16;
      report_data[11] = pressure >> 24;
      report_data[12] = temperature;
      report_data[13] = temperature >> 8;
      device_status |= (1 << 6);
    }
    printLog("%d P=%lu T=%d\r\n", ret, pressure >> 8, temperature);
  }
  if (is_gy302_online) {
    printLog("operating gy302 ...\r\n");
    int8_t ret;
    uint16_t data;
    ret = gy302_read_lx(I2C0, &data);
    if (ret == 0) {
      report_data[14] = data;
      report_data[15] = data >> 8;
      device_status |= (1 << 5);
    }
    printLog("lux: %d %u\r\n", ret, data * 10 / 12);
  }
  // measure supply voltage every 15 min
  if (supply_voltage_count % 30 == 0) {
	uint16_t voltage = get_supply_voltage();
	report_data[2] = voltage;
	report_data[3] = voltage >> 8;
	if (voltage <= 2200) {
    is_end_of_battery = 1;
	}
	printLog("supply voltage: %u\r\n", voltage);
	supply_voltage_count = 0;
  }
  supply_voltage_count++;
  {
    device_status |= is_end_of_battery;
    report_data[1] = device_status;
    printLog("device_status: %u\r\n", device_status);
  }
  flushLog();
  gecko_cmd_gatt_server_write_attribute_value(gattdb_report, 0, sizeof(report_data), report_data);
}


/* Main application */
void appMain(gecko_configuration_t *pconfig)
{
#if DISABLE_SLEEP > 0
  pconfig->sleep.flags = 0;
#endif

  /* Initialize debug prints. Note: debug prints are off by default. See DEBUG_LEVEL in app.h */
  initLog();

  /* Initialize stack */
  gecko_init(pconfig);

  /* init i2c */
  I2CSPM_Init_TypeDef i2c_init;
  i2c_init.port = I2C0;
  i2c_init.sclPort = gpioPortC;
  i2c_init.sclPin = 0;
  i2c_init.sdaPort = gpioPortC;
  i2c_init.sdaPin = 1;
  i2c_init.i2cRefFreq = 9600;
  i2c_init.i2cMaxFreq = 115200;
  I2CSPM_Init(&i2c_init);

  /* init supply voltage (IADC0) */
  init_supply_voltage();

  sl_sleeptimer_delay_millisecond(1000);
  /* detect i2c slaves: sht4x/htu21d */
  is_sht4x_online = sht4x_is_online(I2C0);
  is_htu21d_online = htu21d_is_online(I2C0);
  if (is_sht4x_online) {
    // sht4x_init(I2C0);
  } else if (is_htu21d_online) {
    htu21d_init(I2C0);
  }
  printLog("sht4x is online: %d\r\n", is_sht4x_online);
  printLog("htu21d is online: %d\r\n", is_htu21d_online);
  /* detect i2c slaves: bmp280 */
  is_bmp280_online = bmp280_is_online(I2C0);
  if (is_bmp280_online) {
    bmp280_init(I2C0);
  }
  printLog("bmp280 is online: %d\r\n", is_bmp280_online);
  /* detect i2c slaves: gy302 */
  is_gy302_online = gy302_is_online(I2C0);
  printLog("gy302 is online: %d\r\n", is_gy302_online);
  flushLog();

  /* clear report buffer */
  memset(report_data, 0, sizeof(report_data));

  while (1) {
    /* Event pointer for handling events */
    struct gecko_cmd_packet* evt;

    /* if there are no events pending then the next call to gecko_wait_event() may cause
     * device go to deep sleep. Make sure that debug prints are flushed before going to sleep */
    if (!gecko_event_pending()) {
      flushLog();
    }

    /* Check for stack event. This is a blocking event listener. If you want non-blocking please see UG136. */
    evt = gecko_wait_event();

    /* Handle events */
    switch (BGLIB_MSG_ID(evt->header)) {
      /* This boot event is generated when the system boots up after reset.
       * Do not call any stack commands before receiving the boot event.
       * Here the system is set to start advertising immediately after boot procedure. */
      case gecko_evt_system_boot_id:

        bootMessage(&(evt->data.evt_system_boot));
        printLog("boot event - starting advertising\r\n");

        /* Set tx power to 0 dBm */
        gecko_cmd_system_set_tx_power(0);

        /* Set adv on all 3 channels */
        //gecko_cmd_le_gap_set_advertise_channel_map(0, 7);

        /* Set advertising parameters. 100ms advertisement interval.
         * The first parameter is advertising set handle
         * The next two parameters are minimum and maximum advertising interval, both in
         * units of (milliseconds * 1.6).
         * The last two parameters are duration and maxevents left as default. */
        gecko_cmd_le_gap_set_advertise_timing(0, 2560, 3200, 0, 0);

        /* Start general advertising and enable connections. */
        gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable);

        /* start measurement timer */
#if DEBUG_LEVEL
        gecko_cmd_hardware_set_soft_timer(32768 * 10, TIMER_HANDLE_REQ_MEAS, 0);
#else
        gecko_cmd_hardware_set_soft_timer(32768 * 30, TIMER_HANDLE_REQ_MEAS, 0);
#endif
        break;

      case gecko_evt_le_connection_opened_id:

        printLog("connection opened\r\n");
        /*Set timing parameters
        * Connection interval: 200 msec
        * Slave latency: as defined
        * Supervision timeout: 4500 msec The value in milliseconds must be larger than
        * (1 + latency) * max_interva * 2, where max_interval is given in milliseconds
        */
#if DEBUG_LEVEL
        gecko_cmd_le_connection_set_timing_parameters(evt->data.evt_le_connection_opened.connection, 160, 160, 5, 450, 0, 0xFFFF);
#else
        /* see: https://docs.silabs.com/bluetooth/3.2/general/system-and-performance/optimizing-current-consumption-in-bluetooth-low-energy-devices */
        gecko_cmd_le_connection_set_timing_parameters(evt->data.evt_le_connection_opened.connection, 700, 760, 5, 3200, 0, 0xFFFF);
#endif

        break;

      case gecko_evt_le_connection_closed_id:

        printLog("connection closed, reason: 0x%2.2x\r\n", evt->data.evt_le_connection_closed.reason);

        /* Check if need to boot to OTA DFU mode */
        if (boot_to_dfu) {
          /* Enter to OTA DFU mode */
          gecko_cmd_system_reset(2);
        } else {
          /* Restart advertising after client has disconnected */
          gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable);
        }
        break;

      /* Events related to OTA upgrading
         ----------------------------------------------------------------------------- */

      /* Check if the user-type OTA Control Characteristic was written.
       * If ota_control was written, boot the device into Device Firmware Upgrade (DFU) mode. */
      case gecko_evt_gatt_server_user_write_request_id:

        if (evt->data.evt_gatt_server_user_write_request.characteristic == gattdb_ota_control) {
          /* Set flag to enter to OTA mode */
          boot_to_dfu = 1;
          /* Send response to Write Request */
          gecko_cmd_gatt_server_send_user_write_response(
            evt->data.evt_gatt_server_user_write_request.connection,
            gattdb_ota_control,
            bg_err_success);

          /* Close connection to enter to DFU OTA mode */
          gecko_cmd_le_connection_close(evt->data.evt_gatt_server_user_write_request.connection);
        }
        break;
      /* tick timer to refresh measurement */
      case gecko_evt_hardware_soft_timer_id:
        switch(evt->data.evt_hardware_soft_timer.handle) {
          case TIMER_HANDLE_REQ_MEAS:
            requestData();
            gecko_cmd_hardware_set_soft_timer(32768 >> 2, TIMER_HANDLE_READ_MEAS, 1);
            break;
          case TIMER_HANDLE_READ_MEAS:
            updateData();
            break;
          default:
            break;
        }
    	break;
      /* Add additional event handlers as your application requires */

      default:
        break;
    }
  }
}

/* Print stack version and local Bluetooth address as boot message */
static void bootMessage(struct gecko_msg_system_boot_evt_t *bootevt)
{
#if DEBUG_LEVEL
  bd_addr local_addr;
  int i;

  printLog("stack version: %u.%u.%u\r\n", bootevt->major, bootevt->minor, bootevt->patch);
  local_addr = gecko_cmd_system_get_bt_address()->address;

  printLog("local BT device address: ");
  for (i = 0; i < 5; i++) {
    printLog("%2.2x:", local_addr.addr[5 - i]);
  }
  printLog("%2.2x\r\n", local_addr.addr[0]);
#endif
}
