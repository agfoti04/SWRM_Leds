/**
 * Kiwi Drive - Hiwonder 4-Channel Encoder Motor Driver
 *
 * Hardware changes from original:
 *   - Motor control is now I2C-only (no direct PWM/direction GPIO pins).
 *   - The Hiwonder board (SA8870C) lives at I2C address 0x34.
 *   - Speed is sent as a block of 4 signed bytes to register 0x33 (51).
 *   - Encoder A/B lines still wire directly to ESP32 GPIO for counting.
 *
 * Wiring (I2C):
 *   ESP32 GPIO 21 (SDA) --> Board SDA
 *   ESP32 GPIO 22 (SCL) --> Board SCL
 *   Common GND
 *
 * Only 3 of the 4 motor channels are used (kiwi drive).
 * Channel mapping: M1 = Motor A, M2 = Motor B, M3 = Motor C, M4 unused (speed = 0)
 *
 * Motor: Adafruit 4638 — N20 6V 1:50 magnetic encoder motor
 *   Encoder resolution : 14 counts/rev (pre-gear) × 50 = 700 counts/output-rev
 *   Wire colours       : Red/White → M+/M−  |  Blue → GND  |  Black → VCC (3-5V)
 *                        Yellow → Encoder A  |  Green → Encoder B
 *
 * NOTE: If a motor spins the wrong direction, negate that channel in
 *       setMotorSpeeds(), or flip MOTOR_ENCODER_POLARITY and re-test.
 */

#include <Arduino.h>
#include <Wire.h>
#include <math.h>

// ── I2C ──────────────────────────────────────────────────────────────────────
#define HIWONDER_I2C_ADDR           0x34

// Register addresses (from Hiwonder's published Arduino tutorial)
#define MOTOR_TYPE_ADDR             20   // 0x14
#define MOTOR_ENCODER_POLARITY_ADDR 21   // 0x15
#define MOTOR_FIXED_SPEED_ADDR      51   // 0x33  – 4 signed bytes, one per channel

// Motor type constants (select the one matching your hardware)
#define MOTOR_TYPE_TT               0    // TT plastic-shaft gear motor
#define MOTOR_TYPE_N20              1    // N20 micro gear motor
#define MOTOR_TYPE_JGB              2    // JGB37 / 520 / 310 DC gear motor

// Adafruit 4638 is an N20 motor
#define MOTOR_TYPE                  MOTOR_TYPE_N20
#define MOTOR_ENCODER_POLARITY      0    // 0 = default, 1 = reversed

// Encoder resolution for Adafruit 4638 (N20 6V 1:50)
// 14 raw counts/rev (pre-gear) × 50 gear ratio = 700 counts per output shaft revolution
// Use ENCODER_CPR_4X if decoding both edges of both channels (quadrature 4x)
#define ENCODER_CPR                 700   // counts/output-rev  (2x decoding, one channel interrupt)
#define ENCODER_CPR_4X              1400  // counts/output-rev  (4x decoding, both channel interrupts)

// I2C pins (ESP32 defaults; change if your board uses different ones)
#define I2C_SDA_PIN                 21
#define I2C_SCL_PIN                 22

// ── Encoder GPIO ──────────────────────────────────────────────────────────────
// These still connect directly to ESP32 GPIO — the Hiwonder board passes encoder
// signals through its VCC/GND/A/B header per channel.
const int ENC_A1 = 35;
const int ENC_B1 = 34;

const int ENC_A2 = 32;
const int ENC_B2 = 33;

const int ENC_A3 = 36;
const int ENC_B3 = 39;

volatile long encoder1 = 0;
volatile long encoder2 = 0;
volatile long encoder3 = 0;

// ── Encoder ISRs ─────────────────────────────────────────────────────────────
void IRAM_ATTR handleEnc1() {
    if (digitalRead(ENC_A1) == digitalRead(ENC_B1)) encoder1++;
    else encoder1--;
}

void IRAM_ATTR handleEnc2() {
    if (digitalRead(ENC_A2) == digitalRead(ENC_B2)) encoder2++;
    else encoder2--;
}

void IRAM_ATTR handleEnc3() {
    if (digitalRead(ENC_A3) == digitalRead(ENC_B3)) encoder3++;
    else encoder3--;
}

// ── I2C helpers ──────────────────────────────────────────────────────────────

/**
 * Write an arbitrary byte array to a register on the Hiwonder driver.
 */
static void wireWriteBlock(uint8_t reg, const uint8_t *data, uint8_t len)
{
    Wire.beginTransmission(HIWONDER_I2C_ADDR);
    Wire.write(reg);
    for (uint8_t i = 0; i < len; i++) Wire.write(data[i]);
    Wire.endTransmission();
}

