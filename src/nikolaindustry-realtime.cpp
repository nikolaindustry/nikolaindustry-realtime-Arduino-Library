#include "nikolaindustry-realtime.h"

DNSServer dnsServer;

nikolaindustryrealtime::nikolaindustryrealtime() {}

void nikolaindustryrealtime::begin(const char *ssid, const char *password, const char *_deviceId)
{
  deviceId = _deviceId;

  const char *customHostname = "NIKOLAINDUSTRY_Device";
  WiFi.setHostname(customHostname);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);

  WiFi.begin(ssid, password);
  Serial.print("🔌 Connecting to WiFi");
  unsigned long startAttemptTime = millis();
  const unsigned long timeout = 15000; // 15 seconds

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout)
  {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\n✅ WiFi connected");
    webSocket.beginSSL("nikolaindustry-realtime.onrender.com", 443, ("/?id=" + deviceId).c_str());

    webSocket.onEvent([this](WStype_t type, uint8_t *payload, size_t length)
                      {
      if (type == WStype_TEXT) {
        DynamicJsonDocument doc(2048);
        DeserializationError err = deserializeJson(doc, payload, length);
        if (!err && onMessageCallback) {
          JsonObject obj = doc.as<JsonObject>();
          onMessageCallback(obj);
        }
      } });

    webSocket.setReconnectInterval(5000);
  }
  else
  {
    Serial.println("\n❌ Failed to connect WiFi — switching to AP Mode");
    startAPMode();
  }
}

void nikolaindustryrealtime::loop()
{
  if (apModeActive)
  {
    dnsServer.processNextRequest();
    if (apTimeout > 0 && millis() - apStartTime >= apTimeout)
    {
      Serial.println("⏳ AP Mode timeout reached. Stopping AP...");
      stopAPMode();
    }
    return; // Don't proceed to Wi-Fi or WebSocket logic
  }

  if (!apModeActive && WiFi.status() != WL_CONNECTED)
  {
    Serial.println("⚠️ WiFi disconnected. Attempting to reconnect...");
    WiFi.disconnect();
    WiFi.reconnect();

    unsigned long startAttemptTime = millis();
    const unsigned long timeout = 10000; // 10 seconds timeout

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout)
    {
      delay(500);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("\n✅ WiFi reconnected");
      Serial.println("IP Address: " + WiFi.localIP().toString());
      webSocket.loop();
      connect();
    }
    else
    {
      Serial.println("\n❌ Failed to reconnect WiFi");
      startAPMode(); // ✅ Only call if not already in AP mode
    }
    return; // Skip WebSocket loop this cycle
  }

  if (!apModeActive && WiFi.status() == WL_CONNECTED)
  {
    webSocket.loop();
  }
}

void nikolaindustryrealtime::sendJson(const JsonObject &json)
{
  String output;
  serializeJson(json, output);
  webSocket.sendTXT(output);
}

void nikolaindustryrealtime::setOnMessageCallback(std::function<void(JsonObject &)> callback)
{
  onMessageCallback = callback;
}

void nikolaindustryrealtime::sendTo(const String &targetId, std::function<void(JsonObject &)> payloadBuilder)
{
  DynamicJsonDocument doc(512);
  doc["targetId"] = targetId;
  JsonObject payload = doc.createNestedObject("payload");
  payloadBuilder(payload);
  sendJson(doc.as<JsonObject>());
}

void nikolaindustryrealtime::connect()
{
  webSocket.beginSSL("nikolaindustry-realtime.onrender.com", 443, ("/?id=" + deviceId).c_str());

  webSocket.onEvent([this](WStype_t type, uint8_t *payload, size_t length)
                    {
    if (type == WStype_TEXT) {
      DynamicJsonDocument doc(2048);
      if (!deserializeJson(doc, payload, length) && onMessageCallback) {
        JsonObject obj = doc.as<JsonObject>();
        onMessageCallback(obj);
      }
    } });

  webSocket.setReconnectInterval(5000);
}

void nikolaindustryrealtime::startAPMode(const char *userSSID, const char *userPassword)
{
  if (apModeActive)
    return; // ⚠️ Prevent calling twice

  const char *defaultSSID = "NIKOLAINDUSTRY_Setup";
  const char *defaultPassword = "xVv9O9B4tV";

  const char *apSSID = (userSSID && strlen(userSSID) > 0) ? userSSID : defaultSSID;
  const char *apPassword = (userPassword && strlen(userPassword) > 0) ? userPassword : defaultPassword;

  WiFi.disconnect(true);
  const char *customHostname = "NIKOLAINDUSTRY_AP_Config";
  WiFi.setHostname(customHostname);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSSID, apPassword);
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));

  dnsServer.start(53, "*", WiFi.softAPIP());
  apModeActive = true;
  apStartTime = millis(); // start the timeout timer

  Serial.println("📶 AP Mode started");
  Serial.println("🔑 SSID: " + String(apSSID));
  Serial.println("🌐 Access via: http://192.168.4.1");
}

void nikolaindustryrealtime::stopAPMode()
{
  if (apModeActive)
  {
    dnsServer.stop();
    WiFi.softAPdisconnect(true);
    apModeActive = false;
    Serial.println("🛑 AP Mode stopped");
  }
}

void nikolaindustryrealtime::setAPTimeout(unsigned long timeoutMillis)
{
  apTimeout = timeoutMillis;
  Serial.printf("⏱️ AP Mode timeout set to %lu ms\n", apTimeout);
}

bool nikolaindustryrealtime::isAPModeActive() const {
  return apModeActive;
}
bool nikolaindustryrealtime::isNikolaindustryRealtimeConnected() const {
  return webSocket.isConnected();
}

