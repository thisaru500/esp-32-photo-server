# esp-32-photo-server and live streaming
"ESP32-CAM project that captures images every 10 seconds and uploads them to a remote server. Implements camera configuration, periodic image capture, and HTTP POST requests for seamless cloud integration. Ideal for IoT monitoring, security, or automated data collection applications."

# ESP32-CAM Image Capture & Streaming Server  

This project uses an **ESP32-CAM** to capture images, encode them in **Base64**, and upload them to a server using POST requests. The server decodes the images and saves them in a specified folder. The ESP32-CAM also supports **live video streaming** via its IP address.  

## Features  
- 📸 Capture images with ESP32-CAM  
- 🔄 Encode images in **Base64** and send via HTTP POST  
- 💾 Server decodes and saves images to a given path  
- 📡 Access **live stream** from ESP32-CAM using its IP address  
- 🐾 Can be used for animal photo capturing, surveillance, or monitoring projects  

## Project Flow  
1. ESP32-CAM captures an image.  
2. Image is encoded to **Base64**.  
3. Encoded image is sent to the server with a **POST request**.  
4. Server decodes and saves the image.  
5. ESP32-CAM also provides **live video streaming** via its IP address.  

## Requirements  

### Hardware  
- ESP32-CAM (AI Thinker or similar)  
- FTDI Programmer / ESP32 Board for flashing  
- Power supply  

### Software  
- Arduino IDE or PlatformIO  
- Python 3.x for server  

## Usage  

1. Flash the ESP32-CAM with the provided Arduino code (set your Wi-Fi and server details).  
2. Start the server to receive images.  
3. Access the live stream from the ESP32-CAM using:  
