void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Built-in LED is HIGH");
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  Serial.println("Built-in LED is LOW");
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  
}
