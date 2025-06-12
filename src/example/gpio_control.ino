#include <WiFi.h>
#include "nikolaindustry-realtime.h"

nikolaindustryrealtime realtime;

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);

  WiFi.begin("SENSORFLOW", "12345678");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected");

  realtime.begin("device-123456");

  realtime.setOnMessageCallback([](JsonObject &msg) {
    if (!msg.containsKey("payload")) return;

    JsonObject payload = msg["payload"];
    String senderId = msg["from"] | ""; // Optional: who sent the command

    if (payload["command"] == "gpio") {
      int pin = payload["pin"] | -1;
      String state = payload["state"] | "";

      if (pin == 2 && (state == "ON" || state == "OFF")) {
        digitalWrite(2, state == "ON" ? HIGH : LOW);

        // ✅ Send response back to sender
        if (senderId != "") {
          realtime.sendTo(senderId, [&](JsonObject &respPayload) {
            respPayload["response"] = "GPIO_OK";
            respPayload["pin"] = pin;
            respPayload["state"] = state;
            respPayload["status"] = "done";
          });
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
