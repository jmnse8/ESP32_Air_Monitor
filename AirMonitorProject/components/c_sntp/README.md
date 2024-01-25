# Component SNTP

This component is based on the esp-idf example for sntp

The main function is `sntp_sync_time_init()`

The function starts checking if the time of the esp is set yet, if not it will update it connecting to an sntp server, we are using *0.europe.pool.ntp.org*.

After this the code will put the timezone of Madrid.