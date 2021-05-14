#include <ParallaxJoystick.hpp>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED _display width, in pixels
#define SCREEN_HEIGHT 64 // OLED _display height, in pixels

// Declaration for an SSD1306 _display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)

const int DELAY_LOOP_MS = 5; 
const int DEBOUNCE_WINDOW = 20;
const int DIE_FREQUENCY = 400;
const int SHOOT_FREQUENCY = 700;
const int PLAY_TONE_DURATION_MS = 60;
const int VIBROMOTOR_DURATION_MS = 400;
const int LASER_SPEED = 8;

const int BUTTON_INPUT_PIN = 8;
const int TONE_OUTPUT_PIN = 5;
const int VIBROMOTOR_OUTPUT_PIN = 6;
const int LED_OUTPUT_PIN = 9;

const int JOYSTICK_UPDOWN_PIN = A1;
const int JOYSTICK_LEFTRIGHT_PIN = A0;

const int MAX_ANALOG_VAL = 1023;
const enum JoystickYDirection JOYSTICK_Y_DIR = RIGHT;

unsigned long _vibroMotorStartTimeStamp = -1;

int _shipHeight = 10;
int _shipWidth = _shipHeight / 2;

int _playerX0, _playerY0, _playerX1, _playerY1, _playerX2, _playerY2;
int _enemyX0, _enemyY0, _enemyX1, _enemyY1, _enemyX2, _enemyY2;
int _barrierX0, _barrierY0, _barrierX1, _barrierY1;

int _playerLaserX0, _playerLaserY0, _playerLaserX1, _playerLaserY1;
int _enemyLaserX0, _enemyLaserY0, _enemyLaserX1, _enemyLaserY1;
int _playerLaserMove;
int _enemyLaserMove;

int _playerScoreCounter;

int _enemyYMove;
int _enemyXMove;
int _enemyMoveCounter;

int _enemyShootCounter = 0;

bool _playerDead = false;
bool _enemyDead = false;

bool _gameOver = false;

enum ButtonState {Up, Pressed, Down, Released};
enum LaserState {Still, Moving};
enum GameState {NewGame, Playing, GameOver};
ButtonState _buttonState;
LaserState _playerLaserState;
LaserState _enemyLaserState;
GameState _gameState;

Adafruit_SSD1306 _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
ParallaxJoystick _analogJoystick(JOYSTICK_UPDOWN_PIN, JOYSTICK_LEFTRIGHT_PIN, MAX_ANALOG_VAL, JOYSTICK_Y_DIR);

void setup() {
  Serial.begin(9600);

  pinMode(BUTTON_INPUT_PIN, INPUT_PULLUP);
  pinMode(TONE_OUTPUT_PIN, OUTPUT);
  pinMode(VIBROMOTOR_OUTPUT_PIN, OUTPUT);
  pinMode(LED_OUTPUT_PIN, OUTPUT);

  initializeOledAndShowStartupScreen();
  initializeVariables();
}

void loop() {
  _display.clearDisplay();

  // Either start a new game, play the game, or show the game over screen
  // depending on the current game state
  if (_gameState == NewGame) {
    startNewGame(BUTTON_INPUT_PIN);
  } else if (_gameState == Playing) {
    playGame();
  } else if (_gameState == GameOver) {
    gameOver(BUTTON_INPUT_PIN);
  }

  // Render buffer to screen
  _display.display();

  if(DELAY_LOOP_MS > 0){
    delay(DELAY_LOOP_MS);
  }
}

// Initializes the OLED screen
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

