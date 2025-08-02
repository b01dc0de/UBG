#include "UBG.h"

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
HWND UBG_Platform_Win32::hWindow = {};
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
    if (GlobalState::bRunning)
    {
        MSG Msg = {};
        while (PeekMessageA(&Msg, hWindow, 0, 0, TRUE))
        {
            TranslateMessage(&Msg);
            DispatchMessageA(&Msg);
        }
    }
}

bool UBG_Platform_Win32::Init()
{
    bool bResult = true;

    //SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);

    RECT WorkArea = {};
    if (SystemParametersInfoA(SPI_GETWORKAREA, 0, &WorkArea, 0))
    {
        GlobalState::Width = WorkArea.right - WorkArea.left;
        GlobalState::Height = WorkArea.bottom - WorkArea.top;
    }

    LPCSTR WindowClassName = "UntitledBulletGame";
    DWORD WindowStyle = WS_OVERLAPPEDWINDOW;
    DWORD WindowExStyle = 0;

    WNDCLASSEXA WindowClass = {};
    WindowClass.cbSize = sizeof(WNDCLASSEXA);
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = WndProc_Win32;
    WindowClass.cbClsExtra = 0;
    WindowClass.cbWndExtra = 0;
    WindowClass.hInstance = hInstance;
    WindowClass.lpszClassName = WindowClassName;
    (void)RegisterClassExA(&WindowClass);

    hWindow = CreateWindowExA(
        WindowExStyle,
        WindowClassName,
        WindowClassName,
        WindowStyle,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        GlobalState::Width, GlobalState::Height,
        nullptr, nullptr, nullptr, nullptr
    );

    ShowWindow(hWindow, SW_SHOWNORMAL);

    return bResult;
}

bool UBG_Platform_Win32::Term()
{
    bool bResult = true;

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

