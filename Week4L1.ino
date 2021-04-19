const int INPUT_PD_BUTTON_PIN = 2;
const int INPUT_PU_BUTTON_PIN = 4;
const int INPUT_IPU_BUTTON_PIN = 7;
const int OUTPUT_PD_LED_PIN = 3;
const int OUTPUT_PU_LED_PIN = 5;
const int OUTPUT_IPU_LED_PIN = 6;

void setup() {
  // put your setup code here, to run once:
  pinMode(INPUT_PD_BUTTON_PIN, INPUT);
  pinMode(INPUT_PU_BUTTON_PIN, INPUT);
  pinMode(INPUT_IPU_BUTTON_PIN, INPUT_PULLUP);
  pinMode(OUTPUT_PD_LED_PIN, OUTPUT);
  pinMode(OUTPUT_PU_LED_PIN, OUTPUT);
  pinMode(OUTPUT_IPU_LED_PIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int pdButtonState = digitalRead(INPUT_PD_BUTTON_PIN);
  int puButtonState = digitalRead(INPUT_PU_BUTTON_PIN);
  int ipuButtonState = digitalRead(INPUT_IPU_BUTTON_PIN);
  
  digitalWrite(OUTPUT_PD_LED_PIN, pdButtonState);
  digitalWrite(OUTPUT_PU_LED_PIN, !puButtonState);
  digitalWrite(OUTPUT_IPU_LED_PIN, !ipuButtonState);
  delay(30);
}
