#ifndef __I2C_H
#define __I2C_H

#include "esp_log.h"
#include "driver/i2c.h"

esp_err_t i2c_master_driver_initialize(int i2c_master_port, int i2c_master_sda_io, int i2c_master_scl_io, int i2c_master_freq_hz, int i2c_master_rx_buf_disable, int i2c_master_tx_buf_disable);

#endif