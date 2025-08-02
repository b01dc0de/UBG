#include "UBG.h"

bool GlobalState::bRunning = false;

int UBG_Main()
{
    bool bResult = UBG_PlatformT::Init();

    if (!bResult)
    {
        Outf("[error] Platform initialization failed!\n");
        return -1;
    }

    while (GlobalState::bRunning)
    {
        UBG_PlatformT::Tick();
    }

    bResult &= UBG_PlatformT::Term();
    if (!bResult)
    {
        Outf("[error] Platform termination failed!\n");
        return -1;
    }

    return bResult ? 0 : -1;
}

