#include "UBG.h" // Includes Input.h

bool KeyboardState::GetKey(UBGKeyCode KeyCode)
{
    for (int KeyIdx = 0; KeyIdx < MaxNumKeys; KeyIdx++)
    {
        if (KeysPressed[KeyIdx] == KeyCode)
        {
            return true;
        }
    }
    return false;
}

void KeyboardState::SetKeyUp(UBGKeyCode Key)
{
    for (int KeyIdx = 0; KeyIdx < MaxNumKeys; KeyIdx++)
    {
        if (KeysPressed[KeyIdx] == Key)
        {
            KeysPressed[KeyIdx] = UBG_KEY_NONE;
        }
    }
}

void KeyboardState::SetKeyDown(UBGKeyCode Key)
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

KeyboardState::KeyboardState()
{
    for (int Idx = 0; Idx < MaxNumKeys; Idx++)
    {
        KeysPressed[Idx] = UBG_KEY_NONE;
    }
}

MouseState::MouseState()
{
    MouseX = {};
    MouseY = {};
    bOffscreen = {};
    LeftButton = {};
    RightButton = {};
}

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

