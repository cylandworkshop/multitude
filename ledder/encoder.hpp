#pragma once

constexpr uint8_t ENCODER_STEPS = 48;

uint32_t Duration(uint32_t t0, uint32_t t1)
{
    if (t0 <= t1)
    {
        return t1 - t0;
    }

    return UINT32_MAX - t0 + t1;
}

float Rational(float x)
{
    float _;
    return modff(x, &_);
}

struct EncoderState
{
    static constexpr float ALPHA = 0.05;
    static constexpr int8_t ENCODER_OFFSET = 1;
    uint8_t state = 0;
    int8_t lastStepSize = 0;
    uint32_t lastUpdateTs = 0;
    uint32_t updateDurationCurrent = 0;
    float speed = 0;
    float angle = 0;

    void Update(uint32_t ts, uint32_t dt, uint8_t step)
    {
        step = (step + ENCODER_STEPS + ENCODER_OFFSET) % ENCODER_STEPS; // offset

        if (step % 4 == 2 && state != step && (step > state || (state - step > ENCODER_STEPS / 2)))
        {
            updateDurationCurrent += dt;
            dt = updateDurationCurrent;
            updateDurationCurrent = 0;

            auto const dist = float(step > state ? step - state : ENCODER_STEPS - state + step) / ENCODER_STEPS;
            state = step;

            auto const trueAngle = (1.0 / ENCODER_STEPS) * step;
            auto const trueSpeed = dist / dt;

            lastUpdateTs = ts;
            angle = trueAngle;
            speed = speed == 0 ? trueSpeed : ALPHA * trueSpeed + (1 - ALPHA) * speed;
        } else {
            updateDurationCurrent += dt;
        }
    }

    float GetAngle(uint32_t t)
    {
        uint32_t dt = Duration(lastUpdateTs, t);
        float a = angle + dt * speed;
        a = Rational(a);
        return a;
    }
};

