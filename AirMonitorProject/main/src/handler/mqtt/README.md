# Thingsboard(TB) related stuff

## Register the device in TB: Obtain Access Token
In order to send telemetry, the node has to be registered as an entity in TB, a device, that belongs to a certain profile([Device Profile](https://thingsboard.io/docs/user-guide/device-profiles/)). To do so, your node has to send a petition(in this case we use MQTT) by publishing to a certain topic some values and TB will answer with the access_token or an error message.

1. Subscribe to **/provision/response** to receive the response
1. Publish to **/provision/request** to send the petition, which is a JSON structure that contains:

        {
                "deviceName", "YourDevicesName",
                "provisionDeviceKey", "ObtainedFromDeviceProfilesProvisioningTab",
                "provisionDeviceSecret", "ObtainedFromDeviceProfilesProvisioningTab"
        }

1. TB sends a response to **/provision/response**, whether with your access_token or with an error msg.



## Thingsboard: MQTT over SSL [Tutorial](https://thingsboard.io/docs/user-guide/mqtt-over-ssl/)
### Thingsboard running on Docker

1. Goto the directory where your thingboard's docker-compose file is </li>
1. Create a directory to host the broker's certificates. For example, /.mytb-config
1. Declare some environment variables in a file. For example, create a file named .env </li>

        MQTT_SSL_ENABLED=true
        MQTT_SSL_CREDENTIALS_TYPE=PEM
        MQTT_SSL_PEM_CERT=server.pem
        MQTT_SSL_PEM_KEY=server_key.pem
        MQTT_SSL_PEM_KEY_PASSWORD=secret

    You can include more variables such as:
    * MQTT_SSL_BIND_ADDRESS - the bind address for the MQTT server. Default value 0.0.0.0 indicates all interfaces;
    * MQTT_SSL_BIND_PORT - the bind port for the MQTT server. Default value is 8883;
    * MQTT_SSL_PROTOCOL - ssl protocol name. Default value is TLSv1.2. See java doc for more details;
    * MQTT_SSL_SKIP_VALIDITY_CHECK_FOR_CLIENT_CERT - Skip certificate validity check for client certificates. Default value is false.

1. Edit docker-compose file

    * Open a port for MQTT**S** communication and register /mytb-config as docker's /config

            ports:
                ...
                - "8883:8883"

            volumes:
                ...
                - ./.mytb-config:/config
    
1. Generate the certificate for your MQTT broker

    * Goto /.mytb-config
    * Execute the following commands to generate:
        * server_key.pem: The broker's private key
        * server.pem: The broker's certificate

                openssl ecparam -out server_key.pem -name secp256r1 -genkey
                openssl req -new -key server_key.pem -x509 -nodes -days 365 -out server.pem 


1. Give a copy of server.pem to your MQTT client
1. Restart docker

        docker compose up
