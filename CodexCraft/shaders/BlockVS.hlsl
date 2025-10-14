cbuffer CameraConstants : register(b0)
{
    float4x4 View;
    float4x4 Projection;
    float4x4 ViewProjection;
    float3 CameraPosition;
    float Padding0;
};

struct VSInput
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float4 Color    : COLOR0;
};

struct VSOutput
{
    float4 Position : SV_Position;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float4 Color    : COLOR0;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    float4 worldPos = float4(input.Position, 1.0f);
    output.Position = mul(worldPos, ViewProjection);
    output.Normal = input.Normal;
    output.TexCoord = input.TexCoord;
    output.Color = input.Color;
    return output;
}
