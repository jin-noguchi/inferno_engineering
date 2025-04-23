/*
  Pin assignments:
   - Laser Diode (980MD-30-0813) on DIO3
   - IR Receiver (TSOP13238) on DIO2
   - LED on DIO11
*/
 
// -------------------------
// 1) Pin Definitions
// -------------------------
const int LASER_PIN  = 4;
const int IR_PIN     = 7;
const int LED_PIN    = 8;
const int BUTTON_PIN = 9;
 
// -------------------------
// 2) Timing Settings (ms)
// -------------------------
const unsigned long LASER_PERIOD     = 40; // Pulse the laser every 1000 ms
const unsigned long LASER_ON_TIME    = 30;   // Laser on for 30 ms
const unsigned long LED_ON_TIME      = 10;  // LED stays lit for 500 ms when beam is detected
 
// -------------------------
// 3) Variables for Timing
// -------------------------
unsigned long lastLaserTriggerTime = 0; // When we last triggered the laser
bool laserIsOn = false;                 // State of the laser (on/off)
unsigned long laserTurnOnTime = 0;      // Timestamp when the laser was turned on
 
unsigned long ledTurnOnTime = 0;        // Timestamp when the LED was turned on
bool ledIsOn = false;                   // State of the LED (on/off)
 
void setup() {
  // Configure pins
  pinMode(LASER_PIN, OUTPUT);
  pinMode(IR_PIN, INPUT);    // TSOP13238 output pin
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Use internal pull-up resistor for the button
 
  // Initialize them off
  digitalWrite(LASER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
 
  // (Optional) Begin serial for debug
  Serial.begin(115200);
}

void loop() {
  unsigned long currentTime = millis();
  bool buttonState = digitalRead(BUTTON_PIN) == LOW;
  if (buttonState != laserIsOn) {
    if (buttonState) {
      Serial.println("Laser ON");
      tone(LASER_PIN, 38000);
      //digitalWrite(LASER_PIN, currentTime & 1 ? HIGH : LOW);
      laserIsOn = true;
    } else {
      Serial.println("Laser OFF");
      digitalWrite(LASER_PIN, LOW);
      laserIsOn = false;
    }
  }
 
  // ----------------------------------------------------
  //  B) Check if the IR receiver detects the beam
  // ----------------------------------------------------
  // The TSOP13238 typically goes LOW when it detects
  // a valid 38 kHz IR signal. For a continuous laser,
  // this might never read as "detection" unless modulated
  // at 38 kHz. But let's assume it goes LOW upon detection.
  int irState = digitalRead(IR_PIN);
  // LOW = signal detected (typical for TSOP), HIGH = no signal
 
  if (irState == LOW) {
    // Turn on LED for 500 ms
    ledIsOn = true;
    ledTurnOnTime = currentTime;
    digitalWrite(LED_PIN, HIGH);
    Serial.println("Beam detected! LED on.");
  }
 
  // ----------------------------------------------------
  //  C) Turn off the LED after 500 ms
  // ----------------------------------------------------
  if (ledIsOn && (currentTime - ledTurnOnTime >= LED_ON_TIME)) {
    ledIsOn = false;
    digitalWrite(LED_PIN, LOW);
    Serial.println("LED off.");
  }
 
  // That's it for the main loop
  //delay(1);
}