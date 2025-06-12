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
  bool isNikolaindustryRealtimeConnected();
  void setOnConnectionStatusChange(std::function<void(bool)> callback);

private:
  WebSocketsClient webSocket;
  DNSServer dnsServer;
  
  bool apModeActive = false;
  unsigned long apStartTime = 0;
  unsigned long apTimeout = 0;
  unsigned long lastReconnectAttempt = 0;
  unsigned long reconnectDelay = 5000;           // Start with 5 seconds
  const unsigned long maxReconnectDelay = 60000; // Cap at 60 seconds
  int wifiRetryCount = 0;
  const int maxWifiRetriesBeforeAP = 5;
  String deviceId;

  std::function<void(JsonObject &)> onMessageCallback;
  std::function<void(bool)> onConnectionStatusChange;

  void connect();
};

#endif
