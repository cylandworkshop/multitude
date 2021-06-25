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
AudioConnection          patchCord3(waveform1, 0, mixer1, 0);
// AudioConnection          patchCord3(fade1, 0, mixer1, 0);
AudioConnection          patchCord5(mixer1, dac1);
// GUItool: end automatically generated code

unsigned int freq = 40;
int stp = 1;

const uint16_t STEP_COUNT = 10;
const uint16_t STEP_DELAY = 5000;

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
  mixer1.gain(0, 0.5);

  pinMode(int_led, OUTPUT);

  pinMode(sharp[0], INPUT);
  pinMode(sharp[1], INPUT);
  
  pinMode(step, OUTPUT);
  pinMode(dir, OUTPUT);

  servo.attach(servo_pin);
  servo.write(10);

  strip.begin();
  strip.setBrightness(100);
  strip.clear();
}

int color = 0;

void loop() {
  /*float ax, ay, az;
  float gx, gy, gz, aud1, aud2;
  float mx, my, mz;
  float roll, pitch, heading;

  if (imu.available()) {
    // Read the motion sensors
    imu.readMotionSensor(ax, ay, az, gx, gy, gz, mx, my, mz);

    // Update the SensorFusion filter
    filter.update(gx, gy, gz, ax, ay, az, mx, my, mz);

    // print the heading, pitch and roll
    roll = filter.getRoll();
    pitch = filter.getPitch();
    // heading = filter.getYaw();
    Serial.print(ax);
    Serial.print(',');
    Serial.print(ay);
    // Serial.print(',');
    // Serial.print(az);
    // Serial.print(',');
    Serial.print(gx);
    // Serial.print(',');
    Serial.print(gy);
    // Serial.print(',');
    // Serial.print(gz);
    // Serial.print(',');
    //    Serial.print(mx);
    //    Serial.print(',');
    //    Serial.print(my);
    //    Serial.print(',');
    //    Serial.print(mz);
    Serial.println();
    aud1 = abs(gy);
    aud2 = abs(gx);

    // waveform1.amplitude(0.5 * (1.0 + ax));
    // waveform1.amplitude(1.);
    
    if (aud1 < 3 && count > 150) {
      fade1.fadeOut(1000);
      //waveform1.amplitude(0);
    }//was 0.5 }
    if (aud1 > 3) {
      fade1.fadeIn(100);
      waveform1.amplitude(1);
      freq = map(aud1, 1, 200, 60, 1000);
      waveform1.frequency(freq);
      count = 0;
    }
    if (aud2 < 3 && count2 > 50) {
      fade2.fadeOut(500);
      //waveform1.amplitude(0);
    }//was 0.5 }
    if (aud2 > 3) {
      fade2.fadeIn(100);
      waveform2.amplitude(1);
      freq = map(aud2, 1, 200, 60, 1000);
      waveform2.frequency(freq);
      count2 = 0;
    }
    
    count ++;
    count2 ++;
  }
  */
  

  digitalWrite(int_led, HIGH);

  // rotate cw
  digitalWrite(dir, LOW);
  for(int i = 0; i < STEP_COUNT; i++) {
    digitalWrite(step, LOW);
    delayMicroseconds(200);
    digitalWrite(step, HIGH);

    delayMicroseconds(1000 + analogRead(sharp[1]) * 100);
  }
  
  waveform1.frequency(analogRead(sharp[0]));
  // waveform1.amplitude(1. - analogRead(sharp[1])/1000.);
  waveform1.amplitude(0.2);
  // servo.write(analogRead(sharp[0])/4);

  // fade1.fadeIn(1);
  // delay(50);
  // fade1.fadeOut(40);

  digitalWrite(int_led, LOW);

  strip.setPixelColor(0, color);
  color += 10;
  if(color > 255) color = 0;
  
  strip.show();

  /*for(int n = NUM_LEDS; n > 0; n--) {
    strip.clear();
    
    for(int i = 0; i < 10; i++) {
      int n_i = (n + i - 5) % NUM_LEDS;
      
      int r = (brightness * 7) % 256 * (5 - abs(i - 5))/5;
      int g = (brightness * 3 + n_i) % 256 * (5 - abs(i - 5))/10;
      int b = (brightness * 5 + 2 * n_i) % 256 * (5 - abs(i - 5))/10;
      strip.setPixelColor(n_i, b | (g << 8) | (r << 16));
    }
    
    strip.show();
  
    brightness++;
    
    if(brightness > 100) {
      brightness = 0;
    }
    
    delay(15);
  }
  strip.clear();
  strip.show();*/
  

  delay(analogRead(sharp[0])/10);
}
