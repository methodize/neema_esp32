/*
 * Neema ESP32 - Potentiometer Controlled Stepper Motor for Blinds Tilt Control
 *
 * This sketch reads a potentiometer value and controls a stepper motor's position
 * to open/close window blind slats via the tilt wand mechanism.
 * The potentiometer position is mapped to the stepper motor's angular position.
 *
 * TESTED HARDWARE CONFIGURATION:
 * - Arduino Nano ESP32-S3
 * - Stepperonline 17HS19-2004S1 NEMA 17 stepper (2.0A, 59Ncm torque)
 * - Pololu DRV8825 stepper driver
 * - 10K potentiometer
 * - 12V battery power
 *
 * IMPORTANT - ESP32-S3 uses 3.3V logic (NOT 5V!)
 * - Connect potentiometer to 3.3V (not 5V)
 * - Connect DRV8825 VDD to 3.3V (not 5V)
 * - Connect M0, M1, M2 to 3.3V (not 5V)
 *
 * IMPORTANT - DRV8825 Current Limiting:
 * Must set current limit to ~1.4A (70% of motor's 2.0A rating)
 * Formula: Vref = Current Limit / 2
 * Target Vref = 1.4A / 2 = 0.7V
 * Adjust trimpot on DRV8825 while measuring Vref pin voltage
 *
 * Wiring:
 * - Potentiometer middle pin to A0 (GPIO1)
 * - Potentiometer outer pins to 3.3V and GND (NOT 5V!)
 * - DRV8825 STEP pin to GPIO5
 * - DRV8825 DIR pin to GPIO6
 * - DRV8825 ENABLE pin to GPIO7
 * - DRV8825 VDD to 3.3V (logic power)
 * - DRV8825 M0, M1, M2 to 3.3V for 1/32 microstepping
 * - DRV8825 VMOT to 12V battery positive
 * - DRV8825 GND to battery negative AND ESP32 GND (common ground!)
 * - Connect motor coils to DRV8825 A1, A2, B1, B2
 * - ESP32 VIN to 12V or USB-C for power
 */

#include <AccelStepper.h>

// Pin definitions (ESP32-S3 safe pins)
const int POT_PIN = A0;           // Potentiometer analog input (GPIO1)
const int STEP_PIN = 5;           // Step pulse pin (GPIO5)
const int DIR_PIN = 6;            // Direction pin (GPIO6)
const int ENABLE_PIN = 7;         // Enable pin (GPIO7)

// Stepper motor configuration
const int STEPS_PER_REVOLUTION = 200;  // 17HS19-2004S1: 1.8° per step = 200 steps/rev
const int MICROSTEPS = 32;             // DRV8825 with M0,M1,M2 HIGH = 1/32 microstepping
const int TOTAL_STEPS = STEPS_PER_REVOLUTION * MICROSTEPS;  // 6400 steps per revolution

// Motion parameters
const int MAX_SPEED = 2000;        // Maximum steps per second (adjust for smoothness)
const int ACCELERATION = 1000;     // Steps per second^2 (adjust for responsiveness)

// Blinds-specific configuration
// Adjust ROTATION_RANGE to limit travel (180 = half rotation for typical blinds)
// Set to TOTAL_STEPS for full 360° rotation if needed
const int ROTATION_RANGE = TOTAL_STEPS / 2;  // 180° range for blinds tilt

// Potentiometer reading configuration
// ESP32-S3 has 12-bit ADC (0-4095 range) vs 10-bit (0-1023) on Arduino
const int POT_DEADBAND = 10;       // Analog read tolerance to reduce jitter (higher for 12-bit ADC)
const int POT_MIN = 0;             // Minimum pot value
const int POT_MAX = 4095;          // Maximum pot value (ESP32 12-bit ADC)

// Initialize stepper with DRIVER interface (step, direction pins)
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// Variables
int lastPotValue = -1;
int currentPotValue = 0;
long targetPosition = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);  // Give ESP32 time to initialize serial

  Serial.println("Neema ESP32 - Potentiometer Stepper Controller");
  Serial.println("==============================================");

  // Configure enable pin if used
  if (ENABLE_PIN >= 0) {
    pinMode(ENABLE_PIN, OUTPUT);
    digitalWrite(ENABLE_PIN, LOW);  // Enable driver (active LOW for most drivers)
  }

  // Configure stepper motor
  stepper.setMaxSpeed(MAX_SPEED);
  stepper.setAcceleration(ACCELERATION);
  stepper.setCurrentPosition(0);  // Set current position as zero point

  Serial.println("Setup complete!");
  Serial.print("Board: Arduino Nano ESP32-S3 (3.3V logic)\n");
  Serial.print("ADC Resolution: 12-bit (0-4095)\n");
  Serial.print("Steps per revolution: ");
  Serial.println(TOTAL_STEPS);
  Serial.print("Rotation range: ");
  Serial.print(ROTATION_RANGE);
  Serial.print(" steps (");
  Serial.print((ROTATION_RANGE * 360) / TOTAL_STEPS);
  Serial.println(" degrees)");
  Serial.print("Max speed: ");
  Serial.println(MAX_SPEED);
  Serial.print("Acceleration: ");
  Serial.println(ACCELERATION);
  Serial.println();
}

void loop() {
  // Read potentiometer value (ESP32 has 12-bit ADC: 0-4095)
  currentPotValue = analogRead(POT_PIN);

  // Only update if the value has changed beyond the deadband
  if (abs(currentPotValue - lastPotValue) > POT_DEADBAND) {
    lastPotValue = currentPotValue;

    // Map potentiometer value to stepper position
    // Map full pot range to configured rotation range (default 180° for blinds)
    targetPosition = map(currentPotValue, POT_MIN, POT_MAX, 0, ROTATION_RANGE);

    // Set new target position
    stepper.moveTo(targetPosition);

    // Print debug information
    Serial.print("Pot: ");
    Serial.print(currentPotValue);
    Serial.print(" | Target: ");
    Serial.print(targetPosition);
    Serial.print(" | Current: ");
    Serial.println(stepper.currentPosition());
  }

  // Run the stepper motor (this must be called frequently)
  stepper.run();
}
