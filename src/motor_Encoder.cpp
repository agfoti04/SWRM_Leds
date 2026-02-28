#include <Arduino.h>



const int PWMA = 25;   // PWM
const int AIN1 = 17;   // MA1
const int AIN2 = 21;   // MA2


const int ENC_A = 35;  // AC1
const int ENC_B = 34;  // AC2

//
volatile long encoderCount = 0;

// PWM
const int PWM_CHANNEL = 0;
const int PWM_FREQ = 20000;
const int PWM_RES = 8;

// Based on my understanding, When ENC_A and ENC_B are equal when it is going forward, and unequal when reverse
//This program does not take into account Encoder values to stop yet
//TODO: Add functionality that checks the encoderCount variable to know when the robot has reached its destination and stop the motor
void IRAM_ATTR handleEncoder()
{
    // Robust quadrature decode
    if (digitalRead(ENC_A) == digitalRead(ENC_B))
        encoderCount++;
    else
        encoderCount--;
}

// This is used to set speed and direction of the motor.
void setMotor(int pwm)
{
    if (pwm > 0)
    {
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, HIGH);
        //LedcWrite sets the speed of the motor depennding on the value of pwm, which can be between 0 and 255 
        ledcWrite(PWM_CHANNEL, pwm);
    }
    else if (pwm < 0)
    {
        digitalWrite(AIN1, HIGH);
        digitalWrite(AIN2, LOW);
    
        ledcWrite(PWM_CHANNEL, -pwm);
    }
    else
    {
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, LOW);
        ledcWrite(PWM_CHANNEL, 0);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    // ----- Motor setup -----
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);

    ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RES);
    ledcAttachPin(PWMA, PWM_CHANNEL);

    setMotor(0);

    // ----- Encoder setup -----
    pinMode(ENC_A, INPUT);
    pinMode(ENC_B, INPUT);

    attachInterrupt(digitalPinToInterrupt(ENC_A), handleEncoder, CHANGE);

    Serial.println("Motor A + Encoder Ready");
}

void loop()
{
    Serial.print("Encoder Count: ");
    Serial.println(encoderCount);

    // Run forward
    setMotor(50);
    delay(2000);

    // Stop
    setMotor(0);
    delay(1000);

    // Run reverse
    setMotor(-50);
    delay(2000);

    // Stop
    setMotor(0);
    delay(2000);
}
