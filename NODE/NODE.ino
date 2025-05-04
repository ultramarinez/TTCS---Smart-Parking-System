#include <WiFi.h>

// AP info
const char* ssid = "ESP32-Gateway";
const char* password = "12345678";
const char* serverIP = "192.168.4.1";
const uint16_t serverPort = 8888;

// Sensors
const int sensor1Pin = 34;
const int sensor2Pin = 35;

// ID tag
const String slot1ID = "A1";
const String slot2ID = "A2";

bool lastState1 = false;
bool lastState2 = false;

WiFiClient client;

void setup() {
  Serial.begin(115200);
  pinMode(sensor1Pin, INPUT);
  pinMode(sensor2Pin, INPUT);

  connectToWiFi();
  connectToServer();
}

void loop() {
  bool state1 = digitalRead(sensor1Pin) == LOW;
  bool state2 = digitalRead(sensor2Pin) == LOW;

  if (state1 != lastState1) {
    lastState1 = state1;
    String msg = slot1ID + "," + (state1 ? "full" : "empty");
    Serial.println("Gửi: " + msg);
    sendMessage(msg);
  }

  if (state2 != lastState2) {
    lastState2 = state2;
    String msg = slot2ID + "," + (state2 ? "full" : "empty");
    Serial.println("Gửi: " + msg);
    sendMessage(msg);
  }

  delay(300);
}

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to AP");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n Connected to AP");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void connectToServer() {
  while (!client.connect(serverIP, serverPort)) {
    Serial.println("Fail to connect, retry...");
    delay(1000);
  }
  Serial.println("Connected to AP");
}

void sendMessage(String msg) {
  if (!client.connected()) {
    Serial.println("Lost connection, retry...");
    connectToServer();
  }
  client.println(msg);
}
