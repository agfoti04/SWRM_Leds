#include <Wire.h>

// ===== I2C CONFIGURATION =====
#define I2C_SDA 21
#define I2C_SCL 22
#define MOTOR_DRIVER_ADDR 0x60
#define I2C_CLOCK_SPEED 100000

// ===== MOTOR CHANNELS =====
#define MOTOR1 0
#define MOTOR2 1
#define MOTOR3 2

// ===== REGISTER ADDRESSES (Adjust based on your driver) =====
// Command/Control Registers
#define MOTOR_SPEED_REG 0x01       // Register to set speed
#define MOTOR_DIR_REG 0x04         // Register to set direction

// Feedback/Read Registers
#define SPEED_REG_BASE 0x20        // Base address for speed feedback
#define DIR_REG_BASE 0x30          // Base address for direction feedback
#define COUNT_REG_BASE 0x40        // Base address for encoder count

// ===== ENCODER CONFIGURATION =====
#define TICKS_PER_ROTATION 360     // Pulses per full rotation
#define WHEEL_DIAMETER_CM 6.5      // Wheel diameter in cm
#define PI 3.14159265
#define TICKS_PER_CM (TICKS_PER_ROTATION / (PI * WHEEL_DIAMETER_CM))

// ===== DATA STRUCTURES =====
typedef struct {
  uint8_t speed;                   // Current speed (0-255)
  uint8_t direction;               // Current direction (0=back, 1=fwd)
  uint16_t encoderCount;           // Encoder tick count
  float distanceCM;                // Calculated distance in cm
  unsigned long lastUpdateTime;    // Last time data was read
} MotorData;

MotorData motors[3];

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Initialize I2C
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(I2C_CLOCK_SPEED);
  
  Serial.println("\n=== ESP32 Motor Driver with Encoder ===");
  Serial.println("I2C Initialized");
  Serial.printf("I2C Address: 0x%02X\n", MOTOR_DRIVER_ADDR);
  
  delay(500);
  
  // Stop all motors at startup
  stopAllMotors();
  
  // Reset all encoders
  resetAllEncoders();
  
  Serial.println("System Ready!\n");
}

// ===== MAIN LOOP =====
void loop() {
  // Test 1: Simple forward movement
  Serial.println("\n--- Test 1: All Motors Forward ---");
  moveForward(200);
  delay(3000);
  
  // Read and display data
  updateAllMotorData();
  printAllMotorData();
  
  stopAllMotors();
  delay(1000);
  
  // Test 2: Backward movement
  Serial.println("\n--- Test 2: All Motors Backward ---");
  moveBackward(150);
  delay(3000);
  
  updateAllMotorData();
  printAllMotorData();
  
  stopAllMotors();
  delay(1000);
  
  // Test 3: Individual motor control
  Serial.println("\n--- Test 3: Individual Motor Control ---");
  controlMotor(MOTOR1, 200);   // Forward at 200
  controlMotor(MOTOR2, -150);  // Backward at 150
  controlMotor(MOTOR3, 100);   // Forward at 100
  delay(3000);
  
  updateAllMotorData();
  printAllMotorData();
  
  stopAllMotors();
  delay(1000);
  
  // Test 4: Distance-based movement
  Serial.println("\n--- Test 4: Move Exact Distance (50cm) ---");
  resetAllEncoders();
  moveExactDistance(MOTOR1, 50);  // Move motor 1 exactly 50cm
  
  updateAllMotorData();
  printAllMotorData();
  
  delay(2000);
  
  // Test 5: Synchronized movement
  Serial.println("\n--- Test 5: Synchronized Motor Movement ---");
  resetAllEncoders();
  moveSynchronized(200, 300);  // All motors move until 300 ticks
  
  updateAllMotorData();
  printAllMotorData();
  
  delay(2000);
}

// ============================================
// ===== MOTOR CONTROL FUNCTIONS (WRITE) =====
// ============================================

/**
 * Control a single motor with signed speed
 * @param motorChannel: 0, 1, or 2
 * @param speed: -255 (full backward) to 255 (full forward)
 */
