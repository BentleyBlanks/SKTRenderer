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
    //µ±Ç°²ãµÄ½ÚµãÆ«ÒÆ
    uint cur_node_pool_offset;
    //µ±Ç°²ãµÄ½ÚµãÊýÁ¿
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
//per node
RWStructuredBuffer<uint> node_pool;
RWStructuredBuffer<Param> info;
struct Voxel
{
    float4 color;
    float3 pos;
};
RWStructuredBuffer<Voxel> voxel_list;

//RWStructuredBuffer<uint> counter;
uint convVec4ToRGBA8(float4 val)
{
    return (uint(val.w) & 0x000000FF) << 24U | (uint(val.z) & 0x000000FF) << 16U | (uint(val.y) & 0x000000FF) << 8U | (uint(val.x) & 0x000000FF);
}

void clear()
{
    info[0].brick_number = 0;
    info[0].brick_offset = 0;
    info[0].child_number = 0;
    info[0].parent_level_number = 0;
    info[0].cur_node_pool_offset = 0;
    info[0].divided_number = 1;
    info[0].total_node = info[0].divided_number;
    info[0].avaliable_node = 0;
    node_pool[0] = 0x00000000;
}

void clear_per_level()
{
    info[0].child_number = 0;
    //info[0].parent_level_number = 0;
    info[0].divided_number = 0;
    //all parent num
    info[0].divided_number = info[0].parent_level_number;

    info[0].total_node = info[0].divided_number;
    info[0].avaliable_node = 0;

}

void update_offset()
{
    info[0].cur_node_pool_offset += info[0].child_number;
    info[0].parent_level_number = info[0].child_number;
    //info[0].brick_offset += info[0].child_number;
}

#ifdef A
void update_divided_num(uint id)
{
    uint last_pos = info[0].cur_node_pool_offset + 1;

    
    //[loop]
    //while (last_pos <= )

    if (id >= voxel_list_size)
        return;
    float3 min_ = float3(0, 0, 0);
    float3 max_ = float3(voxel_size, voxel_size, voxel_size);




    int level = 0;
    float3 cur_pos = voxel_list[id].pos;;
    float3 cur_size = voxel_size;
    int cur_idx = 0;
    uint node = node_pool[cur_idx];
    [loop]
    while (level <= cur_levels)
    {
        
        [branch]
        if (level == cur_levels)
        {
            //never reach leaf
            [branch]
            if (cur_levels != max_levels)
            {
                
                
                [branch]
                if ((node & FLAG_SUBDIV) == 1)
                {
                    //InterlockedAdd(info[0].divided_number,1);
                    InterlockedOr(node_pool[cur_idx], FLAG_SUBDIV);
                }
            }
            return;
        }
        //??where to put it??
        //GroupMemoryBarrierWithGroupSync();
        //InterlockedCompareStore(node,0,)
        float3 np = cur_pos / cur_size;
        uint sub_idx = uint(np.x + 0.5) + uint(np.y + 0.5) * 4 + uint(np.z + 0.5) * 2;
        
        cur_size /= 2;
        if (np.x > 0.5)
            cur_pos.x -= cur_size.x;
        if (np.y > 0.5)
            cur_pos.y -= cur_size.x;
        if (np.z > 0.5)
            cur_pos.z -= cur_size.x;

        level++;
        cur_idx += sub_idx;
        node = node_pool[cur_idx];
    }

}
#endif

[numthreads(1, 1, 1)]
void main(uint id : SV_DispatchThreadID)
{
    switch (option)
    {
        case 0:
            clear();
            break;
        case 1:
            clear_per_level();
            break;
        case 2:
            update_offset();
            break;
            /*
        case 3:
            update_divided_num(id);
            break;
            
        case 3:
            generate_draw_call_args();
            break;
        case 4:
*/

    }
}
