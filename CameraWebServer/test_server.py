#!/usr/bin/env python3
from flask import Flask, request, jsonify
import base64
import time
import os

app = Flask(__name__)

API_KEY = "test123"  # Authentication key
received_images = []  # Store received image info

@app.route('/api/save-image', methods=['POST'])
def save_image():
    """Receive photo from ESP32-CAM, decode and save"""
    try:
        auth_header = request.headers.get('Authorization')
        if not auth_header or auth_header != API_KEY:
            return jsonify({"error": "Invalid or missing API key"}), 401

        data = request.get_json()
        if not data or 'image' not in data:
            return jsonify({"error": "No image data received"}), 400

        base64_image = data['image']
        timestamp = data.get('timestamp', time.time())

        try:
            image_data = base64.b64decode(base64_image)
            filename = f"received_image_{int(timestamp)}.jpg"
            with open(filename, 'wb') as f:
                f.write(image_data)

            received_images.append({
                'filename': filename,
                'timestamp': timestamp,
                'size': len(image_data)
            })

            print(f"✅ Saved image: {filename} ({len(image_data)} bytes)")
            return jsonify({"status": "success", "filename": filename})

        except Exception as decode_error:
            return jsonify({"error": f"Base64 decode failed: {str(decode_error)}"}), 400

    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/api/status', methods=['GET'])
def get_status():
    """Check server status and list images"""
    return jsonify({
        "status": "running",
        "images_received": len(received_images),
        "recent_images": received_images[-10:] if received_images else []
    })

if __name__ == '__main__':
    print("ESP32-CAM Image Save Server Starting...")
    print("Access at: http://localhost:5000")
    print("Save endpoint: http://localhost:5000/api/save-image")
    print(f"API Key required: {API_KEY}")
    app.run(host='0.0.0.0', port=5000, debug=True)
