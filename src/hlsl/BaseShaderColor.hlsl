
#pragma pack_matrix(row_major)

struct VS_INPUT
{
    float4 Pos : POSITION;
    float4 RGBA : COLOR;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 RGBA : COLOR;
};

VS_OUTPUT VSMain(VS_INPUT Input)
{
    VS_OUTPUT Output;
    Output.Pos = Input.Pos;
    Output.RGBA = Input.RGBA;
    return Output;
}

float4 PSMain(VS_OUTPUT Input) : SV_Target
{
    float4 OutColor = Input.RGBA;
    return OutColor;
}

