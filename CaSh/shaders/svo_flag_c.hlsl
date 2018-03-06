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
    //
    //uint list_number;
    //当前层的节点偏移
    uint cur_node_pool_offset;
    //当前层的节点数量
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

RWStructuredBuffer<uint> node_pool : register(u0);
RWStructuredBuffer<Param> info : register(u1);
RWStructuredBuffer<Voxel> voxel_list : register(u2);

//0~voxel_size position
float3 get_pos_from_voxel_list(uint id)
{
  return voxel_list[id].pos;
}

void flag_node_address(float3 vpos,float3 root_size)
{
    
    int level = 0;
    float3 cur_pos = vpos;
    float3 cur_size = root_size;
    int cur_idx = 0;
    uint node = node_pool[cur_idx];
    //uint cur_parent_node_head = cur_idx;
    [loop]
    while(level<=cur_levels)
    {
        
        [branch]
        if (level==cur_levels)
        {
            //never reach leaf
            //[branch]
            //if(cur_levels!=max_levels)
            //{
                
                
               // [branch]
                //if ((node & FLAG_SUBDIV)==0)
                //{
                    //InterlockedAdd(info[0].divided_number,1);
                    //InterlockedOr(node_pool[cur_idx], FLAG_SUBDIV);
                    //InterlockedCompareStore(node_pool[cur_idx], 0x00000000, FLAG_SUBDIV);
                    //uint origin;

            /*
            if (node_pool[cur_idx] == 0x00000000)
            {
                InterlockedAdd(info[0].divided_number, 1);
            }
*/
            InterlockedOr(node_pool[cur_idx], FLAG_SUBDIV);
            
                //}
            //}
            return;
        }
        //??where to put it??
        
        //InterlockedCompareStore(node,0,)
        float3 np = cur_pos / cur_size;
        uint sub_idx = uint(np.x + 0.5) + uint(np.y + 0.5) * 4 + uint(np.z + 0.5) * 2;
        
        cur_size /= 2;
        if(np.x>=0.5)
            cur_pos.x -= cur_size.x;
        if (np.y >= 0.5)
            cur_pos.y -= cur_size.x;
        if (np.z >= 0.5)
            cur_pos.z -= cur_size.x;

        level ++;

        if(cur_size.x==1)
            info[0].brick_offset = 0x8000001;

        cur_idx = node & 0x7fffffff;
        cur_idx += sub_idx;
        node = node_pool[cur_idx];
    }
    //root
    //return 0;
}

//标记node_pool自身
[numthreads(1,1,1)]
void main(uint3 idx : SV_DispatchThreadID)
{
  
  uint id = idx.x + idx.y*(voxel_size*voxel_size) + idx.z*voxel_size;
    //info[0].cur_node_pool_offset = id;
  if(id >= voxel_list_size) 
    return;

  float3 min_ = float3(0,0,0);
  float3 max_ = float3(voxel_size,voxel_size,voxel_size);
  float3 voxel_pos = get_pos_from_voxel_list(id);
  flag_node_address(voxel_pos,max_);

}
