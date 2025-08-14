#include "UBG.h"

void UBG_Engine::GameLoop()
{
    ClockT::Tick();
    UBG_PlatformT::Tick();
    UBG_GfxT::Draw();
}

bool UBG_Engine::Init()
{
    Memory::Init();

    bool bResult = UBG_PlatformT::Init() && UBG_GfxT::Init();
    ClockT::Init();

    return bResult;
}

bool UBG_Engine::Term()
{
    bool bResult = UBG_PlatformT::Init() && UBG_GfxT::Term();

    Memory::Term();

    return bResult;
}

UBG_Engine* GlobalEngine = nullptr;

#define MAIN_ASSERT(Exp, Msg) if (!(Exp)) { Outf("[error][fatal]: %s\n", Msg); return -1; }

int UBG_Main()
{
    UBG_Engine Engine = {};
    GlobalEngine = &Engine;
    bool bResult = Engine.Init();

    MAIN_ASSERT(bResult, "Platform initialization failed");

    Engine.Instance = new UBGame{};
    Engine.Instance->Init();

    Engine.bRunning = true;
    while (Engine.bRunning)
    {
        Engine.GameLoop();
    }

    Engine.Instance->Term();
    delete Engine.Instance;
    Engine.Instance = nullptr;

    bResult &= Engine.Term();
    MAIN_ASSERT(bResult, "Platform termination failed");

    return bResult ? 0 : -1;
}

