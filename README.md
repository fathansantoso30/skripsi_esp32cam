# skripsi_esp32cam

Final Project. Programmed for ESP32-CAM AI-Thinker model.

## Features:

1. **Include Libraries:**
   - WiFi.h: For handling WiFi connectivity.
   - ESPAsyncWebServer.h: For creating an asynchronous web server.
   - WebSocketsServer.h: For WebSocket communication.
   - SD_MMC.h: For handling SD card operations.
   - ArduinoJson.h: For working with JSON data.
   - esp_camera.h: ESP32 camera library.
   - Arduino.h: Standard Arduino library.

2. **WiFi Credentials:**
   - Set WiFi credentials (SSID and password) for the ESP32-CAM to operate in SoftAP mode.

3. **Server and WebSocket Setup:**
   - Create an AsyncWebServer instance on port 80.
   - Create a WebSocketsServer instance on port 8888.
   - Define a boolean flag to track WebSocket client connection status.

4. **Camera Configuration:**
   - Define the camera configuration parameters such as pin assignments, frequencies, frame size, pixel format, etc.
   - Initialize the camera using the esp_camera_init() function.

5. **SD Card Initialization:**
   - Initialize the SD card using SD_MMC.begin().
   - Print SD card information like type and size.

6. **WebSocket Event Handler:**
   - Implement a WebSocket event handler function (webSocketEvent) to handle connection and disconnection events.

7. **Setup Function:**
   - Initialize the camera, SD card, and WiFi.
   - Start the WebSocket server.
   - Define HTTP routes for handling video list, playing video, and downloading video.

8. **Video List Handling:**
   - Handle requests for listing video files on the SD card.

9. **Play Video Handling:**
   - Handle requests for playing a specific video file.

10. **Download Video Handling:**
   - Handle requests for downloading a specific video file.

11. **Video File Utilities:**
   - Check if a file is a video file based on its extension.
   - Sanitize strings (remove non-printable characters).

12. **Memory Checking Function:**
   - Print free and total heap memory periodically.

13. **Loop Function:**
   - Continuously check for WebSocket events and manage connections.
   - If a client is connected, capture a frame from the camera and broadcast it to all connected clients.
   - Periodically print memory information.

 
