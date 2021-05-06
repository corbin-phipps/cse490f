#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET 4
Adafruit_SSD1306 _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int BUTTON_INPUT_PIN = 4;

int _x0 = 0;
int _y0 = 50;
int _w = 20;
int _h = 10;

int _delayInterval = 500;

bool _wide = true;

void setup() {
  if (!_display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  pinMode(BUTTON_INPUT_PIN, INPUT_PULLUP);
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

  int buttonVal = digitalRead(BUTTON_INPUT_PIN);
  delay(40);
  int buttonVal2 = digitalRead(BUTTON_INPUT_PIN);

  // Shoot when button is pressed
  if (buttonVal == buttonVal2 && buttonVal == LOW) {
    _delayInterval -= 100;
  }

  delay(_delayInterval);
}
