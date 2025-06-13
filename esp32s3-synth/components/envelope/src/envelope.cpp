#include "envelope.hpp"

Envelope::Envelope(float sampleRate, uint16_t initialBpm)
    : sampleRate(sampleRate),
      bpm(initialBpm),
      attack(sampleRate),
      decay(sampleRate),
      sustain(sampleRate),
      release(sampleRate)
{
    recalculate();
}

void Envelope::setAttack(uint8_t value)
{
    params.attack = std::min(value, static_cast<uint8_t>(envelope::MAX));
    recalculate();
}

void Envelope::setDecay(uint8_t value)
{
    params.decay = std::min(value, static_cast<uint8_t>(envelope::MAX));
    recalculate();
}

void Envelope::setSustain(uint8_t value)
{
    params.sustain = std::min(value, static_cast<uint8_t>(envelope::MAX));
    recalculate();
}

void Envelope::setRelease(uint8_t value)
{
    params.release = std::min(value, static_cast<uint8_t>(envelope::MAX));
    recalculate();
}

void Envelope::setBpm(uint16_t newBpm)
{
    bpm = newBpm;
    recalculate();
}

void Envelope::recalculate()
{
    sustainLevel = static_cast<float>(params.sustain) / static_cast<float>(envelope::MAX);

    attack.recalculate(params.attack, bpm);
    decay.recalculate(params.decay, bpm, sustainLevel);
    sustain.recalculate(params.sustain, bpm);
    release.recalculate(params.release, bpm);
}

void Envelope::gateOn()
{
    attack.enter(0.0f);
    active = &attack;
}

void Envelope::gateOff()
{
    if (active && active != &release) {
        float level = active->currentValue();
        release.enter(level);
        active = &release;
    }
}

float Envelope::next()
{
    if (!active) {
        return 0.0f;
    }

    float value = active->next();

    if (active->isFinished()) {
        if (active == &attack) {
            float level = attack.currentValue();
            if (level <= sustainLevel) {
                sustain.enter(level);
                active = &sustain;
            } else {
                decay.enter(level);
                active = &decay;
            }
        }
        else if (active == &decay) {
            sustain.enter(decay.currentValue());
            active = &sustain;
        }
        else if (active == &sustain) {
            // Should only transition via gateOff
        }
        else if (active == &release) {
            active = nullptr;
        }
    }

    return value;
}

bool Envelope::is_idle() const
{
    return active == nullptr;
}
