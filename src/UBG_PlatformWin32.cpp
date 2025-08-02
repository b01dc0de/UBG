#include "UBG.h"
#if UBG_PLATFORM_WIN32()

void Outf(const char* Fmt, ...)
{
    constexpr size_t BufferLength = 1024;
    char MsgBuffer[BufferLength] = {};

    va_list Args;
    va_start(Args, Fmt);
    vsprintf_s(MsgBuffer, BufferLength, Fmt, Args);
    va_end(Args);

    OutputDebugStringA(MsgBuffer);
}

HINSTANCE UBG_Platform_Win32::hInstance = {};
//HINSTANCE UBG_Platform_Win32::hPrevInstance = {};
//LPSTR UBG_Platform_Win32::lpCmdLine = {};
//int UBG_Platform_Win32::nShowCmd = {};

LRESULT WndProc_Win32(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;

    switch (uMsg)
    {
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            if (wParam == VK_ESCAPE)
            {
                GlobalState::bRunning = false;
            }
        }
        case WM_CLOSE:
        case WM_DESTROY:
        case WM_QUIT:
        {
            GlobalState::bRunning = false;
        } break;
        default:
        {
            lResult = DefWindowProcA(hWnd, uMsg, wParam, lParam);
        }
    }

    return lResult;
}

void UBG_Platform_Win32::Tick()
{

}

bool UBG_Platform_Win32::Init()
{
    bool bResult = true;

    OutputDebugStringA("UBG -- INIT\n");

    return bResult;
}

bool UBG_Platform_Win32::Term()
{
    bool bResult = true;

    OutputDebugStringA("UBG -- TERM\n");

    return bResult;
}

int WinMain
(
    HINSTANCE _hInstance,
    HINSTANCE _hPrevInstance,
    LPSTR     _lpCmdLine,
    int       _nShowCmd
)
{
    UBG_Platform_Win32::hInstance = _hInstance;
    //UBG_Platform_Win32::hPrevInstance = _hPrevInstance;
    //UBG_Platform_Win32::lpCmdLine = _lpCmdLine;
    //UBG_Platform_Win32::nShowCmd = _nShowCmd;
    UNUSED_VAR(_hPrevInstance);
    UNUSED_VAR(_lpCmdLine);
    UNUSED_VAR(_nShowCmd);

    extern int UBG_Main();
    int Result = UBG_Main();
    return Result;
}

#endif // UBG_PLATFORM_WIN32()
