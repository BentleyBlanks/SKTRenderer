Texture2D g_Input0;



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
    const float2 OFFSETS[] = 
		{float2(-4, 0), float2(-3, 0), float2(-2, 0), float2(-1, 0), float2(0, 0), float2(1, 0), float2(2, 0), float2(3, 0), float2(4, 0)};

	static const float g_BlurWeights[] = {
		0.004815026f,
		0.028716039f,
		0.102818575f,
		0.221024189f,
		0.28525234f,
		0.221024189f,
		0.102818575f,
		0.028716039f,
		0.004815026f
	};

    float2 pos = pin.Pos;
    float mipLevel0 = 0;

	float4 
	output  = g_Input0.Load(float3(pos.xy + OFFSETS[0], mipLevel0)) * g_BlurWeights[0];
	output += g_Input0.Load(float3(pos.xy + OFFSETS[1], mipLevel0)) * g_BlurWeights[1];
	output += g_Input0.Load(float3(pos.xy + OFFSETS[2], mipLevel0)) * g_BlurWeights[2];
	output += g_Input0.Load(float3(pos.xy + OFFSETS[3], mipLevel0)) * g_BlurWeights[3];
	output += g_Input0.Load(float3(pos.xy + OFFSETS[4], mipLevel0)) * g_BlurWeights[4];
	output += g_Input0.Load(float3(pos.xy + OFFSETS[5], mipLevel0)) * g_BlurWeights[5];
	output += g_Input0.Load(float3(pos.xy + OFFSETS[6], mipLevel0)) * g_BlurWeights[6];
	output += g_Input0.Load(float3(pos.xy + OFFSETS[7], mipLevel0)) * g_BlurWeights[7];
	output += g_Input0.Load(float3(pos.xy + OFFSETS[8], mipLevel0)) * g_BlurWeights[8];
	
	return output; 
}
