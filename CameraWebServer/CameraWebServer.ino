#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "base64.h"
#include "config.h"
#include "board_config.h"

// Global variables
unsigned long lastPhotoTime = 0;
//bool isPaidUser = false;

// Function declarations
void startCameraServer();
void setupLedFlash();
void takePhotoAndSend();
//void handleServerResponse(const String& response);
//void indicateStatus(bool paid);

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println("Gym Door Access System Starting...");

  // Setup GPIO pins
  pinMode(LED_GPIO, OUTPUT);
  digitalWrite(LED_GPIO, LOW);

  // Camera configuration
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM; 
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = CAMERA_FRAME_SIZE; // Use configuration
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = CAMERA_JPEG_QUALITY; // Use configuration
  config.fb_count = 1;

  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  Serial.println("Camera initialized successfully");

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setSleep(false);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Start camera web server (optional, for debugging)
  startCameraServer();

  Serial.println("Gym Door Access System Ready!");
  
  // Test server connection first
  testServerConnection();
  
  Serial.println("Taking photos every 10 seconds...");
  
  // Initial photo
  lastPhotoTime = millis();
}

void loop() {
  // Check if it's time to take a photo
  if (millis() - lastPhotoTime >= PHOTO_INTERVAL) {
    takePhotoAndSend();
    lastPhotoTime = millis();
  }
  
  // Handle WiFi reconnection if needed
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Reconnecting...");
    WiFi.reconnect();
    delay(5000);
  }
  
  delay(1000); // Small delay to prevent watchdog issues
}

void takePhotoAndSend() {
  Serial.println("Taking photo...");
  
  // Capture photo
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  Serial.printf("Photo captured: %dx%d %db\n", fb->width, fb->height, fb->len);

  // Convert to base64
  String base64Image = Base64Encoder::encode(fb->buf, fb->len);
  
  // Verify base64 encoding
  if (base64Image.length() == 0) {
    Serial.println("ERROR: Base64 encoding failed!");
    esp_camera_fb_return(fb);
    return;
  }
  
  Serial.printf("Base64 encoded size: %d\n", base64Image.length());
  
  // Verify base64 string properties
  if (VERIFY_BASE64) {
    if (base64Image.length() % 4 != 0) {
      Serial.printf("WARNING: Base64 length %d is not multiple of 4\n", base64Image.length());
    }
    Serial.printf("Base64 ends with: %s\n", base64Image.substring(base64Image.length() - 4).c_str());
  }

  // Send to cloud server with retry logic
  if (WiFi.status() == WL_CONNECTED) {
    bool success = false;
    int attempts = 0;
    
    while (!success && attempts < MAX_RETRY_ATTEMPTS) {
      attempts++;
      Serial.printf("Attempt %d/%d: Sending photo to cloud server...\n", attempts, MAX_RETRY_ATTEMPTS);
      
      HTTPClient http;
      http.setTimeout(HTTP_TIMEOUT);
      http.begin(CLOUD_SERVER_URL);
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Authorization", API_KEY);
      
      // Create JSON payload
      String jsonPayload = "{\"image\":\"" + base64Image + "\",\"timestamp\":" + String(millis()) + "}";
      
      int httpResponseCode = http.POST(jsonPayload);
      
      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.printf("HTTP Response code: %d\n", httpResponseCode);
        Serial.println("Response: " + response);
        
        if (httpResponseCode == 200) {
          success = true;
          handleServerResponse(response);
        } else {
          Serial.printf("Server error: %d\n", httpResponseCode);
        }
      } else {
        Serial.printf("HTTP Error code: %d\n", httpResponseCode);
        Serial.println("Error: " + http.errorToString(httpResponseCode));
      }
      
      http.end();
      
      if (!success && attempts < MAX_RETRY_ATTEMPTS) {
        Serial.printf("Retrying in %d ms...\n", HTTP_RETRY_DELAY);
        delay(HTTP_RETRY_DELAY);
      }
    }
    
    if (!success) {
      Serial.println("Failed to send photo after all retry attempts");
    }
  } else {
    Serial.println("WiFi not connected");
  }

  // Return the frame buffer
  esp_camera_fb_return(fb);
}

void handleServerResponse(const String& response) {
  // Parse JSON response
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, response);
  
  if (error) {
    Serial.println("Failed to parse JSON response");
    return;
  }
  
  // Check if user is paid
 /* if (doc.containsKey("isPaid")) {
    isPaidUser = doc["isPaid"].as<bool>();
    Serial.printf("User payment status: %s\n", isPaidUser ? "PAID" : "NOT PAID");
    
    // Indicate status
    indicateStatus(isPaidUser);
  }
  
  // Check for additional response data
  if (doc.containsKey("message")) {
    Serial.println("Server message: " + doc["message"].as<String>());
  }*/
}


/*void indicateStatus(bool paid) {
  if (paid) {
    // Green light pattern for paid user
    for (int i = 0; i < LED_PAID_PATTERN_COUNT; i++) {
      digitalWrite(LED_GPIO, HIGH);
      delay(LED_PAID_PATTERN_ON);
      digitalWrite(LED_GPIO, LOW);
      delay(LED_PAID_PATTERN_OFF);
    }
  } else {
    // Red light pattern for non-paid user
    for (int i = 0; i < LED_DENIED_PATTERN_COUNT; i++) {
      digitalWrite(LED_GPIO, HIGH);
      delay(LED_DENIED_PATTERN_ON);
      digitalWrite(LED_GPIO, LOW);
      delay(LED_DENIED_PATTERN_OFF);
    }
  }
}
*/

void testServerConnection() {
  Serial.println("Testing server connection...");
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    return;
  }
  
  HTTPClient http;
  http.begin(CLOUD_SERVER_URL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", API_KEY);
  
  // Send a simple test request
  String testPayload = "{\"test\":\"connection\",\"timestamp\":" + String(millis()) + "}";
  
  Serial.println("Sending test request...");
  int httpResponseCode = http.POST(testPayload);
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.printf("Connection test SUCCESS! HTTP Response code: %d\n", httpResponseCode);
    Serial.println("Response: " + response);
    
    // Green LED flash for successful connection
   /*/ for (int i = 0; i < 2; i++) {
      digitalWrite(LED_GPIO, HIGH);
      delay(100);
      digitalWrite(LED_GPIO, LOW);
      delay(100);
    }*/
  } else {
    Serial.printf("Connection test FAILED! HTTP Error code: %d\n", httpResponseCode);
    Serial.println("Error: " + http.errorToString(httpResponseCode));
    
    // Red LED flash for failed connection
   /* for (int i = 0; i < 4; i++) {
      digitalWrite(LED_GPIO, HIGH);
      delay(100);
      digitalWrite(LED_GPIO, LOW);
      delay(100);
    }*/
  }
  
  http.end();
}
