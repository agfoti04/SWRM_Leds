#pragma once // Include this file's definitions only once per source file.
#include "kiwi_math.h"

// All editable hardware assumptions and operating limits live here.
// "constexpr" means a constant whose value is known when we compile the code.
// These are NOT all measured properties of your robot. Each group below says
// whether its values come from the original program, a protocol, or our choice.
namespace config {

// HARDWARE BASELINE: copied from SWRM_Leds-master/src/main.cpp.
// I2C uses a device address (0x34) and two signal pins (SDA=data, SCL=clock).
// 0x means hexadecimal: 0x34 is 52 in ordinary decimal, not register 34.
constexpr uint8_t driverAddress = 0x34;
constexpr int sda = 21; // ESP32 GPIO number, NOT a connector's physical position.
constexpr int scl = 22;
constexpr unsigned i2cClockHz = 100000; // Same 100 kHz bus clock as the original.
constexpr unsigned serialBaud = 115200; // Same terminal communication rate.

// HIWONDER PROTOCOL: register numbers identify settings inside the driver.
// The original main.cpp already uses type=20, polarity=21, speed=51, encoder=60.
// PWM=31 and the corrected N20 type=2 come from Hiwonder's published example:
// https://docs.hiwonder.com/projects/Black-Mecanum-Wheel-Chassis/en/latest/docs/3._Arduino_checked.html
constexpr uint8_t motorTypeReg = 20;
constexpr uint8_t encoderPolarityReg = 21;
constexpr uint8_t pwmReg = 31;     // Signed duty command: -100..100 percent.
constexpr uint8_t speedReg = 51;   // Signed speed command: pulses per 10 ms.
constexpr uint8_t encoderReg = 60; // First of four consecutive 32-bit counters.
constexpr uint8_t motorType = 2;  // N20; your Adafruit 4638 is an N20 motor.
constexpr uint8_t encoderPolarity = 1; // Original code's setting; NOT verified.
// Timed PWM driving does not use the encoder speed loop. Polarity matters when
// you later test `jog` / calibrated `move`, which DO use that loop.

// FIRST-TRY TIMED MODE: deliberately chosen starting settings, not manufacturer
// guarantees and not derived from wheel diameter. A 20% PWM command means a
// requested duty cycle, NOT 20% of a known travel speed. It may not overcome
// friction on your robot; `drive` accepts an optional duty up to maxPwm.
constexpr int defaultPwm = 20;
constexpr int maxPwm = 25;                   // Our initial test cap, NOT driver maximum.
constexpr unsigned defaultTimedMs = 500;     // Half a second when duration is omitted.
constexpr unsigned minTimedMs = 50;         // Reject accidental near-zero durations.
constexpr unsigned maxTimedMs = 2000;       // Bound each uncalibrated drive to 2 seconds.
// Longer single-wheel trials make it easier to see motion and type a stop.
// Five seconds is a chosen diagnostic deadline, not a required motor start time.
// Only duration is extended here: the PWM/speed caps stay unchanged, allowing
// us to distinguish a short observation window from other causes of no motion.
constexpr unsigned maxPulseMs = 5000;       // Direct single-wheel PWM: at most 5 seconds.
constexpr unsigned maxJogMs = 5000;         // Direct single-wheel speed: at most 5 seconds.
constexpr unsigned testReportPeriodMs = 1000; // Report remaining time about once a second.
constexpr unsigned timedRefreshMs = 100;    // Re-send PWM/check I2C about 10 times/sec.

// OPTIONAL CALIBRATED DISTANCE MODE: these are initial controller tuning choices.
// They are not motor ratings. `drive` / `motors` do not need these measurements.
constexpr int maxSpeed = 3;                 // Pulses/10ms, NOT percent power.
constexpr unsigned moveTimeoutMs = 30000;   // Stop an unfinished distance move after 30s.
constexpr unsigned stallTimeoutMs = 1200;   // Fault after ~1.2s with no count progress.
constexpr unsigned controlPeriodMs = 20;    // Read distance feedback at about 50 Hz.
constexpr float positionGainPerSec = 2.0f; // Request 2 counts/s for each count of error.
constexpr float speedUnitSeconds = 0.01f;  // Protocol's 10ms = 0.01s, NOT a tuning guess.
constexpr float wheelToleranceCm = 0.1f;   // Accept ~1mm of wheel-travel error.
constexpr int minimumToleranceCounts = 3; // Don't chase individual encoder transitions.
constexpr int progressCounts = 3;          // Count this many transitions as progress.
constexpr int maxEncoderStep = 2000;       // Reject huge jumps/resets between samples.

// SOFTWARE TIMING/BUFFER CHOICES: bounded waits keep stop/deadline checks running.
constexpr unsigned idleStopPeriodMs = 250; // Retry zeros while idle, including after a bus fault.
constexpr unsigned i2cTimeoutMs = 20;      // Bound each I2C transaction's wait.
constexpr unsigned startupWaitMs = 200;  // Brief startup pause; no motion during it.
constexpr unsigned settingWaitMs = 10;    // Brief pause after driver setting writes.
constexpr unsigned loopPauseMs = 1;       // Yield between updates rather than busy-spin.
constexpr unsigned serialLineBytes = 96; // Maximum command storage, including end marker.
constexpr unsigned serialBytesPerLoop = 64; // A flood of input must not starve timers.

// GEOMETRY ASSUMPTION: a regular triangle with M1 at the front, M2 rear left,
// M3 rear right. The wheel's driven axis is tangent to the robot's center.
// Angles describe positive physical rolling direction from robot forward:
//   M1 90 degrees = left; M2 210 = backward/right; M3 330 = forward/right.
// The 120-degree spacing is the ideal kiwi layout, NOT measured wiring.
// Both signs start +1 so the initial motor pattern is definite and repeatable.
// You can discover the actual mapping later with `motors`, `pulse`, and `wheel`.
// Last two zeros mean "unknown diameter and counts/rev": no invented distance.
// Those zeros block `move` in centimeters, but DO NOT block timed `drive`.
inline void defaults(kiwi::Wheel (&w)[3]) {
    w[0] = { 90, 1, 1, 0, 0};
    w[1] = {210, 1, 1, 0, 0};
    w[2] = {330, 1, 1, 0, 0};
}
} // namespace config