from http.server import HTTPServer, SimpleHTTPRequestHandler

# Set the MIME type for WebAssembly files
SimpleHTTPRequestHandler.extensions_map['.wasm'] = 'application/wasm'

port = 8000
# Create the server bound to localhost
httpd = HTTPServer(('localhost', port), SimpleHTTPRequestHandler)

print(f"Serving at http://localhost:{port}/ waiting for connections")
httpd.serve_forever()
