const int VIBROMOTOR_OUTPUT_PIN = 9;

const int POT_INPUT_PIN = A0;

void setup() {
  // put your setup code here, to run once:
  pinMode(VIBROMOTOR_OUTPUT_PIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int potVal = analogRead(POT_INPUT_PIN);
  int vibroStrength = potVal * 255.0 / 1023.0;
  analogWrite(VIBROMOTOR_OUTPUT_PIN, vibroStrength);
}
