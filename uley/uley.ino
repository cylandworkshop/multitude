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
int step = 4;
int dir = 6;

int int_led = 13;

int sharp[] = {22, 23};
int led_strip = 21;

const int NUM_LEDS = 60;

Servo servo;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, led_strip, NEO_RGB + NEO_KHZ800);
int brightness = 0;

void setup() {
  Serial.begin(9600);
  imu.begin();
  filter.begin(50);
  
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

  pinMode(sharp[0], INPUT);
  pinMode(sharp[1], INPUT);
  
  pinMode(step, OUTPUT);
  pinMode(dir, OUTPUT);

  servo.attach(servo_pin);
  servo.write(140);

  strip.begin();
  strip.setBrightness(100);
  strip.clear();
}

int color = 0;
int beep_delay = 0;
unsigned int count = 0;

void loop() {
  float ax, ay, az;
  float gx, gy, gz, gx_abs, gy_abs;
  float mx, my, mz;

  if (imu.available()) {
    // Read the motion sensors
    imu.readMotionSensor(ax, ay, az, gx, gy, gz, mx, my, mz);

    // Update the SensorFusion filter
    filter.update(gx, gy, gz, ax, ay, az, mx, my, mz);
    
    gy_abs = abs(gy);
    gx_abs = abs(gx);
  }

  int distance = max(analogRead(sharp[0]), analogRead(sharp[1]));
  

  digitalWrite(int_led, HIGH);

  // rotate cw
  digitalWrite(dir, count > 120 ? LOW : HIGH);
  for(int i = 0; i < 20 * gy_abs * gx_abs; i++) {
    digitalWrite(step, LOW);
    delayMicroseconds(200);
    digitalWrite(step, HIGH);

    delayMicroseconds(6000 + distance * 2);
  }
  
  if((count - beep_delay) == 20){
    waveform1.frequency(480 * pow(2, (random(7) % 2) * 4./12.));
    waveform1.amplitude(0.9);
    
    if(distance < 100) {
      servo.write(80);
    }
    
    fade1.fadeIn(1);
    delay(10);
    fade1.fadeOut(90);
    delay(5);
    fade1.fadeIn(2);
    delay(5);
    fade1.fadeOut(70);
  }
  
  if((count - beep_delay) > 26) {
    beep_delay = count;
    servo.write(140);
  }

  digitalWrite(int_led, LOW);

  strip.clear();

  int r = 0;
  int g = 0;
  int b = 0;
  
  if(distance > 100) {
    r = 70 * (count % 2);
  } else {
    b =  random (30);
    r = brightness + (count % 2);
  }
    
  for(int n = 0; n < NUM_LEDS; n++) {
    strip.setPixelColor(n, b | (r << 8) | (g << 16));
  }
  strip.show();

  brightness += 10;
  if(brightness > 100) {
    brightness = 0;
  }
    
  count++;
  if(count > 400) count = 0;

  delay(20 + distance/5);
}
