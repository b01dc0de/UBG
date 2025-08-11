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

void HandleKeyboardInput_Win32(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_KEYDOWN: { KeyboardState::SetKeyDown(wParam); } break;
        case WM_KEYUP: { KeyboardState::SetKeyUp(wParam); } break;
        default:
        {
            ASSERT(false);
        } break;
    }
}

void HandleMouseInput_Win32(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_MOUSEMOVE:
        {
            // NOTE: This code is taken from Windowsx.h GET_X/Y_LPARAM(...)
            int X = ((int)(short)LOWORD(lParam));
            int Y = ((int)(short)HIWORD(lParam));
            MouseState::SetMousePos(X, Y, false);
        } break;
        case WM_MOUSELEAVE:
        {
            MouseState::SetMousePos(0, 0, true);
        } break;
        case WM_LBUTTONDOWN:
        {
            MouseState::SetLeftButton(true);
        } break;
        case WM_LBUTTONUP:
        {
            MouseState::SetLeftButton(false);
        } break;
        case WM_RBUTTONDOWN:
        {
            MouseState::SetRightButton(true);
        } break;
        case WM_RBUTTONUP:
        {
            MouseState::SetRightButton(false);
        } break;
        default:
        {
            ASSERT(false);
        } break;
    }
}


// TODO: Move this static var
static WINDOWPLACEMENT WindowPlacement = { sizeof(WindowPlacement) };
void ToggleFullscreen(HWND Window)
{
    // NOTE: From Raymond Chen on MSDN
    // https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353

    DWORD Style = GetWindowLong(Window, GWL_STYLE);

    if (Style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO MonitorInfo = { sizeof(MonitorInfo) };
        if (GetWindowPlacement(Window, &WindowPlacement) &&
            GetMonitorInfoA(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
        {
            SetWindowLongA(Window, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(Window, HWND_TOP,
                MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLongA(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(Window, &WindowPlacement);
        SetWindowPos(Window, nullptr, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

LRESULT WndProc_Win32(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;

    switch (uMsg)
    {
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            if (uMsg == WM_KEYUP && wParam == VK_ESCAPE)
            {
                GlobalState::bRunning = false;
            }
            else if (uMsg == WM_KEYUP && wParam == VK_F11)
            {
                ToggleFullscreen(hWnd);
            }
            else
            {
                HandleKeyboardInput_Win32(uMsg, wParam, lParam);
            }
        } break;
        case WM_MOUSEMOVE:
        case WM_MOUSELEAVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        {
            HandleMouseInput_Win32(uMsg, wParam, lParam);
        } break;
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

#if _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _DEBUG

    //SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);

    /*
    RECT WorkArea = {};
    if (SystemParametersInfoA(SPI_GETWORKAREA, 0, &WorkArea, 0))
    {
        GlobalState::Width = WorkArea.right - WorkArea.left;
        GlobalState::Height = WorkArea.bottom - WorkArea.top;
    }
    */

    LPCSTR WindowClassName = "UntitledBulletGame";
    DWORD WindowStyle = WS_OVERLAPPEDWINDOW;

    WNDCLASSEXA WindowClass = {};
    WindowClass.cbSize = sizeof(WNDCLASSEXA);
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = WndProc_Win32;
    WindowClass.hInstance = hInstance;
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
    WindowClass.lpszClassName = WindowClassName;
    (void)RegisterClassExA(&WindowClass);

    hWindow = CreateWindowExA(
        0, // WindowExStyle
        WindowClassName,
        WindowClassName,
        WindowStyle,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        GlobalState::Width, GlobalState::Height,
        nullptr, nullptr, hInstance, nullptr
    );

    ToggleFullscreen(hWindow);

    ShowWindow(hWindow, SW_SHOW);

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

