#include <Arduino.h>
#include <math.h>

//MOTOR A 
const int PWMA = 25;
const int AIN1 = 17;
const int AIN2 = 21;
const int ENC_A1 = 35;
const int ENC_B1 = 34;

//MOTOR B
const int PWMB = 26;
const int BIN1 = 18;
const int BIN2 = 19;
const int ENC_A2 = 32;
const int ENC_B2 = 33;

//MOTOR C
const int PWMC = 27;
const int CIN1 = 22;
const int CIN2 = 23;
const int ENC_A3 = 36;
const int ENC_B3 = 39;
//
volatile long encoder1 = 0;
volatile long encoder2 = 0;
volatile long encoder3 = 0;

// PWM
#define PWM_FREQ 20000
#define PWM_RES 8

#define CH_A 0
#define CH_B 1
#define CH_C 2

// Based on my understanding, When ENC_A and ENC_B are equal when it is going forward, and unequal when reverse
//This program does not take into account Encoder values to stop yet
//TODO: Add functionality that checks the encoderCount variable to know when the robot has reached its destination and stop the motor
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

// This is used to set speed and direction of the motor.
void setMotorA(int pwm)
{
    if(abs(pwm) < 30) pwm = 0; //deadband
    pwm = constrain(pwm, -255, 255);

    if (pwm > 0)
    {
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, HIGH);
        ledcWrite(CH_A, pwm);
    }
    else if (pwm < 0)
    {
        digitalWrite(AIN1, HIGH);
        digitalWrite(AIN2, LOW);
        ledcWrite(CH_A, -pwm);
    }
    else
    {
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, LOW);
        ledcWrite(CH_A, 0);
    }
}

void setMotorB(int pwm)
{
    if(abs(pwm) < 30) pwm = 0; //deadband
    pwm = constrain(pwm, -255, 255);

    if (pwm > 0)
    {
        digitalWrite(BIN1, LOW);
        digitalWrite(BIN2, HIGH);
        ledcWrite(CH_B, pwm);
    }
    else if (pwm < 0)
    {
        digitalWrite(BIN1, HIGH);
        digitalWrite(BIN2, LOW);
        ledcWrite(CH_B, -pwm);
    }
    else
    {
        digitalWrite(BIN1, LOW);
        digitalWrite(BIN2, LOW);
        ledcWrite(CH_B, 0);
    }
}

void setMotorC(int pwm)
{
    if(abs(pwm) < 30) pwm = 0; //deadband
    pwm = constrain(pwm, -255, 255);

    if (pwm > 0)
    {
        digitalWrite(CIN1, LOW);
        digitalWrite(CIN2, HIGH);
        ledcWrite(CH_C, pwm);
    }
    else if (pwm < 0)
    {
        digitalWrite(CIN1, HIGH);
        digitalWrite(CIN2, LOW);
        ledcWrite(CH_C, -pwm);
    }
    else
    {
        digitalWrite(CIN1, LOW);
        digitalWrite(CIN2, LOW);
        ledcWrite(CH_C, 0);
    }
}

//kinematics
void computeSpeeds(float x, float y, float w, float &s1, float &s2, float &s3)
{
    // Kiwi drive inverse kinematics
    s1 = (-0.33 * x) + (0.58 * y) + (0.33 * w);
    s2 = (-0.33 * x) + (-0.58 * y) + (0.33 * w);
    s3 = ( 0.67 * x) + (0.00 * y) + (0.33 * w);
}

void normalize(float &s1, float &s2, float &s3)
{
    float maxVal = max(fabs(s1), max(fabs(s2), fabs(s3)));

    if (maxVal > 1.0)
    {
        s1 /= maxVal;
        s2 /= maxVal;
        s3 /= maxVal;
    }
}

//driving function
void driveRobot(float x, float y, float w)
{
    float s1, s2, s3;

    computeSpeeds(x, y, w, s1, s2, s3);
    normalize(s1, s2, s3);

    int m1 = (int)(s1 * 255);
    int m2 = (int)(s2 * 255);
    int m3 = (int)(s3 * 255);

    setMotorA(m1);
    setMotorB(m2);
    setMotorC(m3);
}

void eStopRobot()
{
    setMotorA(0);
    setMotorB(0);
    setMotorC(0);
}

//setup
void setup()
{
    Serial.begin(115200);

    // Motor pins
    pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
    pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
    pinMode(CIN1, OUTPUT); pinMode(CIN2, OUTPUT);

    // PWM setup
    ledcSetup(CH_A, PWM_FREQ, PWM_RES);
    ledcAttachPin(PWMA, CH_A);

    ledcSetup(CH_B, PWM_FREQ, PWM_RES);
    ledcAttachPin(PWMB, CH_B);

    ledcSetup(CH_C, PWM_FREQ, PWM_RES);
    ledcAttachPin(PWMC, CH_C);

    // Encoder pins
    pinMode(ENC_A1, INPUT_PULLUP); pinMode(ENC_B1, INPUT_PULLUP);
    pinMode(ENC_A2, INPUT_PULLUP); pinMode(ENC_B2, INPUT_PULLUP);
    pinMode(ENC_A3, INPUT_PULLUP); pinMode(ENC_B3, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(ENC_A1), handleEnc1, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENC_A2), handleEnc2, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENC_A3), handleEnc3, CHANGE);

    Serial.println("Kiwi Drive Robot Ready");
}

void loop()
{
    Serial.printf("Enc1: %ld | Enc2: %ld | Enc3: %ld\n", encoder1, encoder2, encoder3);

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
