Texture2D lumin_buffer;
SamplerState samp;

struct VS_Output
{  
    float4 Pos : SV_POSITION;              
    float2 Tex : TEXCOORD0;
};

cbuffer CBZ
{
    float middleGrey;
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
    float a = lumin_buffer.SampleLevel(samp,float2(0.5,0.5),8).g;
    //float c = lumin_buffer.SampleLevel(samp,float2(0.5,0.5),5).r;
    float b = exp(a);
    float f = middleGrey/(b+0.0001);
    return float4(f,a,b,1);

}
