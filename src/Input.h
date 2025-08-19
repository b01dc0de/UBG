#ifndef INPUT_H
#define INPUT_H

enum UBGKeyCode
{
    UBG_KEY_NONE = 0,
    // A-Z:
    UBG_KEY_A,
    UBG_KEY_B,
    UBG_KEY_C,
    UBG_KEY_D,
    UBG_KEY_E,
    UBG_KEY_F,
    UBG_KEY_G,
    UBG_KEY_H,
    UBG_KEY_I,
    UBG_KEY_J,
    UBG_KEY_K,
    UBG_KEY_L,
    UBG_KEY_M,
    UBG_KEY_N,
    UBG_KEY_O,
    UBG_KEY_P,
    UBG_KEY_Q,
    UBG_KEY_R,
    UBG_KEY_S,
    UBG_KEY_T,
    UBG_KEY_U,
    UBG_KEY_V,
    UBG_KEY_W,
    UBG_KEY_X,
    UBG_KEY_Y,
    UBG_KEY_Z,
    // 0-9:
    UBG_KEY_0,
    UBG_KEY_1,
    UBG_KEY_2,
    UBG_KEY_3,
    UBG_KEY_4,
    UBG_KEY_5,
    UBG_KEY_6,
    UBG_KEY_7,
    UBG_KEY_8,
    UBG_KEY_9,
    // F1-F12:
    UBG_KEY_F1,
    UBG_KEY_F2,
    UBG_KEY_F3,
    UBG_KEY_F4,
    UBG_KEY_F5,
    UBG_KEY_F6,
    UBG_KEY_F7,
    UBG_KEY_F8,
    UBG_KEY_F9,
    UBG_KEY_F10,
    UBG_KEY_F11,
    UBG_KEY_F12,
    // Non-alphanumeric/function:
    UBG_KEY_ESCAPE,
    UBG_KEY_TICK, // or ~
    UBG_KEY_DASH, // or _
    UBG_KEY_EQUALS, // or +
    UBG_KEY_BACKSPACE,
    UBG_KEY_TAB,
    UBG_KEY_LEFT_SQUARE, // or {
    UBG_KEY_RIGHT_SQUARE, // or }
    UBG_KEY_BACKSLASH, // or |
    UBG_KEY_SEMICOLON, // or :
    UBG_KEY_QUOTE, // or "
    UBG_KEY_ENTER,
    UBG_KEY_COMMA, // or <
    UBG_KEY_PERIOD, // or .
    UBG_KEY_SLASH, // or ?
    UBG_KEY_SPACE,
    // Arrow keys:
    UBG_KEY_ARROW_UP,
    UBG_KEY_ARROW_LEFT,
    UBG_KEY_ARROW_DOWN,
    UBG_KEY_ARROW_RIGHT,
    // Count:
    UBG_KEY_COUNT
};

struct KeyboardState
{
    static constexpr int MaxNumKeys = 8;
    int KeysPressed[MaxNumKeys];

    bool GetKey(UBGKeyCode KeyCode);
    void SetKeyUp(UBGKeyCode KeyCode);
    void SetKeyDown(UBGKeyCode KeyCode);
    KeyboardState();
};

struct MouseState
{
    int MouseX;
    int MouseY;
    bool bOffscreen;
    int LeftButton;
    int RightButton;

    void SetMousePos(int X, int Y, bool bOffscreen);
    void SetLeftButton(bool bDown);
    void SetRightButton(bool bDown);
    MouseState();
};

struct InputT
{
    KeyboardState Keyboard;
    MouseState Mouse;
};

#endif // INPUT_H

