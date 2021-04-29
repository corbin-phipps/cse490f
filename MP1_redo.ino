#include <Servo.h>
#include "src/RGBConverter/RGBConverter.h"

class SunsetLight {
  private: 
    const int _redPin, _greenPin, _bluePin, _photoResPin;
    const int _maxRgbValue = 255;
    const int _maxCount = 13; // max hue is 0.13
    const unsigned long _delayInterval = 200;
    const float _photoResMin = 0.0;
    const float _photoResMax = 60.0;
    const float _lightnessMin = 0.01;
    const float _lightnessMax = 0.6;

    int _count;
    float _hue;
    float _saturation;
    float _lightness;
    float _hueStep;

    bool _ascending;

    RGBConverter _rgbConverter;

    unsigned long _lastToggledTimestamp;

  public:
    // Constructor
    SunsetLight(int redPin, int greenPin, int bluePin, int photoResPin) : 
        _redPin(redPin), _greenPin(greenPin), _bluePin(bluePin), _photoResPin(photoResPin) {
    
      _hue = 0;  
      _saturation = 1;
      _lightness = 0.5;
      _hueStep = 0.01f;

      _count = 0;
      _ascending = true;

      pinMode(_redPin, OUTPUT);
      pinMode(_greenPin, OUTPUT);
      pinMode(_bluePin, OUTPUT);
    }

    // Sets the RGB values of the RGB LED
    void setColor(int red, int green, int blue) {
      analogWrite(_redPin, red);
      analogWrite(_greenPin, green);
      analogWrite(_bluePin, blue);
    }

