import http.server
import ssl
from urllib.parse import urlparse, parse_qs
import psycopg2
import json
import os


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

         # Verificar si la solicitud es para OTA
        if parsed_url.path == '/ota':
            self.handle_ota_request(query_params)
        else:
            # Procesar la solicitud de consulta de base de datos
            self.handle_database_query(query_params)


    def handle_database_query(self, query_params):
        # Obtener el valor de 'deviceName' de los parámetros de la consulta
        device_name = query_params.get('deviceName', [''])[0]

        # Consultar la base de datos
        result = self.query_database(device_name)

        # Enviar la respuesta al cliente
        self.send_response(200)
        self.send_header('Content-type', 'text/plain')
        self.end_headers()
        self.wfile.write(str(result).encode('utf-8'))


    def handle_ota_request(self, query_params):
        # Obtener los parámetros de la consulta
        device_name = query_params.get('name', [''])[0]
        version = query_params.get('version', [''])[0]

        # Construir la ruta del archivo .bin
        bin_file_path = os.path.join('ota', f'{device_name}_{version}.bin')

        # Verificar si el archivo .bin existe
        if os.path.exists(bin_file_path):
            # Enviar el archivo .bin como respuesta
            self.send_response(200)
            self.send_header('Content-type', 'application/octet-stream')
            self.send_header('Content-Disposition', f'attachment; filename={os.path.basename(bin_file_path)}')
            self.end_headers()

            with open(bin_file_path, 'rb') as bin_file:
                self.wfile.write(bin_file.read())
        else:
            # Enviar una respuesta de error si el archivo no existe
            self.send_response(404)
            self.send_header('Content-type', 'text/plain')
            self.end_headers()
            self.wfile.write(b'File not found')

            

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
    '''
    # Create an SSL context
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.load_cert_chain('server.crt', 'server.key')

    # Configurar y ejecutar el servidor
    server_address = ('0.0.0.0', 9876)
    httpd = http.server.HTTPServer(server_address, MyHandler)
    
    # Configurar el contexto SSL con los certificados
    httpd.socket = context.wrap_socket(httpd.socket, server_side=True)
    '''

    # Configurar y ejecutar el servidor
    server_address = ('0.0.0.0', 9876)
    httpd = http.server.HTTPServer(server_address, MyHandler)

    print('Servidor en ejecución en http://localhost:9876...')
    httpd.serve_forever()

