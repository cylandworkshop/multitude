#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <NXPMotionSense.h>
#include <Wire.h>
#include <EEPROM.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=188,305
AudioEffectFade          fade1;          //xy=342,259
AudioMixer4              mixer1;         //xy=474,325

AudioOutputAnalog        dac1;           //xy=620,318

AudioConnection          patchCord2(waveform1, fade1);
// AudioConnection          patchCord3(, 0, mixer1, 0);
AudioConnection          patchCord3(fade1, 0, mixer1, 0);
AudioConnection          patchCord5(mixer1, dac1);
// GUItool: end automatically generated code

unsigned int freq = 440;
int stp = 1;

const uint16_t STEP_COUNT = 1000;
const uint16_t STEP_DELAY = 100;

NXPMotionSense imu;
NXPSensorFusion filter;

int count,count2 = 0;
int step_en = 17;
int step = 8;
int dir = 9;
int pir = 16;
int encoder_a = 14;
int encoder_b = 15;

Encoder enc(encoder_a, encoder_b);

int int_led = 13;

int led[] = {3, 4, 10};


void setup() {
  Serial.begin(9600);
  imu.begin();
  filter.begin(50);

  Serial.println ("Start prop shield RAW player");

  dac1.analogReference(EXTERNAL); // much louder!
  delay(50); // time for DAC voltage stable

  // turn on the amplifier
  pinMode(5, OUTPUT); // enable amp
  digitalWrite(5, HIGH);
   
  delay(10); // allow time to wake up
  
  AudioMemory(15);

  waveform1.begin(WAVEFORM_TRIANGLE);
  waveform1.frequency(100);
  waveform1.amplitude(1.0);

  // Set initial volume
  mixer1.gain(0, 0.9);

  pinMode(int_led, OUTPUT);

  for(int i = 0; i < 3; i++) {
    pinMode(led[i], OUTPUT);
    digitalWrite(led[i], LOW);
  }

  pinMode(step_en, OUTPUT);
  pinMode(step, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(pir, INPUT_PULLDOWN);

  // pinMode(encoder_a, INPUT);
  // pinMode(encoder_b, INPUT);

  digitalWrite(step, HIGH);

  // enable stepper
  digitalWrite(int_led, HIGH);
  digitalWrite(step_en, HIGH);

  waveform1.amplitude(1.);
  fade1.fadeOut(40);

  delay(500);
}

void handle_enc() {
  waveform1.frequency(freq);

  fade1.fadeIn(1);
  digitalWrite(led[0], HIGH);
  delay(50);
  digitalWrite(led[0], LOW);
  fade1.fadeOut(40);
  
  freq *= 1.3;
  if(freq > 1400) freq -= 1124;
}

void update() {
  static int current_enc = 0;

  int enc_value = abs(enc.read());
  if(enc_value > 47) { // full round
    enc.write(1);
    enc_value = 1;
  }

  enc_value = enc_value * 10 / 48;
  
  if(enc_value != current_enc) {
    current_enc = enc_value;
    handle_enc();
  }
}

int timeout = 0;
const int TIMEOUT = 5000;

uint32_t motor_time = 0;

bool run = false;

int work_time = 0;

void loop() {
  if((millis() - work_time) < 60000) {
    int step_time = 3; // (6 - motor_phase/10);

    digitalWrite(dir, LOW);
    // analogWrite(led[0], 0);
    // analogWrite(led[1], 255);
    for(int i = 0; i < STEP_COUNT; i++) {
      digitalWrite(step, LOW);
      delayMicroseconds(100);
      digitalWrite(step, HIGH);
  
      update();
  
      delay(step_time);
    }
  } else {
    // stop rotating
    digitalWrite(step_en, LOW);
  }

  if((millis() - work_time) > 100000) {
    digitalWrite(int_led, LOW);
    digitalWrite(step_en, LOW);
    waveform1.amplitude(0.);
    analogWrite(led[2], 2);
      
    // wait for PIR
    while(digitalRead(pir) == LOW) {
      delay(100);
    }

    // wakeup again
    work_time = millis();

    waveform1.amplitude(1.);
    analogWrite(led[2], 0);
    digitalWrite(int_led, HIGH);
      
    digitalWrite(step_en, HIGH);
  }
}
