# nikolaindustry-realtime Arduino Library

This library provides easy integration with the NIKOLAINDUSTRY real-time server for ESP32 boards.

## Features
- Secure (`wss://`)
- JSON message parsing and sending
- Callback-based message handler
- Automatic reconnect
- Simplified helper function `sendTo(...)` for error-free messaging

## Installation
1. Download the `.zip` file.
2. In Arduino IDE, go to **Sketch > Include Library > Add .ZIP Library...**
3. Select the downloaded file.

## Usage Example
```cpp
#include <WiFi.h>
#include <ArduinoJson.h>
#include <nikolaindustry-realtime.h>

// Replace with your actual Wi-Fi credentials
const char* ssid = "SENSORFLOW";
const char* password = "12345678";

// Unique device ID used to connect to the WebSocket server
const char* deviceId = "esp32-demo-device-001";

nikolaindustryrealtime realtime;

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi and WebSocket server
  realtime.begin(ssid, password, deviceId);

  // Set a callback for incoming messages
  realtime.setOnMessageCallback([](JsonObject& msg) {
    Serial.println("📩 Received message:");
    serializeJsonPretty(msg, Serial);
    Serial.println();

    // Access the payload
    if (msg.containsKey("payload")) {
      JsonObject payload = msg["payload"];

      // Example: control onboard LED
      if (payload.containsKey("led")) {
        int ledState = payload["led"];
        digitalWrite(2, ledState ? HIGH : LOW);
        Serial.printf("💡 LED turned %s\n", ledState ? "ON" : "OFF");
      }
    }
  });

  // Setup onboard LED
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
}

void loop() {
  realtime.loop();

  // Send heartbeat/status every 10 seconds
  static unsigned long lastSent = 0;
  if (millis() - lastSent > 10000) {
    lastSent = millis();

    // Send a JSON message to the server
    realtime.sendTo("server", [](JsonObject& payload) {
      payload["status"] = "alive";
      payload["uptime"] = millis() / 1000;
      payload["message"] = "Hello from ESP32";
    });

    Serial.println("🚀 Sent heartbeat message");
  }
}

```
