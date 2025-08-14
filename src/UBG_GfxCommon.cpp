#include "UBG.h" // E UBG.Gfx.h E UBG_GfxCommon.h

void Camera::Ortho(float ResX, float ResY, float fDepth)
{
    View = m4f::Identity();
    Proj = m4f::Identity();

    Proj.V0.X = +2.0f / ResX;
    Proj.V1.Y = +2.0f / ResY;
    View.V2.Z = -2.0f / fDepth;
}

