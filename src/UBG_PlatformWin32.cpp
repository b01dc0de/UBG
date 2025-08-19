#include "UBG.h" // E UBG_Platform.h E UBG_PlatformWin32.h

static_assert(UBG_PLATFORM_IMPL_WIN32(), "UBG_Platform_Win32.cpp: File being built without Win32 Platform impl defined");

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

void DebugBreakpoint()
{
    DebugBreak();
}

// Based on table in https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
UBGKeyCode VKCode_To_UBGCodeLUT[] =
{
    UBG_KEY_NONE, // 0x00
    UBG_KEY_NONE, // 0x01   VK_LBUTTON      Left mouse button
    UBG_KEY_NONE, // 0x02   VK_RBUTTON      Right mouse button
    UBG_KEY_NONE, // 0x03   VK_CANCEL       Control-break processing
    UBG_KEY_NONE, // 0x04   VK_MBUTTON      Middle mouse button
    UBG_KEY_NONE, // 0x05   VK_XBUTTON1     X1 mouse button
    UBG_KEY_NONE, // 0x06   VK_XBUTTON2     X2 mouse button
    UBG_KEY_NONE, // 0x07                   Reserved
    UBG_KEY_BACKSPACE, // 0x08   VK_BACK         Backspace key
    UBG_KEY_TAB, // 0x09   VK_TAB          Tab key
    UBG_KEY_NONE, // 0x0A                   Reserved
    UBG_KEY_NONE, // 0x0B                   Reserved
    UBG_KEY_NONE, // 0x0C   VK_CLEAR        Clear key
    UBG_KEY_ENTER, // 0x0D   VK_RETURN       Enter key
    UBG_KEY_NONE, // 0x0E                   Unassigned
    UBG_KEY_NONE, // 0x0F                   Unassigned

    UBG_KEY_NONE, // 0x10   VK_SHIFT        Shift key
    UBG_KEY_NONE, // 0x11   VK_CONTROL      Ctrl key
    UBG_KEY_NONE, // 0x12   VK_MENU         Alt key
    UBG_KEY_NONE, // 0x13   VK_PAUSE        Pause key
    UBG_KEY_NONE, // 0x14   VK_CAPITAL      Capslock
    UBG_KEY_NONE, // 0x15   VK_KANA/VK_HANGUL       IME Kana mode/IME Hangul mode
    UBG_KEY_NONE, // 0x16   VK_IME_ON       IME On
    UBG_KEY_NONE, // 0x17   VK_JUNJA        IME Juja mode
    UBG_KEY_NONE, // 0x18   VK_FINAL        IME final mode
    UBG_KEY_NONE, // 0x19   VK_HANJA/VK_KANJI       IME Hanja mode/IME Kanji mode
    UBG_KEY_NONE, // 0x1A   VK_IME_OFF              IME Off
    UBG_KEY_ESCAPE, // 0x1B   VK_ESCAPE               Esc key
    UBG_KEY_NONE, // 0x1C   VK_CONVERT              IME convert
    UBG_KEY_NONE, // 0x1D   VK_NONCONVERT           IME nonconvert
    UBG_KEY_NONE, // 0x1E   VK_ACCEPT               IME accept
    UBG_KEY_NONE, // 0x1F   VK_MODECHANGE           IME mode change request

    UBG_KEY_SPACE, // 0x20   VK_SPACE                Spacebar key
    UBG_KEY_NONE, // 0x21   VK_PRIOR                Page up key
    UBG_KEY_NONE, // 0x22   VK_NEXT                 Page down key
    UBG_KEY_NONE, // 0x23   VK_END                  End key
    UBG_KEY_NONE, // 0x24   VK_HOME                 Home key
    UBG_KEY_ARROW_LEFT, // 0x25   VK_LEFT                 Left arrow key
    UBG_KEY_ARROW_UP, // 0x26   VK_UP                   Up arrow key
    UBG_KEY_ARROW_RIGHT, // 0x27   VK_RIGHT                Right arrow key
    UBG_KEY_ARROW_DOWN, // 0x28   VK_DOWN                 Down arrow key
    UBG_KEY_NONE, // 0x29   VK_SELECT               Select key
    UBG_KEY_NONE, // 0x2A   VK_PRINT                Print key
    UBG_KEY_NONE, // 0x2B   VK_EXECUTE              Execute key
    UBG_KEY_NONE, // 0x2C   VK_SNAPSHOT             Print screen key
    UBG_KEY_NONE, // 0x2D   VK_INSERT               Insert key
    UBG_KEY_NONE, // 0x2E   VK_DELETE               Delete key
    UBG_KEY_NONE, // 0x2F   VK_HELP                 Help key

    UBG_KEY_0, // 0x30                           '0'
    UBG_KEY_1, // 0x31                           '1'
    UBG_KEY_2, // 0x32                           '2'
    UBG_KEY_3, // 0x33                           '3'
    UBG_KEY_4, // 0x34                           '4'
    UBG_KEY_5, // 0x35                           '5'
    UBG_KEY_6, // 0x36                           '6'
    UBG_KEY_7, // 0x37                           '7'
    UBG_KEY_8, // 0x38                           '8'
    UBG_KEY_9, // 0x39                           '9'
    UBG_KEY_NONE, // 0x3A                           Undefined
    UBG_KEY_NONE, // 0x3B                           Undefined
    UBG_KEY_NONE, // 0x3C                           Undefined
    UBG_KEY_NONE, // 0x3D                           Undefined
    UBG_KEY_NONE, // 0x3E                           Undefined
    UBG_KEY_NONE, // 0x3F                           Undefined

    UBG_KEY_NONE, // 0x40                           Undefined
    UBG_KEY_A, // 0x41                           'A'
    UBG_KEY_B, // 0x42                           'B'
    UBG_KEY_C, // 0x43                           'C'
    UBG_KEY_D, // 0x44                           'D'
    UBG_KEY_E, // 0x45                           'E'
    UBG_KEY_F, // 0x46                           'F'
    UBG_KEY_G, // 0x47                           'G'
    UBG_KEY_H, // 0x48                           'H'
    UBG_KEY_I, // 0x49                           'I'
    UBG_KEY_J, // 0x4A                           'J'
    UBG_KEY_K, // 0x4B                           'K'
    UBG_KEY_L, // 0x4C                           'L'
    UBG_KEY_M, // 0x4D                           'M'
    UBG_KEY_N, // 0x4E                           'N'
    UBG_KEY_O, // 0x4F                           'O'

    UBG_KEY_P, // 0x50                           'P'
    UBG_KEY_Q, // 0x51                           'Q'
    UBG_KEY_R, // 0x52                           'R'
    UBG_KEY_S, // 0x53                           'S'
    UBG_KEY_T, // 0x54                           'T'
    UBG_KEY_U, // 0x55                           'U'
    UBG_KEY_V, // 0x56                           'V'
    UBG_KEY_W, // 0x57                           'W'
    UBG_KEY_X, // 0x58                           'X'
    UBG_KEY_Y, // 0x59                           'Y'
    UBG_KEY_Z, // 0x5A                           'Z'
    UBG_KEY_NONE, // 0x5B   VK_LWIN                 Left Windows logo key
    UBG_KEY_NONE, // 0x5C   VK_RWIN                 Right Windows logo key
    UBG_KEY_NONE, // 0x5D   VK_APPS                 Application key
    UBG_KEY_NONE, // 0x5E                           Reserved
    UBG_KEY_NONE, // 0x5F   VK_SLEEP                Computer sleep key

    UBG_KEY_NONE, // 0x60   VK_NUMPAD0              Numeric keypad 0 key
    UBG_KEY_NONE, // 0x61   VK_NUMPAD1              Numeric keypad 1 key
    UBG_KEY_NONE, // 0x62   VK_NUMPAD2              Numeric keypad 2 key
    UBG_KEY_NONE, // 0x63   VK_NUMPAD3              Numeric keypad 3 key
    UBG_KEY_NONE, // 0x64   VK_NUMPAD4              Numeric keypad 4 key
    UBG_KEY_NONE, // 0x65   VK_NUMPAD5              Numeric keypad 5 key
    UBG_KEY_NONE, // 0x66   VK_NUMPAD6              Numeric keypad 6 key
    UBG_KEY_NONE, // 0x67   VK_NUMPAD7              Numeric keypad 7 key
    UBG_KEY_NONE, // 0x68   VK_NUMPAD8              Numeric keypad 8 key
    UBG_KEY_NONE, // 0x69   VK_NUMPAD9              Numeric keypad 9 key
    UBG_KEY_NONE, // 0x6A   VK_MULTIPLY             Multiply key
    UBG_KEY_NONE, // 0x6B   VK_ADD                  Add key
    UBG_KEY_NONE, // 0x6C   VK_SEPARATOR            Separator key
    UBG_KEY_NONE, // 0x6D   VK_SUBTRACT             Subtract key
    UBG_KEY_NONE, // 0x6E   VK_DECIMAL              Decimal key
    UBG_KEY_NONE, // 0x6F   VK_DIVIDE               Divide key

    UBG_KEY_F1, // 0x70   VK_F1                   F1 key
    UBG_KEY_F2, // 0x71   VK_F2                   F2 key
    UBG_KEY_F3, // 0x72   VK_F3                   F3 key
    UBG_KEY_F4, // 0x73   VK_F4                   F4 key
    UBG_KEY_F5, // 0x74   VK_F5                   F5 key
    UBG_KEY_F6, // 0x75   VK_F6                   F6 key
    UBG_KEY_F7, // 0x76   VK_F7                   F7 key
    UBG_KEY_F8, // 0x77   VK_F8                   F8 key
    UBG_KEY_F9, // 0x78   VK_F9                   F9 key
    UBG_KEY_F10, // 0x79   VK_F10                  F10 key
    UBG_KEY_F11, // 0x7A   VK_F11                  F11 key
    UBG_KEY_F12, // 0x7B   VK_F12                  F12 key
    UBG_KEY_NONE, // 0x7C   VK_F13                  F13 key
    UBG_KEY_NONE, // 0x7D   VK_F14                  F14 key
    UBG_KEY_NONE, // 0x7E   VK_F15                  F15 key
    UBG_KEY_NONE, // 0x7F   VK_F16                  F16 key

    UBG_KEY_NONE, // 0x80   VK_F17                  F17 key
    UBG_KEY_NONE, // 0x81   VK_F18                  F18 key
    UBG_KEY_NONE, // 0x82   VK_F19                  F19 key
    UBG_KEY_NONE, // 0x83   VK_F20                  F20 key
    UBG_KEY_NONE, // 0x84   VK_F21                  F21 key
    UBG_KEY_NONE, // 0x85   VK_F22                  F22 key
    UBG_KEY_NONE, // 0x86   VK_F23                  F23 key
    UBG_KEY_NONE, // 0x87   VK_F24                  F24 key
    UBG_KEY_NONE, // 0x88                           Reserved
    UBG_KEY_NONE, // 0x89                           Reserved
    UBG_KEY_NONE, // 0x8A                           Reserved
    UBG_KEY_NONE, // 0x8B                           Reserved
    UBG_KEY_NONE, // 0x8C                           Reserved
    UBG_KEY_NONE, // 0x8D                           Reserved
    UBG_KEY_NONE, // 0x8E                           Reserved
    UBG_KEY_NONE, // 0x8F                           Reserved

    UBG_KEY_NONE, // 0x90   VK_NUMLOCK              Num lock key
    UBG_KEY_NONE, // 0x91   VK_SCROLL               Scroll lock key
    UBG_KEY_NONE, // 0x92                           OEM specific
    UBG_KEY_NONE, // 0x93                           OEM specific
    UBG_KEY_NONE, // 0x94                           OEM specific
    UBG_KEY_NONE, // 0x95                           OEM specific
    UBG_KEY_NONE, // 0x96                           OEM specific
    UBG_KEY_NONE, // 0x97                           Unassigned
    UBG_KEY_NONE, // 0x98                           Unassigned
    UBG_KEY_NONE, // 0x99                           Unassigned
    UBG_KEY_NONE, // 0x9A                           Unassigned
    UBG_KEY_NONE, // 0x9B                           Unassigned
    UBG_KEY_NONE, // 0x9C                           Unassigned
    UBG_KEY_NONE, // 0x9D                           Unassigned
    UBG_KEY_NONE, // 0x9E                           Unassigned
    UBG_KEY_NONE, // 0x9F                           Unassigned

    UBG_KEY_NONE, // 0xA0   VK_LSHIFT               Left Shift key
    UBG_KEY_NONE, // 0xA1   VK_RSHIFT               Right Shift key
    UBG_KEY_NONE, // 0xA2   VK_LCONTROL             Left Ctrl key
    UBG_KEY_NONE, // 0xA3   VK_RCONTROL             Right Ctrl key
    UBG_KEY_NONE, // 0xA4   VK_LMENU                Left Alt key
    UBG_KEY_NONE, // 0xA5   VK_RMENU                Right Alt key
    UBG_KEY_NONE, // 0xA6   VK_BROWSER_BACK         Browser Back key
    UBG_KEY_NONE, // 0xA7   VK_BROWSER_FORWARD      Browser Forward key
    UBG_KEY_NONE, // 0xA8   VK_BROWSER_REFRESH      Browser Refresh key
    UBG_KEY_NONE, // 0xA9   VK_BROWSER_STOP         Browser Stop key
    UBG_KEY_NONE, // 0xAA   VK_BROWSER_SEARCH       Browser Search key
    UBG_KEY_NONE, // 0xAB   VK_BROWSER_FAVORITES    Browser Favorites key
    UBG_KEY_NONE, // 0xAC   VK_BROWSER_HOME         Browser Start and Home key
    UBG_KEY_NONE, // 0xAD   VK_VOLUME_MUTE          Volume Mute key
    UBG_KEY_NONE, // 0xAE   VK_VOLUME_DOWN          Volume Down key
    UBG_KEY_NONE, // 0xAF   VK_VOLUME_UP            Volume Up key

    UBG_KEY_NONE, // 0xB0   VK_MEDIA_NEXT_TRACK     Next Track key
    UBG_KEY_NONE, // 0xB1   VK_MEDIA_PREV_TRACK     Previous Track key
    UBG_KEY_NONE, // 0xB2   VK_MEDIA_STOP           Stop Media key
    UBG_KEY_NONE, // 0xB3   VK_MEDIA_PLAY_PAUSE     Play/Pause Media key
    UBG_KEY_NONE, // 0xB4   VK_LAUNCH_MAIL          Start Mail key
    UBG_KEY_NONE, // 0xB5   VK_LAUNCH_MEDIA_SELECT  Select Media key
    UBG_KEY_NONE, // 0xB6   VK_LAUNCH_APP1          Start Application 1 key
    UBG_KEY_NONE, // 0xB7   VK_LAUNCH_APP2          Start Application 2 key
    UBG_KEY_NONE, // 0xB8                           Reserved
    UBG_KEY_NONE, // 0xB9                           Reserved
    UBG_KEY_SEMICOLON, // 0xBA   VK_OEM_1                Varies. For US ANSI: Semicolon and Colon key
    UBG_KEY_EQUALS, // 0xBB   VK_OEM_PLUS             Any region: Equals and Plus key
    UBG_KEY_COMMA, // 0xBC   VK_OEM_COMMA            Any region: Comma and Less Than key
    UBG_KEY_DASH, // 0xBD   VK_OEM_MINUS            Any region: Dash and Underscore key
    UBG_KEY_PERIOD, // 0xBE   VK_OEM_PERIOD           Any region: Period and Greater Than key
    UBG_KEY_SLASH, // 0xBF   VK_OEM_2                Varies. For US ANSI: Forward Slash and Question Mark key

    UBG_KEY_TICK, // 0xC0   VK_OEM_3                Varies. For US ANSI: Grave Accent and Tilde key
    UBG_KEY_NONE, // 0xC1                           Reserved
    UBG_KEY_NONE, // 0xC2                           Reserved
    UBG_KEY_NONE, // 0xC3                           Reserved
    UBG_KEY_NONE, // 0xC4                           Reserved
    UBG_KEY_NONE, // 0xC5                           Reserved
    UBG_KEY_NONE, // 0xC6                           Reserved
    UBG_KEY_NONE, // 0xC7                           Reserved
    UBG_KEY_NONE, // 0xC8                           Reserved
    UBG_KEY_NONE, // 0xC9                           Reserved
    UBG_KEY_NONE, // 0xCA                           Reserved
    UBG_KEY_NONE, // 0xCB                           Reserved
    UBG_KEY_NONE, // 0xCC                           Reserved
    UBG_KEY_NONE, // 0xCD                           Reserved
    UBG_KEY_NONE, // 0xCE                           Reserved
    UBG_KEY_NONE, // 0xCF                           Reserved

    UBG_KEY_NONE, // 0xD0                           Reserved
    UBG_KEY_NONE, // 0xD1                           Reserved
    UBG_KEY_NONE, // 0xD2                           Reserved
    UBG_KEY_NONE, // 0xD3                           Reserved
    UBG_KEY_NONE, // 0xD4                           Reserved
    UBG_KEY_NONE, // 0xD5                           Reserved
    UBG_KEY_NONE, // 0xD6                           Reserved
    UBG_KEY_NONE, // 0xD7                           Reserved
    UBG_KEY_NONE, // 0xD8                           Reserved
    UBG_KEY_NONE, // 0xD9                           Reserved
    UBG_KEY_NONE, // 0xDA                           Reserved
    UBG_KEY_LEFT_SQUARE, // 0xDB   VK_OEM_4                Varies. For US ANSI: Left Brace key
    UBG_KEY_BACKSLASH, // 0xDC   VK_OEM_5                Varies. For US ANSI: Backslash and Pipe key
    UBG_KEY_RIGHT_SQUARE, // 0xDD   VK_OEM_6                Varies. For US ANSI: Right Brace key
    UBG_KEY_QUOTE, // 0xDE   VK_OEM_7                Varies. For US ANSI: Apostrophe and Double Quotation Mark key
    UBG_KEY_NONE, // 0xDF   VK_OEM_8                Varies. For Canadian CSA: Right Ctrl key

    UBG_KEY_NONE, // 0xE0                           Reserved
    UBG_KEY_NONE, // 0xE1                           OEM specific
    UBG_KEY_NONE, // 0xE2   VK_OEM_102              Varies. For European ISO: Backslash and Pipe key
    UBG_KEY_NONE, // 0xE3                           OEM specific
    UBG_KEY_NONE, // 0xE4                           OEM specific
    UBG_KEY_NONE, // 0xE5   VK_PROCESSKEY           IME PROCESS key
    UBG_KEY_NONE, // 0xE6                           OEM specific
    UBG_KEY_NONE, // 0xE7   VK_PACKET               Used to pass Unicode as keystrokes
    UBG_KEY_NONE, // 0xE8                           Unassigned
    UBG_KEY_NONE, // 0xE9                           OEM specific
    UBG_KEY_NONE, // 0xEA                           OEM specific
    UBG_KEY_NONE, // 0xEB                           OEM specific
    UBG_KEY_NONE, // 0xEC                           OEM specific
    UBG_KEY_NONE, // 0xED                           OEM specific
    UBG_KEY_NONE, // 0xEE                           OEM specific
    UBG_KEY_NONE, // 0xEF                           OEM specific

    UBG_KEY_NONE, // 0xF0                           OEM specific
    UBG_KEY_NONE, // 0xF1                           OEM specific
    UBG_KEY_NONE, // 0xF2                           OEM specific
    UBG_KEY_NONE, // 0xF3                           OEM specific
    UBG_KEY_NONE, // 0xF4                           OEM specific
    UBG_KEY_NONE, // 0xF5                           OEM specific
    UBG_KEY_NONE, // 0xF6   VK_ATTN                 Attn key
    UBG_KEY_NONE, // 0xF7   VK_CRSEL                CrSel key (?)
    UBG_KEY_NONE, // 0xF8   VK_EXSEL                ExSel key (?)
    UBG_KEY_NONE, // 0xF9   VK_EREOF                Erase EOF key
    UBG_KEY_NONE, // 0xFA   VK_PLAY                 Play key
    UBG_KEY_NONE, // 0xFB   VK_ZOOM                 Zoom key
    UBG_KEY_NONE, // 0xFC   VK_NONAME               Reserved
    UBG_KEY_NONE, // 0xFD   VK_PA1                  PA1 key (?)
    UBG_KEY_NONE // 0xFE   VK_OEM_CLEAR            Clear key
};
static constexpr size_t SizeVKLUT = ARRAY_SIZE(VKCode_To_UBGCodeLUT);

