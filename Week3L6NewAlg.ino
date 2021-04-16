#include "src/RGBConverter/RGBConverter.h"

const int RGB_RED_PIN = 6;
const int RGB_GREEN_PIN  = 5;
const int RGB_BLUE_PIN  = 3;
const int DELAY_INTERVAL = 50; // interval in ms between incrementing hues
const byte MAX_RGB_VALUE = 255;

float _hue = 0;
float _saturation = 0;
float _lightness = 0.2;
float _hue_step = 0.001f;
float _sat_step = 0.01f;

RGBConverter _rgbConverter;

void setup() {
  // put your setup code here, to run once:
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  // Convert current hue, saturation, and lightness to RGB
  // The library assumes hue, saturation, and lightness range from 0 - 1
  // and that RGB ranges from 0 - 255
  // If lightness is equal to 1, then the RGB LED will be white
  byte rgb[3];
  _rgbConverter.hslToRgb(_hue, _saturation, _lightness, rgb);

  setColor(rgb[0], rgb[1], rgb[2]); 

  _hue += _hue_step;
  _saturation += _sat_step;

  if(_hue > 1.0){
    _hue = 0;
  }
  if(_saturation > 1.0){
    _saturation = 0;
  }

  delay(DELAY_INTERVAL);
}

void setColor(int red, int green, int blue) {
  analogWrite(RGB_RED_PIN, red);
  analogWrite(RGB_GREEN_PIN, green);
  analogWrite(RGB_BLUE_PIN, blue);    
}
