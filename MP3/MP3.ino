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
enum GameMode {ChooseWord, RandomWord};
ButtonState _buttonState;
GameState _gameState;
GameMode _gameMode = ChooseWord;

/* GLOBAL VARS */
Adafruit_SSD1306 _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
ParallaxJoystick _analogJoystick(JOYSTICK_UPDOWN_PIN, JOYSTICK_LEFTRIGHT_PIN, MAX_ANALOG_VAL, JOYSTICK_Y_DIR);

String wordToDraw = "";
bool useRandWord = true;
int _currWordIndex = 0;
bool _wordHasBeenChosen = false;

int _playerVoteCount = 0;
int _computerVoteCount = 0;

String oledString = ""; // Will be reused and set to different values (otherwise running into memory issues with too many Strings)
int x, y, textWidth, textHeight; // Will be reused for every _display print statement
String rcvdSerialData = "";

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(A2)); // Use noise from unconnected analog pin to help randomize the Arduino random function

  pinMode(BUTTON_INPUT_PIN, INPUT_PULLUP);

  initializeOledAndShowStartupScreen();
}

void loop() {
  if (_gameState == NewGame) {
    startNewGame(BUTTON_INPUT_PIN);
  } else if (_gameState == Choose) {
    chooseWord(BUTTON_INPUT_PIN);
  } else if (_gameState == PlayerDraw) {
    playerDraw();
  } else if (_gameState == ComputerDraw) {
    computerDraw();
  } else if (_gameState == Vote) {
    vote();
  } else if (_gameState == EndGame) {
    // No need to display anything here since the scores are displayed in the Vote game state and winner announced in p5.js sketch
  }

  // Render buffer to screen
  _display.display();

  delay(DELAY_LOOP_MS);
}

// OLED initialization code. Borrowed from Makeability Lab: https://github.com/makeabilitylab/arduino/blob/master/OLED/AnalogBallSize/AnalogBallSize.ino
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

// Handles the NewGame game state
// Displays title and both game modes, which can be selected by moving the joystick up and down and pressing the button
// Code for centering text inspired by Makeability Lab: https://github.com/makeabilitylab/arduino/blob/master/OLED/HelloWorld/HelloWorld.ino
void startNewGame(int btnPin) {
  // Read up/down value of joystick and map to value between -1 (down) and 1 (up)
  _analogJoystick.read();
  int upDownVal = _analogJoystick.getUpDownVal();
  int normalizedUpDownVal = map(upDownVal, 0, _analogJoystick.getMaxAnalogValue() + 1, -1, 2);

  // Set game mode based on movement of joystick
  if (normalizedUpDownVal == 1) {
    _gameMode = ChooseWord;
    _display.clearDisplay();
  } else if (normalizedUpDownVal == -1) {
    _gameMode = RandomWord;
    _display.clearDisplay();
  }

  // Display the game title
  oledString = "Draw It!";
  _display.setTextColor(WHITE, BLACK);
  _display.setTextSize(2);
  _display.getTextBounds(oledString, 0, 0, &x, &y, &textWidth, &textHeight);
  _display.setCursor(_display.width() / 2 - textWidth / 2, _display.height() / 2 - textHeight / 2 - 15);
  _display.println(oledString);

  // Display the game mode option for "choose word". 
  // If currently selected as game mode, text is black with white background.
  // Otherwise, text is white with black background
  oledString = "Choose Word";
  if (_gameMode == ChooseWord) {
    _display.setTextColor(BLACK, WHITE);
  } else {
    _display.setTextColor(WHITE, BLACK);
  }
  _display.setTextSize(1);
  _display.getTextBounds(oledString, 0, 0, &x, &y, &textWidth, &textHeight);
  _display.setCursor(_display.width() / 2 - textWidth / 2, _display.height() / 2 - textHeight / 2 + 15);
  _display.println(oledString);
  _display.display();

  // Display the game mode option for "random word". 
  // If currently selected as game mode, text is black with white background.
  // Otherwise, text is white with black background
  oledString = "Random Word";
  if (_gameMode == RandomWord) {
    _display.setTextColor(BLACK, WHITE);
  } else {
    _display.setTextColor(WHITE, BLACK);
  }
  _display.setTextSize(1);
  _display.getTextBounds(oledString, 0, 0, &x, &y, &textWidth, &textHeight);
  _display.setCursor(_display.width() / 2 - textWidth / 2, _display.height() / 2 - textHeight / 2 + 25);
  _display.println(oledString);
  _display.display();
  
  // FSM for button state (need to hold button for a bit, like 1/2 second, just can't be super quick tap)
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
      if (_gameMode == ChooseWord) {
        useRandWord = false;
        Serial.println("choosing word");
      } else { // _gameMode == RandomWord
        useRandWord = true;
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

// Handles the Choose game state
// Determines word to be drawn. If using a random word, a word is randomly generated from a fixed list.
// If choosing a word, use joystick to cycle through fixed list of words, with current word displayed on OLED
void chooseWord(int btnPin) {
  _display.clearDisplay();
  
  int numWords = 26;
  char words[][11] = {"angel", "ant", "barn", "basket", "bee", "bicycle", "book", "bridge", 
      "bus", "cactus", "cat", "chair", "duck", "eye", "hand", "key", "map", "octopus", "pig", 
      "rain", "skull", "snail", "spider", "trombone", "truck", "windmill"};

  if (useRandWord) {
    int randWordIndex = random(0, numWords);
    wordToDraw = words[randWordIndex];
    _wordHasBeenChosen = true;
  } else {
    // Read the left/right value of the joystick and map to values between -1 (left) and 1 (right)
    _analogJoystick.read();
    int leftRightVal = _analogJoystick.getLeftRightVal();
    int normalizedLeftRightVal = map(leftRightVal, 0, _analogJoystick.getMaxAnalogValue() + 1, -1, 2);

    // Change word based on joystick movement (increase index if moved right, decrease if moved left)
    if (normalizedLeftRightVal == 1) { // move right
      _currWordIndex++;
      if (_currWordIndex == numWords) { // went past end of list
        _currWordIndex = 0;
      }
    } else if (normalizedLeftRightVal == -1) { // move left
      _currWordIndex--;
      if (_currWordIndex == -1) { // went past beginning of list
        _currWordIndex = numWords - 1;
      }
    }
    wordToDraw = words[_currWordIndex];

    oledString = words[_currWordIndex];
    _display.setTextColor(WHITE, BLACK);
    _display.setTextSize(2);
    _display.getTextBounds(oledString, 0, 0, &x, &y, &textWidth, &textHeight);
    _display.setCursor(_display.width() / 2 - textWidth / 2, _display.height() / 2 - textHeight / 2);
    _display.println(oledString);

    // FSM for button state (need to hold button for a bit, like 1/2 second, just can't be super quick tap)
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
        _wordHasBeenChosen = true;
      }
    }
  }

  if (_wordHasBeenChosen) {
    _gameState = PlayerDraw; 
    Serial.println(wordToDraw);
  }
}

