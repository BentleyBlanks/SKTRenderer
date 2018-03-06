Texture2D origin_buffer;
SamplerState sam;

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

    float4 c = origin_buffer.Sample(sam,pin.Tex).rgba;
    c.a = 1;
    return c;
    /*
    return float4(c,0,0,1);
    float toned = c.r / (c.r + 1.f);
    toned = pow(toned,1/2.2);
    return float4(toned,toned,toned,1);
*/
}