void HandleKeyboardInput_Win32(u32 uMsg, WPARAM wParam, LPARAM lParam)
{
    UNUSED_VAR(lParam);
    switch (uMsg)
    {
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        {
            UBGKeyCode KeyCode = UBG_KEY_NONE;
            if (wParam < SizeVKLUT)
            {
                KeyCode = VKCode_To_UBGCodeLUT[wParam];
            }

            if (KeyCode != UBG_KEY_NONE)
            {
                GlobalEngine->Input->Keyboard.SetKeyDown(KeyCode);
            }
        } break;
        case WM_SYSKEYUP:
        case WM_KEYUP:
        {
            UBGKeyCode KeyCode = UBG_KEY_NONE;
            if (wParam < SizeVKLUT)
            {
                KeyCode = VKCode_To_UBGCodeLUT[wParam];
            }

            if (KeyCode != UBG_KEY_NONE)
            {
                GlobalEngine->Input->Keyboard.SetKeyUp(KeyCode);
            }
        } break;
        default:
        {
            ASSERT(false);
        } break;
    }
}

void HandleMouseInput_Win32(u32 uMsg, WPARAM wParam, LPARAM lParam)
{
    UNUSED_VAR(wParam);
    switch (uMsg)
    {
        case WM_MOUSEMOVE:
        {
            // NOTE: This code is taken from Windowsx.h GET_X/Y_LPARAM(...)
            int X = ((int)(short)LOWORD(lParam));
            int Y = ((int)(short)HIWORD(lParam));
            GlobalEngine->Input->Mouse.SetMousePos(X, Y, false);
        } break;
        case WM_MOUSELEAVE:
        {
            GlobalEngine->Input->Mouse.SetMousePos(0, 0, true);
        } break;
        case WM_LBUTTONDOWN:
        {
            GlobalEngine->Input->Mouse.SetLeftButton(true);
        } break;
        case WM_LBUTTONUP:
        {
            GlobalEngine->Input->Mouse.SetLeftButton(false);
        } break;
        case WM_RBUTTONDOWN:
        {
            GlobalEngine->Input->Mouse.SetRightButton(true);
        } break;
        case WM_RBUTTONUP:
        {
            GlobalEngine->Input->Mouse.SetRightButton(false);
        } break;
        default:
        {
            ASSERT(false);
        } break;
    }
}

