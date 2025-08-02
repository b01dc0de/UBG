#include "UBG_PlatformWin32.h"

int WinMain
(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nShowCmd
)
{
    OutputDebugStringA("UBG -- INIT\n");
#if _DEBUG
    DebugBreak();
#endif // _DEBUG
}

