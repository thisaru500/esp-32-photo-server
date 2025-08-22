#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "TDD"
#define WIFI_PASSWORD "11111111"

// Cloud Server Configuration
#define CLOUD_SERVER_URL "http://10.75.0.29:5000/api/save-image"
#define API_KEY "test123"

// System Configuration
#define PHOTO_INTERVAL 10000        // Photo interval in milliseconds (10 seconds)
#define DOOR_UNLOCK_DURATION 10000  // How long to keep door unlocked (10 seconds)
#define LED_GPIO 4                  // LED GPIO pin for status indication
#define DOOR_LOCK_GPIO 2            // Door lock control GPIO pin

// Camera Configuration
#define CAMERA_FRAME_SIZE FRAMESIZE_SXGA  // VGA resolution for faster processing
#define CAMERA_JPEG_QUALITY 60           // JPEG quality (0-100)
#define CAMERA_BRIGHTNESS 1              // Camera brightness (-2 to 2)
#define CAMERA_CONTRAST 1                // Camera contrast (-2 to 2)
#define CAMERA_SATURATION 1             // Camera saturation (-2 to 2)

           

// HTTP Configuration
#define HTTP_TIMEOUT 30000              // HTTP request timeout in milliseconds
#define MAX_RETRY_ATTEMPTS 3            // Maximum retry attempts for failed requests
#define HTTP_RETRY_DELAY 2000           // Delay between retries in milliseconds

// Debug Configuration
#define DEBUG_MODE true                 // Enable debug output
#define VERIFY_BASE64 true              // Verify base64 encoding before sending

// Status LED Patterns
/*
#define LED_PAID_PATTERN_COUNT 3        // Number of blinks for paid user
#define LED_PAID_PATTERN_ON 200         // LED on time for paid user (ms)
#define LED_PAID_PATTERN_OFF 200        // LED off time for paid user (ms)
#define LED_DENIED_PATTERN_COUNT 5      // Number of blinks for denied user
#define LED_DENIED_PATTERN_ON 100       // LED on time for denied user (ms)
#define LED_DENIED_PATTERN_OFF 100      // LED off time for denied user (ms)
*/
#endif // CONFIG_H
