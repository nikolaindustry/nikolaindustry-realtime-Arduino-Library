#include "nikolaindustry-realtime.h"

nikolaindustryrealtime::nikolaindustryrealtime() {}

void nikolaindustryrealtime::begin(const char *_deviceId)
{
  deviceId = _deviceId;

  if (WiFi.status() == WL_CONNECTED)
  {
    connect();
  }
  else
  {
    Serial.println("❌ WiFi not connected. WebSocket not started.");
  }
}

void nikolaindustryrealtime::connect()
{
  webSocket.beginSSL("nikolaindustry-realtime.onrender.com", 443, ("/?id=" + deviceId).c_str());

  webSocket.onEvent([this](WStype_t type, uint8_t *payload, size_t length)
                    {
    DynamicJsonDocument doc(2048);
    switch (type) {
      case WStype_CONNECTED:
        Serial.println("🟢 WebSocket connected");
        if (onConnectionStatusChange) onConnectionStatusChange(true);
        break;
      case WStype_DISCONNECTED:
        Serial.println("🔴 WebSocket disconnected");
        if (onConnectionStatusChange) onConnectionStatusChange(false);
        break;
      case WStype_TEXT:
        
        if (!deserializeJson(doc, payload, length) && onMessageCallback) {
          JsonObject obj = doc.as<JsonObject>();
          onMessageCallback(obj);
        }
        break;
      default:
        break;
    } });

  webSocket.setReconnectInterval(5000);
}

void nikolaindustryrealtime::loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    webSocket.loop();
  }
}

void nikolaindustryrealtime::sendJson(const JsonObject &json)
{
  String output;
  if (serializeJson(json, output))
  {
    webSocket.sendTXT(output);
  }
  else
  {
    Serial.println("❌ Failed to serialize JSON!");
  }
}

void nikolaindustryrealtime::sendTo(const String &targetId, std::function<void(JsonObject &)> payloadBuilder)
{
  DynamicJsonDocument doc(512);
  doc["targetId"] = targetId;
  JsonObject payload = doc.createNestedObject("payload");
  payloadBuilder(payload);
  sendJson(doc.as<JsonObject>());
}

void nikolaindustryrealtime::setOnMessageCallback(std::function<void(JsonObject &)> callback)
{
  onMessageCallback = callback;
}

void nikolaindustryrealtime::setOnConnectionStatusChange(std::function<void(bool)> callback)
{
  onConnectionStatusChange = callback;
}

bool nikolaindustryrealtime::isNikolaindustryRealtimeConnected()
{
  return webSocket.isConnected();
}
