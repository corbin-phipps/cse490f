class MorseCodeAlphabet {
  private:
    unsigned long dot = 200;
    unsigned long dash = 500;

  public:
    unsigned long O[3] = {dot, dot, dot};
    unsigned long S[3] = {dash, dash, dash};
    //etc.
};

class Blinker {
  private:
    const int _pin;
    unsigned long _interval;

    int _state;
    unsigned long _lastToggledTimestamp;
    
  public:
    // Constructor
    Blinker(int pin) : _pin(pin) {
      _state = LOW;
      _lastToggledTimestamp = 0;
      _interval = 0;
      pinMode(_pin, OUTPUT);
    }

    // Calculates whether to toggle output state based on the set interval
    void update() {
      int hasSent = 0;
      while (hasSent == 0) {
        unsigned long currentTimestampMs = millis();
        if (currentTimestampMs - _lastToggledTimestamp >= _interval) {
          _lastToggledTimestamp = currentTimestampMs;
          _state = !_state;
          digitalWrite(_pin, _state);

          hasSent = 1;
        }    
      }
      while (hasSent == 1) {
        unsigned long currentTimestampMs = millis();
        if (currentTimestampMs - _lastToggledTimestamp >= _interval) {
          _lastToggledTimestamp = currentTimestampMs;
          _state = !_state;
          digitalWrite(_pin, _state);

          hasSent = 0;
        }    
      }
    }

    void setInterval(int blinkInterval) {
      _interval = blinkInterval;
    }

    template <size_t n>
    void sendLetter(unsigned long (&letter) [n]) {
      int arrSize = sizeof(letter) / sizeof(letter[0]);
      
      for (int i = 0; i < arrSize; i++) {
        setInterval(letter[i]);
        update();
      }
    }
};

Blinker _led1Blinker(2);
Blinker _led2Blinker(5);
Blinker _led3Blinker(9);

MorseCodeAlphabet alphabet;

int hasSent = 0;
void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:

  
  if (hasSent == 0) {
    _led3Blinker.sendLetter(alphabet.S); 
    hasSent = 1;
  }
  delay(1000);
  hasSent = 0;
  if (hasSent == 0) {
    _led2Blinker.sendLetter(alphabet.O); 
    hasSent = 1;
  }
  delay(1000);
  hasSent = 0;
  if (hasSent == 0) {
    _led1Blinker.sendLetter(alphabet.S); 
    hasSent = 1;
  }
  hasSent = 0;
  delay(1000);
}