void ToggleFullscreen(HWND Window)
{
    // TODO: Move this static var
    static WINDOWPLACEMENT WindowPlacement = { sizeof(WindowPlacement) };

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

LRESULT WndProc_Win32(HWND hWnd, u32 uMsg, WPARAM wParam, LPARAM lParam)
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
                GlobalEngine->bRunning = false;
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
            GlobalEngine->bRunning = false;
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
    if (GlobalEngine->bRunning)
    {
        MSG Msg = {};
        while (PeekMessageA(&Msg, hWindow, 0, 0, TRUE))
        {
            TranslateMessage(&Msg);
            DispatchMessageA(&Msg);
        }
    }
}

static HINSTANCE WinMain_hInst = {};

bool UBG_Platform_Win32::Init()
{
    bool bResult = true;

    hInstance = WinMain_hInst;
    hWindow = {};
    //hPrevInstance = {};
    //lpCmdLine = {};
    //nShowCmd = {};

#if _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _DEBUG

    // TODO: Do we care about this?
    //SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);

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
        CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr, nullptr, hInstance, nullptr
    );

    // Get Width/Height info from whichever monitor hWindow is on
    MONITORINFO MonitorInfo = { sizeof(MonitorInfo) };
    GetMonitorInfoA(MonitorFromWindow(hWindow, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo);
    GlobalEngine->Width = MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left;
    GlobalEngine->Height = MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top;

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
    WinMain_hInst = _hInstance;
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

