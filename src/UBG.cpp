#include "UBG.h"

void UBG_Engine::GameLoop()
{
    ClockT::Tick();
    PlatformState->Tick();
    Instance->Update();
    GfxState->Draw();
}

bool UBG_Engine::Init()
{
    Memory::Init();

    PlatformState = new UBG_PlatformT{};
    GfxState = new UBG_GfxT{};

    bool bResult = PlatformState->Init() && GfxState->Init();
    ClockT::Init();

    return bResult;
}

bool UBG_Engine::Term()
{
    bool bResult = GfxState->Term() && PlatformState->Term();
    
    delete GfxState;
    delete PlatformState;

    Memory::Term();

    return bResult;
}

UBG_Engine* GlobalEngine = nullptr;

int UBG_Main()
{
#define MAIN_ASSERT(Exp, Msg) if (!(Exp)) { Outf("[error][fatal]: %s\n", Msg); return -1; }

    UBG_Engine Engine = {};
    GlobalEngine = &Engine;
    bool bResult = Engine.Init();

    MAIN_ASSERT(bResult, "Engine initialization failed");

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
    GlobalEngine = nullptr;

    return bResult ? 0 : -1;
}

