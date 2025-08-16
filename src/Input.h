#ifndef INPUT_H
#define INPUT_H

struct KeyboardState
{
    static constexpr int MaxNumKeys = 8;
    int KeysPressed[MaxNumKeys];

    bool GetKey(int KeyCode);
    void SetKeyUp(int KeyCode);
    void SetKeyDown(int KeyCode);
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

