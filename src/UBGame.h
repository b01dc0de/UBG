#ifndef UBGAME_H
#define UBGAME_H

struct UBGame
{
    bool Init();
    bool Term();
    void Update();
    void Draw(UBG_GfxContextT* Context);
};

#endif // UBGAME_H

