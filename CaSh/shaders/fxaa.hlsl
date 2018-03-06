Texture2D scene;
SamplerState fxss;
cbuffer fxaa_param
{
    float2 screenxy;
};

struct VS_Output
{  
    float4 Pos : SV_POSITION;              
    noperspective float2 Tex : TEXCOORD0;
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
    float FXAA_SPAN_MAX = 8.0;
    float FXAA_REDUCE_MUL = 1.0/8.0;
    float FXAA_REDUCE_MIN = 1.0/128.0;

    float2 uv = pin.Tex;
    float3 luma=float3(0.299, 0.587, 0.114);

    float lumaTL  = dot(luma, scene.Sample(fxss, uv + (float2(-1.0, -1.0) / screenxy)).xyz);
    float lumaTR  = dot(luma, scene.Sample(fxss, uv + (float2(1.0, -1.0) / screenxy)).xyz);
    float lumaBL  = dot(luma, scene.Sample(fxss, uv + (float2(-1.0, 1.0) / screenxy)).xyz);
    float lumaBR  = dot(luma, scene.Sample(fxss, uv + (float2(1.0, 1.0) / screenxy)).xyz);
    float lumaM   = dot(luma, scene.Sample(fxss, uv).xyz);

    float2 dir;
    dir.x = -((lumaTL + lumaTR) - (lumaBL + lumaBR));
    dir.y = ((lumaTL + lumaBL) - (lumaTR + lumaBR));

    float dirReduce = max((lumaTL + lumaTR + lumaBL + lumaBR) * (FXAA_REDUCE_MUL * 0.25), FXAA_REDUCE_MIN);
    float inverseDirAdjustment = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(float2(FXAA_SPAN_MAX, FXAA_SPAN_MAX), 
        max(float2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * inverseDirAdjustment)) / screenxy;

    float3 result1 = (1.0/2.0) * (
        scene.Sample(fxss, uv + (dir * float2(1.0/3.0 - 0.5,1.0/3.0 - 0.5))).xyz +
        scene.Sample(fxss, uv + (dir * float2(2.0/3.0 - 0.5,2.0/3.0 - 0.5))).xyz);

    float3 result2 = result1 * (1.0/2.0) + (1.0/4.0) * (
        scene.Sample(fxss, uv + (dir * float2(0.0/3.0 - 0.5,0.0/3.0 - 0.5))).xyz +
        scene.Sample(fxss, uv + (dir * float2(3.0/3.0 - 0.5,3.0/3.0 - 0.5))).xyz);

    float lumaMin = min(lumaM, min(min(lumaTL, lumaTR), min(lumaBL, lumaBR)));
    float lumaMax = max(lumaM, max(max(lumaTL, lumaTR), max(lumaBL, lumaBR)));
    float lumaResult2 = dot(luma, result2);

    float4 frag_color;
    if(lumaResult2 < lumaMin || lumaResult2 > lumaMax)
        frag_color = float4(result1, 1.0);
    else
        frag_color = float4(result2, 1.0);

    return frag_color;
}