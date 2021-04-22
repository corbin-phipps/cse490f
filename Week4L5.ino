#define KEY_C 262
#define KEY_D 294
#define KEY_E 330
#define KEY_F 350
#define KEY_G 392
#define KEY_A 440
#define KEY_B 494
#define KEY_C2 523

const int OUTPUT_LED_PIN = LED_BUILTIN;
const int OUTPUT_PIEZO_PIN = 2;
const int INPUT_FSR_PIN = A0;
const int DELAY_MS = 20; // how often to read from the sensor input

void setup() {
  pinMode(OUTPUT_LED_PIN, OUTPUT);
  pinMode(OUTPUT_PIEZO_PIN, OUTPUT);
  pinMode(INPUT_FSR_PIN, INPUT);
  Serial.begin(9600);
}

void loop() {

  // Read the force-sensitive resistor value
  int fsrVal = analogRead(INPUT_FSR_PIN);

  // Remap the value for output. 
  int ledVal = map(fsrVal, 0, 1023, 0, 255);
  int freq = map(fsrVal, 0, 1023, 50, 1500); // change min/max freq here

  // only play sound if the user is pressing on the FSR
  if(fsrVal > 0){
    if (freq < 230) {
      tone(OUTPUT_PIEZO_PIN, KEY_C);
    } else if (freq < 410) {
      tone(OUTPUT_PIEZO_PIN, KEY_D);
    } else if (freq < 590) {
      tone(OUTPUT_PIEZO_PIN, KEY_E);
    } else if (freq < 770) {
      tone(OUTPUT_PIEZO_PIN, KEY_F);
    } else if (freq < 950) {
      tone(OUTPUT_PIEZO_PIN, KEY_G);
    } else if (freq < 1130) {
      tone(OUTPUT_PIEZO_PIN, KEY_A);
    } else if (freq < 1310) {
      tone(OUTPUT_PIEZO_PIN, KEY_B);
    } else {
      tone(OUTPUT_PIEZO_PIN, KEY_C2);
    }
  }else{
    noTone(OUTPUT_PIEZO_PIN);
  }

  // Print the raw sensor value and the converted led value (e,g., for Serial Plotter)
  Serial.print(fsrVal);
  Serial.print(",");
  Serial.println(ledVal);
  Serial.print(",");
  Serial.println(freq);

  // Write out the LED value. 
  analogWrite(OUTPUT_LED_PIN, ledVal);

  delay(DELAY_MS);
}
