#include "UBG.h" // Includes Input.h

int KeyboardState::KeysPressed[] = { };

void KeyboardState::SetKeyUp(int Key)
{
    for (int KeyIdx = 0; KeyIdx < MaxNumKeys; KeyIdx++)
    {
        if (KeysPressed[KeyIdx] == Key)
        {
            KeysPressed[KeyIdx] = 0;
        }
    }
}

void KeyboardState::SetKeyDown(int Key)
{
    int FirstZeroIdx = -1;
    bool bFound = false;
    for (int KeyIdx = 0; KeyIdx < MaxNumKeys; KeyIdx++)
    {
        if (KeysPressed[KeyIdx] == 0&& FirstZeroIdx == -1)
        {
            FirstZeroIdx = KeyIdx;
        }
        if (KeysPressed[KeyIdx] == Key)
        {
            bFound = true;
        }
    }

    if (!bFound && FirstZeroIdx != -1)
    {
        KeysPressed[FirstZeroIdx] = Key;
    }
}

int MouseState::MouseX = {};
int MouseState::MouseY = {};
bool MouseState::bOffscreen = {};
int MouseState::LeftButton = {};
int MouseState::RightButton = {};

void MouseState::SetMousePos(int X, int Y, bool _bOffscreen)
{
    if (_bOffscreen)
    {
        MouseX = -1;
        MouseY = -1;
        bOffscreen = true;
    }
    else
    {
        MouseX = X;
        MouseY = Y;
    }
}

void MouseState::SetLeftButton(bool bDown)
{
    LeftButton = bDown;
}

void MouseState::SetRightButton(bool bDown)
{
    RightButton = bDown;
}

