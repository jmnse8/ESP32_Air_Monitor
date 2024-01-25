# CoAP

Se ha hecho una copia del proyecto original para crear una demostración de CoAP restandole algunas funcionalidades prescindibles para nuestro objetivo, como el provisionamiento del wifi o el contador de personas bluetooth.

En CoAP el dispositivo se registra en thingsboard en un grupo y este le devuelve su token generado. Este token posteriormente lo guardamos en la nvs para no tener que registrarnos la próxima vez que el dispositivo se encienda.

Para provisionarse manda un post a api/v1/provision mandado un json con los datos para provisionarse (deviceName, provisionDeviceKey, provisionDeviceSecret).

El token que se manda en el post se guarga para que cuando llegue en el handler de de mensajes recibidos(`message_handler()`) sepa que esa es la respuesta de thingsboard con el token, esta respuesta se manda después por un evento y se coge del json para guardarlo en la nvs y en el componente de coap para que pueda mandar las telemetrias.

Las mediciones se mandan mediante un post no confirmable en api/v1/device_token/telemetry,
en él se manda de manera independiente cada telemetría en un json.