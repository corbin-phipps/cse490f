const int POT_ANALOG_PIN = 0;
const int LED_PIN = 9;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int potVal = analogRead(POT_ANALOG_PIN);
  Serial.println(potVal);
  delay(50);

  int outVal = map(potVal, 0, 1023, 0, 255);
  analogWrite(LED_PIN, outVal);
}