// Initializes all of the variables for a fresh game (in setup and after starting a new game after losing)
void initializeVariables() {
  // Set the location of the player's ship
  _playerX0 = 0;
  _playerY0 = (SCREEN_HEIGHT / 2) - (_shipHeight / 2);
  _playerX1 = 0;
  _playerY1 = (SCREEN_HEIGHT / 2) + (_shipHeight / 2);
  _playerX2 = _shipWidth;
  _playerY2 = SCREEN_HEIGHT / 2;

  // Set the location of the enemy's ship
  _enemyX0 = SCREEN_WIDTH - 1;
  _enemyY0 = (SCREEN_HEIGHT / 2) - (_shipHeight / 2);
  _enemyX1 = SCREEN_WIDTH - 1;
  _enemyY1 = (SCREEN_HEIGHT / 2) + (_shipHeight / 2);
  _enemyX2 = SCREEN_WIDTH - _shipWidth - 1;
  _enemyY2 = SCREEN_HEIGHT / 2;  

  // Set the location of the vertical barrier in the middle of the screen
  _barrierX0 = SCREEN_WIDTH / 2;
  _barrierY0 = 0;
  _barrierX1 = SCREEN_WIDTH / 2;
  _barrierY1 = SCREEN_HEIGHT;

  // Set the location of the player's laser to be in the middle of the player's ship
  _playerLaserX0 = _playerX0;
  _playerLaserY0 = _playerY2;
  _playerLaserX1 = _playerX2;
  _playerLaserY1 = _playerY2;

  // Set the location of the enemy's laser to be in the middle of the enemy's ship
  _enemyLaserX0 = _enemyX2;
  _enemyLaserY0 = _enemyY2;
  _enemyLaserX1 = _enemyX0;
  _enemyLaserY1 = _enemyY2;

  // Set the speed of both lasers to 0
  _enemyLaserMove = 0;
  _playerLaserMove = 0;

  // Initialize a random number of pixels for the enemy to move in both the X and Y directions
  _enemyYMove = random(-4, 5);
  _enemyXMove = random(-4, 5);
  _enemyMoveCounter = 0;

  // Reset the score to 0
  _playerScoreCounter = 0;

  // Initialize the states of the button, lasers, and game
  _buttonState = Up;
  _playerLaserState = Still;
  _enemyLaserState = Still;
  _gameState = NewGame;
}

// Displays the home screen and handles the logic for the button,
// which is used to start the new game
void startNewGame(int btnPin) {
  _display.clearDisplay();
  digitalWrite(LED_OUTPUT_PIN, LOW);
  initializeVariables();

  _display.setTextSize(1);
  _display.setTextColor(WHITE, BLACK);
  _display.setCursor(0, 0);
  _display.println("Press Button to Start New Game");
  _display.display();

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
        _gameState = Playing;
        break;
      }
    }
  }
}

// Handles all of the game logic, including both ships moving, both ships shooting lasers,
// collision detection, sound, haptics, handling game over, and drawing to the OLED screen
void playGame() {
  
  readPlayerShoot();

  moveLasers();

  handleDeaths();

  moveShips();

  checkBoundaryCollisions();

  drawShapes();  
}

