Texture2D orign_tex;



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
		{float2(-1, -1), float2(0, -1), float2(1, -1), float2(-1,0 ), float2(0, 0), float2(1, 0), float2(-1, 1), float2(0, 1), float2(1, 1)};

    float2 pos = pin.Pos;
    float mipLevel0 = 0;

	//float4 output  = orign_tex.Load(float3(pos.xy*2+OFFSETS[0], mipLevel0));
    float4
	output  = orign_tex.Load(float3(2*pos.xy + OFFSETS[0], mipLevel0));
	output += orign_tex.Load(float3(2*pos.xy + OFFSETS[1], mipLevel0));
	output += orign_tex.Load(float3(2*pos.xy + OFFSETS[2], mipLevel0));
	output += orign_tex.Load(float3(2*pos.xy + OFFSETS[3], mipLevel0));
	output += orign_tex.Load(float3(2*pos.xy + OFFSETS[4], mipLevel0));
	output += orign_tex.Load(float3(2*pos.xy + OFFSETS[5], mipLevel0));
	output += orign_tex.Load(float3(2*pos.xy + OFFSETS[6], mipLevel0));
	output += orign_tex.Load(float3(2*pos.xy + OFFSETS[7], mipLevel0));
	output += orign_tex.Load(float3(2*pos.xy + OFFSETS[8], mipLevel0));
    output/=9.0;
	
	return output; 
}
