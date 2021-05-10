const int VIBROMOTOR_OUTPUT_PIN = 9;

void setup() {
  // put your setup code here, to run once:
  pinMode(VIBROMOTOR_OUTPUT_PIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(VIBROMOTOR_OUTPUT_PIN, HIGH);
  delay(500);
  digitalWrite(VIBROMOTOR_OUTPUT_PIN, LOW);
  delay(500);
}
