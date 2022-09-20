#include <Servo.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <NXPMotionSense.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=188,305
AudioEffectFade          fade1;          //xy=342,259
AudioMixer4              mixer1;         //xy=474,325

AudioOutputAnalog        dac1;           //xy=620,318

AudioConnection          patchCord2(waveform1, fade1);
// AudioConnection          patchCord3(waveform1, 0, mixer1, 0);
AudioConnection          patchCord3(fade1, 0, mixer1, 0);
AudioConnection          patchCord5(mixer1, dac1);
// GUItool: end automatically generated code

unsigned int freq = 40;
int stp = 1;


NXPMotionSense imu;
NXPSensorFusion filter;

int servo_pin = 9;

int step_0 = 4;
int dir_0 = 6;

int step_1 = 15;
int dir_1 = 16;
int en = 17;

int int_led = 13;

int sharp[] = {22, 23};
int led_strip = 21;
int amp_en = 5;

const int NUM_LEDS = 60;

int end_a = 10;
int end_b = 11;

Servo servo;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, led_strip, NEO_RGB + NEO_KHZ800);
int brightness = 0;

void setup() {
  Serial.begin(9600);
  
  imu.begin();
  filter.begin(50);
  
  dac1.analogReference(EXTERNAL); // much louder!
  delay(50); // time for DAC voltage stable
  
  AudioMemory(15);

  waveform1.begin(WAVEFORM_TRIANGLE);

  // Set initial volume
  mixer1.gain(0, 0.9);

  pinMode(int_led, OUTPUT);

  pinMode(sharp[0], INPUT);
  pinMode(sharp[1], INPUT);
  
  pinMode(step_0, OUTPUT);
  pinMode(dir_0, OUTPUT);
  
  pinMode(step_1, OUTPUT);
  pinMode(dir_1, OUTPUT);
  pinMode(en, OUTPUT);
  
  digitalWrite(en, HIGH);

  pinMode(amp_en, OUTPUT);
  digitalWrite(amp_en, LOW);

  pinMode(end_a, INPUT);
  pinMode(end_b, INPUT);

  strip.begin();
  strip.setBrightness(100);
  strip.clear();

  randomSeed(analogRead(sharp[0]));
}

int color = 0;
int beep_delay = 0;
unsigned int count = 0;

void test_stepper() {
  int r = random(100, 300);
  const int STEPS = 50;
  
  digitalWrite(en, LOW);
  delay(50);
  
  digitalWrite(dir_0, HIGH);
  for(int i = 0; i < STEPS; i++) {
    digitalWrite(step_0, LOW);
    delayMicroseconds(20);
    digitalWrite(step_0, HIGH);

    delayMicroseconds(500 + r * 50);
  }
  digitalWrite(dir_0, LOW);
  for(int i = 0; i < STEPS; i++) {
    digitalWrite(step_0, LOW);
    delayMicroseconds(20);
    digitalWrite(step_0, HIGH);

    delayMicroseconds(500 + r * 50);
  }

  digitalWrite(dir_1, HIGH);
  for(int i = 0; i < STEPS; i++) {
    digitalWrite(step_1, LOW);
    delayMicroseconds(20);
    digitalWrite(step_1, HIGH);

    delayMicroseconds(500 + r * 50);
  }
  digitalWrite(dir_1, LOW);
  for(int i = 0; i < STEPS; i++) {
    digitalWrite(step_1, LOW);
    delayMicroseconds(20);
    digitalWrite(step_1, HIGH);

    delayMicroseconds(500 + r * 50);
  }
  digitalWrite(en, HIGH);
}

void test_sound() {
  digitalWrite(amp_en, HIGH);
  delay(20);

  int r = random(100, 300);
  waveform1.amplitude(0.5);
  waveform1.frequency(r);

  delay(100);

  waveform1.amplitude(0.0);
  
  digitalWrite(amp_en, LOW);
}

void test_rgb() {
  strip.clear();
  strip.setPixelColor(0, 255, 0, 0);
  strip.show();
  delay(100);
  strip.setPixelColor(1, 0, 255, 0);
  strip.show();
  delay(100);
  strip.setPixelColor(2, 0, 0, 255);
  strip.show();
  delay(500);
  strip.clear();
  strip.show();
}
// 30..160
void test_servo() {
  servo.attach(servo_pin);
  delay(1);
  servo.write(30);
  delay(300);
  servo.write(160);
  delay(300);
  servo.write(30);
  delay(100);
  servo.detach();
}

void test_sensors() {
  static float ax, ay, az;
  static float gx, gy, gz, gx_abs, gy_abs;
  static float mx, my, mz;
  
  if (imu.available()) {
    // Read the motion sensors
    imu.readMotionSensor(ax, ay, az, gx, gy, gz, mx, my, mz);

    // Update the SensorFusion filter
    filter.update(gx, gy, gz, ax, ay, az, mx, my, mz);
    
    gy_abs = abs(gy);
    gx_abs = abs(gx);

    Serial.printf("ax: %f, ay: %f, az: %f\n", ax, ay, az);
  }

  Serial.printf("distance a: %d, b: %d\n", analogRead(sharp[0]), analogRead(sharp[1]));

  Serial.printf("end a: %d, b: %d\n", digitalRead(end_a), digitalRead(end_b));
}

void loop() {
  test_stepper();
  test_sound();
  test_servo();
  test_rgb();
  test_sensors();
  
  delay(1000);
}