// ── Motor control ─────────────────────────────────────────────────────────────

/**
 * Send speeds to all four channels at once.
 * Values are signed, range roughly -100 to +100 (the board does its own PWM).
 * Channel 4 (m4) is unused in kiwi drive; pass 0.
 *
 * Sign convention: positive = "forward" as mounted.
 * If a motor spins the wrong way, negate that channel here, or flip
 * MOTOR_ENCODER_POLARITY and re-test.
 */
static void setMotorSpeeds(int8_t m1, int8_t m2, int8_t m3, int8_t m4 = 0)
{
    uint8_t payload[4] = {
        (uint8_t)m1,
        (uint8_t)m2,
        (uint8_t)m3,
        (uint8_t)m4
    };
    wireWriteBlock(MOTOR_FIXED_SPEED_ADDR, payload, 4);
}

// ── Kinematics ────────────────────────────────────────────────────────────────

/**
 * Kiwi drive inverse kinematics.
 * x, y, w are normalised inputs in [-1, 1].
 * Outputs s1..s3 are in [-1, 1].
 */
static void computeSpeeds(float x, float y, float w,
                           float &s1, float &s2, float &s3)
{
    s1 = (-0.33f * x) + ( 0.58f * y) + (0.33f * w);
    s2 = (-0.33f * x) + (-0.58f * y) + (0.33f * w);
    s3 = ( 0.67f * x) + ( 0.00f * y) + (0.33f * w);
}

// FIX - not actual normalization
static void normalize(float &s1, float &s2, float &s3)
{
    float maxVal = max(fabsf(s1), max(fabsf(s2), fabsf(s3)));
    if (maxVal > 1.0f) {
        s1 /= maxVal;
        s2 /= maxVal;
        s3 /= maxVal;
    }
    // (no need to check for ==0, as division by 0 only if all zeros, which can't exceed 1.0f)
}

// ── Public drive interface ────────────────────────────────────────────────────

/**
 * Drive the robot.
 * x, y, w : normalised velocity components in [-1, 1].
 * The Hiwonder driver accepts signed speeds up to ±100 from host.
 */
void driveRobot(float x, float y, float w)
{
    float s1, s2, s3;
    computeSpeeds(x, y, w, s1, s2, s3);
    normalize(s1, s2, s3);

    // Scale to the driver's [-100, 100] speed range
    int8_t m1 = (int8_t)(s1 * 100.0f);
    int8_t m2 = (int8_t)(s2 * 100.0f);
    int8_t m3 = (int8_t)(s3 * 100.0f);

    setMotorSpeeds(m1, m2, m3, 0);
}

void eStopRobot()
{
    setMotorSpeeds(0, 0, 0, 0);
}

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup()
{
    Serial.begin(115200);

    // I2C init
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(100000);   // 100 kHz — conservative; bump to 400000 if needed
    delay(100);              // let the driver board boot

    // Tell the driver what motor type is connected
    uint8_t motorType = MOTOR_TYPE;
    wireWriteBlock(MOTOR_TYPE_ADDR, &motorType, 1);
    delay(10);

    uint8_t encPolarity = MOTOR_ENCODER_POLARITY;
    wireWriteBlock(MOTOR_ENCODER_POLARITY_ADDR, &encPolarity, 1);
    delay(10);

    // Encoder pins — still direct GPIO
    pinMode(ENC_A1, INPUT_PULLUP); pinMode(ENC_B1, INPUT_PULLUP);
    pinMode(ENC_A2, INPUT_PULLUP); pinMode(ENC_B2, INPUT_PULLUP);
    pinMode(ENC_A3, INPUT_PULLUP); pinMode(ENC_B3, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(ENC_A1), handleEnc1, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENC_A2), handleEnc2, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENC_A3), handleEnc3, CHANGE);

    Serial.println("Kiwi Drive (Hiwonder I2C) Ready");
}

// ── Loop ──────────────────────────────────────────────────────────────────────
void loop()
{
    Serial.printf("Enc1: %ld | Enc2: %ld | Enc3: %ld\n",
                  encoder1, encoder2, encoder3);

    // Forward
    driveRobot(0, 1, 0);
    delay(2000);

    // Backward
    driveRobot(0, -1, 0);
    delay(2000);

    // Turn Left
    driveRobot(0, 0, 1);
    delay(2000);

    // Forward
    driveRobot(0, 1, 0);
    delay(2000);

    // Turn Right
    driveRobot(0, 0, -1);
    delay(2000);

    // Forward
    driveRobot(0, 1, 0);
    delay(2000);

    // Stop
    eStopRobot();
    delay(2000);
}
