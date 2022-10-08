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
    static constexpr float ALPHA = 0.05;
    uint8_t state = 0;
    int8_t lastStepSize = 0;
    uint32_t lastUpdateTs = 0;
    uint32_t updateDurationCurrent = 0;
    float updateDuration = 0;

    void Update(uint32_t ts, uint32_t dt, uint8_t step)
    {
        if (state != step && (step > state || (state - step > ENCODER_STEPS / 2)))
        {
            updateDurationCurrent += dt;
            dt = updateDurationCurrent;

            // if (dt > 100)
            // {
                updateDurationCurrent = 0;
                updateDuration = ALPHA * dt + (1 - ALPHA) * updateDuration;

                if (lastStepSize < 0)
                {
                    lastStepSize = 1;
                }

                state = step;
                lastUpdateTs = ts;
            // }
        }
        else
        {
            updateDurationCurrent += dt;
        }
    }

    float GetAngle(uint32_t t)
    {
        uint32_t timeSinceLastUpdate = Duration(lastUpdateTs, t);

        // uint32_t updateDuration = 0;
        // for (size_t i = 0; i < DURATIONS_NUM; ++i)
        // {
        //     updateDuration += updateDurations[i];
        // }

        // updateDuration /= DURATIONS_NUM;

        // float stepSign = lastStepSize > 0 ? 1.0 : -1.0;
        float stepSign = 1;
        float lerp = (updateDuration > 0) ?
            (float(timeSinceLastUpdate) * stepSign) / (float(updateDuration)) :
            1.0;

        if (lerp < -1.0)
        {
            lerp = -1.0;
        }

        if (lerp > 1.0)
        {
            lerp = 1.0;
        }

        uint32_t newPos = (uint32_t(lerp) + state) % ENCODER_STEPS;
        float _;
        return (float(newPos) + modff(lerp, &_)) / float(ENCODER_STEPS);
    }
};

