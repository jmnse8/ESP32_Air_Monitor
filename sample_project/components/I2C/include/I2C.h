#ifndef __I2C_H
#define __I2C_H

#include "esp_log.h"
#include "driver/i2c.h"

#define I2C_TIMEOUT_MS              1000

#define ACK_CHECK_EN                0x1
#define ACK_CHECK_DIS               0x0

#define ACK_VAL                     0x0
#define NACK_VAL                    0x1

#define SI7021_I2C_ADDR             0x40

esp_err_t i2c_master_driver_initialize(int i2c_master_port, int i2c_master_sda_io, int i2c_master_scl_io, int i2c_master_freq_hz, int i2c_master_rx_buf_disable, int i2c_master_tx_buf_disable);

esp_err_t readResponseBytes(const i2c_port_t i2c_num, uint8_t *output, const size_t nbytes, void *intf_ptr);

esp_err_t writeCommandBytes(const i2c_port_t i2c_num, const uint8_t *i2c_command, const size_t nbytes, void *intf_ptr);

#endif