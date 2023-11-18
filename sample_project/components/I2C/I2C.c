#include <stdio.h>
#include "I2C.h"



esp_err_t i2c_master_driver_initialize(int i2c_master_port_p, int i2c_master_sda_io, int i2c_master_scl_io, int i2c_master_freq_hz, int i2c_master_rx_buf_disable, int i2c_master_tx_buf_disable)
{
    //ESP_LOGI("TAG", "%i - %i - %i - %i - %i - %i",   i2c_master_port_p, i2c_master_sda_io, i2c_master_scl_io, i2c_master_freq_hz, i2c_master_rx_buf_disable, i2c_master_tx_buf_disable);

    // i2c_master_port_p, i2c_master_sda_io, i2c_master_scl_io, i2c_master_freq_hz, i2c_master_rx_buf_disable, i2c_master_tx_buf_disable
    int i2c_master_port = i2c_master_port_p;
    /* i2c_config_t conf;

    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = i2c_master_sda_io;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = i2c_master_scl_io;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = i2c_master_freq_hz; */

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = i2c_master_sda_io,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = i2c_master_scl_io,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = i2c_master_freq_hz,
        // .clk_flags = 0,          /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
    };
    
    esp_err_t err = i2c_param_config(i2c_master_port, &conf);
    if (err != ESP_OK) {
        return err;
    }
    return i2c_driver_install(i2c_master_port, conf.mode, i2c_master_rx_buf_disable, i2c_master_tx_buf_disable, 0);
}

esp_err_t readResponseBytes(const i2c_port_t i2c_num, uint8_t *output, const size_t nbytes, void *intf_ptr){

    uint8_t chip_addr = *(uint8_t*)intf_ptr;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);

    // write the 7-bit address of the sensor to the queue, using the last bit
    // to indicate we are performing a read.
    i2c_master_write_byte(cmd, chip_addr << 1 | I2C_MASTER_READ, ACK_CHECK_EN);

    // read nbytes number of bytes from the response into the buffer. make
    // sure we send a NACK with the final byte rather than an ACK.
    for (size_t i = 0; i < nbytes; i++)
    {
        i2c_master_read_byte(cmd, &output[i], i == nbytes - 1
                                              ? NACK_VAL
                                              : ACK_VAL);
    }

    // send all queued commands, blocking until all commands have been sent.
    // note that this is *not* thread-safe.
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

esp_err_t writeCommandBytes(const i2c_port_t i2c_num, const uint8_t *i2c_command, const size_t nbytes, void *intf_ptr){

    uint8_t chip_addr = *(uint8_t*)intf_ptr;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);

    // write the 7-bit address of the sensor to the bus, using the last bit to
    // indicate we are performing a write.
    i2c_master_write_byte(cmd, chip_addr << 1 | I2C_MASTER_WRITE, ACK_CHECK_EN);

    for (size_t i = 0; i < nbytes; i++)
        i2c_master_write_byte(cmd, i2c_command[i], ACK_CHECK_EN);

    // send all queued commands, blocking until all commands have been sent.
    // note that this is *not* thread-safe.
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}