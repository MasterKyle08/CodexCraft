Texture2D BlockAtlas : register(t0);
SamplerState BlockSampler : register(s0);

struct PSInput
{
    float4 Position : SV_Position;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float4 Color    : COLOR0;
};

float4 main(PSInput input) : SV_Target
{
    float4 texColor = BlockAtlas.Sample(BlockSampler, input.TexCoord);
    float3 litColor = texColor.rgb * input.Color.rgb;
    return float4(litColor, texColor.a * input.Color.a);
}
