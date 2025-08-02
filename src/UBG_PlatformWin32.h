#ifndef UBG_PLATFORMWIN32_H
#define UBG_PLATFORMWIN32_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

struct UBG_Platform_Win32
{
    static HINSTANCE hInstance;
    static HWND hWindow;
    //static HINSTANCE hPrevInstance;
    //static LPSTR lpCmdLine;
    //static int nShowCmd;

    static void Tick();
    static bool Init();
    static bool Term();
};

using UBG_PlatformT = UBG_Platform_Win32;

#endif // UBG_PLATFORMWIN32_H

