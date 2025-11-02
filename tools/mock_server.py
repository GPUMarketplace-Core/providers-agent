#!/usr/bin/env python3
"""
Mock HTTP server for testing heartbeat client.

Usage:
    python mock_server.py [port]

Example:
    python mock_server.py 8080
    # Then use endpoint: http://localhost:8080/heartbeat
"""

from http.server import HTTPServer, BaseHTTPRequestHandler
import json
import sys
from datetime import datetime

class HeartbeatHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        # Get content length
        content_length = int(self.headers.get('Content-Length', 0))
        
        # Read body
        body = self.rfile.read(content_length)
        
        try:
            # Parse JSON
            data = json.loads(body)
            
            # Log the heartbeat
            timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            print(f"[{timestamp}] Heartbeat received:")
            print(f"  Timestamp: {data.get('timestamp', 'N/A')}")
            print(f"  Status: {data.get('status', 'N/A')}")
            print(f"  Agent ID: {data.get('agent_id', 'N/A')}")
            print()
            
            # Send success response
            response = {
                "status": "ok",
                "message": "Heartbeat received",
                "server_time": datetime.now().isoformat() + "Z"
            }
            
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(response).encode())
            
        except json.JSONDecodeError:
            # Invalid JSON
            self.send_response(400)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            error = {"status": "error", "message": "Invalid JSON"}
            self.wfile.write(json.dumps(error).encode())
    
    def log_message(self, format, *args):
        # Suppress default logging
        pass

def run_server(port=8080):
    server_address = ('', port)
    httpd = HTTPServer(server_address, HeartbeatHandler)
    
    print(f"Mock heartbeat server running on port {port}")
    print(f"Endpoint: http://localhost:{port}/heartbeat")
    print("Press Ctrl+C to stop")
    print()
    
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down server...")
        httpd.shutdown()

if __name__ == '__main__':
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8080
    run_server(port)
