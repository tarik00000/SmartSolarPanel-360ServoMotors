#include <Servo.h>

// Servo objects
Servo horizontal;
int horizontalStop = 90; // Value to stop horizontal servo (adjust if needed)
int horizontalSpeed = 0;

Servo vertical;
int verticalStop = 90;   // Value to stop vertical servo (adjust if needed)
int verticalSpeed = 0;
// Reverse direction flags (change to true if needed)
bool reverseHorizontal = true;
bool reverseVertical = false;
int dayNightSamples[10];  // 10 uzoraka
int dayNightSampleIndex = 0;

int ledPin = 5; // LED povezana na digitalni pin 5
int ldrDayNightPin = A4;  // Use actual analog pin for your new day/night LDR

// LDR pin connections (Adjust to your actual pins)
int ldrlt = A1;
int ldrrt = A0;
int ldrld = A3;
int ldrrd = A2;
const int numSamples = 10;
int ldrSamples[numSamples];
int sampleIndex = 0;
bool ledState = false; // false = OFF, true = ON

int getSmoothedLDR(int pin) {
  ldrSamples[sampleIndex] = analogRead(pin);
  sampleIndex = (sampleIndex + 1) % numSamples;
  
  int sum = 0;
  for (int i = 0; i < numSamples; i++) {
    sum += ldrSamples[i];
  }
  return sum / numSamples;
}
// // Potentiometer pins (Adjust to your actual pins, or comment out if not used)
// int delayPotPin = A4;
// int tolerancePotPin = A5;

// Tolerance for light difference to trigger movement
int tol = 40; // You can adjust this value

// Speed mapping limits (Adjust these based on your servos)
int minSpeed = 20;
int maxSpeed = 90;
int maxDifference = 500; // Maximum expected difference in LDR readings

void setup() {
  pinMode(ledPin, OUTPUT);
  
  Serial.begin(9600);
  Serial.println("360 Servo Solar Tracker Ready!");

  horizontal.attach(10); // Horizontal servo on pin 10 (Adjust if needed)
  vertical.attach(9);   // Vertical servo on pin 9 (Adjust if needed)
 
  horizontal.write(horizontalStop); // Stop horizontal servo
  vertical.write(verticalStop);     // Stop vertical servo

  delay(1000); // Give servos time to settle
}

void loop() {
  // Read all LDRs
  int lt = analogRead(ldrlt);
  int rt = analogRead(ldrrt);
  int ld = analogRead(ldrld);
  int rd = analogRead(ldrrd);

dayNightSamples[dayNightSampleIndex] = rd; // sample A2 directly
dayNightSampleIndex = (dayNightSampleIndex + 1) % 10;

int dayNightSum = 0;
for (int i = 0; i < 10; i++) {
  dayNightSum += dayNightSamples[i];
}
int dayNightValue = dayNightSum / 10;

if (dayNightValue >= 350) {
  ledState = true;   // Noć → LED ON
} else if (dayNightValue >= 150) {
  ledState = false;  // Dan → LED OFF
}

digitalWrite(ledPin, ledState ? HIGH : LOW);

if (ledState) {
  Serial.println("LED ON - Noć");
} else {
  Serial.println("LED OFF - Dan");
}


  // Compute average values
  int avt = (lt + rt) / 2;
  int avd = (ld + rd) / 2;
  int avl = (lt + ld) / 2;
  int avr = (rt + rd) / 2;

  int dvert = avt - avd;
  int dhoriz = avl - avr;

  Serial.print("LT: "); Serial.print(lt);
  Serial.print(" RT: "); Serial.print(rt);
  Serial.print(" LD: "); Serial.print(ld);
  Serial.print(" RD: "); Serial.println(rd);
  Serial.print("dV: "); Serial.print(dvert);
  Serial.print("dH: "); Serial.println(dhoriz);




  // Movement logic - only move when difference is meaningful
  if (abs(dvert) > tol) {
    verticalSpeed = map(abs(dvert), tol, maxDifference, minSpeed, maxSpeed);
    verticalSpeed = constrain(verticalSpeed, minSpeed, maxSpeed);
    if (avt > avd) {
      int val = verticalStop - verticalSpeed;
      if (reverseVertical) val = verticalStop + verticalSpeed;
      vertical.write(val);
      Serial.print(" V: UP ("); Serial.print(val); Serial.println(")");
    } else {
      int val = verticalStop + verticalSpeed;
      if (reverseVertical) val = verticalStop - verticalSpeed;
      vertical.write(val);
      Serial.print(" V: DOWN ("); Serial.print(val); Serial.println(")");
    }
  } else {
    vertical.write(verticalStop);
    Serial.println(" V: STOP");
  }

  if (abs(dhoriz) > tol) {
    horizontalSpeed = map(abs(dhoriz), tol, maxDifference, minSpeed, maxSpeed);
    horizontalSpeed = constrain(horizontalSpeed, minSpeed, maxSpeed);
    if (avl > avr) {
      int val = horizontalStop + horizontalSpeed;
      if (reverseHorizontal) val = horizontalStop - horizontalSpeed;
      horizontal.write(val);
      Serial.print(" H: LEFT ("); Serial.print(val); Serial.println(")");
    } else {
      int val = horizontalStop - horizontalSpeed;
      if (reverseHorizontal) val = horizontalStop + horizontalSpeed;
      horizontal.write(val);
      Serial.print(" H: RIGHT ("); Serial.print(val); Serial.println(")");
    }
  } else {
    horizontal.write(horizontalStop);
    Serial.println(" H: STOP");
  }

  delay(400); // Add delay to avoid jittery movement
}