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

constexpr uint8_t ENCODER_STEPS = 48;

volatile uint8_t beep = 0;
uint8_t activity = 0;

void test_input() {
  Serial.print(digitalRead(ENDSTOP));
  Serial.print(digitalRead(ENC_0));
  Serial.println(digitalRead(ENC_1));
}

uint32_t Duration(uint32_t t0, uint32_t t1)
{
    if (t0 <= t1)
    {
        return t1 - t0;
    }

    return UINT32_MAX - t0 + t1;
}

int8_t scan_encoder(bool* encoder_state, bool* new_state) {
    int8_t change = 0;

    if(encoder_state[0] == false && encoder_state[1] == true) {
        if(new_state[0] == true && new_state[1] == true) {
            change = 1;
        }
        if(new_state[0] == false && new_state[1] == false) {
            change = -1;
        }
    } else if(encoder_state[0] == false && encoder_state[1] == false) {
        if(new_state[0] == false && new_state[1] == true) {
            change = 1;
        }
        if(new_state[0] == true && new_state[1] == false) {
            change = -1;
        }
    } else if(encoder_state[0] == true && encoder_state[1] == false) {
        if(new_state[0] == false && new_state[1] == false) {
            change = 1;
        }
        if(new_state[0] == true && new_state[1] == true) {
            change = -1;
        }
    } else if(encoder_state[0] == true && encoder_state[1] == true) {
        if(new_state[0] == true && new_state[1] == false) {
            change = 1;
        }
        if(new_state[0] == false && new_state[1] == true) {
            change = -1;
        }
    } else {

    }

    encoder_state[0] = new_state[0];
    encoder_state[1] = new_state[1];

    return change;
}

void handle_encoder(bool endstop, bool enc_0, bool enc_1) {
  constexpr uint8_t STARTING_ENCODER_VALUE = -1;

  static int8_t encoder_value = STARTING_ENCODER_VALUE;
  static uint32_t lastTimestamp = 0;

  static bool encoder_state[2] = {false, false};
  bool new_state[2] = {enc_0, enc_1};
  int8_t diff = scan_encoder(encoder_state, new_state);
  encoder_value -= diff;

  if(endstop) {
    encoder_value = STARTING_ENCODER_VALUE;
  }

  if (encoder_value < 0)
  {
    encoder_value += ENCODER_STEPS;
  }

  encoder_value = encoder_value % ENCODER_STEPS;

  int8_t beat = encoder_value / 4; // 3/4, 4 measure = 12 beats
  static int8_t prev_beat = 0;

  if(beat != prev_beat) {
    if(beat % 3 == 0) {
      beep = 2;
    } else {
      beep = 1;
    }
  }
  prev_beat = beat;

  const uint32_t t = millis();

  if (lastTimestamp != 0 && diff != 0)
  {
  // Serial.println("enc change");
    uint32_t dt = Duration(lastTimestamp, t);
    if (dt >= UINT16_MAX)
    {
      dt = UINT16_MAX;
    }
    Serial.write('$');
    Serial.write(int8_t(encoder_value));
    Serial.write(uint8_t(dt));
    Serial.write(uint8_t(dt >> 8));
  }

  lastTimestamp = t;
}

bool enc_0_state = false;
bool enc_1_state = false;
bool endstop_state = false;

void make_step() {
  static uint8_t step_counter = 0;
  static bool step_high = true;

  if (step_counter == 0)
  {
    digitalWrite(STEP, step_high ? HIGH : LOW);
  }

  step_high = !step_high;
  if (!step_high)
  {
    return;
  }
  step_counter = (step_counter + 1) % 3;
  
  {
    bool state = digitalRead(ENDSTOP);
    if(state != endstop_state) {
      endstop_state = state;
      handle_encoder(endstop_state, enc_0_state, enc_1_state);
    }
  }
  {
    bool state = digitalRead(ENC_0);
    if(state != enc_0_state) {
      enc_0_state = state;
      handle_encoder(endstop_state, enc_0_state, enc_1_state);
    }
  }
  {
    bool state = digitalRead(ENC_1);
    if(state != enc_1_state) {
      enc_1_state = state;
      handle_encoder(endstop_state, enc_0_state, enc_1_state);
    }
  }

}

void setup() {
  pinMode(EN_12, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_0, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);

  pinMode(SPEAKER, OUTPUT);

  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);

  pinMode(ENC_0, INPUT_PULLUP);
  pinMode(ENC_1, INPUT_PULLUP);

  pinMode(ENDSTOP, INPUT);

  Serial.begin(9600);

  Timer1.initialize(750);
  Timer1.attachInterrupt(make_step);

  digitalWrite(EN_12, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
  attachInterrupt(digitalPinToInterrupt(ENC_0), handle_encoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_1), handle_encoder, CHANGE);
}

void led_blink() {
  digitalWrite(LED_0, HIGH);
  delay(20);
  digitalWrite(LED_0, LOW);

  delay(1);

  digitalWrite(LED_1, HIGH);
  delay(20);
  digitalWrite(LED_1, LOW);

  delay(1);

  digitalWrite(LED_2, HIGH);
  delay(20);
  digitalWrite(LED_2, LOW);

  delay(1);
}

void drone() {
  static uint32_t t = 0;
  uint8_t v = (t*t)/(t^t>>12);
  digitalWrite(SPEAKER, v > 127);
  t++;
}

void loop() {
  static uint32_t lastMotorToggle = 0;

  auto const t = millis();
  if (false)
  {
    activity = (activity + 1) % 2;
    if (activity == 0)
    {
      digitalWrite(EN_12, HIGH);
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else
    {
      digitalWrite(EN_12, LOW);
      digitalWrite(LED_BUILTIN, LOW);
    }
    lastMotorToggle = t;
  }

  int16_t timeout = 500;
  while(beep == 0 && timeout--) {
    delay(1);
  }

  if(timeout > 0) {
    // Serial.println("#!");

    for(uint32_t i = 0; i < 30; i++) {
      digitalWrite(SPEAKER, HIGH);
      delayMicroseconds(beep == 1 ? 20 : 30);
      digitalWrite(SPEAKER, LOW);
      delayMicroseconds(beep == 1 ? 1280 : 820);
    }
    digitalWrite(SPEAKER, LOW);
    led_blink();

    beep = 0;
  } else {

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
