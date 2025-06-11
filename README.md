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

nikolaindustryrealtime realtime;

void setup() {
  Serial.begin(115200);
  realtime.begin("YourSSID", "YourPassword", "esp32-001");
  realtime.setOnMessageCallback([](JsonObject& msg) {
    serializeJsonPretty(msg, Serial);
  });

  // Send JSON in one line
  realtime.sendTo("server", [](JsonObject& payload) {
    payload["hello"] = "world";
  });
}

void loop() {
  realtime.loop();
}
```
