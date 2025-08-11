#ifndef UBG_GFXCOMMON_H
#define UBG_GFXCOMMON_H

struct VxColor
{
    v4f Pos;
    v4f Col;
};

struct VxTex
{
    v4f Pos;
    v2f UV;
};

struct VxMin
{
    v4f Pos;
};

struct Camera
{
    m4f View;
    m4f Proj;

    void Ortho(float ResX, float ResY, float fDepth);
};

#endif // UBG_GFXCOMMON_H

