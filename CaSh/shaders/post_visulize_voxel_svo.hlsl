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
	int voxel_size;
};


Texture3D<uint> brick_pool : register(t0);
StructuredBuffer<uint> node_pool : register(t1);

float4 toFloat4(uint value)
{
	return float4(float((value & 0x000000FF)), float((value & 0x0000FF00) >> 8U), float((value & 0x00FF0000) >> 16U), float((value & 0xFF000000) >> 24U)) / 255.0f;
}

uint trace_svo(float3 pos,float size)
{
	float3 vpos = pos;

    int level = 0;
    float3 cur_pos = vpos;
    float3 cur_size = voxel_size;
    int cur_idx = 0;
    uint node = node_pool[cur_idx];
    [loop]
    while (true)
    {
        if (level == 9)
        {
            uint brick_idx = node&0x7fffffff;
            uint idx = brick_idx;
            if(idx==0)
            	return 0;
            uint y = idx / (size * size) ;
            uint z = idx % (size * size) / size ;
            uint x = idx % (size * size) % size ;
            uint cc = (brick_pool[uint3(x,y,z)]);
            if(cc==0)
                cc = 0xff0000ff;
            return cc;
        }

        float3 np = cur_pos / cur_size;
        uint sub_idx = uint(np.x + 0.5) + uint(np.y + 0.5) * 4 + uint(np.z + 0.5) * 2;
        
        cur_size /= 2;
        if (np.x >= 0.5)
            cur_pos.x -= cur_size;
        if (np.y >= 0.5)
            cur_pos.y -= cur_size;
        if (np.z >= 0.5)
            cur_pos.z -= cur_size;

        level++;
        cur_idx = node & 0x7fffffff;
        if(cur_idx==0)
        	return 0;
        cur_idx += sub_idx;
        node = node_pool[cur_idx];
    }
    return 0;
}

float4 main_ps(VS_Output pin): SV_TARGET0
{
    uint size;
    brick_pool.GetDimensions(size, size, size);

    float tx = 2*pin.Tex.x - 1;
	float ty = 2*(1-pin.Tex.y) - 1;
	float vx = tx*view_dist* tan_fov_x;
	float vy = ty*view_dist*tan_fov_y;
	
	//cm
	float3 view_dir = float3(vx,vy,view_dist);
	view_dir = normalize(view_dir);
    //view_dir = mul(float4(view_dir,0),inv_mv_mat).xyz;
   
    int3 texDimensions = int3(voxel_size,voxel_size,voxel_size);
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
            pos.y>0&&pos.y<texDimensions.y/16&&
            pos.z>0&&pos.z<texDimensions.z/16     
        )
        {
            color.rgb = float3(0.8,0.5,0.5);
            float3 s = float3(pos.x*16,pos.y*16,pos.z*16);
            uint a = trace_svo(s,size);
            if(a!=0)
            {
            	color.rgb = toFloat4(a);
                color.a = 1;
                return color;
            }
        }
    }
    color.a = 1;
	return color;

}
