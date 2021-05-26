/**
 * Receives a string off of the serial port and prints it out
 * on the OLED display.
 * 
 * By Jon E. Froehlich
 * @jonfroehlich
 * http://makeabilitylab.io
 * 
 */

// #include <SPI.h> // Comment out when using i2c
#include <Wire.h>
const int DELAY_MS = 5;

const int ANALOG_INPUT_PIN_X = A0;
const int ANALOG_INPUT_PIN_Y = A1;
const int MAX_ANALOG_INPUT = 1023;

int _lastAnalogValX = -1;
int _lastAnalogValY = -1;

boolean _alwaysSendData = true;

void setup() {
  Serial.begin(115200);
}

void loop() {
  int analogValX = analogRead(ANALOG_INPUT_PIN_X);
  int analogValY = analogRead(ANALOG_INPUT_PIN_Y);

  if (_alwaysSendData || _lastAnalogValX != analogValX) {
    float sizeFracX = analogValX / (float)MAX_ANALOG_INPUT;
    Serial.print(sizeFracX, 4);
    Serial.print(",");
  }
  if (_alwaysSendData || _lastAnalogValY != analogValY) {
    float sizeFracY = analogValY / (float)MAX_ANALOG_INPUT;
    Serial.print(sizeFracY, 4);
  }

  Serial.println();

  _lastAnalogValX = analogValX;
  _lastAnalogValY = analogValY;
  delay(DELAY_MS);
}
