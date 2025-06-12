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
#include <nikolaindustry-realtime.h>

const char *WIFI_SSID = "SENSORFLOW";
const char *WIFI_PASSWORD = "12345678";
const char *DEVICE_ID = "esp32-device-123";  // Unique ID for this ESP32

nikolaindustryrealtime realtime;

// AP timeout duration (2 minutes)
const unsigned long AP_TIMEOUT_DURATION = 120000;

unsigned long lastAPStart = 0;
bool retriedWiFiAfterAP = false;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Start realtime communication
  realtime.begin(WIFI_SSID, WIFI_PASSWORD, DEVICE_ID);

  // Set AP timeout inside the library (internal auto-stop)
  realtime.setAPTimeout(AP_TIMEOUT_DURATION);

  // WebSocket message received callback
  realtime.setOnMessageCallback([](JsonObject &msg) {
    Serial.println("📥 Received message:");
    serializeJsonPretty(msg, Serial);
    Serial.println();

     // Test send (can be triggered from a button instead)
  realtime.sendTo("target-device-456", [](JsonObject &payload) {
    payload["message"] = "Hello from ESP32!";
    payload["status"] = "test";
  });

  });

  // Connection status monitor
  realtime.setOnConnectionStatusChange([](bool connected) {
    Serial.printf("🔌Realtime Socket status: %s\n", connected ? "CONNECTED" : "DISCONNECTED");
  });

 
}

void loop() {
  realtime.loop();

  // Reconnect Wi-Fi after AP timeout
  if (realtime.isAPModeActive()) {
    if (lastAPStart == 0) {
      lastAPStart = millis();
    }

    if (!retriedWiFiAfterAP && millis() - lastAPStart >= AP_TIMEOUT_DURATION) {
      Serial.println("🕒 AP Mode timed out. Retrying Wi-Fi connection...");
      realtime.stopAPMode();
      delay(1000); // Short delay before reconnect
      realtime.begin(WIFI_SSID, WIFI_PASSWORD, DEVICE_ID); // Retry Wi-Fi
      retriedWiFiAfterAP = true;
      lastAPStart = 0;
    }
  } else {
    lastAPStart = 0;
    retriedWiFiAfterAP = false;
  }

  // Optional: Check connection every 5 seconds
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 5000) {
    lastCheck = millis();
    if (realtime.isNikolaindustryRealtimeConnected()) {
      Serial.println("✅ Realtime is connected.");
    } else {
      Serial.println("❌ Realtime not connected.");
    }
  }
}


```
