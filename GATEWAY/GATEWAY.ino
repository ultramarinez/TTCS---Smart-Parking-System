#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

#define SERIAL_BAUDRATE 115200
#define TCP_PORT 8888

// Cấu hình AP
const char* ssid = "ESP32-Gateway";
const char* password = "12345678";

WiFiServer server(TCP_PORT);
WiFiClient clients[5];  // Hỗ trợ tối đa 5 client

void setup() {
  Serial.begin(SERIAL_BAUDRATE);

  // Khởi tạo AP
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP: ");
  Serial.println(IP);

  server.begin();
}

void loop() {
  // Chấp nhận kết nối mới
  if (server.hasClient()) {
    for (int i = 0; i < 5; i++) {
      if (!clients[i] || !clients[i].connected()) {
        clients[i] = server.available();
        Serial.print("Client connected: ");
        Serial.println(clients[i].remoteIP());
        break;
      }
    }
  }

  // Xử lý dữ liệu từ các client
  for (int i = 0; i < 5; i++) {
    if (clients[i] && clients[i].connected()) {
      while (clients[i].available()) {
        String data = clients[i].readStringUntil('\n');
        Serial.printf("[CAM%d]%s\n", i, data.c_str());  // Gửi kèm ID CAM qua Serial
      }
    }
  }
}