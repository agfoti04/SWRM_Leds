#include <Arduino.h>
#include <Wire.h>
#include <math.h>

// ── I2C ──────────────────────────────────────────────────────────────────────
#define HIWONDER_I2C_ADDR           0x34

// Register addresses (from Hiwonder's published Arduino tutorial)
#define MOTOR_TYPE_ADDR             20   // 0x14
#define MOTOR_ENCODER_POLARITY_ADDR 21   // 0x15
#define MOTOR_FIXED_SPEED_ADDR      51   // 0x33  – 4 signed bytes, one per channel


// Encoder register addresses (Hiwonder I2C driver)
#define ENCODER1_ADDR               60   // 0x3C  – 4 bytes, signed int32, little-endian
#define ENCODER2_ADDR               64   // 0x40
#define ENCODER3_ADDR               68   // 0x44

// Motor type constants (select the one matching your hardware)
#define MOTOR_TYPE_TT               0    // TT plastic-shaft gear motor
#define MOTOR_TYPE_N20              1    // N20 micro gear motor
#define MOTOR_TYPE_JGB              2    // JGB37 / 520 / 310 DC gear motor

// Adafruit 4638 is an N20 motor
#define MOTOR_TYPE                  MOTOR_TYPE_N20
#define MOTOR_ENCODER_POLARITY      1    // 0 = default, 1 = reversed

// Encoder resolution for Adafruit 4638 (N20 6V 1:50)
// 14 raw counts/rev (pre-gear) × 50 gear ratio = 700 counts per output shaft revolution
#define ENCODER_CPR                 700   // counts/output-rev  (2x decoding)
#define ENCODER_CPR_4X              1400  // counts/output-rev  (4x decoding)

// I2C pins (ESP32 defaults; change if your board uses different ones)
#define I2C_SDA_PIN                 21
#define I2C_SCL_PIN                 22

// ── I2C helpers ───────────────────────────────────────────────────────────────

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
//sets zeros to be written to encoder registers. 
static void resetEncoder(uint8_t reg)
{
    uint8_t zeros[4] = {0, 0, 0, 0};
    wireWriteBlock(reg, zeros, 4);
}

void resetAllEncoders()
{
    resetEncoder(ENCODER1_ADDR);
    resetEncoder(ENCODER2_ADDR);
    resetEncoder(ENCODER3_ADDR);
}

/**
 * Read an arbitrary byte array from a register on the Hiwonder driver.
 * Returns true on success.
 */

static bool wireReadBlock(uint8_t reg, uint8_t *buf, uint8_t len)
{
    Wire.beginTransmission(HIWONDER_I2C_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;  // false = repeated start

    Wire.requestFrom((uint8_t)HIWONDER_I2C_ADDR, len);
    for (uint8_t i = 0; i < len && Wire.available(); i++)
        buf[i] = Wire.read();
    return true;
}

// ── Encoder reading ───────────────────────────────────────────────────────────

/**
 * Read a signed 32-bit encoder count from the given register address.
 * The Hiwonder board stores counts as little-endian int32.
 */
//reads all encoder values from each motor register.
static int32_t readEncoder(uint8_t reg)
{
    uint8_t buf[4] = {0};
    wireReadBlock(reg, buf, 4);
    return (int32_t)(buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24));
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
    s1 = (0.33f * x) + ( 0.58f * y) + (-0.33f * w);
    s2 = (0.33f * x) + (-0.58f * y) + (-0.33f * w);
    s3 = ( 0.67f * x) + ( 0.00f * y) + (0.33f * w);
}

static void normalize(float &s1, float &s2, float &s3)
{
    float maxVal = max(fabsf(s1), max(fabsf(s2), fabsf(s3)));
    if (maxVal > 1.0f) {
        s1 /= maxVal;
        s2 /= maxVal;
        s3 /= maxVal;
    }
}

// ── Public drive interface ────────────────────────────────────────────────────

/**
 * Drive the robot.
 * x, y, w : normalised velocity components in [-1, 1].
 * Y value of 1 is backward, -1 is forward.
 * X value of 1 is right, -1 is left.
 * W value of 1 is clockwise rotation, -1 is counterclockwise.
 */
void driveRobot(float x, float y, float w)
{
    float s1, s2, s3;
    computeSpeeds(x, y, w, s1, s2, s3);
    normalize(s1, s2, s3);

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
    delay(100);              

    // Tell the driver what motor type is connected
    uint8_t motorType = MOTOR_TYPE;
    wireWriteBlock(MOTOR_TYPE_ADDR, &motorType, 1);
    delay(10);

    // Set encoder polarity (direction)
    uint8_t encPolarity = MOTOR_ENCODER_POLARITY;
    wireWriteBlock(MOTOR_ENCODER_POLARITY_ADDR, &encPolarity, 1);
    delay(10);

    Serial.println("Kiwi Drive (Hiwonder I2C) Ready");

    //Resets encoder values in each motor back to zero.
    resetAllEncoders();
}

// ── Loop ──────────────────────────────────────────────────────────────────────
void loop()
{
    int32_t e1 = readEncoder(ENCODER1_ADDR);
    int32_t e2 = readEncoder(ENCODER2_ADDR);
    int32_t e3 = readEncoder(ENCODER3_ADDR);

     Serial.printf("Enc1: %ld | Enc2: %ld | Enc3: %ld\n", e1, e2, e3);

    
      driveRobot(0, 1, 0);
      delay(3000);
       // driveRobot(0, 0, 0);
      //delay(3000);


    // Forward
    //  driveRobot(0, -0.5, 0);
    //  delay(3000);
    //Right
    // driveRobot(0.05, 0, 0);
    // delay(3000);
    //Backward
    // driveRobot(0, 0.5, 0);
    // delay(3000);
    //Left
    // driveRobot(-0.05, 0, 0);
    // delay(3000);

    // Stop
    eStopRobot();
    delay(2000);
}