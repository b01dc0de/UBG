
#pragma pack_matrix(row_major)

cbuffer WorldBuffer : register(b0)
{
    matrix World;
}

cbuffer ViewProjBuffer : register(b1)
{
    matrix View;
    matrix Proj;
}

struct VS_INPUT
{
    float4 Pos : POSITION;
    float4 Rect : RECT;
    float4 RGBA : COLOR;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 RGBA : COLOR;
};

float2 RectTransform(float2 InPos, float4 InRect)
{
    /* NOTE:
        InRect.x == PosX        InRect.y = PosY
        InRect.z == SizeX       InRect.w == SizeY
    */
    float AdjX = InRect.x + (InPos.x * InRect.z);
    float AdjY = InRect.y + (InPos.y * InRect.w);
    return float2(AdjX, AdjY);
}

VS_OUTPUT VSMain(VS_INPUT Input)
{
    VS_OUTPUT Output;
    float2 RectPoint = RectTransform(Input.Pos.xy, Input.Rect);
    Output.Pos = mul(float4(RectPoint, Input.Pos.zw), World);
    Output.Pos = mul(Output.Pos, View);
    Output.Pos = mul(Output.Pos, Proj);
    Output.RGBA = Input.RGBA;
    return Output;
}

float4 PSMain(VS_OUTPUT Input) : SV_Target
{
    return Input.RGBA;
}

