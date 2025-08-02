#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

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

