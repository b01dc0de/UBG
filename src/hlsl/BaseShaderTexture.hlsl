
#pragma pack_matrix(row_major)

Texture2D MainTexture : register(t0);
SamplerState MainSampler : register(s0);

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
    Output.TexUV = Input.TexUV;
    return Output;
}

float4 PSMain(VS_OUTPUT Input) : SV_Target
{
    float4 OutColor = MainTexture.Sample(MainSampler, Input.TexUV);
    return OutColor;
}

