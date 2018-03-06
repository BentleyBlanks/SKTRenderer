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
    //��ǰ���Ľڵ�ƫ��
    uint cur_node_pool_offset;
    //��ǰ���Ľڵ�����
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
Texture3D<uint> brick_pool : register(t0);
RWStructuredBuffer<uint> node_pool : register(u0);
RWStructuredBuffer<Param> info : register(u1);


void alloc_brick(uint parent_offset)
{
    float3 size;
    brick_pool.GetDimensions(size.x,size.y,size.z);
    size /= 3;
    uint off = 1;
    //3x3x3
    node_pool[parent_offset] += info[0].brick_offset + off;
    InterlockedAdd(info[0].brick_number, 1);

}

void alloc_node_address(uint id)
{

    uint parent_offset = info[0].cur_node_pool_offset - info[0].parent_level_number + id + 1;
    if(cur_levels==0)
    {
        parent_offset -= 1;
        //info[0].brick_offset += 1;
    }
    uint parent = node_pool[parent_offset];
    [branch]
    if (parent & FLAG_SUBDIV)
    {
        InterlockedAdd(info[0].avaliable_node, 1);
        uint d;
        InterlockedAdd(info[0].child_number, 8, d);
        node_pool[parent_offset]|= (info[0].cur_node_pool_offset + d + 1);   
    }
    else
    {
        //if(cur_levels==max_levels+1)
          //  alloc_brick(parent_offset);
    }

}

[numthreads(1,1,1)]
void main(uint3 idx : SV_DispatchThreadID)
{
    uint id = idx.x + idx.y*(voxel_size*voxel_size) + idx.z*voxel_size;
    if(id >= info[0].divided_number)
        return;
    alloc_node_address(id);
}