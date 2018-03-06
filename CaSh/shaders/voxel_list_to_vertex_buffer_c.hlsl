cbuffer CB
{
    int cur_levels;
    int voxel_size;
    uint voxel_list_size;
    int max_levels;
    uint option;
};
struct Voxel
{
    float4 color;
    float3 pos;
};
RWByteAddressBuffer indirect_buffer : register(u0);
RWStructuredBuffer<Voxel> voxel_list : register(u1);

[numthreads(1,1,1)]
void main(uint3 idx : SV_DispatchThreadID)
{
    uint id = idx.x + idx.y*(voxel_size*voxel_size) + idx.z*voxel_size;
    if(id>=voxel_list_size) return;
    Voxel v = voxel_list[id];
    float3 pos = v.pos;
    float4 cc = v.color;
    indirect_buffer.Store(id*32 ,asuint(cc.x));
    indirect_buffer.Store(id*32 + 1*4,asuint(cc.y));
    indirect_buffer.Store(id*32 + 2*4,asuint(cc.z));
    indirect_buffer.Store(id*32 + 3*4 ,asuint( cc.w));
    indirect_buffer.Store(id*32 + 4*4 , asuint( pos.x));
    indirect_buffer.Store(id*32 + 5*4 , asuint(pos.y));            
    indirect_buffer.Store(id*32 + 6*4 , asuint(pos.z));
    indirect_buffer.Store(id*32 + 7*4,asuint( 1.0));
}