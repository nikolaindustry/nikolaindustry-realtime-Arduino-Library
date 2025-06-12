#ifndef NIKOLAINDUSTRY_REALTIME_H
#define NIKOLAINDUSTRY_REALTIME_H

#include <Arduino.h>
#include <WebSocketsClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <functional>
#include <DNSServer.h>

class nikolaindustryrealtime
{
public:
  nikolaindustryrealtime();
  void begin(const char *ssid, const char *password, const char *deviceId);
  void loop();
  void sendJson(const JsonObject &json);
  void setOnMessageCallback(std::function<void(JsonObject &)> callback);
  void sendTo(const String &targetId, std::function<void(JsonObject &)> payloadBuilder);
  void startAPMode(const char *apSSID = nullptr, const char *apPassword = nullptr);
  void stopAPMode();
  void setAPTimeout(unsigned long timeoutMillis);
  bool isAPModeActive() const;
  bool isNikolaindustryRealtimeConnected() const;



private:
  WebSocketsClient webSocket;
  bool apModeActive = false;
  unsigned long apStartTime = 0;
  unsigned long apTimeout = 0;

  DNSServer dnsServer;
  std::function<void(JsonObject &)> onMessageCallback;
  String deviceId;
  void connect();
};

#endif
