#include "UBG.h" // Includes UBGame.h

struct UBGameImpl
{
    GfxSystem System;

    bool Init()
    {
        return System.Init((UBG_GfxT*)GlobalEngine->GfxState);
    }
    void Update()
    {
    }
    void Draw(UBG_GfxContextT* Context)
    {
        System.Entities.DrawAll(Context, &System);
    }
    bool Term()
    {
        return true;
    }
};

UBGameImpl* Impl = nullptr;

bool UBGame::Init()
{
    Impl = new UBGameImpl;
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

void UBGame::Draw(UBG_GfxContextT* Context)
{
    Impl->Draw(Context);
}

