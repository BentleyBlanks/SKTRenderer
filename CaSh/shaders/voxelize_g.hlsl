struct GOut
{
  float4 position : SV_POSITION;
  float3 normal : NORMAL;
  float3 color : COLOR;
  float2 tex : TEXCOORD;
  noperspective float2 vp : POSITION1;
  float3 vpw : POSITION2;  
  nointerpolation uint main_axis_index : INDEX;
  nointerpolation float4 aabb : AABB;

};

struct VOut
{
  float4 position : SV_POSITION;
  float3 normal:NORMAL;
  float2 tex : TEXCOORD;
};

cbuffer GSBuffer
{
  matrix v;
  matrix o;
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


//3dmax 512 <=> 1000x1000


[maxvertexcount(3)]
void main(triangle VOut gin[3],inout TriangleStream<GOut> stream)
{
    float3 ccc = float3(1,1,0);
    const float3 axis[] = {
        float3(1,0,0),
        float3(0,1,0),
        float3(0,0,1),
    };
    float3x3 m33;
    m33[0] = axis[0];
    m33[1] = axis[1];
    m33[2] = axis[2];

    float4 v0 = gin[0].position;
    float4 v1 = gin[1].position;
    float4 v2 = gin[2].position;

    float3 ev01 = (v1 - v0).xyz;
    float3 ev12 = (v2 - v1).xyz;
    float3 ev20 = (v0 - v2).xyz;

    float3 nw = normalize(cross(-ev01,ev12));

    float3 s = mul(m33,nw);
    float3 sabs = abs(s);
    float max_axis = max(sabs.x,max(sabs.y,sabs.z));
    int idx = -10;
    if(max_axis==sabs.x)
        idx = 1*sign(s.x);
    else if(max_axis==sabs.y)
        idx = 2*sign(s.y);
    else if(max_axis==sabs.z)
        idx = 3*sign(s.z);

    float4 new_vert[3];
    float3 posw[3];// = {float3((p0n + 1)*0.5,gin[0].position.z),float3((p1n + 1)*0.5,gin[1].position.z),float3((p2n + 1)*0.5,gin[2].position.z)};
    uint ma[3]={2,2,2};
    for(int j=0;j<3;++j)
    {


        idx = -abs(idx);ma[j] = -idx;
        if(idx==-1)
        {
               new_vert[j] = float4(gin[j].position.zyx,1);
//new_vert[j].x*=-1;
            new_vert[j] = mul(new_vert[j],v);
            new_vert[j] = mul(new_vert[j],o);
            //new_vert[j].x*=-1;
            gin[j].position.xyz = new_vert[j].xyz;
            
//gin[j].position.x=-2;
//gin[j].position.z+=0.8;
            //gin[j].position.xyz = new_vert[j].zyx;
//new_vert[j].x*=-1;
//new_vert[j].z-=0.5;
//new_vert[j].z*=2;
//new_vert[j].x+=1;
           posw[j] = new_vert[j].xyz;
           //posw[j].z += 1;
//posw[j].z -= 0.5;
           //posw[j].z *= 0.5;
           

        }
        else if(idx==-2)
        {    
        new_vert[j] = float4(gin[j].position.xzy,1);
//new_vert[j].y*=-1;
            new_vert[j] = mul(new_vert[j],v);
        new_vert[j] = mul(new_vert[j],o);
//new_vert[j].y*=-1;
//new_vert[j].z-=1/128.f;
//new_vert[j].y*=-0.4;
//new_vert[j].z += 0.5;
        //gin[j].position.xyz = new_vert[j].xzy;
        //gin[j].position.y*=0.5;

    gin[j].position.xyz = new_vert[j].xyz;
        //gin[j].position.z*=0.05;
//new_vert[j].y*=-1;

        posw[j] = new_vert[j].xyz;
  //         posw[j].z += 1;
    //       posw[j].z *= 0.5;



        }
        
        else if(idx==-3)
        { 
           new_vert[j] = float4(gin[j].position.xyz,1);
           new_vert[j] = mul(new_vert[j],v);
        new_vert[j] = mul(new_vert[j],o);
        gin[j].position.xyz = new_vert[j].xyz;
        //gin[j].position.z *= 0.5;
        //gin[j].position.x -= 1;
        posw[j] = new_vert[j].xyz;

        }
        
        
        else
            ccc = float3(1,0,0);
            /*
            new_vert[j] = float4(gin[j].position.xzy,1);
new_vert[j].y*=-1;
            new_vert[j] = mul(new_vert[j],v);
        new_vert[j] = mul(new_vert[j],o);
new_vert[j].y*=-1;

        gin[j].position.xyz = new_vert[j].xzy;
        gin[j].position.z*=0.5;
        gin[j].position.y*=2;
        */
gin[j].position.w = 1;
            //gin[j].position.xyz = new_vert[j].zyx;
            //gin[j].position.x*=2;
//gin[j].position.z*=2;

//gin[j].position.z*=2;
//gin[j].position.y*=0.5;
            //new_vert[j] = mul(new_vert[j],v);
        //new_vert[j] = mul(new_vert[j],o);
  //      gin[j].position = new_vert[j];
        
//        gin[j].position = mul(gin[j].position,v);
  //      gin[j].position = mul(gin[j].position,o);
        //posw[j] = 
        
    }

#ifdef NOCR
    GOut output;
    float2 ass[3] = {gin[0].position.xy,gin[1].position.xy,gin[2].position.xy};
    [unroll] 
    for(int l =0;l<3;++l)
    {
        output.position = float4(ass[l]*gin[l].position.w,
            gin[l].position.zw);
        output.normal = mul(gin[l].normal,(float3x3)v);
        output.color = ccc;
        output.tex = gin[l].tex;
        output.aabb = float4(0,0,0,0);
        output.vp = ass[l];
        output.vpw = float3(posw[l].xy*0.5+0.5,posw[l].z);
        output.main_axis_index = ma[l];
        stream.Append(output);
    }
    stream.RestartStrip();
#endif

#ifndef CR
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





    //handle degenerate triangle,ensure there is no zore vector be operated by cross
    
    if(length(e1)<min(dx,dy)&&length(e2)<min(dx,dy)&&length(e3)<min(dx,dy))
    {
        return;
    }
    



    //if(dot(normalize(-e1),normalize(e2))>0)
    
    float2 normal01 = cross(float3(e1,0),float3(0,0,-1)).xy;
    float2 normal12 = cross(float3(e2,0),float3(0,0,-1)).xy;
    float2 normal20 = cross(float3(e3,0),float3(0,0,-1)).xy;
    
    normal01 = normalize(normal01);
    normal12 = normalize(normal12);
    normal20 = normalize(normal20);

    //just adjust normal donnot influce vertex
    float degenerate_degree = min(dx,dy)/1.5;
    float degenerate_fix = 20*max(dx,dy);///1.5;
    float le1 = length(e1);
    float le2 = length(e2);
    float le3 = length(e3);
    if(le1+le2<=le3+degenerate_degree)
    {
        //return;
        float2 temp = p1.xy - normal20*degenerate_fix;
        e1 = temp.xy - p0.xy;
        e2 = p2.xy - temp.xy;
        //p1.xy = temp;
    }
    else
    if(le1+le3<=le2+degenerate_degree)
    {
        //return;
        float2 temp = p0.xy - normal12*degenerate_fix;
        e1 = p1.xy - temp.xy;
        e3 = temp.xy - p2.xy;
        //p0.xy = temp;
    }
    else
    if(le3+le2<=le1+degenerate_degree)
    {
        //return;
        float2 temp = p2.xy - normal01*degenerate_fix;
        e2 = temp.xy - p1.xy;
        e3 = p0.xy - temp.xy;
        //p2.xy = temp;
    }
    normal01 = cross(float3(e1,0),float3(0,0,-1)).xy;
    normal12 = cross(float3(e2,0),float3(0,0,-1)).xy;
    normal20 = cross(float3(e3,0),float3(0,0,-1)).xy;
    normal01 = normalize(normal01);
    normal12 = normalize(normal12);
    normal20 = normalize(normal20);

    float hl = scale*length(dxy)*0.5f;
    
    if(cross(float3(-e1,0),float3(e2,0)).z<0)
        hl = -hl;
        


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
    float2 ass[3] = {p0n,p1n,p2n};
    [unroll] 
    for(int l =0;l<3;++l)
    {
        output.position = float4(ass[l]*gin[l].position.w,gin[l].position.zw);
        output.normal = mul(gin[l].normal,(float3x3)v);
        output.color = ccc;
        output.tex = gin[l].tex;
        output.aabb = aabb;
        output.vp = ass[l];
        output.vpw = float3(ass[l]*0.5+0.5,gin[l].position.z);
        output.main_axis_index = ma[l];
        stream.Append(output);
    }
    stream.RestartStrip();
#endif
#endif

}