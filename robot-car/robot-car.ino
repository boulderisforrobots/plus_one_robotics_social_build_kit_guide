/*
 * Firmware for the ”2WD Ultrasonic Motor Robot Car Kit”
 *
 * Stephen A. Edwards
 * Modified by: Brandon Gigous
 *
 * Hardware configuration :
 * A pair of DC motors driven by an L298N H bridge motor driver
 * An HC−SR04 ultrasonic range sensor mounted atop a small hobby servo
*/
#include <Servo.h>

#define NUM_ANGLES 7

Servo servo;

// Ultrasonic Module pins
const int trigPin = 13; // 10 microsecond high pulse causes chirp, wait 50 us
const int echoPin = 12; // Width of high pulse indicates distance

// Servo motor that aims ultrasonic sensor.
const int servoPin = 11; // PWM output for hobby servo

// Motor control pins : L298N H bridge

const int in1Pin = 7; // Left motor Direction 1
const int enAPin = 6; // Left motor PWM speed control
const int in2Pin = 5; // Left motor Direction 2
const int in3Pin = 4; // Right motor Direction 1
const int enBPin = 3; // Right motor PWM speed control
const int in4Pin = 2; // Right motor Direction 2

enum Motor {
  LEFT,
  RIGHT
};

// Set motor speed: 255 full ahead, -255 full reverse, 0 stop
void go(enum Motor m, int speed) {
  bool flagForward;
  int speedForward;
  int speedBackward;
  int pinForward;
  int pinBackward;
  int pinSpeedControl;

  if (m == LEFT) {
    pinForward = in1Pin;
    pinBackward = in2Pin;
    pinSpeedControl = enAPin;
  } else {
    pinForward = in3Pin;
    pinBackward = in4Pin;
    pinSpeedControl = enBPin;
  }

  if (speed > 0) {
    flagForward = HIGH;
  } else {
    flagForward = LOW;
    // analogWrite takes value as positive integer, so make speed positive
    speed = -speed;
  }

  digitalWrite(pinForward, flagForward);
  digitalWrite(pinBackward, !flagForward);
  analogWrite(pinSpeedControl, speed);
}

void testMotors() {
  static int speed[8] = {
    128,
    255,
    128,
    0,
    -128,
    -255,
    -128,
    0
  };

  go(RIGHT, 0);
  for (unsigned char i = 0; i < 8; i++) {
    go(LEFT, speed[i]);
    delay(1000);
  }
  for (unsigned char i = 0; i < 8; i++) {
    go(RIGHT, speed[i]);
    delay(1000);
  }
}

signed int readDistance() {
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  unsigned long period = pulseIn(echoPin, HIGH);
  return period * 343 / 2000;
}

void servo_setup() {
  servo.attach(servoPin);
  while (!servo.attached()) {}
}

unsigned char sensorAngle[NUM_ANGLES] = {
  60,
  70,
  80,
  90,
  100,
  110,
  120
};
unsigned int distance[NUM_ANGLES];

// Scan the area ahead by sweeping the ultrasonic sensor left and right
// and recording the distance observed. This takes a reading, then
// sends the servo to the next angle. Call repeatedly once every 50 ms or so.
void readNextDistance() {
  static unsigned char angleIndex = 0;
  static signed char step = 1;
  distance[angleIndex] = readDistance();
  angleIndex += step;
  if (angleIndex == NUM_ANGLES - 1) step = -1;
  else if (angleIndex == 0) step = 1;
  servo.write(sensorAngle[angleIndex]);
}

// Initial conﬁguration
//
//   Conﬁgure the input and output pins
//   Center the servo
//   Turn off the motors
//   Test the motors
//   Scan the surroundings once
//
void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(trigPin, LOW);
  pinMode(enAPin, OUTPUT);
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(in3Pin, OUTPUT);
  pinMode(in4Pin, OUTPUT);
  pinMode(enBPin, OUTPUT);

  servo.attach(servoPin);
  servo.write(90);
  go(LEFT, 0);
  go(RIGHT, 0);
  testMotors();
  // Scan the surroundings before starting
  servo.write(sensorAngle[0]);
  delay(200);
  for (unsigned char i = 0; i < NUM_ANGLES; i++) {
    readNextDistance();
  }
  delay(200);
}

// Main loop: 
//
//   Get the next sensor reading
//   If anything appears to be too close, back up 
//   Otherwise, go forward
//
void loop() {
  readNextDistance();
  // See if something is too close at any angle
  unsigned char tooClose = 0;
  for (unsigned char i = 0; i < NUM_ANGLES; i++) {
    if (distance[i] < 300)
      tooClose = 1;
  }

  if (tooClose) {
    // Something's nearby: back up left
    go(LEFT, -180);
    go(RIGHT, -80);
  } else {
    // Nothing in our way: go forward
    go(LEFT, 255);
    go(RIGHT, 255);
  }
  // Check the next direction in 50 ms
  delay(50);
}
