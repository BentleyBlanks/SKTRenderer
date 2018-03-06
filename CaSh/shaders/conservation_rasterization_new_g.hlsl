struct VOut
{
  float4 position : SV_POSITION;
  float3 normal:NORMAL;
  float3 color : COLOR;
  float2 tex : TEXCOORD;
};

struct GOut
{
  float4 position : SV_POSITION;
  float3 normal : NORMAL;
  float3 color : COLOR;
  float2 tex : TEXCOORD;
  noperspective float2 vp : POSITION1;
  nointerpolation float4 aabb : AABB;
};

cbuffer GSBuffer
{
  int screen_size_x;
  int screen_size_y;
  float scale;
};

void proc_aabb(float2 pt,inout float4 aabb)
{
    if(pt.x>aabb.z) aabb.z = pt.x;
    if(pt.x<aabb.x) aabb.x = pt.x;
    if(pt.y>aabb.w) aabb.w = pt.y;
    if(pt.y<aabb.y) aabb.y = pt.y;
}

[maxvertexcount(3)]
void main(triangle VOut gin[3],
#ifdef DBPOINT
inout PointStream<GOut> stream)
#else
inout TriangleStream<GOut> stream)
#endif
{
    //minx,miny,maxx,maxy
    float4 aabb = float4(1,1,-1,-1);
    float4 p0 = gin[0].position/gin[0].position.w;
    float4 p1 = gin[1].position/gin[1].position.w;
    float4 p2 = gin[2].position/gin[2].position.w;
    float dx = 2.0/screen_size_x;
    float dy = 2.0/screen_size_y;
    float2 dxy = float2(dx,dy);
    float2 e1 = p1.xy - p0.xy;
    float2 e2 = p2.xy - p1.xy;
    float2 e3 = p0.xy - p2.xy;
    

    if(cross(float3(-e1,0),float3(e2,0)).z<0)
    {
        e1 = -e1;
        e2 = -e2;
        e3 = -e3;
    }

    //handle degenerate triangle,ensure there is no zore vector be operated by cross 
    if(length(e1)<0.0001)
    {
        e1 = min(dx,dy);
    }
    if(length(e2)<0.0001)
    {
        e2 = min(dx,dy);
    }    
    if(length(e3)<0.0001)
    {
        e3 = min(dx,dy);
    }
    float2 normal01 = cross(float3(e1,0),float3(0,0,-1)).xy;
    float2 normal12 = cross(float3(e2,0),float3(0,0,-1)).xy;
    float2 normal20 = cross(float3(e3,0),float3(0,0,-1)).xy;
    
    normal01 = normalize(normal01);
    normal12 = normalize(normal12);
    normal20 = normalize(normal20);

    float hl = scale*length(dxy)*0.5;
    float2 p0n = p0.xy + hl*(e1/dot(e1,normal20)+e3/dot(e3,normal01));
    float2 p1n = p1.xy + hl*(e2/dot(e2,normal01)+e1/dot(e1,normal12));
    float2 p2n = p2.xy + hl*(e3/dot(e3,normal12)+e2/dot(e2,normal20));


    proc_aabb(p0.xy,aabb);
    proc_aabb(p1.xy,aabb);
    proc_aabb(p2.xy,aabb);
    aabb.zw += dxy/2;
    aabb.xy -= dxy/2;


    float2 p0l1 = p0.xy + hl*normal01;
    float2 p0l2 = p0.xy + hl*normal20;
    float2 p1l2 = p1.xy + hl*normal12;
    float2 p1l0 = p1.xy + hl*normal01;
    float2 p2l0 = p2.xy + hl*normal20;
    float2 p2l1 = p2.xy + hl*normal12;


    //make aabb strong
    proc_aabb(p0l1,aabb);
    proc_aabb(p0l2,aabb);
    proc_aabb(p1l2,aabb);
    proc_aabb(p1l0,aabb);
    proc_aabb(p2l1,aabb);
    proc_aabb(p2l0,aabb);
      

#ifndef DBPOINT
    GOut output;





    output.position = float4(p0n*gin[0].position.w,gin[0].position.zw);
    output.normal = gin[0].normal;
    output.color = gin[0].color;
    output.tex = gin[0].tex;
    output.aabb = aabb;
    output.vp = p0n;
    stream.Append(output);

    output.position = float4(p1n*gin[1].position.w,gin[1].position.zw);
    output.normal = gin[1].normal;
    output.color = gin[1].color;
    output.tex = gin[1].tex;
    output.aabb = aabb;
    output.vp = p1n;
    stream.Append(output);

    output.position = float4(p2n*gin[2].position.w,gin[2].position.zw);
    output.normal = gin[2].normal;
    output.color = gin[2].color;
    output.tex = gin[2].tex;
    output.aabb = aabb;
    output.vp = p2n;
    stream.Append(output);

    stream.RestartStrip();


    
#endif

#ifdef DBPOINT

//  ////modify
    output.position = float4((p0 + p1)/2 + 0.1*(normalize(normal01)),0.5,1);//gin[0].position;
    output.normal = gin[0].normal;
    output.color = float3(1,0,0);
    output.tex = gin[0].tex;
    output.aabb = aabb;
    stream.Append(output);

    output.position = float4((p1 + p2)/2 + 0.1*(normalize(normal12)),0.5,1);//gin[1].position;
    output.normal = gin[1].normal;
    output.color = float3(1,0,0);
    output.tex = gin[1].tex;
    output.aabb = aabb;
    stream.Append(output);
    

    output.position = float4((p0 + p2)/2 + 0.1*(normalize(normal20)),0.5,1);//gin[2].position;
    output.normal = gin[2].normal;
    output.color = float3(1,0,0);
    output.tex = gin[2].tex;
    output.aabb = aabb;
    stream.Append(output);

//origin
    output.position = gin[0].position;
    output.normal = gin[0].normal;
    output.color = float3(1,1,0);
    output.tex = gin[0].tex;
    output.aabb = aabb;
    stream.Append(output);

    output.position = gin[1].position;
    output.normal = gin[1].normal;
    output.color = float3(1,1,0);
    output.tex = gin[1].tex;
    output.aabb = aabb;
    stream.Append(output);
    

    output.position = gin[2].position;
    output.normal = gin[2].normal;
    output.color = float3(1,1,0);
    output.tex = gin[2].tex;
    output.aabb = aabb;
    stream.Append(output);

//AABB
    output.position = float4(aabb.xy,0.5,1);//gin[2].position;
    output.normal = gin[2].normal;
    output.color = float3(1,0,1);
    output.tex = gin[2].tex;
    output.aabb = aabb;
    stream.Append(output);


    output.position = float4(aabb.zw,0.5,1);//gin[2].position;
    output.normal = gin[0].normal;
    output.color = float3(1,0,1);
    output.tex = gin[0].tex;
    output.aabb = aabb;
    stream.Append(output);

        output.position = float4(aabb.xw,0.5,1);//gin[2].position;
    
    output.normal = gin[1].normal;
    output.color = float3(1,0,1);
    output.tex = gin[1].tex;
    output.aabb = aabb;
    stream.Append(output);
    

        output.position = float4(aabb.zy,0.5,1);//gin[2].position;
    
    output.normal = gin[2].normal;
    output.color = float3(1,0,1);
    output.tex = gin[2].tex;
    output.aabb = aabb;
    stream.Append(output);

//

float3 ccc = float3(0,0,1);
            output.position = float4(p0l1,0.5,1);//gin[2].position;
    output.normal = gin[2].normal;
    output.color = ccc;
    output.tex = gin[2].tex;
    output.aabb = aabb;
    stream.Append(output);


        output.position = float4(p0l2,0.5,1);//gin[2].position;

    output.normal = gin[0].normal;
    output.color = ccc;
    output.tex = gin[0].tex;
    output.aabb = aabb;
    stream.Append(output);

        output.position = float4(p1l0,0.5,1);//gin[2].position;
    
    output.normal = gin[1].normal;
    output.color = ccc;
    output.tex = gin[1].tex;
    output.aabb = aabb;
    stream.Append(output);
    

        output.position = float4(p1l2,0.5,1);//gin[2].position;
    
    output.normal = gin[2].normal;
    output.color = ccc;
    output.tex = gin[2].tex;
    output.aabb = aabb;
    stream.Append(output);

            output.position = float4(p2l0,0.5,1);//gin[2].position;
    
    output.normal = gin[1].normal;
    output.color = ccc;
    output.tex = gin[1].tex;
    output.aabb = aabb;
    stream.Append(output);
    

        output.position = float4(p2l1,0.5,1);//gin[2].position;
    
    output.normal = gin[2].normal;
    output.color = ccc;
    output.tex = gin[2].tex;
    output.aabb = aabb;
    stream.Append(output);
#endif


    
}