void controlMotor(uint8_t motorChannel, int8_t speed) {
  if (motorChannel > 2) {
    Serial.printf("ERROR: Invalid motor channel %d\n", motorChannel);
    return;
  }
  
  // Constrain speed
  speed = constrain(speed, -255, 255);
  
  // Determine direction and absolute speed
  uint8_t direction = (speed >= 0) ? 1 : 0;  // 1 = forward, 0 = backward
  uint8_t absSpeed = abs(speed);
  
  // Set direction
  Wire.beginTransmission(MOTOR_DRIVER_ADDR);
  Wire.write(MOTOR_DIR_REG);
  Wire.write(motorChannel);
  Wire.write(direction);
  uint8_t dirError = Wire.endTransmission();
  
  // Set speed
  Wire.beginTransmission(MOTOR_DRIVER_ADDR);
  Wire.write(MOTOR_SPEED_REG);
  Wire.write(motorChannel);
  Wire.write(absSpeed);
  uint8_t speedError = Wire.endTransmission();
  
  if (dirError == 0 && speedError == 0) {
    Serial.printf("Motor %d: Speed=%d, Dir=%s\n", motorChannel, absSpeed, direction ? "Forward" : "Backward");
  } else {
    Serial.printf("ERROR: Failed to control motor %d\n", motorChannel);
  }
}

/**
 * Stop a single motor
 */
void stopMotor(uint8_t motorChannel) {
  controlMotor(motorChannel, 0);
}

/**
 * Stop all motors
 */
void stopAllMotors() {
  for (int i = 0; i < 3; i++) {
    controlMotor(i, 0);
  }
  Serial.println("All motors stopped");
}

/**
 * Move all motors forward
 */
void moveForward(uint8_t speed) {
  for (int i = 0; i < 3; i++) {
    controlMotor(i, speed);
  }
  Serial.printf("Moving forward at speed %d\n", speed);
}

/**
 * Move all motors backward
 */
void moveBackward(uint8_t speed) {
  for (int i = 0; i < 3; i++) {
    controlMotor(i, -speed);
  }
  Serial.printf("Moving backward at speed %d\n", speed);
}

/**
 * Move a motor until it reaches target distance
 */
void moveExactDistance(uint8_t motorChannel, float targetDistanceCM) {
  uint16_t targetTicks = targetDistanceCM * TICKS_PER_CM;
  
  Serial.printf("Motor %d: Moving %.1f cm (target: %d ticks)\n", 
    motorChannel, targetDistanceCM, targetTicks);
  
  // Start motor
  controlMotor(motorChannel, 200);
  
  // Monitor until target reached
  while (true) {
    uint16_t currentCount = readEncoderCount(motorChannel);
    float currentDistance = currentCount / TICKS_PER_CM;
    
    Serial.printf("  Progress: %d/%d ticks (%.2f/%.1f cm)\n", 
      currentCount, targetTicks, currentDistance, targetDistanceCM);
    
    if (currentCount >= targetTicks) {
      stopMotor(motorChannel);
      Serial.printf("Motor %d reached target distance\n", motorChannel);
      break;
    }
    
    delay(100);
  }
}

/**
 * Move all motors synchronized until target ticks
 */
void moveSynchronized(uint8_t speed, uint16_t targetTicks) {
  Serial.printf("Synchronizing all motors to %d ticks at speed %d\n", targetTicks, speed);
  
  // Reset all encoders
  resetAllEncoders();
  
  // Start all motors
  for (int i = 0; i < 3; i++) {
    controlMotor(i, speed);
  }
  
  // Monitor until all reach target
  while (true) {
    uint16_t count1 = readEncoderCount(MOTOR1);
    uint16_t count2 = readEncoderCount(MOTOR2);
    uint16_t count3 = readEncoderCount(MOTOR3);
    
    Serial.printf("  M1: %d, M2: %d, M3: %d / Target: %d\n", 
      count1, count2, count3, targetTicks);
    
    // Stop each motor when it reaches target
    if (count1 >= targetTicks) stopMotor(MOTOR1);
    if (count2 >= targetTicks) stopMotor(MOTOR2);
    if (count3 >= targetTicks) stopMotor(MOTOR3);
    
    // Check if all are done
    if (count1 >= targetTicks && count2 >= targetTicks && count3 >= targetTicks) {
      Serial.println("All motors synchronized and stopped");
      break;
    }
    
    delay(50);
  }
}

