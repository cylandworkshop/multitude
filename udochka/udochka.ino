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

unsigned int freq = 40;
int stp = 1;

const uint16_t STEP_COUNT = 100;
const uint16_t STEP_DELAY = 10000;

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
  mixer1.gain(0, 0.5);

  pinMode(int_led, OUTPUT);

  for(int i = 0; i < 3; i++) {
    pinMode(led[i], OUTPUT);
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

  delay(500);
}

void loop() {
  float ax, ay, az;
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
    waveform1.amplitude(1.);

    /*
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
    */
  }

  if(digitalRead(pir) == LOW && 0) {
    analogWrite(led[0], 0);
    analogWrite(led[1], 0);
    analogWrite(led[2], 0);
    waveform1.amplitude(0.);
    
    digitalWrite(int_led, LOW);
    digitalWrite(step_en, LOW);
    
    delay(10);
    return;
  } else {
    analogWrite(led[2], 10);
    digitalWrite(int_led, HIGH);
    digitalWrite(step_en, HIGH);
  }

  // rotate cw
  digitalWrite(dir, LOW);
  for(int i = 0; i < STEP_COUNT; i++) {
    analogWrite(led[0], 255);
    analogWrite(led[1], 0);
    digitalWrite(step, LOW);
    delayMicroseconds(50);
    analogWrite(led[0], 0);
    analogWrite(led[1], 255);
    digitalWrite(step, HIGH);

    delayMicroseconds(STEP_DELAY);

    int angle = enc.read();
    waveform1.frequency(200 + angle * 5);
  }
  
  // rotate ccw
  // digitalWrite(dir, HIGH);
  for(int i = 0; i < STEP_COUNT; i++) {
    analogWrite(led[0], 0);
    analogWrite(led[1], 255);
    digitalWrite(step, LOW);
    delayMicroseconds(50);
    analogWrite(led[0], 255);
    analogWrite(led[1], 0);
    digitalWrite(step, HIGH);

    delayMicroseconds(STEP_DELAY);

    int angle = enc.read();
    // waveform1.frequency(200 + angle * 5);
  }

  waveform1.frequency(freq);

  fade1.fadeIn(1);
  delay(50);
  fade1.fadeOut(40);

  
  freq *= 1.3;
  if(freq > 1400) freq -= 1124 + enc.read();
  

  delay(100);
}
