// Parallax code from Makeability Lab: https://github.com/makeabilitylab/arduino/blob/master/MakeabilityLab_Arduino_Library/src/ParallaxJoystick.hpp
#include <ParallaxJoystick.hpp>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED _display width, in pixels
#define SCREEN_HEIGHT 64 // OLED _display height, in pixels

// Declaration for an SSD1306 _display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)

/* CONSTANTS */
const int DELAY_LOOP_MS = 5;
const int DEBOUNCE_WINDOW = 20;

const int BUTTON_INPUT_PIN = 8;

const int JOYSTICK_UPDOWN_PIN = A1;
const int JOYSTICK_LEFTRIGHT_PIN = A0;
const int MAX_ANALOG_VAL = 1023;

const enum JoystickYDirection JOYSTICK_Y_DIR = RIGHT;

/* FSM STATES */
enum ButtonState {Up, Pressed, Down, Released};
enum GameState {NewGame, Choose, PlayerDraw, ComputerDraw, Vote, EndGame};
ButtonState _buttonState;
GameState _gameState;

/* GLOBAL VARS */
Adafruit_SSD1306 _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
ParallaxJoystick _analogJoystick(JOYSTICK_UPDOWN_PIN, JOYSTICK_LEFTRIGHT_PIN, MAX_ANALOG_VAL, JOYSTICK_Y_DIR);

String wordToDraw = "";
bool useRandWord = true;

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(A2));

  pinMode(BUTTON_INPUT_PIN, INPUT_PULLUP);

  initializeOledAndShowStartupScreen();
}

void loop() {
  _display.clearDisplay();

  if (_gameState == NewGame) {
    startNewGame(BUTTON_INPUT_PIN);
  } else if (_gameState == Choose) {
    chooseWord(BUTTON_INPUT_PIN);
    Serial.println(wordToDraw);
  } else if (_gameState == PlayerDraw) {
    playerDraw();
  } else if (_gameState == ComputerDraw) {
    computerDraw();
  } else if (_gameState == Vote) {
    
  } else if (_gameState == EndGame) {
    
  }

  // Render buffer to screen
  _display.display();

  delay(DELAY_LOOP_MS);
}

void initializeOledAndShowStartupScreen(){
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!_display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  // Clear the buffer
  _display.clearDisplay();

  _display.setTextSize(1);
  _display.setTextColor(WHITE, BLACK);
  _display.setCursor(0, 0);
  _display.println("Screen initialized!");
  _display.display();
  delay(500);
  _display.clearDisplay();
}

void startNewGame(int btnPin) {
  _display.clearDisplay();
  
  int x, y, textWidth, textHeight;
  String nameString = "Let's Draw";
  String chooseString = "Choose Word";
  String randomString = "Random Word";

  _display.setTextColor(WHITE, BLACK);
  _display.setTextSize(2);
  _display.getTextBounds(nameString, 0, 0, &x, &y, &textWidth, &textHeight);
  _display.setCursor(_display.width() / 2 - textWidth / 2, _display.height() / 2 - textHeight / 2 - 15);
  _display.println(nameString);

  _display.setTextSize(1);
  _display.getTextBounds(chooseString, 0, 0, &x, &y, &textWidth, &textHeight);
  _display.setCursor(_display.width() / 2 - textWidth / 2, _display.height() / 2 - textHeight / 2 + 15);
  _display.println(chooseString);
  _display.display();

  _display.setTextSize(1);
  _display.getTextBounds(randomString, 0, 0, &x, &y, &textWidth, &textHeight);
  _display.setCursor(_display.width() / 2 - textWidth / 2, _display.height() / 2 - textHeight / 2 + 25);
  _display.println(randomString);
  _display.display();

  // TODO: set useRandWord to true/false depending on if player wants to choose word or have random word
  // TODO: if choosing word, send "choosing word" over Serial

  while (true) {
    // FSM for button state
    if (_buttonState == Up) {
      if (isButtonPressed(btnPin)) {
        _buttonState = Pressed;
      }
    } else if (_buttonState == Pressed) {
      if (isButtonPressed(btnPin)) {
        _buttonState = Down;
      } else {
        _buttonState = Up;
      }
    } else if (_buttonState == Down) {
      if (!isButtonPressed(btnPin)) {
        _buttonState = Released;
      }
    } else if (_buttonState == Released) {
      if (isButtonPressed(btnPin)) {
        _buttonState = Down;
      } else {
        _buttonState = Up;
        _gameState = Choose;
        break;
      }
    }
  }
}

// Determines if the given button is pressed, accounting for debouncing
boolean isButtonPressed(int btnPin) {
  // Read button and debounce
  int buttonVal = digitalRead(btnPin);
  delay(DEBOUNCE_WINDOW);
  int buttonVal2 = digitalRead(btnPin);

  // Shoot when button is pressed
  if (buttonVal == buttonVal2 && buttonVal == LOW) {
    return true;
  }  

  return false;
}

void chooseWord(int btnPin) {
  int numWords = 35;
  char words[numWords][10] = { "ambulance", "angel", "ant", "backpack", "barn", "basket", "bee", "bicycle", "book", "bridge", 
      "bus", "butterfly", "cactus", "cat", "chair", "dolphin", "duck", "elephant", "eye", "hand", "helicopter",
      "key", "map", "octopus", "paintbrush", "pig", "rain", "skull", "snail", "snowflake", "spider", "toothbrush", 
      "trombone", "truck", "windmill"}; // TODO: Add more
  if (useRandWord) {
    int randWordIndex = random(0, numWords);
    wordToDraw = words[randWordIndex];
  } else {
    // TODO: display possible words and use joystick/button to choose
  }

  _gameState = PlayerDraw;
}

void playerDraw() {
  _display.clearDisplay();
  
  int x, y, textWidth, textHeight;
  String drawString = "Draw a " + wordToDraw + "!";

  _display.setTextColor(WHITE, BLACK);
  _display.setTextSize(1);
  _display.getTextBounds(drawString, 0, 0, &x, &y, &textWidth, &textHeight);
  _display.setCursor(_display.width() / 2 - textWidth / 2, _display.height() / 2 - textHeight / 2);
  _display.println(drawString);

  if (Serial.available() > 0) {
    String rcvdSerialData = Serial.readStringUntil('\n');
    if (rcvdSerialData == "ComputerDraw") {
      _gameState = ComputerDraw;
    }
  }
}

void computerDraw() {
  _display.clearDisplay();

  int x, y, textWidth, textHeight;
  String drawString = "Computer is drawing a " + wordToDraw + "...";

  _display.setTextColor(WHITE, BLACK);
  _display.setTextSize(1);
  _display.getTextBounds(drawString, 0, 0, &x, &y, &textWidth, &textHeight);
  _display.setCursor(_display.width() / 2 - textWidth / 2, _display.height() / 2 - textHeight / 2);
  _display.println(drawString);  
}
