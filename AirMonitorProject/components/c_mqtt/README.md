# Component MQTT

## TO USE THIS COMPONENT, ADD IN YOUR MAIN:
```
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "protocol_examples_common.h"

ESP_ERROR_CHECK(nvs_flash_init());
ESP_ERROR_CHECK(esp_netif_init());
ESP_ERROR_CHECK(esp_event_loop_create_default());
ESP_ERROR_CHECK(example_connect());
```

### If can't find protocol_examples_common, add in CMakelists.txt(the one at the same level as /main):
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)


## Authentication: username & pwd
[Tutorial](http://www.steves-internet-guide.com/mqtt-username-password-example/)

| USERNAME  |    PWD   |
|-----------|----------|
|   user1   |   user1  |


## Authentication: certificate
http://www.steves-internet-guide.com/mosquitto-tls/
1. Generate Certificate Authority (CA):

```
openssl genpkey -algorithm RSA -out ca.key
openssl req -x509 -new -nodes -key ca.key -sha256 -days 365 -out ca.crt
```

2. Generate Broker Certificate:
```
openssl genpkey -algorithm RSA -out broker.key
openssl req -new -key broker.key -out broker.csr -sha256
openssl x509 -req -in broker.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out broker.crt -days 365 -sha256
```

3. Generate Client Certificate:
```
openssl genpkey -algorithm RSA -out client.key
openssl req -new -key client.key -out client.csr -sha256
openssl x509 -req -in client.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out client.crt -days 365 -sha256
```