    // Custom map function using floats rather than integers
    float floatMap(float x, float in_min, float in_max, float out_min, float out_max) {
      return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    // Returns a float value between 0 and 1 for the lightness, based on the value from the photoresistor
    float getLightness(int photoResVal, float photoResMin, float photoResMax, float lightnessMin, float lightnessMax) {
      float photoResValFloat;
      float lightnessVal;
      
      if (photoResVal > (int)photoResMax) {
        photoResValFloat = photoResMax;
      } else if (photoResVal < (int)photoResMin) {
        photoResValFloat = photoResMin;
      } else {
        photoResValFloat = (float)photoResVal;
      }

      lightnessVal = floatMap(photoResValFloat, photoResMin, photoResMax, lightnessMin, lightnessMax);
      lightnessVal = (lightnessMax + lightnessMin) - lightnessVal;

      return lightnessVal;
    }

    // Mode 1. Crossfades the RGB LED across typical sunset colors (yellows, oranges, reds)
    // Hue values go up from 0.0-0.13, then back down from 0.13-0.0
    // Lightness is inversely proportional to the readings from the photo resistor
    void crossfade() {
      unsigned long currentTimestampMs = millis();

      if (currentTimestampMs - _lastToggledTimestamp >= _delayInterval) {
        _lastToggledTimestamp = currentTimestampMs;

        int photoResVal = analogRead(_photoResPin);
                
        _lightness = getLightness(photoResVal, _photoResMin, _photoResMax, _lightnessMin, _lightnessMax);

        byte rgb[3];
        _rgbConverter.hslToRgb(_hue, _saturation, _lightness, rgb);
        
        Serial.print(photoResVal);
        Serial.print(", ");
        Serial.print(_hue);
        Serial.print(", ");
        Serial.print(_saturation);
        Serial.print(", ");
        Serial.println(_lightness);

        setColor(rgb[0], rgb[1], rgb[2]);
        
        if (_ascending) {
          _hue += _hueStep;
        } else {
          _hue -= _hueStep;
        }
        _count += 1;

        if (_count == _maxCount) {
          _ascending = !_ascending;
          _count = 0;
        }
      }
    }

    // Mode 2. Uses a lofi slide potentiometer as a voltage divider to change the hue of the RGB LED
    void slideChangeColor(int potPin) {
      unsigned long currentTimestampMs = millis();

      if (currentTimestampMs - _lastToggledTimestamp >= _delayInterval) {
        _lastToggledTimestamp = currentTimestampMs;
      
        int potVal = analogRead(potPin);
        Serial.println(potVal);      

        float hueVal = floatMap(potVal, 0, 1023, 0, 1);

        byte rgb[3];
        _rgbConverter.hslToRgb(hueVal, _saturation, _lightness, rgb);
        setColor(rgb[0], rgb[1], rgb[2]);
      }
    }

    // Returns the distance an object is from the ultrasonic sensor in cm
    // Thank you to Woolsey Workshop for the code
    // https://www.woolseyworkshop.com/2020/04/10/interfacing-ultrasonic-distance-sensors-with-an-arduino-uno/ 
    float readDistanceSensor(int triggerPin, int echoPin) {
      digitalWrite(triggerPin, LOW);
      delayMicroseconds(2);
      digitalWrite(triggerPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(triggerPin, LOW);
      float duration = pulseIn(echoPin, HIGH);  // in microseconds
      float distance = duration * 0.0343 / 2.0;  // in centimeters
      return distance;  
    }

    // Mode 3. Uses the distance from the ultrasonic sensor to change the lightness of the RGB LED with a set hue (orange)
    void distanceChangeLightness(int triggerPin, int echoPin) {
      unsigned long currentTimestampMs = millis();

      if (currentTimestampMs - _lastToggledTimestamp >= _delayInterval) {
        _lastToggledTimestamp = currentTimestampMs;

        float distance = readDistanceSensor(triggerPin, echoPin);
        Serial.println(distance);

        float hueVal = 0.05; // keep consistent hue (and saturation)
        float lightnessVal = floatMap((int)distance, 0, 100, 0, 1);

        byte rgb[3];
        _rgbConverter.hslToRgb(hueVal, _saturation, lightnessVal, rgb);
        setColor(rgb[0], rgb[1], rgb[2]);
      }
    }
};

const int INPUT_ANALOG_PHOTO_RES_PIN = 0;
const int INPUT_ANALOG_LOFI_POT_PIN = 1;

const int INPUT_BUTTON_1_PIN = 8;
const int INPUT_BUTTON_2_PIN = 9;
const int INPUT_BUTTON_3_PIN = 10;

const int DIST_ECHO_PIN = 11;
const int DIST_TRIGGER_PIN = 12;

const int RGB_RED_PIN = 6;
const int RGB_GREEN_PIN = 5;
const int RGB_BLUE_PIN = 3;

const boolean _buttonsAreActiveLow = true;
const int _debounceWindow = 50;

enum ButtonState {One, Two, Three};
ButtonState buttonState;

SunsetLight sunsetLight(RGB_RED_PIN, RGB_GREEN_PIN, RGB_BLUE_PIN, INPUT_ANALOG_PHOTO_RES_PIN);

void setup() {
  Serial.begin(9600);
  
  pinMode(INPUT_BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(INPUT_BUTTON_2_PIN, INPUT_PULLUP);
  pinMode(INPUT_BUTTON_3_PIN, INPUT_PULLUP);
  pinMode(DIST_ECHO_PIN, INPUT);
  pinMode(DIST_TRIGGER_PIN, OUTPUT);
}

void loop() {
  // Sets buttonState based on which button is pressed
  if (isButtonPressed(INPUT_BUTTON_1_PIN)) {
    buttonState = One;
  } else if (isButtonPressed(INPUT_BUTTON_2_PIN)) {
    buttonState = Two;
  } else if (isButtonPressed(INPUT_BUTTON_3_PIN)) {
    buttonState = Three;
  }

  // Sets the mode of the sunset light based on the buttonState
  if (buttonState == One) {
    sunsetLight.crossfade();
  } else if (buttonState == Two) {
    sunsetLight.slideChangeColor(INPUT_ANALOG_LOFI_POT_PIN);
  } else if (buttonState == Three) {
    sunsetLight.distanceChangeLightness(DIST_TRIGGER_PIN, DIST_ECHO_PIN);
  }
}

// Returns a boolean value representing whether or not a button is pressed
boolean isButtonPressed(int btnPin) {
  int btnVal;
  int btnVal1 = digitalRead(btnPin);
  delay(_debounceWindow);
  int btnVal2 = digitalRead(btnPin);
  if (btnVal1 == btnVal2) {
    btnVal = btnVal1;
  }
  if (_buttonsAreActiveLow && btnVal == LOW) {
    return true;
  } 

  return false;
}
