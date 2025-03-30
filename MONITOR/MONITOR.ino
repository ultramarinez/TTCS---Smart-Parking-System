#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <WebServer.h>
#include <WiFi.h>

const char *ssid = "ESP32-Gateway";
const char *password = "12345678";

Adafruit_ILI9341 tft = Adafruit_ILI9341(17, 16, 23, 18, 5, 19);
WiFiClient tcpClient;
const char *gatewayIP = "192.168.4.1"; // Địa chỉ IP của Gateway
const int tcpPort = 8888;
WebServer server(8080);
#define SLOT_COUNT 8
#define SLOT_WIDTH 70
#define SLOT_HEIGHT 40
#define PADDING 10

struct Slot {
  int x, y;
  String id;
  bool isFull;
};

Slot slots[SLOT_COUNT];

void setup() {
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(3);

  initSlots();

  tft.fillScreen(ILI9341_BLACK);
  drawAllSlots();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/update", handleUpdate);
  server.on("/status", handleStatus);
  server.begin();

  if (tcpClient.connect(gatewayIP, tcpPort)) {
    tcpClient.println("MONITOR"); // Gửi định danh
    Serial.println("Connected to gateway");
  }
}

void loop() {
  server.handleClient();
  if (tcpClient.connected() && tcpClient.available()) {
    String command = tcpClient.readStringUntil('\n');
    command.trim();
    processData(command); // Cập nhật trạng thái slot
  }
}

void handleSerial() {
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    processData(data);
  }
}

void initSlots() {
  // Hàng 1
  for (int i = 0; i < 4; i++) {
    slots[i].x = PADDING + i * (SLOT_WIDTH + PADDING);
    slots[i].y = PADDING;
    slots[i].id = "A" + String(i + 1);
    slots[i].isFull = false;
  }

  // Hàng 2
  for (int i = 4; i < 8; i++) {
    slots[i].x = PADDING + (i - 4) * (SLOT_WIDTH + PADDING);
    slots[i].y = PADDING * 2 + SLOT_HEIGHT;
    slots[i].id = "B" + String(i - 3);
    slots[i].isFull = false;
  }
}

void processData(String data) {
  data.trim();
  Serial.print("Nhận dữ liệu: ");
  Serial.println(data);

  int separator = data.indexOf(',');
  if (separator != -1) {
    String slotId = data.substring(0, separator);
    String status = data.substring(separator + 1);

    for (int i = 0; i < SLOT_COUNT; i++) {
      if (slots[i].id == slotId) {
        bool newStatus = (status == "full");
        if (slots[i].isFull != newStatus) {
          slots[i].isFull = newStatus;
          drawSlot(i);
          updateClients(slotId, status);
        }
        return;
      }
    }
  }
}

void updateClients(String slotId, String status) {
  String message = slotId + "," + status;
  server.sendHeader("Cache-Control", "no-cache");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", message);
}

void drawAllSlots() {
  for (int i = 0; i < SLOT_COUNT; i++) {
    drawSlot(i);
  }
}

void drawSlot(int index) {
  Slot s = slots[index];

  tft.drawRect(s.x, s.y, SLOT_WIDTH, SLOT_HEIGHT, ILI9341_WHITE);

  uint16_t bgColor = s.isFull ? ILI9341_RED : ILI9341_GREEN;
  tft.fillRect(s.x + 1, s.y + 1, SLOT_WIDTH - 2, SLOT_HEIGHT - 2, bgColor);

  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);

  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(s.id, 0, 0, &x1, &y1, &w, &h);

  int xText = s.x + (SLOT_WIDTH - w) / 2;
  int yText = s.y + (SLOT_HEIGHT - h) / 2;

  tft.setCursor(xText, yText);
  tft.print(s.id);
}

void handleRoot() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  String html = R"(
  <!DOCTYPE html>
  <html>
  <head>
    <title>ESP Parking System</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      .container { display: grid; grid-template-columns: repeat(4, 1fr); gap: 10px; }
      .slot { padding: 20px; border: 2px solid #333; border-radius: 5px; text-align: center; }
      .status { font-size: 0.8em; margin-top: 5px; }
    </style>
    <script>
      function updateSlot(slotId) {
        fetch('/update?slot=' + slotId)
          .then(response => response.text())
          .then(data => {
            const element = document.getElementById(slotId);
            element.style.backgroundColor = data.includes('full') ? '#ff0000' : '#00ff00';
            element.querySelector('.status').innerHTML = data;
          });
      }
      
      function refreshStatus() {
        fetch('/status')
          .then(response => response.json())
          .then(data => {
            Object.keys(data).forEach(slotId => {
              const element = document.getElementById(slotId);
              element.style.backgroundColor = data[slotId] ? '#ff0000' : '#00ff00';
              element.querySelector('.status').innerHTML = data[slotId] ? 'FULL' : 'EMPTY';
            });
          });
      }
      
      setInterval(refreshStatus, 1000);
    </script>
  </head>
  <body>
    <h1>ESP Parking Monitor</h1>
    <div class="container">)";

  for (int i = 0; i < SLOT_COUNT; i++) {
    html += "<div id='" + slots[i].id +
            "' class='slot' onclick='updateSlot(\"" + slots[i].id + "\")'>";
    html += "<div>" + slots[i].id + "</div>";
    html += "<div class='status'>" +
            String(slots[i].isFull ? "FULL" : "EMPTY") + "</div>";
    html += "</div>";
  }

  html += R"(
    </div>
  </body>
  </html>)";

  server.send(200, "text/html", html);
}

void handleUpdate() {
  if (server.hasArg("slot")) {
    String slotId = server.arg("slot");
    for (int i = 0; i < SLOT_COUNT; i++) {
      if (slots[i].id == slotId) {
        slots[i].isFull = !slots[i].isFull;
        drawSlot(i);
        server.send(200, "text/plain", slots[i].isFull ? "full" : "empty");
        return;
      }
    }
  }
  server.send(400, "text/plain", "Invalid request");
}

void handleStatus() {
  String json = "{";
  for (int i = 0; i < SLOT_COUNT; i++) {
    json +=
        "\"" + slots[i].id + "\":" + String(slots[i].isFull ? "true" : "false");
    if (i < SLOT_COUNT - 1)
      json += ",";
  }
  json += "}";
  server.send(200, "application/json", json);
}