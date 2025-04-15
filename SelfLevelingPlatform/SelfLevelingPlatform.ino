#include <Wire.h>
#include "MPU6050_tockn.h"
#include "PID.h"

// Actuator Control Pins
#define ACT1_EXTEND 9  // Actuator 1 Extend (PWM)
#define ACT1_RETRACT 8 // Actuator 1 Retract (PWM)
#define ACT2_EXTEND 7  // Actuator 2 Extend (PWM)
#define ACT2_RETRACT 6 // Actuator 2 Retract (PWM)

// IMU Setup
MPU6050 mpu6050(Wire);

// PID Controllers (Pitch & Roll)
// If Too Sluggish - Increase Kp by 0.2 increments
// If Overshoots - Increase Kd by 0.1 increments
// If Steady Error - Increase Ki by 0.01 increments

PIDConfig pitchConfig(1.0f, 0.1f, 0.2f, 12.0f);  // kp, ki, kd
PID pitchPID(pitchConfig);

PIDConfig rollConfig(1.0f, 0.1f, 0.2f, 12.0f);  // kp, ki, kd
PID rollPID(rollConfig);


void setup() {
  Serial.begin(9600);
  Wire.begin();
  
  // Initialize MPU6050
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true); // Calibrate gyro (keep IMU flat!)
  
  // Set actuator control pins
  pinMode(ACT1_EXTEND, OUTPUT);
  pinMode(ACT1_RETRACT, OUTPUT);
  pinMode(ACT2_EXTEND, OUTPUT);
  pinMode(ACT2_RETRACT, OUTPUT);
  
  Serial.println("System Ready");
}

void loop() {
  mpu6050.update();
  
  // Get filtered angles from IMU (in degrees)
  float pitch = mpu6050.getAngleX(); // Platform front/back tilt
  float roll = mpu6050.getAngleY();  // Platform left/right tilt
  
  // Compute PID adjustments
  float pitchAdjustment = pitchPID.compute(-pitch); // Invert if needed
  float rollAdjustment = rollPID.compute(roll);
  
  // Apply to actuators
  controlActuator(ACT1_EXTEND, ACT1_RETRACT, rollAdjustment);  // Actuator 1 controls roll
  controlActuator(ACT2_EXTEND, ACT2_RETRACT, pitchAdjustment); // Actuator 2 controls pitch
  
  // Debug output
  Serial.print("Pitch: "); Serial.print(pitch);
  Serial.print(" | Roll: "); Serial.print(roll);
  Serial.print(" | PID Out: "); Serial.print(pitchAdjustment); 
  Serial.print(", "); Serial.println(rollAdjustment);
  
  delay(20); // ~50Hz control loop
}

// Controls a single actuator based on PID output
void controlActuator(int extendPin, int retractPin, float pidOutput) {
  if (pidOutput > 0.5) {
    analogWrite(extendPin, map(pidOutput, 0, 12, 0, 255)); // Extend (PWM)
    digitalWrite(retractPin, LOW);
  } 
  else if (pidOutput < -0.5) {
    digitalWrite(extendPin, LOW);
    analogWrite(retractPin, map(-pidOutput, 0, 12, 0, 255)); // Retract (PWM)
  } 
  else {
    digitalWrite(extendPin, LOW); // Stop
    digitalWrite(retractPin, LOW);
  }
}