// Handles PlayerDraw game state
// Displays the word to be drawn onto the OLED screen, and updates game state if received message from p5.js sketch
void playerDraw() {
  _display.clearDisplay();
  
  oledString = "Draw a " + wordToDraw + "!";
  _display.setTextColor(WHITE, BLACK);
  _display.setTextSize(1);
  _display.getTextBounds(oledString, 0, 0, &x, &y, &textWidth, &textHeight);
  _display.setCursor(_display.width() / 2 - textWidth / 2, _display.height() / 2 - textHeight / 2);
  _display.println(oledString);

  if (Serial.available() > 0) {
    rcvdSerialData = Serial.readStringUntil('\n');
    if (rcvdSerialData == "ComputerDraw") {
      _gameState = ComputerDraw;
    }
  }
}

// Handles ComputerDraw game state
// Displays word to be drawn onto the OLED screen, and updates game state if received message from p5.js sketch
void computerDraw() {
  _display.clearDisplay();

  oledString = "Computer is drawing a " + wordToDraw + "...";
  _display.setTextColor(WHITE, BLACK);
  _display.setTextSize(1);
  _display.getTextBounds(oledString, 0, 0, &x, &y, &textWidth, &textHeight);
  _display.setCursor(_display.width() / 2 - textWidth / 2, _display.height() / 2 - textHeight / 2);
  _display.println(oledString);  

  if (Serial.available() > 0) {
    rcvdSerialData = Serial.readStringUntil('\n');
    if (rcvdSerialData == "Vote") {
      _gameState = Vote;
    }
  }
}

// Handles the Vote game state
// Displays the current vote counts for both the player and the computer, with the vote count received from the p5.js sketch
void vote() {
  _display.clearDisplay();

  _display.setTextColor(WHITE, BLACK);
  _display.setTextSize(1);

  oledString = "Player";
  _display.getTextBounds(oledString, 0, 0, &x, &y, &textWidth, &textHeight);
  _display.setCursor(_display.width() / 4 - textWidth / 2, _display.height() / 4 - textHeight / 2);
  _display.println(oledString);

  oledString = "Computer";
  _display.getTextBounds(oledString, 0, 0, &x, &y, &textWidth, &textHeight);
  _display.setCursor((_display.width() - _display.width() / 4) - textWidth / 2, _display.height() / 4 - textHeight / 2);
  _display.println(oledString);

  oledString = (String)_playerVoteCount; 
  _display.getTextBounds(oledString, 0, 0, &x, &y, &textWidth, &textHeight);
  _display.setCursor(_display.width() / 4 - textWidth / 2, _display.height() / 2 - textHeight / 2);
  _display.println(oledString);

  oledString = (String)_computerVoteCount;
  _display.getTextBounds(oledString, 0, 0, &x, &y, &textWidth, &textHeight);
  _display.setCursor((_display.width() - _display.width() / 4) - textWidth / 2, _display.height() / 2 - textHeight / 2);
  _display.println(oledString);
 
  if (Serial.available() > 0) {
    rcvdSerialData = Serial.readStringUntil('\n');
    if (rcvdSerialData == "playerVote") {
      _playerVoteCount++;
    } else if (rcvdSerialData == "computerVote") {
      _computerVoteCount++;
    } else if (rcvdSerialData == "startNewGame") {
      _gameState = NewGame;
    }
  }
}
