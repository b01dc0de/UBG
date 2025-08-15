#ifndef INPUT_H
#define INPUT_H

struct KeyboardState
{
    static constexpr int MaxNumKeys = 8;
    static int KeysPressed[MaxNumKeys];

    static bool GetKey(int KeyCode);
    static void SetKeyUp(int KeyCode);
    static void SetKeyDown(int KeyCode);
};

struct MouseState
{
    static int MouseX;
    static int MouseY;
    static bool bOffscreen;
    static int LeftButton;
    static int RightButton;

    static void SetMousePos(int X, int Y, bool bOffscreen);
    static void SetLeftButton(bool bDown);
    static void SetRightButton(bool bDown);
};

#endif // INPUT_H

