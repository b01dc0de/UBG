#ifndef GAME_DEMO_H
#define GAME_DEMO_H

struct UBGameImplDemo
{
    GfxSystem System;

    MeshStateID idTriangle;
    MeshStateID idQuad;
    MeshStateID idQuadMin;
    RenderEntityID idTriangleColor;
    RenderEntityID idQuadTexture;
    RenderEntityID idQuadUnicolor;

    bool Init();
    void Update();
    void Draw();
    bool Term();
};

#endif // GAME_DEMO_H

