
#pragma pack_matrix(row_major)

cbuffer UnicolorBuffer// : register(b2)
{
    float4 Unicolor;
}

struct VS_INPUT
{
    float4 Pos : POSITION;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Unicolor : COLOR;
};

VS_OUTPUT VSMain(VS_INPUT Input)
{
    VS_OUTPUT Output;
    Output.Pos = Input.Pos;
    Output.Unicolor = Unicolor;
    return Output;
}

float4 PSMain(VS_OUTPUT Input) : SV_Target
{
    float4 OutColor = Input.Unicolor;
    return OutColor;
}