// ============================================
// ===== ENCODER READ FUNCTIONS (READ) =====
// ============================================

/**
 * Read current speed from motor driver (0-255)
 */
uint8_t readMotorSpeed(uint8_t motorChannel) {
  if (motorChannel > 2) {
    Serial.printf("ERROR: Invalid motor channel %d\n", motorChannel);
    return 0;
  }
  
  uint8_t registerAddress = SPEED_REG_BASE + motorChannel;
  
  Wire.beginTransmission(MOTOR_DRIVER_ADDR);
  Wire.write(registerAddress);
  if (Wire.endTransmission() != 0) {
    Serial.printf("ERROR: Failed to request speed from motor %d\n", motorChannel);
    return 0;
  }
  
  Wire.requestFrom(MOTOR_DRIVER_ADDR, 1);
  
  if (Wire.available()) {
    uint8_t speed = Wire.read();
    return speed;
  }
  
  return 0;
}

/**
 * Read current direction from motor driver (0 or 1)
 */
uint8_t readMotorDirection(uint8_t motorChannel) {
  if (motorChannel > 2) {
    Serial.printf("ERROR: Invalid motor channel %d\n", motorChannel);
    return 0;
  }
  
  uint8_t registerAddress = DIR_REG_BASE + motorChannel;
  
  Wire.beginTransmission(MOTOR_DRIVER_ADDR);
  Wire.write(registerAddress);
  if (Wire.endTransmission() != 0) {
    Serial.printf("ERROR: Failed to request direction from motor %d\n", motorChannel);
    return 0;
  }
  
  Wire.requestFrom(MOTOR_DRIVER_ADDR, 1);
  
  if (Wire.available()) {
    uint8_t direction = Wire.read();
    return direction;
  }
  
  return 0;
}

/**
 * Read encoder count (total ticks)
 */
uint16_t readEncoderCount(uint8_t motorChannel) {
  if (motorChannel > 2) {
    Serial.printf("ERROR: Invalid motor channel %d\n", motorChannel);
    return 0;
  }
  
  uint8_t registerAddress = COUNT_REG_BASE + motorChannel;
  
  Wire.beginTransmission(MOTOR_DRIVER_ADDR);
  Wire.write(registerAddress);
  if (Wire.endTransmission() != 0) {
    Serial.printf("ERROR: Failed to request encoder count from motor %d\n", motorChannel);
    return 0;
  }
  
  Wire.requestFrom(MOTOR_DRIVER_ADDR, 2);  // Request 2 bytes
  
  if (Wire.available() >= 2) {
    uint8_t highByte = Wire.read();
    uint8_t lowByte = Wire.read();
    uint16_t count = (highByte << 8) | lowByte;
    return count;
  }
  
  return 0;
}

/**
 * Calculate distance traveled from encoder count
 */
float calculateDistance(uint16_t encoderCount) {
  return encoderCount / TICKS_PER_CM;
}

/**
 * Reset encoder count to 0
 */
void resetEncoder(uint8_t motorChannel) {
  if (motorChannel > 2) {
    Serial.printf("ERROR: Invalid motor channel %d\n", motorChannel);
    return;
  }
  
  Wire.beginTransmission(MOTOR_DRIVER_ADDR);
  Wire.write(0x50);  // Reset command (adjust if needed)
  Wire.write(motorChannel);
  if (Wire.endTransmission() == 0) {
    Serial.printf("Encoder %d reset\n", motorChannel);
  } else {
    Serial.printf("ERROR: Failed to reset encoder %d\n", motorChannel);
  }
}

/**
 * Reset all encoders
 */
void resetAllEncoders() {
  for (int i = 0; i < 3; i++) {
    resetEncoder(i);
  }
}

// ============================================
// ===== DATA UPDATE & DISPLAY FUNCTIONS =====
// ============================================

/**
 * Update data for all motors
 */
