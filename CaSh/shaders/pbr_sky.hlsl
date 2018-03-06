cbuffer cbuffer
{
    float fov_x;
    float fov_y;
    float view_panel_dist;
    float3 sphere_pos_v;
    //matrix inv_projection;

};


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

float 

float4 main_ps(VS_Output pin): SV_TARGET0
{
    float earthR = 6360000;
    float  atmosphereR = 6420000;
    float hr = 7994;
    float hm = 1200;


    float tx = 2*pin.Tex.x - 1;
    float ty = 2*(1-pin.Tex.y) - 1;
    float vx = tx * view_panel_dist * tan(fov_x);
    float vy = ty * view_panel_dist * tan(fov_y);
    float3 view_dir = float3(vx,vy,view_panel_dist);
    view_dir = nomalize(view_dir);
}
