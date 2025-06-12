#include "nikolaindustry-realtime.h"
#include <esp_task_wdt.h>

nikolaindustryrealtime::nikolaindustryrealtime() {}

void nikolaindustryrealtime::begin(const char *ssid, const char *password, const char *_deviceId) {
  deviceId = _deviceId;

  const char *customHostname = "NIKOLAINDUSTRY_Device";
  WiFi.setHostname(customHostname);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);

  WiFi.begin(ssid, password);
  Serial.print("🔌 Connecting to WiFi");
  unsigned long startAttemptTime = millis();
  const unsigned long timeout = 15000;

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
    delay(500);
    Serial.print(".");
    esp_task_wdt_reset();  // ✅ prevent watchdog timeout
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi connected");
    webSocket.beginSSL("nikolaindustry-realtime.onrender.com", 443, ("/?id=" + deviceId).c_str());

    webSocket.onEvent([this](WStype_t type, uint8_t *payload, size_t length) {
      if (type == WStype_CONNECTED) {
        Serial.println("🟢 Socket connected");
        if (onConnectionStatusChange) onConnectionStatusChange(true);  // ✅
      } else if (type == WStype_DISCONNECTED) {
        Serial.println("🔴 Socket disconnected");
        if (onConnectionStatusChange) onConnectionStatusChange(false);  // ✅
      } else if (type == WStype_TEXT) {
        DynamicJsonDocument doc(2048);
        DeserializationError err = deserializeJson(doc, payload, length);
        if (!err && onMessageCallback) {
          JsonObject obj = doc.as<JsonObject>();
          onMessageCallback(obj);
        }
      }
    });

    webSocket.setReconnectInterval(5000);
  } else {
    Serial.println("\n❌ Failed to connect WiFi — switching to AP Mode");
    startAPMode();
  }
}

void nikolaindustryrealtime::loop() {
  if (apModeActive) {
    dnsServer.processNextRequest();
    if (apTimeout > 0 && millis() - apStartTime >= apTimeout) {
      Serial.println("⏳ AP Mode timeout reached. Stopping AP...");
      stopAPMode();
    }
    return;
  }

  if (!apModeActive && WiFi.status() != WL_CONNECTED) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt >= reconnectDelay) {
      lastReconnectAttempt = now;
      Serial.printf("🔁 Attempting WiFi reconnect #%d...\n", wifiRetryCount + 1);

      WiFi.disconnect();
      WiFi.reconnect();

      unsigned long startAttemptTime = millis();
      const unsigned long timeout = 10000;

      while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
        delay(500);
        Serial.print(".");
        esp_task_wdt_reset();  // ✅ watchdog during reconnect
      }

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ WiFi reconnected");
        reconnectDelay = 5000;
        wifiRetryCount = 0;
        connect();
      } else {
        Serial.println("\n❌ WiFi reconnect failed");
        wifiRetryCount++;
        reconnectDelay = min(reconnectDelay * 2, maxReconnectDelay);

        if (wifiRetryCount >= maxWifiRetriesBeforeAP) {
          Serial.println("🚨 Max WiFi retries reached. Switching to AP Mode.");
          startAPMode();
        }
      }
    }
    return;
  }

  if (!apModeActive && WiFi.status() == WL_CONNECTED) {
    webSocket.loop();
  }
}

void nikolaindustryrealtime::sendJson(const JsonObject &json) {
  String output;
  if (serializeJson(json, output)) {
    webSocket.sendTXT(output);
  } else {
    Serial.println("❌ Failed to serialize JSON before sending!");
  }
}

void nikolaindustryrealtime::setOnMessageCallback(std::function<void(JsonObject &)> callback) {
  onMessageCallback = callback;
}

void nikolaindustryrealtime::setOnConnectionStatusChange(std::function<void(bool)> callback) {
  onConnectionStatusChange = callback;
}

void nikolaindustryrealtime::sendTo(const String &targetId, std::function<void(JsonObject &)> payloadBuilder) {
  DynamicJsonDocument doc(512);
  doc["targetId"] = targetId;
  JsonObject payload = doc.createNestedObject("payload");
  payloadBuilder(payload);
  sendJson(doc.as<JsonObject>());
}

void nikolaindustryrealtime::connect() {
  webSocket.beginSSL("nikolaindustry-realtime.onrender.com", 443, ("/?id=" + deviceId).c_str());

  webSocket.onEvent([this](WStype_t type, uint8_t *payload, size_t length) {
    if (type == WStype_CONNECTED) {
      Serial.println("🟢 WebSocket connected");
      if (onConnectionStatusChange) onConnectionStatusChange(true);
    } else if (type == WStype_DISCONNECTED) {
      Serial.println("🔴 WebSocket disconnected");
      if (onConnectionStatusChange) onConnectionStatusChange(false);
    } else if (type == WStype_TEXT) {
      DynamicJsonDocument doc(2048);
      if (!deserializeJson(doc, payload, length) && onMessageCallback) {
        JsonObject obj = doc.as<JsonObject>();
        onMessageCallback(obj);
      }
    }
  });

  webSocket.setReconnectInterval(5000);
}

void nikolaindustryrealtime::startAPMode(const char *userSSID, const char *userPassword) {
  if (apModeActive) return;

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
  apStartTime = millis();

  Serial.println("📶 AP Mode started");
  Serial.println("🔑 SSID: " + String(apSSID));
  Serial.println("🌐 Access via: http://192.168.4.1");
}

void nikolaindustryrealtime::stopAPMode() {
  if (apModeActive) {
    dnsServer.stop();
    WiFi.softAPdisconnect(true);
    apModeActive = false;
    Serial.println("🛑 AP Mode stopped");
  }
}

void nikolaindustryrealtime::setAPTimeout(unsigned long timeoutMillis) {
  apTimeout = timeoutMillis;
  Serial.printf("⏱️ AP Mode timeout set to %lu ms\n", apTimeout);
}

bool nikolaindustryrealtime::isAPModeActive() const {
  return apModeActive;
}

bool nikolaindustryrealtime::isNikolaindustryRealtimeConnected()  {
  return webSocket.isConnected();
}
