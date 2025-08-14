#include "UBG.h" // Includes UBGame.h

struct UBGameImpl
{
    ListID<float> Objects;
    TypeID A_ID;
    TypeID B_ID;
    TypeID C_ID;
    TypeID D_ID;

    bool Init()
    {
        Objects = {};
        Objects.Init();
        A_ID = Objects.Create();
        B_ID = Objects.Create();
        C_ID = Objects.Create();
        D_ID = Objects.Create();

        float& rA = Objects.Get(A_ID);
        rA = 1.0f;
        float& rB = Objects.Get(B_ID);
        rB = 1.0f;
        float& rC = Objects.Get(C_ID);
        rC = 1.0f;
        float& rD = Objects.Get(D_ID);
        rD = 1.0f;

        return true;
    }
    void Update()
    {
    }
    void Draw(UBG_GfxContextT* Context)
    {
        (void)Context;
    }
    bool Term()
    {
        Objects.Destroy(B_ID);
        Objects.Destroy(C_ID);
        Objects.Destroy(A_ID);
        Objects.Destroy(D_ID);

        TypeID E_ID = Objects.Create();
        TypeID F_ID = Objects.Create();
        TypeID G_ID = Objects.Create();
        TypeID H_ID = Objects.Create();

        Objects.Destroy(E_ID);
        Objects.Destroy(H_ID);
        Objects.Destroy(G_ID);
        Objects.Destroy(F_ID);

        Objects.Term();
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

