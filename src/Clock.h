#ifndef CLOCK_H
#define CLOCK_H

// TODO: Make this non-static

struct ClockT
{
    static u64 Freq;
    static u64 Epoch;

    static u64 CurrTs;
    static f64 CurrTime;
    static f64 LastFrameDuration;

    static void Init();
    static void Tick();
};

#endif // CLOCK_H

