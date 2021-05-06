#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET 4
Adafruit_SSD1306 _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int _x0 = 0;
int _y0 = 50;
int _w = 20;
int _h = 10;

bool _wide = true;

void setup() {
  if (!_display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
}

void loop() {
  // Clear the display
  _display.clearDisplay();

  int temp = 0;
  if (_wide) {
    _x0 += _w;
    _y0 -= _h;
  } else {
    _x0 += _w;
    _y0 += _w;
  }
  
  _wide = !_wide;
  temp = _w;
  _w = _h;
  _h = temp;

  if (_x0 + _w >= _display.width()) {
    _x0 = 0;
    _y0 = 50;
    _w = 20;
    _h = 10;
    _wide = true;
  }

  // Put in drawing routines
  _display.fillRect(_x0, _y0, _w, _h, SSD1306_WHITE);

  // Render graphics buffer to screen
  _display.display();

  delay(200);
}
