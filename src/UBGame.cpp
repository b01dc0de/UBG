#include "UBG.h" // Includes UBGame.h
#include "Game/Impl.h"
#include "Game/Demo.h"

#define RUN_DEMO() (0)
#if RUN_DEMO()
using UBGameImplT = UBGameImplDemo;
#else
using UBGameImplT = UBGameImpl;
#endif // RUN_DEMO()
UBGameImplT* Impl = nullptr;

bool UBGame::Init()
{
    Impl = new UBGameImplT{};
    return Impl->Init();
}

bool UBGame::Term()
{
    bool bResult = Impl->Term();
    delete Impl;
    return bResult;
}

void UBGame::Update()
{
    Impl->Update();
}

void UBGame::Draw()
{
    Impl->Draw();
}

