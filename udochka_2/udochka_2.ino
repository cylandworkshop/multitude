#include <TimerOne.h>

const uint32_t EN_12 = A5;
const uint32_t LED_0 = A4;
const uint32_t LED_1 = A2;
const uint32_t LED_2 = A3;

const uint32_t STEP = 9;
const uint32_t DIR = 6;

const uint32_t ENC_0 = A0;
const uint32_t ENC_1 = A1;

const uint32_t ENDSTOP = 2;

const uint32_t SPEAKER = 5;

void handle_encoder() {
  Serial.println("enc change");
}

void make_step() {
  digitalWrite(STEP, HIGH);
  delayMicroseconds(5);
  digitalWrite(STEP, LOW);
}

void setup() {
  pinMode(EN_12, OUTPUT);
  pinMode(LED_0, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);

  pinMode(SPEAKER, OUTPUT);
  
  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);

  pinMode(ENC_0, INPUT_PULLUP);
  pinMode(ENC_1, INPUT_PULLUP);

  pinMode(ENDSTOP, INPUT);

  Serial.begin(115200);

  Timer1.initialize(800);
  Timer1.attachInterrupt(make_step);

  digitalWrite(EN_12, HIGH);

  // attachInterrupt(digitalPinToInterrupt(ENC_0), handle_encoder, CHANGE);
}

void led_blink() {
  digitalWrite(LED_0, HIGH);
  delay(50);
  digitalWrite(LED_0, LOW);

  delay(50);
  
  digitalWrite(LED_1, HIGH);
  delay(50);
  digitalWrite(LED_1, LOW);

  delay(50);

  digitalWrite(LED_2, HIGH);
  delay(50);
  digitalWrite(LED_2, LOW);

  delay(50);

  /*digitalWrite(EN_12, HIGH);
  delay(50);

  for(uint16_t i = 0; i < 100; i++) {
    
  }
  
  digitalWrite(EN_12, LOW);

  delay(50);*/
}

void test_input() {
  Serial.print(digitalRead(ENDSTOP));
  Serial.print(digitalRead(ENC_0));
  Serial.println(digitalRead(ENC_1));
  delay(100);
}

void drone() {
  static uint32_t t = 0;
  uint8_t v = (t*t)/(t^t>>12);
  digitalWrite(SPEAKER, v > 127);
  t++;
}

void loop() {
  
  while(digitalRead(ENC_0) == LOW) {
    // drone();
  }
  
  for(uint32_t i = 0; i < 30; i++) {
    digitalWrite(SPEAKER, HIGH);
    delayMicroseconds(100);
    digitalWrite(SPEAKER, LOW);
    delayMicroseconds(1200);
  }
  digitalWrite(SPEAKER, LOW);
  led_blink();

  while(digitalRead(ENC_0) == HIGH) {
    // drone();
  }
  
  // test_input();

  
  /*for(uint32_t i = 0; i < 30; i++) {
    digitalWrite(SPEAKER, HIGH);
    delayMicroseconds(100);
    digitalWrite(SPEAKER, LOW);
    delayMicroseconds(1700);
  }
  delay(200);
  for(uint32_t i = 0; i < 10; i++) {
    digitalWrite(SPEAKER, HIGH);
    delayMicroseconds(100);
    digitalWrite(SPEAKER, LOW);
    delayMicroseconds(1700);
  }
  delay(200);
  for(uint32_t i = 0; i < 10; i++) {
    digitalWrite(SPEAKER, HIGH);
    delayMicroseconds(1700);
    digitalWrite(SPEAKER, LOW);
    delayMicroseconds(1700);
  }
  */

  // rotate_blink();
  
  // delay(200);
}
