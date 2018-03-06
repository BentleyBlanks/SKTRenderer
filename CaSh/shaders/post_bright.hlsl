Texture2D color_texture;
SamplerState bright_sampler;

cbuffer CB
{
    float bloomThreshold;
};

struct VS_Output
{  
    float4 Pos : SV_POSITION;              
    float2 Tex : TEXCOORD0;
};

VS_Output main_vs(uint id : SV_VertexID)
{
    VS_Output Output;
    Output.Tex = float2((id << 1) & 2, id & 2);
    Output.Pos = float4(Output.Tex * float2(2,-2) + float2(-1,1), 0, 1);
    return Output;
}


float4 main_ps(VS_Output pin): SV_TARGET0
{
    return max(color_texture.Sample(bright_sampler, pin.Tex) - bloomThreshold, (float4)0.0f);
}
