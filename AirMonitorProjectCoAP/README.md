# CoAP

Se ha hecho una copia del proyecto original para crear una demostración de CoAP restandole algunas funcionalidades prescindibles para nuestro objetivo, como el provisionamiento del wifi o el contador de personas bluetooth.

En CoAP a diferencia de la estrategia que se usa en mqtt aquí el dispositivo se registra en thingsboard en un grupo y este le devuelve su token generado. Este token posteriormente lo guardamos en la nvs para no tener que registrarnos la próxima vez que el dispositivo se encienda