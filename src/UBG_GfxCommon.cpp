#include "UBG.h" // E UBG.Gfx.h E UBG_GfxCommon.h

void Camera::Ortho(float ResX, float ResY, float fDepth)
{
    View = m4f::Identity();
    Proj = m4f::Identity();

    Proj.V0.X = +2.0f / ResX;
    Proj.V1.Y = +2.0f / ResY;
    View.V2.Z = -2.0f / fDepth;
}

void Camera::Perspective(v3f _Pos, v3f _LookAt)
{
    View = {};
    Proj = {};

    static const m4f NDC = Mult(m4f::Trans(0.0f, 0.0f, 1.0f), m4f::Scale(1.0f, 1.0f, 0.5f));

    constexpr v3f AbsUp{ 0.0f, 1.0f, 0.0f };
    v3f Forward = Norm(-(_LookAt - _Pos));
    v3f Right = Norm(Cross(AbsUp, Forward));
    v3f Up = Norm(Cross(Forward, Right));

    View.V0 = { Right.X, Up.X, Forward.X, 0.0f };
    View.V1 = { Right.Y, Up.Y, Forward.Y, 0.0f };
    View.V2 = { Right.Z, Up.Z, Forward.Z, 0.0f };
    View.V3.X = -Dot(_Pos, Right);
    View.V3.Y = -Dot(_Pos, Up);
    View.V3.Z = -Dot(_Pos, Forward);
    View.V3.W = 1.0f;

    constexpr f32 fFOV = 45.0f;
    constexpr f32 fAspectRatio = 16.0f / 9.0f;
    const f32 fD = 1.0f / tanf(fFOV / 2.0f);

    constexpr f32 fNearDist = 1.0f;
    constexpr f32 fFarDist = 1000.0f;
    constexpr f32 fDistDelta = fFarDist - fNearDist;

    Proj.V0.X = fD / fAspectRatio;
    Proj.V1.Y = fD;
    Proj.V2.Z = -(fFarDist + fNearDist) / fDistDelta;
    Proj.V2.W = -1.0f;
    Proj.V3.Z = (-2.0f * fFarDist * fNearDist) / fDistDelta;
    Proj = Proj * NDC;
}

