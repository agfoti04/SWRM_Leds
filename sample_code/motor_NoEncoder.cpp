

#include <Arduino.h>

//Motor A Pins 
const int PWMA = 25;   // PWM pin
const int AIN1 = 17;   // Direction 1
const int AIN2 = 21;   // Direction 2

// ===== PWM Settings =====
const int PWM_CHANNEL = 0;
const int PWM_FREQ = 20000;   // Hz
const int PWM_RES = 8;        // 8-bit (0–255)

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("Motor A Basic Test");

    // Sets direction pins to Output
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);

    // Setup PWM correctly
    //TODO: Does PWM need to be plugged in to work.
    ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RES);
    ledcAttachPin(PWMA, PWM_CHANNEL);

    // Initilize Low
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);
    ledcWrite(PWM_CHANNEL, 0);
}

void loop()
{
    Serial.println("Forward");

    // MA2 High is forward, MA1 High is reverse
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    ledcWrite(PWM_CHANNEL, 200);   // ~80% speed
    delay(3000);

    Serial.println("Stop");

    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);
    ledcWrite(PWM_CHANNEL, 0);
    delay(2000);

    Serial.println("Reverse");

    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    ledcWrite(PWM_CHANNEL, 200);
    delay(3000);

    Serial.println("Stop");

    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);
    ledcWrite(PWM_CHANNEL, 0);
    delay(2000);
}