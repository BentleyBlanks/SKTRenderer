//per child
#define FLAG_SUBDIV 0x80000000


cbuffer CB
{
    int cur_levels;
    int voxel_size;
    uint voxel_list_size;
    int max_levels;
    uint option;
};


struct Param
{
    int cur_node_pool_offset;
    uint parent_level_number;
    //
    uint child_number;
    //
    uint divided_number;
    //
    uint brick_offset;
    //
    uint brick_number;

    //debug
    uint total_node;
    uint avaliable_node;

};
struct Voxel
{
    float4 color;
    float3 pos;
};
//per node
RWTexture3D<uint> brick_pool : register(u0);
RWStructuredBuffer<uint> node_pool : register(u1);
RWStructuredBuffer<Param> info : register(u2);
StructuredBuffer<Voxel> voxel_list : register(t0);

//RWStructuredBuffer<uint> counter;
uint convVec4ToRGBA8(float4 val)
{
    return (uint(val.w) & 0x000000FF) << 24 | (uint(val.z) & 0x000000FF) << 16 | (uint(val.y) & 0x000000FF) << 8 | (uint(val.x) & 0x000000FF);
}

void init_leaf(uint id,uint size)
{
    float3 vpos = voxel_list[id].pos;

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
            
                //uint addr = node & 0x7fffffff;
                uint save_addr = 0;
                InterlockedAdd(info[0].brick_offset, 1, save_addr);
                uint idx = save_addr;
                uint y = idx / (size * size) ;
                uint z = idx % (size * size) / size ;
                uint x = idx % (size * size) % size ;
                brick_pool[uint3(x,y,z)] = convVec4ToRGBA8(voxel_list[id].color*255);
                uint data = 0x00000000|save_addr;
                InterlockedCompareStore(node_pool[cur_idx],0x00000000,data);
                //InterlockedOr(node_pool[cur_idx],save_addr);
                //InterlockedAnd(node_pool[cur_idx],0x7fffffff);
                
                /*
                size /= 3;
                uint idx = addr;
                uint y = idx / (size * size) * 3;
                uint z = idx % (size * size) / size * 3;
                uint x = idx % (size * size) % size * 3;
                [unroll]
                for (int i = 0; i < 3; ++i)
                    [unroll]
                    for (int j = 0; j < 3; ++j)
                        [unroll]
                        for (int k = 0; k < 3; ++k)
                        {
                            brick_pool[uint3(x + i, y + j, z + k)] = convVec4ToRGBA8(voxel_list[id].color);
                        }
*/
            
            return;
        }
        //??where to put it??
        //GroupMemoryBarrierWithGroupSync();
        //InterlockedCompareStore(node,0,)
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
            info[0].brick_number = 111;
        cur_idx += sub_idx;
        node = node_pool[cur_idx];
    }
}


void init_inner(uint id)
{
    //cur_node_pool_offset already update
    uint offset = info[0].cur_node_pool_offset - info[0].parent_level_number + id + 1;
    node_pool[offset] = 0x00000000;
}

[numthreads(1, 1, 1)]
void main_leaf(uint3 id : SV_DispatchThreadID)
{
    uint idx = id.x + id.y*(voxel_size*voxel_size) + id.z*voxel_size;
    if(idx>=voxel_list_size)
        return;
    uint size1;
    brick_pool.GetDimensions(size1, size1, size1);
    init_leaf(idx,size1);
}


[numthreads(1, 1, 1)]
void main_inner(uint3 idx : SV_DispatchThreadID)
{
    
    uint id = idx.x + idx.y*(voxel_size*voxel_size) + idx.z*voxel_size;
    if(id>=info[0].child_number)
        return;
        
    init_inner(idx.x);
}
