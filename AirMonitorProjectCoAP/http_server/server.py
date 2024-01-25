import http.server
from urllib.parse import urlparse, parse_qs
import psycopg2
import json


# Configuración de la base de datos PostgreSQL
db_config = {
    'dbname': 'thingsboard',
    'user': 'postgres',
    'password': 'postgres',
    'host': 'localhost',
    'port': '5432'
}

# Manejador personalizado para el servidor
class MyHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        # Parsear la URL
        parsed_url = urlparse(self.path)
        query_params = parse_qs(parsed_url.query)

        # Obtener el valor de 'deviceName' de los parámetros de la consulta
        device_name = query_params.get('deviceName', [''])[0]

        # Consultar la base de datos
        result = self.query_database(device_name)

        # Enviar la respuesta al cliente
        self.send_response(200)
        self.send_header('Content-type', 'text/plain')
        self.end_headers()
        self.wfile.write(str(result).encode('utf-8'))

    def query_database(self, device_name):
        # Conectar a la base de datos
        connection = psycopg2.connect(**db_config)
        cursor = connection.cursor()

        try:
            # Consulta SQL para obtener room y floor
            query = "SELECT room, floor FROM dispositivos WHERE deviceName = %s;"
            cursor.execute(query, (device_name,))
            result = cursor.fetchone()

            if result:
                device_info = {"deviceCtx": f"{result[1]}/{result[0]}"}
                return json.dumps(device_info)
            else:
                return json.dumps({"error": "No se encontraron coincidencias para el dispositivo."})


        except Exception as e:
            return json.dumps({"error": f"Error al consultar la base de datos: {e}"})

        finally:
            # Cerrar la conexión a la base de datos
            cursor.close()
            connection.close()

if __name__ == '__main__':
    # Configurar y ejecutar el servidor
    server_address = ('0.0.0.0', 9876)
    httpd = http.server.HTTPServer(server_address, MyHandler)
    print('Servidor en ejecución en http://localhost:9876...')
    httpd.serve_forever()

