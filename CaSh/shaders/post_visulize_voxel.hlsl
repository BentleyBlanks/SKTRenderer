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

cbuffer CB
{
    matrix inv_mv_mat;
	float view_dist;
	float tan_fov_x;
	float tan_fov_y;
};

//m
Texture3D<uint> voxel;


float4 toFloat4(uint value)
{
	return float4(float((value & 0x000000FF)), float((value & 0x0000FF00) >> 8U), float((value & 0x00FF0000) >> 16U), float((value & 0xFF000000) >> 24U)) / 255.0f;
}


float4 main_ps(VS_Output pin): SV_TARGET0
{
    float tx = 2*pin.Tex.x - 1;
	float ty = 2*(1-pin.Tex.y) - 1;
	float vx = tx*view_dist* tan_fov_x;
	float vy = ty*view_dist*tan_fov_y;
	
	//cm
	float3 view_dir = float3(vx,vy,view_dist);
	view_dir = normalize(view_dir);
    //view_dir = mul(float4(view_dir,0),inv_mv_mat).xyz;

    

    int3 texDimensions;
	voxel.GetDimensions(texDimensions.x, texDimensions.y, texDimensions.z);
    //float3 wp = mul(float4(0,0,0,1),inv_mv_mat).xyz;
    float sample_len = 128/2;
    int sample_n = 1024;
    float march_seg = sample_len/sample_n;
    float4 color = float4(0,0,0,1);
    [loop]
    for(int i=0;i<sample_n;++i)
    {
        float3 pos = march_seg*i*view_dir;
        pos = mul(float4(pos,1),inv_mv_mat).xyz;
        if(
            pos.x>0&&pos.x<texDimensions.x/16&&
            pos.y>0/16&&pos.y<texDimensions.y/16&&
            pos.z>0/16&&pos.z<texDimensions.z/16     
        )
        {
            color.rgb = float3(0.8,0.5,0.5);
            int3 s = int3(pos.x*16,pos.y*16,pos.z*16);
            uint a = voxel.Load(float4(s,0));
            if(a!=0)
            {
                //return float4(0,0,1,1);
                color = toFloat4(a);
                color.a = 1;
                return color;
            }
        }
    }

    color.a = 1;
	return color;
}
