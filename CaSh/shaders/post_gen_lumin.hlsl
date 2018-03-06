Texture2D hdr_color_gen_lun;
SamplerState sampler_gen_lum;



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

// Approximates luminance from an RGB value
float CalcLuminance(float3 color)
{
//    return dot(color, float3(0.299f, 0.587f, 0.114f));
    
    return ( log(dot(color, float3(0.299f, 0.587f, 0.114f))+ 1.f));
}

float4 main_ps(VS_Output pin): SV_TARGET0
{


    float3 c = hdr_color_gen_lun.Sample(sampler_gen_lum,pin.Tex).xyz;

    float lum = CalcLuminance(c);
    /*
    if(lum>1)
    return float4(c,1);
    */
    /*
    if(isnan(lum))
    return float4(1,0,0,1);
    */
    return float4(lum,lum,lum,1);


}
