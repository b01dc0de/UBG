#include "UBG.h"

constexpr int DefaultWindowWidth = 1280;
constexpr int DefaultWindowHeight = 720;

bool GlobalState::bRunning = {};
int GlobalState::Width = DefaultWindowWidth;
int GlobalState::Height = DefaultWindowHeight;

bool UBG_Init()
{
    return UBG_PlatformT::Init() && UBG_GfxT::Init();
}

void UBG_GameLoop()
{
    UBG_PlatformT::Tick();
}

bool UBG_Term()
{
    return UBG_PlatformT::Term();
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

