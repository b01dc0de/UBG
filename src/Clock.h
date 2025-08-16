#ifndef CLOCK_H
#define CLOCK_H

struct ClockT
{
    u64 Freq;
    u64 Epoch;

    u64 CurrTs;
    f64 CurrTime;
    f64 LastFrameDuration;

    void Tick();
    ClockT();
};

#endif // CLOCK_H

