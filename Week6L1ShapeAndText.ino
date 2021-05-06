#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET 4
Adafruit_SSD1306 _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  if (!_display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
}

void loop() {
  // Clear the display
  _display.clearDisplay();

  const char str[] = "Triangle";
  _display.setTextSize(1);
  _display.setTextColor(WHITE, BLACK);

  // Put in drawing routines
  _display.fillTriangle(5, 5, 5, 11, 10, 8, SSD1306_WHITE);
  _display.setCursor(20, 8);
  _display.print(str);

  // Render graphics buffer to screen
  _display.display();
}