void updateAllMotorData() {
  for (int i = 0; i < 3; i++) {
    updateMotorData(i);
  }
}

/**
 * Update data for a single motor
 */
void updateMotorData(uint8_t motorChannel) {
  motors[motorChannel].speed = readMotorSpeed(motorChannel);
  motors[motorChannel].direction = readMotorDirection(motorChannel);
  motors[motorChannel].encoderCount = readEncoderCount(motorChannel);
  motors[motorChannel].distanceCM = calculateDistance(motors[motorChannel].encoderCount);
  motors[motorChannel].lastUpdateTime = millis();
}

/**
 * Print data for all motors
 */
void printAllMotorData() {
  Serial.println("\n=== Motor Data ===");
  for (int i = 0; i < 3; i++) {
    printMotorData(i);
  }
}

/**
 * Print data for a single motor
 */
void printMotorData(uint8_t motorChannel) {
  Serial.printf("Motor %d:\n", motorChannel + 1);
  Serial.printf("  Speed: %d/255\n", motors[motorChannel].speed);
  Serial.printf("  Direction: %s\n", motors[motorChannel].direction ? "Forward" : "Backward");
  Serial.printf("  Encoder Count: %d ticks\n", motors[motorChannel].encoderCount);
  Serial.printf("  Distance: %.2f cm\n", motors[motorChannel].distanceCM);
  Serial.printf("  Last Update: %lu ms ago\n\n", millis() - motors[motorChannel].lastUpdateTime);
}

/**
 * Print single motor status in one line
 */
void printMotorStatus(uint8_t motorChannel) {
  Serial.printf("M%d: Spd=%d Dir=%s Cnt=%d Dist=%.1f\n",
    motorChannel + 1,
    motors[motorChannel].speed,
    motors[motorChannel].direction ? "F" : "B",
    motors[motorChannel].encoderCount,
    motors[motorChannel].distanceCM
  );
}

/**
 * Monitor motor slip/stall detection
 */
void detectMotorSlip(uint8_t motorChannel) {
  uint16_t count1 = readEncoderCount(motorChannel);
  delay(500);
  uint16_t count2 = readEncoderCount(motorChannel);
  
  uint16_t ticksPerSecond = (count2 - count1) * 2;  // ×2 because 500ms = 0.5s
  
  if (readMotorSpeed(motorChannel) > 0 && ticksPerSecond == 0) {
    Serial.printf("WARNING: Motor %d is stalled or slipping!\n", motorChannel);
  } else {
    Serial.printf("Motor %d: %d ticks/sec\n", motorChannel, ticksPerSecond);
  }
}

// ============================================
// ===== OPTIONAL: ADVANCED FUNCTIONS =====
// ============================================

/**
 * Smooth ramp speed up
 */
void rampMotor(uint8_t motorChannel, uint8_t targetSpeed, uint16_t rampTimeMS) {
  uint8_t steps = 20;
  uint16_t stepTime = rampTimeMS / steps;
  int8_t speedStep = (int8_t)targetSpeed / steps;
  
  Serial.printf("Ramping motor %d to %d over %d ms\n", motorChannel, targetSpeed, rampTimeMS);
  
  for (int i = 0; i < steps; i++) {
    int8_t currentSpeed = speedStep * (i + 1);
    controlMotor(motorChannel, currentSpeed);
    delay(stepTime);
  }
}

/**
 * Check if motor reached target speed
 */
boolean motorAtTargetSpeed(uint8_t motorChannel, uint8_t targetSpeed, uint8_t tolerance) {
  uint8_t currentSpeed = readMotorSpeed(motorChannel);
  return (currentSpeed >= (targetSpeed - tolerance)) && 
         (currentSpeed <= (targetSpeed + tolerance));
}

/**
 * Get average distance traveled by all motors
 */
float getAverageDistance() {
  float total = 0;
  for (int i = 0; i < 3; i++) {
    total += motors[i].distanceCM;
  }
  return total / 3.0;
}

/**
 * Emergency stop all motors
 */
void emergencyStop() {
  Serial.println("\n!!! EMERGENCY STOP !!!");
  stopAllMotors();
  
  // Optional: Add buzzer, LED, etc.
}