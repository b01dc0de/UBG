
#pragma pack_matrix(row_major)

Texture2D MainTexture : register(t0);
SamplerState MainSampler : register(s0);

#ifndef ENABLE_WVP_TRANSFORM
    #define ENABLE_WVP_TRANSFORM (0)
#endif // ifndef ENABLE_WVP_TRANSFORM

#if ENABLE_WVP_TRANSFORM
cbuffer WorldBuffer : register(b0)
{
    matrix World;
}
cbuffer ViewProjBuffer : register(b1)
{
    matrix View;
    matrix Proj;
}
#endif // ENABLE_WVP_TRANSFORM

struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 TexUV : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 TexUV : TEXCOORD;
};

VS_OUTPUT VSMain(VS_INPUT Input)
{
    VS_OUTPUT Output;
    Output.Pos = Input.Pos;
#if ENABLE_WVP_TRANSFORM
    Output.Pos = mul(Output.Pos, World);
    Output.Pos = mul(Output.Pos, View);
    Output.Pos = mul(Output.Pos, Proj);
#endif // ENABLE_WVP_TRANSFORM
    Output.TexUV = Input.TexUV;
    return Output;
}

float4 PSMain(VS_OUTPUT Input) : SV_Target
{
    float4 OutColor = MainTexture.Sample(MainSampler, Input.TexUV);
    return OutColor;
}

