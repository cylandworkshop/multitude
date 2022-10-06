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

struct EncoderState
{
    static constexpr uint8_t INVALID_STATE = 0xff;

    uint8_t state = INVALID_STATE;
    uint8_t lastStepSize = 0;
    uint32_t lastUpdateTs = 0;
    uint32_t updateDuration = 0;

    void Update(uint32_t t, uint8_t step)
    {
        if (state != step)
        {
            updateDuration = Duration(lastUpdateTs, t);
            if (state != INVALID_STATE)
            {
                lastStepSize = step - state;
            }

            state = step;
            lastUpdateTs = t;
        }
    }

    float GetAngle(uint32_t t)
    {
        uint32_t timeSinceLastUpdate = Duration(lastUpdateTs, t);

        float lerp = (updateDuration && lastStepSize) ? (float(timeSinceLastUpdate) * float(lastStepSize)) / (float(updateDuration)) : 1.0;
        uint32_t newPos = (uint32_t(lerp) + state) % ENCODER_STEPS;
        float _;
        return (float(newPos) + modff(lerp, &_)) / float(ENCODER_STEPS);
    }
};

