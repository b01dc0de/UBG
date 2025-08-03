#include "UBG.h" // UBG.h includes Clock.h

u64 ClockT::Freq = {};
u64 ClockT::Epoch = {};
u64 ClockT::CurrTs= {};
f64 ClockT::CurrTime = {};
f64 ClockT::LastFrameDuration = {};

static_assert(UBG_PLATFORM_IMPL_WIN32(), "[build] ClockT isn't implemented for non-Win32 platforms! (yet)");

void ClockT::Init()
{
    LARGE_INTEGER Timestamp = {};
    QueryPerformanceFrequency(&Timestamp);
    Freq = Timestamp.QuadPart;

    QueryPerformanceCounter(&Timestamp);
    Epoch = Timestamp.QuadPart;

    CurrTs = Epoch;
    CurrTime = 0.0;
    LastFrameDuration = 0.0f;
}

void ClockT::Tick()
{
    LARGE_INTEGER Timestamp = {};
    QueryPerformanceCounter(&Timestamp);

    u64 LastFrameTs = Timestamp.QuadPart - CurrTs;
    LastFrameDuration = (double)LastFrameTs / (double)Freq;
    CurrTs = Timestamp.QuadPart;
    CurrTime = (double)(CurrTs - Epoch) / (double)Freq;

    constexpr bool bDebugPrint = false;
    if (bDebugPrint)
    {
        static double LastTimePrint = 0.0f;
        if ((CurrTime - LastTimePrint) > 1.0f)
        {
            LastTimePrint = CurrTime;
            Outf("[time] CurrTime: %.02f, LastFrameDuration (ms): %.04f\n", CurrTime, LastFrameDuration * 1000.0);
        }
    }
}
