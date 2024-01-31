#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <SD_MMC.h>
#include <ArduinoJson.h>
#include "esp_camera.h"
#include <Arduino.h>

#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"
// ===========================
// Enter your WiFi credentials
// ===========================
const char* ssid = "ESP32-CAM";
const char* password = "test1234";

AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(8888);
bool isClientConnected;

//Stores the camera configuration parameters
camera_config_t config;

// Function to initialize WiFi in SoftAP mode
void initWiFi() {
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
}

// Function to initialize SD card
void initSDCard() {
  if (!SD_MMC.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
  // ... (rest of the SD card initialization)
  uint8_t cardType = SD_MMC.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SD_SC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SD_HC");
  } else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
}

void initCamera() {
  // Pin definition for CAMERA_MODEL_AI_THINKER
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  // camera config
  config.xclk_freq_hz = 10000000; // Camera frequency 10-20 MHz
  config.frame_size = FRAMESIZE_SVGA; // FRAMESIZE (800 x 600)
  config.pixel_format = PIXFORMAT_JPEG; // Image Format: YUV422,GRAYSCALE,RGB565,JPEG
  config.fb_location = CAMERA_FB_IN_PSRAM; // CAMERA_FB_IN_DRAM; location where the camera frame buffers will be stored
  /* 
  if you notice that the images taken with the ESP32-CAM are cut in half, 
  or with strange colors, that’s probably a sign that you 
  need to lower the jpeg_quality (select a higher number).
  */
  config.jpeg_quality = 20; // 0-63 lower number means higher quality
  config.fb_count = 2; // if fb_count more than one, the driver will work in continuous mode.
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY; // CAMERA_GRAB_LATEST. Sets when buffers should be filled

  // Initialize the Camera
  esp_err_t err = esp_camera_init(&config);
  // Cek if error display message
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // change the image settings
  sensor_t * s = esp_camera_sensor_get();
  s->set_contrast(s, 2);       // -2 to 2
  s->set_saturation(s, -2);     // -2 to 2

}

// Function to handle video list request
void handleVideoList(AsyncWebServerRequest *request) {
  // ... (rest of the code to handle video list request)
  DynamicJsonDocument jsonDoc(1024);
  JsonArray jsonArray = jsonDoc.createNestedArray("data");

  File root = SD_MMC.open("/");
  File file = root.openNextFile();

  while (file) {
    if (!file.isDirectory() && isVideoFile(file.name())) {
      JsonObject fileObject = jsonArray.createNestedObject();

      // Sanitize file name and path
      String sanitizedFileName = sanitizeString(file.name());
      String sanitizedFilePath = sanitizeString(file.path());

      fileObject["name"] = sanitizedFileName;
      fileObject["path"] = sanitizedFilePath;
      fileObject["size"] = file.size();
      fileObject["lastWrite"] = file.getLastWrite();
    }
    file = root.openNextFile();
  }

  String jsonResponse;
  serializeJson(jsonDoc, jsonResponse);
  request->send(200, "application/json", jsonResponse);
}

// Function to handle playing a video
void handlePlayVideo(AsyncWebServerRequest *request) {
  // ... (rest of the code to handle playing a video)
  String videoFile = request->getParam("file")->value();
  File file = SD_MMC.open(videoFile);

  if (!file) {
    request->send(404, "text/plain", "File not found");
    return;
  }
  file.close();
  request->send(SD_MMC, videoFile, "video/mp4");
  
}

// Function to handle downloading a video
void handleDownloadVideo(AsyncWebServerRequest *request) {
  // ... (rest of the code to handle downloading a video)
  String videoFile = request->getParam("file")->value();
  File file = SD_MMC.open(videoFile);

  if (!file) {
    request->send(404, "text/plain", "File not found");
    return;
  }
  file.close();
  request->send(SD_MMC, videoFile, "application/octet-stream", true);
  
}

// Function to check if a file is a video file
bool isVideoFile(const String &fileName) {
  // ... (rest of the code to check if a file is a video file)
  String lowerCaseFileName = fileName;
  lowerCaseFileName.toLowerCase();
  return (lowerCaseFileName.endsWith(".mp4") || lowerCaseFileName.endsWith(".mkv"));
}

// Function to sanitize a string (remove non-printable characters)
String sanitizeString(const String &input) {
  // ... (rest of the code to sanitize a string)
  String sanitized = "";
  for (char c : input) {
    if (isPrintable(c)) {
      sanitized += c;
    }
  }
  return sanitized;
}

// Function to check if a character is printable
bool isPrintable(char c) {
  // ... (rest of the code to check if a character is printable)
  return (c >= 32 && c <= 126);
}

// websocket event handler 
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    // To-Do on each event
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.println("Disconnected!");
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                Serial.print("Connected IP address:");
                Serial.println(ip);
                isClientConnected = true;
            }
            break;
    }
}

void setup() {
  Serial.begin(115200);
  // Initialize the camera  
  Serial.print("Initializing the camera module...");
  initCamera(); // Initialize the camera
  Serial.println(" Ok!");

  delay(1000);

  initSDCard(); // Initialize SD Card
  initWiFi(); // Initialize ESP32 WiFi Access Point 

  webSocket.begin(); // start websocket server
  webSocket.onEvent(webSocketEvent); // listen to event from websocket client

  // Route to list video files
  server.on("/videos", HTTP_GET, handleVideoList);

  // Route to play video
  server.on("/play", HTTP_GET, handlePlayVideo);

  // Route to download video
  server.on("/download", HTTP_GET, handleDownloadVideo);

  server.begin();
}

void checkMemory() {
// Print free and total heap memory
  Serial.print("Free Heap: ");
  Serial.print(ESP.getFreeHeap() / 1024); // Convert to kilobytes for better readability
  Serial.print(" KB");

  Serial.print("\tTotal Heap: ");
  Serial.print(ESP.getHeapSize() / 1024); // Convert to kilobytes
  Serial.print(" KB");

  Serial.println();

  delay(100000); // Adjust the delay as needed
}

void loop() {
  // used to continuously check for incoming data and manage the connection
  webSocket.loop();
  if(isClientConnected){
    // a pointer that can hold the memory address of a camera_fb_t object.
    camera_fb_t *fb = NULL;
  
    // a region in memory where the image/frame is temporarily stored
    fb = esp_camera_fb_get();
    if(!fb){
      Serial.println("Camera capture failed");
      // return the frame buffer back to the driver for reuse
      esp_camera_fb_return(fb);
      return;
    }
  
    // Send Binary data to connected client
    webSocket.broadcastBIN((const uint8_t*) fb->buf, fb->len); // payload (image), payload length
    // Return the frame buffer back to the driver for reuse
    // print test line 274 blocking code after
    Serial.println("fb broadcasted"); 
    esp_camera_fb_return(fb);    
  }
  checkMemory();
}
