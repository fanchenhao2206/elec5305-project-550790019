from http.server import HTTPServer, SimpleHTTPRequestHandler

SimpleHTTPRequestHandler.extensions_map[".wasm"] = "application/wasm"

server = HTTPServer(("localhost", 8000), SimpleHTTPRequestHandler)

print("Serving at http://localhost:8000/")
server.serve_forever()
