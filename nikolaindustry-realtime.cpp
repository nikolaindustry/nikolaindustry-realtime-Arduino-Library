#include "nikolaindustry-realtime.h"

nikolaindustryrealtime::nikolaindustryrealtime() {}

void nikolaindustryrealtime::begin(const char* ssid, const char* password, const char* _deviceId) {
  deviceId = _deviceId;
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected");

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

  webSocket.setReconnectInterval(5000);
}

void nikolaindustryrealtime::loop() {
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