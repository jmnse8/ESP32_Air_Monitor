# Component I2C

In this component we abstract the comunnication I2C to a separate component.

It has three functions:

1. `i2c_master_driver_initialize`: it initialize the i2c driver. It receives these parameters of configuration:
    - i2c_master_port
    - i2c_master_sda_io
    - i2c_master_scl_io
    - i2c_master_freq_hz
    - i2c_master_rx_buf_disable
    - i2c_master_tx_buf_disable
2. `readResponseBytes`: It receives these parameters of configuration:
    - i2c_num
    - output
    - nbytes
    - intf_ptr

3. `writeCommandBytes`: It receives these parameters of configuration:
    - i2c_port_t i2c_num
    - i2c_command
    - nbytes
    - intf_ptr