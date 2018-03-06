#define FLAG_SUBDIV 0x80000000


cbuffer CB
{
    int cur_levels;
    int voxel_size;
    uint voxel_list_size;
    int max_levels;
    uint option;
};

float4 toFloat4(uint value)
{
	return float4(float((value & 0x000000FF)), float((value & 0x0000FF00) >> 8U), float((value & 0x00FF0000) >> 16U), float((value & 0xFF000000) >> 24U)) / 255.0f;
}

uint convVec4ToRGBA8(float4 val)
{
    return (uint(val.w) & 0x000000FF) << 24 | (uint(val.z) & 0x000000FF) << 16 | (uint(val.y) & 0x000000FF) << 8 | (uint(val.x) & 0x000000FF);
}

  struct Vertex_PC
  {
	  float4 color;      
	  float4 pos;
  };


Texture3D<uint> brick_pool : register(t0);
StructuredBuffer<uint> node_pool : register(t1);
RWByteAddressBuffer indirect_buffer : register(u0);
RWStructuredBuffer<uint> counter : register(u1);

void trace_svo(float3 pos,float size)
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
        if (level == cur_levels)
        {
            
            
            
            uint brick_idx = node&0x7fffffff;
            uint idx = brick_idx;
            if(idx==0)
            	return;
            uint y = idx / (size * size) ;
            uint z = idx % (size * size) / size ;
            uint x = idx % (size * size) % size ;
            uint idx1 = counter.IncrementCounter();
            uint ccc = brick_pool[uint3(x,y,z)];
            {
            //some check! to remove
            if(ccc==0)
                ccc = 0xff0000ff;
            
            if(((node&0x80000000)>>31)==1)
                ccc = 0x000000ff;
            }
            float4 cc = toFloat4(ccc);
            indirect_buffer.Store(idx1*32 ,asuint(cc.x));
            indirect_buffer.Store(idx1*32 + 1*4,asuint(cc.y));
            indirect_buffer.Store(idx1*32 + 2*4,asuint(cc.z));
            indirect_buffer.Store(idx1*32 + 3*4 ,asuint( cc.w));
            indirect_buffer.Store(idx1*32 + 4*4 , asuint( pos.x));
            indirect_buffer.Store(idx1*32 + 5*4 , asuint(pos.y));            
            indirect_buffer.Store(idx1*32 + 6*4 , asuint(pos.z));
            indirect_buffer.Store(idx1*32 + 7*4,asuint( 0.0));
            return;
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
        	return ;
        cur_idx += sub_idx;
        node = node_pool[cur_idx];
    }
    return ;
}

[numthreads(1,1,1)]
void main(uint3 id : SV_DispatchThreadID)
{


    uint y = id.y;
    uint z = id.z;
    uint x = id.x;
    uint size;
    brick_pool.GetDimensions(size, size, size);
    float3 p = float3(x,y,z);
    trace_svo(p,size);
}