// Displays the game over screen and controls the button logic,
// which moves into the NewGame state
void gameOver(int btnPin) {
  _display.clearDisplay();
  digitalWrite(LED_OUTPUT_PIN, HIGH);

  _display.setTextSize(1);
  _display.setTextColor(WHITE, BLACK);
  _display.setCursor(0, 0);
  _display.println("GAME OVER. Press button to return to home");
  _display.display();

  _buttonState = Up;
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
        _gameState = NewGame;
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

// Returns whether or not the player's ship has been hit by the enemy's laser
boolean playerHit(int shipLeftX, int shipRightX, int shipY0, int shipY1, int laserLeftX, int laserY) {
  if (laserLeftX <= shipRightX && laserLeftX >= shipLeftX && laserY >= shipY0 && laserY <= shipY1) {
    _playerDead = true;
    return true;
  }

  return false;
}

// Returns whether or not the enemy's ship has been hit by the player's laser
boolean enemyHit(int shipLeftX, int shipRightX, int shipY0, int shipY1, int laserRightX, int laserY) {
  if (laserRightX >= shipLeftX && laserRightX <= shipRightX && laserY >= shipY0 && laserY <= shipY1) {
    _enemyDead = true;
    return true;
  }

  return false;
}

// Returns whether or not the given laser if off screen
boolean laserOffScreen(int laserLeftX, int laserRightX) {
  if (laserLeftX >= SCREEN_WIDTH || laserRightX <= 0) {
    return true;  
  }

  return false;
}

// Reads the button to determine the player's laser state, as well as trigger sound when shooting
void readPlayerShoot() {
    // FSM for button state, only allows one shot per button press
  if (_buttonState == Up) {
    noTone(TONE_OUTPUT_PIN);
    if (isButtonPressed(BUTTON_INPUT_PIN)) {
      _buttonState = Pressed;
    }
  } else if (_buttonState == Pressed) {
    if (isButtonPressed(BUTTON_INPUT_PIN)) {
      _buttonState = Down;
      if (_playerLaserState == Still) {
        tone(TONE_OUTPUT_PIN, SHOOT_FREQUENCY);
        _playerLaserState = Moving;
      }
    } else {
      _buttonState = Up;
      noTone(TONE_OUTPUT_PIN);
    }
  } else if (_buttonState == Down) {
    noTone(TONE_OUTPUT_PIN);
    if (!isButtonPressed(BUTTON_INPUT_PIN)) {
      _buttonState = Released;
    }
  } else if (_buttonState == Released) {
    noTone(TONE_OUTPUT_PIN);
    if (isButtonPressed(BUTTON_INPUT_PIN)) {
      _buttonState = Down;
    } else {
      _buttonState = Up;
    }
  }
}

// Determines location of lasers and when to shoot based on laser states
void moveLasers() {
  // Counter for enemy to shoot every 50 loop cycles
  if (_enemyShootCounter == 50) {
    _enemyShootCounter = 0;
    _enemyLaserState = Moving;
  }
  _enemyShootCounter++;

  // FSM for player laser
  if (_playerLaserState == Still) {
    _playerLaserX0 = _playerX0;
    _playerLaserY0 = _playerY2;
    _playerLaserX1 = _playerX2;
    _playerLaserY1 = _playerY2;
    _playerLaserMove = 0;  
  } else if (_playerLaserState == Moving) {
    _playerLaserMove = LASER_SPEED;  
    if (enemyHit(_enemyX2, _enemyX0, _enemyY0, _enemyY1, _playerLaserX1, _playerLaserY0) || laserOffScreen(_playerLaserX0, _playerLaserX1)) {
      _playerLaserState = Still;
    }
  }

  // FSM for enemy laser
  if (_enemyLaserState == Still) {
    _enemyLaserX0 = _enemyX2;
    _enemyLaserY0 = _enemyY2;
    _enemyLaserX1 = _enemyX0;
    _enemyLaserY1 = _enemyY2;  
    _enemyLaserMove = 0;
  } else if (_enemyLaserState == Moving) {
    _enemyLaserMove = LASER_SPEED;
    if (playerHit(_playerX0, _playerX2, _playerY0, _playerY1, _enemyLaserX0, _enemyLaserY0) || laserOffScreen(_enemyLaserX0, _enemyLaserX1)) {
      _enemyLaserState = Still;
    }
  }  
}

// Play sounds and haptics when a death occurs, increment score when enemy dies, change game state to GameOver when player dies
void handleDeaths() {
  // Vibro motor and sound when someone dies
  if (_playerDead || _enemyDead) {
    if (_playerDead) {
      _gameOver = true;
    }
    if (_enemyDead) {
      _playerScoreCounter++;
    }
    _playerDead = false;
    _enemyDead = false;

    tone(TONE_OUTPUT_PIN, DIE_FREQUENCY, PLAY_TONE_DURATION_MS);
    digitalWrite(VIBROMOTOR_OUTPUT_PIN, HIGH);
    _vibroMotorStartTimeStamp = millis();
  }
  if (_vibroMotorStartTimeStamp != -1) {
    if (millis() - _vibroMotorStartTimeStamp > VIBROMOTOR_DURATION_MS) {
      _vibroMotorStartTimeStamp = -1;
      digitalWrite(VIBROMOTOR_OUTPUT_PIN, LOW);

      if (_gameOver) {
        _gameState = GameOver;
        _gameOver = false;
      }
    }
  }  
}

// Determines amount of pixels for each ship to move, then updates the locations
void moveShips() {
  // Read analog joystick to control player ship
  _analogJoystick.read();
  int upDownVal = _analogJoystick.getUpDownVal();
  int leftRightVal = _analogJoystick.getLeftRightVal();

  // Translate joystick movement into amount of pixels to move
  int playerYMove = map(upDownVal, 0, _analogJoystick.getMaxAnalogValue() + 1, -4, 5);
  int playerXMove = map(leftRightVal, 0, _analogJoystick.getMaxAnalogValue() + 1, -4, 5);

  // Randomly change enemy's amount of pixels to move every 5 loop cycles
  if (_enemyMoveCounter == 5) {
    _enemyMoveCounter = 0;
    
    _enemyYMove = random(-4, 5);
    _enemyXMove = random(-4, 5);
  }
  _enemyMoveCounter++;

  // Update player ship location
  _playerX0 += playerXMove;
  _playerY0 -= playerYMove;
  _playerX1 += playerXMove;
  _playerY1 -= playerYMove;
  _playerX2 += playerXMove;
  _playerY2 -= playerYMove;

  // Update enemy ship location
  _enemyX0 += _enemyXMove;
  _enemyY0 -= _enemyYMove;
  _enemyX1 += _enemyXMove;
  _enemyY1 -= _enemyYMove;
  _enemyX2 += _enemyXMove;
  _enemyY2 -= _enemyYMove;

  // Update player laser location
  _playerLaserX0 += _playerLaserMove;
  _playerLaserX1 += _playerLaserMove;

  // Update enemy laser location
  _enemyLaserX0 -= _enemyLaserMove;
  _enemyLaserX1 -= _enemyLaserMove;  
}

// Detects whether or not a ship has reached the boundary of the screen or the barrier,
// and adjusts the positions to contain the ships within their boundaries
void checkBoundaryCollisions() {
  // Boundary detection for player ship X-axis
  if (_playerX0 <= 0) {
    _playerX0 = 0;
    _playerX1 = 0;
    _playerX2 = _shipWidth;
  } else if (_playerX2 >= _barrierX0) {
    _playerX0 = _barrierX0 - _shipWidth - 1;
    _playerX1 = _barrierX0 - _shipWidth - 1;
    _playerX2 = _barrierX0 - 1;
  }

  // Boundary detection for player ship Y-axis
  if (_playerY0 <= 0) {
    _playerY0 = 0;
    _playerY1 = _shipHeight;
    _playerY2 = _shipHeight / 2;
  } else if (_playerY1 >= SCREEN_HEIGHT) {
    _playerY0 = SCREEN_HEIGHT - _shipHeight - 1;
    _playerY1 = SCREEN_HEIGHT - 1;
    _playerY2 = SCREEN_HEIGHT - (_shipHeight / 2) - 1;
  }

  // Boundary detection for enemy ship X-axis
  if (_enemyX2 <= _barrierX0) {
    _enemyX0 = _barrierX0 + _shipWidth;
    _enemyX1 = _barrierX0 + _shipWidth;
    _enemyX2 = _barrierX0;
  } else if (_enemyX0 >= SCREEN_WIDTH) {
    _enemyX0 = SCREEN_WIDTH - 1;
    _enemyX1 = SCREEN_WIDTH - 1;
    _enemyX2 = SCREEN_WIDTH - _shipWidth - 1;
  }

  // Boundary detection for enemy ship Y-axis
  if (_enemyY0 <= 0) {
    _enemyY0 = 0;
    _enemyY1 = _shipHeight;
    _enemyY2 = _shipHeight / 2;
  } else if (_enemyY1 >= SCREEN_HEIGHT) {
    _enemyY0 = SCREEN_HEIGHT - _shipHeight - 1;
    _enemyY1 = SCREEN_HEIGHT - 1;
    _enemyY2 = SCREEN_HEIGHT - (_shipHeight / 2) - 1;
  }  
}

// Draws the ships, lasers, and the middle barrier
void drawShapes() {
  // Put in drawing routines
  _display.fillTriangle(_playerX0, _playerY0, _playerX1, _playerY1, _playerX2, _playerY2, SSD1306_WHITE);
  _display.fillTriangle(_enemyX0, _enemyY0, _enemyX1, _enemyY1, _enemyX2, _enemyY2, SSD1306_WHITE);
  _display.drawLine(_barrierX0, _barrierY0, _barrierX1, _barrierY1, SSD1306_WHITE);
  _display.drawLine(_playerLaserX0, _playerLaserY0, _playerLaserX1, _playerLaserY1, SSD1306_WHITE);
  _display.drawLine(_enemyLaserX0, _enemyLaserY0, _enemyLaserX1, _enemyLaserY1, SSD1306_WHITE);

  _display.setTextSize(1);
  _display.setTextColor(WHITE, BLACK);
  _display.setCursor(0, 0);
  _display.println(_playerScoreCounter);
}
