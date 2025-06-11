#include "nikolaindustry-realtime.h"

nikolaindustryrealtime::nikolaindustryrealtime() {}

void nikolaindustryrealtime::begin(const char* ssid, const char* password, const char* _deviceId) {
  deviceId = _deviceId;

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("🔌 Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected");

  // Connect to WebSocket server
  webSocket.beginSSL("nikolaindustry-realtime.onrender.com", 443, ("/?id=" + deviceId).c_str());

  webSocket.onEvent([this](WStype_t type, uint8_t * payload, size_t length) {
    if (type == WStype_TEXT) {
      DynamicJsonDocument doc(2048);
      DeserializationError err = deserializeJson(doc, payload, length);
      if (!err && onMessageCallback) {
        JsonObject obj = doc.as<JsonObject>();
        onMessageCallback(obj);
      }
    }
  });

  webSocket.setReconnectInterval(5000); // WebSocket auto-reconnect every 5s
}

void nikolaindustryrealtime::loop() {
  // Check WiFi connection and reconnect if disconnected
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi disconnected. Attempting to reconnect...");
    WiFi.disconnect();
    WiFi.reconnect();

    unsigned long startAttemptTime = millis();
    const unsigned long timeout = 10000; // 10 seconds timeout

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
      delay(500);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n✅ WiFi reconnected");
    } else {
      Serial.println("\n❌ Failed to reconnect WiFi");
      return; // Don't run WebSocket loop if no WiFi
    }
  }

  webSocket.loop();
}

void nikolaindustryrealtime::sendJson(const JsonObject& json) {
  String output;
  serializeJson(json, output);
  webSocket.sendTXT(output);
}

void nikolaindustryrealtime::setOnMessageCallback(std::function<void(JsonObject&)> callback) {
  onMessageCallback = callback;
}

void nikolaindustryrealtime::sendTo(const String& targetId, std::function<void(JsonObject&)> payloadBuilder) {
  DynamicJsonDocument doc(512);
  doc["targetId"] = targetId;
  JsonObject payload = doc.createNestedObject("payload");
  payloadBuilder(payload);
  sendJson(doc.as<JsonObject>());
}


