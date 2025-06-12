#include <nikolaindustry-realtime.h>

const char *WIFI_SSID = "YOUR_WIFI_SSID";
const char *WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char *DEVICE_ID = "esp32-device-123";

nikolaindustryrealtime realtime;

// Custom handler to show configuration page
void handleConfigPage() {
  WebServer *server = realtime.getAPServer();

  if (server->hasArg("ssid") && server->hasArg("password")) {
    String ssid = server->arg("ssid");
    String password = server->arg("password");

    Serial.println("📥 Received credentials:");
    Serial.println("SSID: " + ssid);
    Serial.println("Password: " + password);

    server->send(200, "text/plain", "✅ Credentials received. Restarting...");
    delay(1000);
    ESP.restart(); // Or save and reconnect manually
  } else {
    String html = R"rawliteral(
      <!DOCTYPE html><html><body>
      <h2>Configure WiFi</h2>
      <form action="/" method="post">
        SSID: <input name="ssid" type="text"><br><br>
        Password: <input name="password" type="password"><br><br>
        <input type="submit" value="Connect">
      </form>
      </body></html>
    )rawliteral";
    server->send(200, "text/html", html);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  realtime.begin(WIFI_SSID, WIFI_PASSWORD, DEVICE_ID);
  realtime.setAPTimeout(60000); // 60 seconds timeout for AP mode

  delay(1000);  // Allow AP to start

  WebServer *server = realtime.getAPServer();
  if (server) {
    server->on("/", HTTP_GET, handleConfigPage);
    server->on("/", HTTP_POST, handleConfigPage);
    server->begin();
    Serial.println("📡 AP Web Server started.");
  }
}

void loop() {
  realtime.loop();

  if (realtime.isAPModeActive()) {
    WebServer *server = realtime.getAPServer();
    if (server) {
      server->handleClient();
    }
  }
}
