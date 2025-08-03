#include "UBG.h"

constexpr int DefaultWindowWidth = 1280;
constexpr int DefaultWindowHeight = 720;

bool GlobalState::bRunning = {};
int GlobalState::Width = DefaultWindowWidth;
int GlobalState::Height = DefaultWindowHeight;

bool UBG_Init()
{
    bool bResult = UBG_PlatformT::Init() && UBG_GfxT::Init();
    ClockT::Init();
    return bResult;
}

void UBG_GameLoop()
{
    ClockT::Tick();
    UBG_PlatformT::Tick();
    UBG_GfxT::Draw();
}

bool UBG_Term()
{
    bool bResult = UBG_PlatformT::Init() && UBG_GfxT::Term();
    return bResult;
}

int UBG_Main()
{
    bool bResult = UBG_Init();

    if (!bResult)
    {
        Outf("[error] Platform initialization failed!\n");
        return -1;
    }

    GlobalState::bRunning = true;
    while (GlobalState::bRunning)
    {
        UBG_GameLoop();
    }

    bResult &= UBG_Term();
    if (!bResult)
    {
        Outf("[error] Platform termination failed!\n");
        return -1;
    }

    return bResult ? 0 : -1;
}

