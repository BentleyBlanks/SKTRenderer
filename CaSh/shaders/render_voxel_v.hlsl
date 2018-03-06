struct VS_Output
{  
    float4 Pos : SV_POSITION;
};

cbuffer CB
{
  matrix m;
};

VS_Output main(uint id : SV_VertexID)
{
  const float voxel_size = 128;
    VS_Output Output;
    float y = id/(voxel_size*voxel_size);
    float z = id%(voxel_size*voxel_size)/voxel_size;
    float x = id%(voxel_size*voxel_size)%voxel_size;
    
    Output.Pos = float4(float3(x+0.5,y+0.5,z+0.5),1);
    Output.Pos = mul(Output.Pos,m);
    //Output.Pos = mul(Output.Pos,v);
    //Output.Pos = mul(Output.Pos,p);
    return Output;
}

