#pragma once
#include <cmath>
#include <cstdint>

namespace kiwi {
// Pure calculations: this file never writes to hardware or waits for a motor.
// pi relates wheel diameter to circumference. Its value is mathematical, not tuned.
constexpr float pi = 3.14159265358979323846f;
constexpr float maxMoveCm = 50; // Our first-test distance limit, not a motor rating.

// One record per driver channel. Values are set by config::defaults(), then can
// be edited in RAM with the `wheel` and `cal` serial commands.
struct Wheel {
    float angleDeg;         // Physical rolling direction, CCW from robot forward.
    int motorSign;          // Driver sign that rolls along angleDeg.
    int encoderSign;        // Count sign produced by a positive DRIVER command.
    float diameterCm;
    float countsPerRev;    // Measured through this driver, at the wheel shaft.
};

// Check a wheel record before using it. Timed PWM needs direction and motor sign
// only, so it passes needScale=false. Distance mode additionally needs measured
// diameter and CPR. The broad diameter/CPR ranges here are input sanity limits,
// chosen to catch typing errors; they do not specify your actual hardware.
inline bool valid(const Wheel& w, bool needScale = true) {
    return std::isfinite(w.angleDeg) && std::fabs(w.angleDeg) <= 360
        && (w.motorSign == 1 || w.motorSign == -1)
        && (w.encoderSign == 1 || w.encoderSign == -1)
        && (!needScale || (std::isfinite(w.diameterCm) && w.diameterCm >= 1
            && w.diameterCm <= 50 && std::isfinite(w.countsPerRev)
            && w.countsPerRev >= 10 && w.countsPerRev <= 100000));
}

// Turn two dimensionless direction weights into three PWM duty commands.
// forward=1,left=0 means "try the assumed forward pattern"; neither number is
// a centimeter measurement. Inputs must each be between -1 and +1.
//
// Derivation: a wheel can actively drive along its rolling axis. Project the
// desired chassis vector onto that axis using a dot product:
//   contribution = cos(angle)*forward + sin(angle)*left
// For the default 90/210/330 degree angles this is:
//   M1 = left
//   M2 = -sqrt(3)/2 * forward - 1/2 * left
//   M3 = +sqrt(3)/2 * forward - 1/2 * left
// Thus 0.866... and 0.5 come from trigonometry of a 120-degree kiwi layout.
// motorSign then accounts for reversed motor mounting/wiring. We intentionally
// do NOT use encoderSign, diameter or CPR: this mode has no encoder feedback.
//
// This is the same ROLE as computeSpeeds() in the original main.cpp, but it is
// a newly derived ideal-kiwi mix, not an unverified copy of its coefficients.
// Multiplying PWM duty in these ratios is only an approximation to wheel speed;
// unequal friction, battery charge, and unknown wiring can cause turns/drift.
inline bool directionPwm(float forward, float left, const Wheel (&w)[3],
                         int power, int8_t (&out)[3]) {
    for (int8_t& n : out) n = 0; // Failure always leaves a zero candidate command.
    if (!std::isfinite(forward) || !std::isfinite(left)
        || std::fabs(forward) > 1 || std::fabs(left) > 1
        || power < 1 || power > 100) return false;
    float mix[3] = {}; // Three calculated contributions, initially no motion.
    float peak = 1;   // Only scale DOWN oversized diagonals; don't amplify small inputs.
    for (int i = 0; i < 3; ++i) {
        if (!valid(w[i], false)) return false;
        const float angle = w[i].angleDeg*pi/180; // Degrees -> radians for sin/cos.
        mix[i] = (std::cos(angle)*forward + std::sin(angle)*left)*w[i].motorSign;
        if (std::fabs(mix[i]) > peak) peak = std::fabs(mix[i]);
    }
    for (int i = 0; i < 3; ++i)
        out[i] = int8_t(std::lround(power*mix[i]/peak)); // Driver needs whole signed bytes.
    return true;
}

// Equal 120-degree wheel placement is assumed. For translation, each wheel's
// driven travel is the dot product of chassis displacement and rolling direction.
inline bool targets(float forwardCm, float leftCm, const Wheel (&w)[3],
                    float (&out)[3]) {
    for (float& n : out) n = 0;
    if (!std::isfinite(forwardCm) || !std::isfinite(leftCm)
        || std::hypot(forwardCm, leftCm) > maxMoveCm) return false;
    for (int i = 0; i < 3; ++i) if (!valid(w[i])) return false;
    for (int i = 0; i < 3; ++i) {
        const float a = w[i].angleDeg*pi/180; // Trig functions take radians.
        const float travel = std::cos(a)*forwardCm + std::sin(a)*leftCm;
        out[i] = travel/(pi*w[i].diameterCm)*w[i].countsPerRev
            *w[i].motorSign*w[i].encoderSign;
    }
    return true;
}

// Handle a 32-bit encoder wrapping during a short move, without signed overflow.
inline int64_t delta(int32_t now, int32_t start) {
    const uint32_t bits = uint32_t(now) - uint32_t(start);
    return bits & 0x80000000u ? int64_t(bits) - 0x100000000LL : int64_t(bits);
}
inline int32_t decode(const uint8_t* b) {
    // "Little endian" means least-significant byte first. Shift each byte by
    // 8 more bits to put it in its correct place in the 32-bit count. Combine
    // using bitwise OR. Unsigned shifts avoid signed-overflow problems.
    const uint32_t u = uint32_t(b[0]) | (uint32_t(b[1]) << 8)
        | (uint32_t(b[2]) << 16) | (uint32_t(b[3]) << 24);
    // Bit 31 is the sign bit; subtract 2^32 to interpret a negative count.
    return int32_t(u & 0x80000000u ? int64_t(u)-0x100000000LL : int64_t(u));
}

// Common scaling preserves the wheel command ratios until integer quantization.
inline void scale(float (&values)[3], float limit) {
    float peak = limit; // Never amplify commands that are already below the limit.
    for (float v : values) if (std::fabs(v) > peak) peak = std::fabs(v);
    if (peak > limit) for (float& v : values) v *= limit/peak;
}
} // namespace kiwi