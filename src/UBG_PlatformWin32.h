#ifndef UBG_PLATFORMWIN32_H
#define UBG_PLATFORMWIN32_H

#include <crtdbg.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// C/C++ std lib headers:
#include <stdio.h>

struct UBG_Platform_Win32
{
    HINSTANCE hInstance;
    HWND hWindow;
    //HINSTANCE hPrevInstance;
    //LPSTR lpCmdLine;
    //int nShowCmd;

    void Tick();
    bool Init();
    bool Term();
};

using UBG_PlatformT = UBG_Platform_Win32;

#endif // UBG_PLATFORMWIN32_H

