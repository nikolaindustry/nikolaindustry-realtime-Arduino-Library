# nikolaindustry-realtime Arduino Library

This library provides easy integration with the NIKOLAINDUSTRY real-time server for ESP32 boards.

## Features
- Secure (`wss://`)
- JSON message parsing and sending
- Callback-based message handler
- Automatic reconnect
- Simplified helper function `sendTo(...)` for error-free messaging

## Installation
1. Download the `.zip` file.
2. In Arduino IDE, go to **Sketch > Include Library > Add .ZIP Library...**
3. Select the downloaded file.

## Usage Example
```cpp
#include <nikolaindustry-realtime.h>

nikolaindustryrealtime realtime;

void setup() {
  Serial.begin(115200);
  realtime.begin("YourSSID", "YourPassword", "esp32-001");
  realtime.setOnMessageCallback([](JsonObject& msg) {
    serializeJsonPretty(msg, Serial);
  });

  // Send JSON in one line
  realtime.sendTo("server", [](JsonObject& payload) {
    payload["hello"] = "world";
  });
}

void loop() {
  realtime.loop();
}
```

# NIKOLAINDUSTRY Realtime Arduino Library License

Copyright (c) 2025 NIKOLAINDUSTRY

## License Type: Proprietary - All Rights Reserved

This software and associated files (the "Library") are the exclusive property of **NIKOLAINDUSTRY**. You are **NOT** permitted to copy, modify, distribute, sublicense, or reverse engineer any part of this code without express written permission from NIKOLAINDUSTRY.

### You MAY:
- Use this Library **exclusively** with NIKOLAINDUSTRY hardware or software platforms.
- Integrate the Library into closed-source commercial or industrial applications developed **in partnership with** NIKOLAINDUSTRY.
- Contact NIKOLAINDUSTRY for licensing terms to enable distribution or broader use.

### You MAY NOT:
- Redistribute, sell, sublicense, or disclose this Library or any derivative works.
- Modify or decompile the source code for any purpose other than evaluation or integration with authorized systems.
- Use this Library in any open-source project, public-facing repository, or third-party product without prior written approval.

## Disclaimers

- **NO WARRANTY**: This software is provided "as is" without any warranties of any kind, whether express or implied, including but not limited to warranties of merchantability or fitness for a particular purpose.

- **NO LIABILITY**: In no event shall NIKOLAINDUSTRY, its employees, or its affiliates be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including hardware damage, data loss, or system failure) arising in any way from the use or inability to use this Library.

- **NOT FOR CRITICAL SYSTEMS**: This Library is **not certified** for use in life-support, nuclear, military, or other safety-critical systems. Use in such environments is **strictly prohibited**.

## Enforcement

Unauthorized use, duplication, or distribution of this Library may result in civil and criminal penalties and will be prosecuted to the maximum extent possible under law.

---

For commercial licensing, OEM partnerships, or distribution rights, contact:

**NIKOLAINDUSTRY**  
Email: support@nikolaindustry.com  
Website: [https://nikolaindustry.com](https://nikolaindustry.com)
