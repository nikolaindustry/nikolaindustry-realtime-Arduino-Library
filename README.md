
# NikolaIndustry Realtime Library for ESP32

The `nikolaindustry-realtime` library enables ESP32 devices to communicate securely with a nikolaindustry-realtime server, auto-reconnect on disconnection, switch to AP mode on repeated Wi-Fi failures, and handle real-time JSON messaging.

## 📦 Features

- Auto-connect to nikolaindustry-realtime over SSL
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
#include <WiFi.h>
#include "nikolaindustry-realtime.h"

const char* ssid = "SENSORFLOW";
const char* pass = "12345678";
const char* deviceid = "device-123";

nikolaindustryrealtime realtime;

void setup() {
  Serial.begin(115200);


  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected");

  realtime.begin(deviceid);

  realtime.setOnMessageCallback([](JsonObject& msg) {
    if (!msg.containsKey("payload")) return;

    JsonObject payload = msg["payload"];
    JsonArray commands = payload["commands"];

    for (JsonObject commandObj : commands) {
      const char* command = commandObj["command"];

      if (strcmp(command, "GPIO_MANAGEMENT") == 0) {
        JsonArray actions = commandObj["actions"];

        for (JsonObject actionObj : actions) {
          const char* action = actionObj["action"];
          JsonObject params = actionObj["params"];

          // Extract parameters
          int gpio = atoi(params["gpio"] | "0");
          String pinmodeStr = params["pinmode"] | "OUTPUT";
          String statusStr = params["status"] | "LOW";

          // Resolve pin mode
          int mode = OUTPUT;
          if (pinmodeStr == "INPUT") mode = INPUT;
          else if (pinmodeStr == "INPUT_PULLUP") mode = INPUT_PULLUP;

          // Resolve output state
          int value = LOW;
          if (statusStr == "HIGH") value = HIGH;

          // Apply pin configuration
          pinMode(gpio, mode);

          if (strcmp(action, "ON") == 0 || strcmp(action, "OFF") == 0) {
            digitalWrite(gpio, value);
            Serial.printf("Pin %d set to %s with mode %s\n", gpio, statusStr.c_str(), pinmodeStr.c_str());
          }
        }
      }
    }
  });


  realtime.setOnConnectionStatusChange([](bool connected) {
    Serial.println(connected ? "🟢 WS Connected" : "🔴 WS Disconnected");
  });
}

void loop() {
  realtime.loop();
}

````

### ✅ Example 1: Turn ON GPIO 12 as OUTPUT (e.g., turn on LED)
````
{
  "targetId": "device-123",
  "payload": {
    "commands": [
      {
        "command": "GPIO_MANAGEMENT",
        "actions": [
          {
            "action": "ON",
            "params": {
              "gpio": "12",
              "pinmode": "OUTPUT",
              "status": "HIGH"
            }
          }
        ]
      }
    ]
  }
}
````
### ✅ Example 2: Turn OFF GPIO 14 configured as OUTPUT
````
{
  "targetId": "device-123",
  "payload": {
    "commands": [
      {
        "command": "GPIO_MANAGEMENT",
        "actions": [
          {
            "action": "OFF",
            "params": {
              "gpio": "14",
              "pinmode": "OUTPUT",
              "status": "LOW"
            }
          }
        ]
      }
    ]
  }
}
````
### ✅ Example 3: Configure GPIO 25 as INPUT_PULLUP (no write)
````
{
  "targetId": "device-123",
  "payload": {
    "commands": [
      {
        "command": "GPIO_MANAGEMENT",
        "actions": [
          {
            "action": "ON",
            "params": {
              "gpio": "25",
              "pinmode": "INPUT_PULLUP",
              "status": "LOW"
            }
          }
        ]
      }
    ]
  }
}
````
⚠️ Although digitalWrite() still applies to INPUT_PULLUP, usually you don’t need to write in this mode — it enables an internal pull-up resistor.

### ✅ Example 4: Toggle multiple GPIOs at once
````
{
  "targetId": "device-123",
  "payload": {
    "commands": [
      {
        "command": "GPIO_MANAGEMENT",
        "actions": [
          {
            "action": "ON",
            "params": {
              "gpio": "5",
              "pinmode": "OUTPUT",
              "status": "HIGH"
            }
          },
          {
            "action": "OFF",
            "params": {
              "gpio": "18",
              "pinmode": "OUTPUT",
              "status": "LOW"
            }
          }
        ]
      }
    ]
  }
}
````
### ✅ Example 5: Configure GPIO 4 as INPUT, no output
````
{
  "targetId": "device-123",
  "payload": {
    "commands": [
      {
        "command": "GPIO_MANAGEMENT",
        "actions": [
          {
            "action": "ON",
            "params": {
              "gpio": "4",
              "pinmode": "INPUT",
              "status": "LOW"
            }
          }
        ]
      }
    ]
  }
}
````
This configures the pin, but the status has no real effect since it's an input. Still accepted for uniformity.


---

## 🔧 Class: `nikolaindustryrealtime`

### `begin(const char *ssid, const char *password, const char *deviceId)`

Initializes Wi-Fi, connects to nikolaindustry-realtime, and sets up auto-reconnect.

* `ssid`: Wi-Fi network name.
* `password`: Wi-Fi password.
* `deviceId`: Unique ID passed to nikolaindustry-realtime as query parameter.

---

### `loop()`

Must be called in the `loop()` function to process nikolaindustry-realtime events and manage AP/Wi-Fi recovery.

---

### `sendJson(const JsonObject &json)`

Sends a raw JSON object over nikolaindustry-realtime. Useful for full control of payload structure.

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

Registers a callback that notifies when nikolaindustry-realtime connects or disconnects.

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

Returns `true` if the nikolaindustry-realtime is currently connected.

---

## 🔁 Reconnection Logic

* Retries Wi-Fi with exponential backoff (starting at 5s, capped at 60s).
* After `maxWifiRetriesBeforeAP` attempts (default: 5), it switches to AP mode.
* If `setAPTimeout()` is used, it will exit AP mode after the timeout and retry Wi-Fi.

---





