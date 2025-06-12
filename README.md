Here’s a `README.md` file that explains each function of your `nikolaindustry-realtime` ESP32 library, written in a clean and structured way:

---

````markdown
# NikolaIndustry Realtime Library for ESP32

The `nikolaindustry-realtime` library enables ESP32 devices to communicate securely with a WebSocket server, auto-reconnect on disconnection, switch to AP mode on repeated Wi-Fi failures, and handle real-time JSON messaging.

## 📦 Features

- Auto-connect to WebSocket over SSL
- JSON-based messaging (using `ArduinoJson`)
- Callback for incoming messages
- Callback for connection status changes
- Retry logic with exponential backoff
- Automatic switch to AP mode after multiple Wi-Fi failures
- AP mode timeout and recovery
- Send JSON messages to specific target devices

---

## 🚀 Getting Started

### Example Usage

```cpp
#include <nikolaindustry-realtime.h>

const char* WIFI_SSID = "your_wifi_ssid";
const char* WIFI_PASSWORD = "your_wifi_password";
const char* DEVICE_ID = "esp32-unique-device-id";

nikolaindustryrealtime realtime;

void setup() {
  Serial.begin(115200);
  realtime.begin(WIFI_SSID, WIFI_PASSWORD, DEVICE_ID);

  realtime.setOnMessageCallback([](JsonObject &msg) {
    Serial.println("Received:");
    serializeJsonPretty(msg, Serial);
    Serial.println();
  });

  realtime.setOnConnectionStatusChange([](bool connected) {
    Serial.printf("WebSocket: %s\n", connected ? "Connected" : "Disconnected");
  });

  realtime.sendTo("target-device-id", [](JsonObject &payload) {
    payload["status"] = "ping";
  });
}

void loop() {
  realtime.loop();
}
````

---

## 🔧 Class: `nikolaindustryrealtime`

### `begin(const char *ssid, const char *password, const char *deviceId)`

Initializes Wi-Fi, connects to WebSocket, and sets up auto-reconnect.

* `ssid`: Wi-Fi network name.
* `password`: Wi-Fi password.
* `deviceId`: Unique ID passed to WebSocket as query parameter.

---

### `loop()`

Must be called in the `loop()` function to process WebSocket events and manage AP/Wi-Fi recovery.

---

### `sendJson(const JsonObject &json)`

Sends a raw JSON object over WebSocket. Useful for full control of payload structure.

---

### `sendTo(const String &targetId, std::function<void(JsonObject &)> payloadBuilder)`

Builds and sends a JSON payload with a `targetId`. Simplifies common messaging.

* `targetId`: Destination device.
* `payloadBuilder`: Lambda that fills the `payload` object.

Example:

```cpp
realtime.sendTo("target-device", [](JsonObject &payload) {
  payload["temp"] = 25;
});
```

---

### `setOnMessageCallback(std::function<void(JsonObject &)> callback)`

Registers a callback that triggers when a valid JSON message is received.

---

### `setOnConnectionStatusChange(std::function<void(bool)> callback)`

Registers a callback that notifies when WebSocket connects or disconnects.

* `true` = connected
* `false` = disconnected

---

### `startAPMode(const char *ssid = nullptr, const char *password = nullptr)`

Switches the ESP32 to AP mode with optional SSID and password.
If none are provided:

* Default SSID: `NIKOLAINDUSTRY_Setup`
* Default Password: `xVv9O9B4tV`

---

### `stopAPMode()`

Stops AP mode and reverts to station mode (STA).

---

### `setAPTimeout(unsigned long timeoutMillis)`

Sets AP mode timeout in milliseconds. After the timeout, AP mode stops automatically.

---

### `isAPModeActive() const`

Returns `true` if AP mode is active.

---

### `isNikolaindustryRealtimeConnected()`

Returns `true` if the WebSocket is currently connected.

---

## 🔁 Reconnection Logic

* Retries Wi-Fi with exponential backoff (starting at 5s, capped at 60s).
* After `maxWifiRetriesBeforeAP` attempts (default: 5), it switches to AP mode.
* If `setAPTimeout()` is used, it will exit AP mode after the timeout and retry Wi-Fi.

---

## 🌐 WebSocket URL

WebSocket connects to:

```
wss://nikolaindustry-realtime.onrender.com/?id=<deviceId>
```

---

## 🧰 Dependencies

* [ArduinoJson](https://arduinojson.org/)
* [WebSockets](https://github.com/Links2004/arduinoWebSockets)
* [WiFi](https://www.arduino.cc/en/Reference/WiFi)
* [DNSServer](https://github.com/espressif/arduino-esp32)

---

## 📄 License

MIT License – Use freely for commercial and personal projects.

---

## 📞 Need Help?

Feel free to open an issue or contact the Nikola Industry team for feature requests or troubleshooting.

```

---

Let me know if you want me to generate a PDF or auto-upload this into your repo structure!
```



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
