#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// Chân kết nối màn hình (điều chỉnh theo phần cứng thực tế)
Adafruit_ILI9341 tft = Adafruit_ILI9341(17, 16, 23, 18, 5, 19);

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
}

void loop() {
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
    slots[i].id = "A" + String(i+1);
    slots[i].isFull = false;
  }
  
  // Hàng 2
  for (int i = 4; i < 8; i++) {
    slots[i].x = PADDING + (i-4) * (SLOT_WIDTH + PADDING);
    slots[i].y = PADDING*2 + SLOT_HEIGHT;
    slots[i].id = "B" + String(i-3);
    slots[i].isFull = false;
  }
}

void processData(String data) {
  data.trim();
  int separator = data.indexOf(',');
  
  if (separator != -1) {
    String slotId = data.substring(0, separator);
    String status = data.substring(separator+1);
    
    for (int i = 0; i < SLOT_COUNT; i++) {
      if (slots[i].id == slotId) {
        slots[i].isFull = (status == "full");
        drawSlot(i);
        return;
      }
    }
  }
}

void drawAllSlots() {
  for (int i = 0; i < SLOT_COUNT; i++) {
    drawSlot(i);
  }
}

void drawSlot(int index) {
  Slot s = slots[index];
  
  // Vẽ khung
  tft.drawRect(s.x, s.y, SLOT_WIDTH, SLOT_HEIGHT, ILI9341_WHITE);
  
  // Tô màu nền
  uint16_t bgColor = s.isFull ? ILI9341_RED : ILI9341_GREEN;
  tft.fillRect(s.x+1, s.y+1, SLOT_WIDTH-2, SLOT_HEIGHT-2, bgColor);
  
  // Tính toán vị trí text
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(s.id, 0, 0, &x1, &y1, &w, &h);
  
  int xText = s.x + (SLOT_WIDTH - w)/2;
  int yText = s.y + (SLOT_HEIGHT - h)/2;
  
  tft.setCursor(xText, yText);
  tft.print(s.id);
}