#ifndef NIKOLAINDUSTRY_REALTIME_H
#define NIKOLAINDUSTRY_REALTIME_H

#include <Arduino.h>
#include <WebSocketsClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <functional>

class nikolaindustryrealtime {
public:
  nikolaindustryrealtime();
  void begin(const char* ssid, const char* password, const char* deviceId);
  void loop();
  void sendJson(const JsonObject& json);
  void setOnMessageCallback(std::function<void(JsonObject&)> callback);
  void sendTo(const String& targetId, std::function<void(JsonObject&)> payloadBuilder);

private:
  WebSocketsClient webSocket;
  std::function<void(JsonObject&)> onMessageCallback;
  String deviceId;
  void connect();
};

#endif