# How does Provisioning work in this project

After erasing flash content and booting for the first time, it will be in PROVISIONING MODE as a softAP.
Connect your provision device (PC) to the AP created by the ESP, navigate to your esp-idf directory /tools/esp_prov and execute the following code:

```
python esp_prov.py --transport softap --sec_ver 1 --ssid yourWifiSSID --passphrase yourWifiPWD --custom_data "{\"access_token\":\"theAccessToken2TB\"}"

```
The device will be provided of wifi credentials and the custom-data "access_token" and store them in NVS. Then, it will wait 10 seconds and restart.

After restarting, it will connect to wifi and then to TB using the credentials stored in NVS.