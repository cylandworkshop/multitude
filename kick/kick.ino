int led = 13;
int trig = 2;
int echo = 3;
int power = 4;

void setup() {
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  pinMode(power, OUTPUT);
  digitalWrite(power, HIGH);
  
  pinMode(led, OUTPUT);
  Serial.begin(9600);
}

int read_distance() {
  // Clears the trigPin condition
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(echo, HIGH);
  // Calculating the distance
  int distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)

  return distance;
}

void loop() {
  int distance = read_distance();
  
  Serial.println(distance);

  if(distance < 110) {
    digitalWrite(led, HIGH);
    delay(50);
    digitalWrite(led, LOW);
    
    delay(5000); // cooldown
  }
}
