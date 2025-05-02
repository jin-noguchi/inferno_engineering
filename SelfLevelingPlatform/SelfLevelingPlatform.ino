#include <Wire.h>
#include <MPU6050_light.h>
#include "PID.h"

// Actuator Control Pins
#define ACT1_EXTEND 30  // Actuator 1 Extend (PWM)
#define ACT1_RETRACT 28 // Actuator 1 Retract (PWM)
#define ACT1_CON 3 // Actuator 1 Retract (PWM)

#define ACT2_EXTEND 32  // Actuator 2 Extend (PWM)
#define ACT2_RETRACT 34 // Actuator 2 Retract (PWM)
#define ACT2_CON 2 // Actuator 1 Retract (PWM)


// IMU Setup
MPU6050 mpu(Wire);
unsigned long timer = 0;

// PID Controllers (Pitch & Roll)
// If Too Sluggish - Increase Kp by 0.2 increments
// If Overshoots - Increase Kd by 0.1 increments
// If Steady Error - Increase Ki by 0.01 increments

PIDConfig pitchConfig(60.0f, 2.f, 0.2f, 128.0f);  // kp, ki, kd
PID pitchPID(pitchConfig);

PIDConfig rollConfig(60.f, 2.f, 0.2f, 128.0f);  // kp, ki, kd
PID rollPID(rollConfig);


void setup() {
  Serial.begin(9600);
  Wire.begin();
  
  // Initialize MPU6050
  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while (status != 0) { } // stop everything if could not connect to MPU6050
  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  mpu.calcOffsets(); // gyro and accelero
  Serial.println("Done!n");
  
  // Set actuator control pins
  pinMode(ACT1_EXTEND, OUTPUT);
  pinMode(ACT1_RETRACT, OUTPUT);
  pinMode(ACT2_EXTEND, OUTPUT);
  pinMode(ACT2_RETRACT, OUTPUT);
  
  Serial.println("System Ready");
}

void loop() {
  mpu.update();
    float pitch = mpu.getAngleX();
    // Serial.print("X : ");
    // Serial.print(mpu.getAngleX());
    float roll = mpu.getAngleY();
    // Serial.print(" Y : ");
    // Serial.println(mpu.getAngleY());
    timer = millis();
  
  // Compute PID adjustments
  float pitchAdjustment = pitchPID.compute(pitch); // Invert if needed
  float rollAdjustment = rollPID.compute(-roll);
  
  // Apply to actuators
  controlActuator(ACT1_EXTEND, ACT1_RETRACT, ACT1_CON, pitchAdjustment - rollAdjustment);  // Actuator 1 controls roll
  controlActuator(ACT2_EXTEND, ACT2_RETRACT, ACT2_CON, pitchAdjustment + rollAdjustment); // Actuator 2 controls pitch
  
  // Debug output
  Serial.print("Pitch: "); Serial.print(pitch);
  Serial.print(" | Roll: "); Serial.print(roll);
  Serial.print(" | PID Out: "); Serial.print(pitchAdjustment); 
  Serial.print(", "); Serial.println(rollAdjustment);
  
  delay(20); // ~50Hz control loop
}

// Controls a single actuator based on PID output
void controlActuator(int extendPin, int retractPin, int conPin, float pidOutput) {
  if (pidOutput > 0) {
    analogWrite(conPin, min(fabsf(pidOutput), 255.f)); // Extend (PWM)
    digitalWrite(extendPin, HIGH);
    digitalWrite(retractPin, LOW);
  } 
  else if (pidOutput < 0) {
    digitalWrite(extendPin, LOW);
    digitalWrite(retractPin, HIGH);
    analogWrite(conPin, min(fabsf(pidOutput), 255.f)); // Extend (PWM)
  } 
  else {
    digitalWrite(extendPin, LOW); // Stop
    digitalWrite(retractPin, LOW);
  }
}