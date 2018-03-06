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
  float3 normal:NORMAL;
  float3 color : COLOR;
  float2 tex : TEXCOORD;
};

cbuffer GSBuffer:register(b1)
{
  int screen_size_x;
  int screen_size_y;
  int s;
}

GOut gv[9];
int num[3];

#define POINT 0

void processVertex(VOut vert,float2 n1,float2 n2,int base_idx,

#if POINT
inout PointStream<GOut> stream)
#else
inout TriangleStream<GOut> stream)
#endif
{
  float dx = 2.0/screen_size_x;
  float dy = 2.0/screen_size_y;
  float2 dxy = float2(dx,dy);

  float2 a = sign(n1);
  float2 b = sign(n2);

  //a = float2(n1.x/abs(n1.x),n1.y/abs(n1.y));
  //b = float2(n2.x/abs(n2.x),n2.y/abs(n2.y));


  if(a.x==b.x&&a.y==b.y)
  {

    //同一象限
    float4 newp0 = float4(a*dxy*vert.position.w+vert.position.xy,vert.position.zw);
    GOut v1;
    v1.position = newp0;
    v1.normal = vert.normal;
    v1.tex = vert.tex;
    v1.color = float3(1,0,0);

    gv[base_idx*3+0] = v1;
    num[base_idx] = 1;
    #if POINT
    stream.Append(v1);
    #endif
    //v1.position.y += 2*dxy*vert.position.w;
     //stream.Append(v1);


  }
else
 if(a.x==-b.x&&a.y==-b.y)
  {

    //对角象限
    float2 nn = normalize(n2) + normalize(n1);
    if(length(nn)<0.0001)
    {
      GOut v1;
          v1.position = float4(vert.position.xy + 3*dxy*vert.position.w ,0.5,1);
    v1.normal = vert.normal;
    v1.tex = vert.tex;
     v1.color = float3(1,0,1);
     #if POINT
     stream.Append(v1);
     #endif
    }

    float2 on = sign(nn);
    //on = float2(on.x/abs(on.x),on.y/abs(on.y));

    GOut v1;
    GOut v2;
    GOut v3;

    float4 newp01 = float4(on*dxy*vert.position.w + vert.position.xy,vert.position.zw);
    float4 newp02 = float4(a*dxy*vert.position.w + vert.position.xy,vert.position.zw);
    float4 newp03 = float4(b*dxy*vert.position.w + vert.position.xy,vert.position.zw);


    v1.position = newp01;
    v1.normal = vert.normal;
    v1.tex = vert.tex;
    v2.position = newp02;
    v2.normal = vert.normal;
    v2.tex = vert.tex;
    v3.position = newp03;
    v3.normal = vert.normal;
    v3.tex = vert.tex;

    v1.color = float3(1,0,0);
    v2.color = float3(0,1,0);
    v3.color = float3(0,0,1);

    gv[base_idx*3 + 0] = v2;
    gv[base_idx*3 + 1] = v1;
    gv[base_idx*3 + 2] = v3;
    
    #if POINT
    stream.Append(v1);
    
    stream.Append(v2);
    
    stream.Append(v3);
    #endif
    num[base_idx] = 3;

  }

  else
  {
/*
    if(!((a.x == b.x&&a.y==-b.y) || (a.x==-b.x&&a.y==b.y)))
    {
      GOut v1;
          v1.position = float4(float2(5,5)*dxy ,0.5,1);
    v1.normal = vert.normal;
    v1.tex = vert.tex;
     v1.color = float3(1,0,1);
     stream.Append(v1);
     
     return;
    }
    */
    //相邻象限
    GOut v1;
    GOut v2;
    float4 newp02 = float4(a*dxy*vert.position.w + vert.position.xy,vert.position.zw);
    float4 newp03 = float4(b*dxy*vert.position.w + vert.position.xy,vert.position.zw);
    v1.position = newp02;
    v1.normal = vert.normal;
    v1.tex = vert.tex;
    v2.position = newp03;
    v2.normal = vert.normal;
    v2.tex = vert.tex;

    v1.color = float3(0,0,1);
    v2.color = float3(0,0,1);

    gv[base_idx*3 + 0] = v1;
    gv[base_idx*3 + 1] = v2;
    num[base_idx] = 2;
    #if POINT
    stream.Append(v1);
    stream.Append(v2);
    #endif
  }

}


[maxvertexcount(18)]
void main(triangle VOut gin[3],
#if POINT
inout PointStream<GOut> stream)
#else
inout TriangleStream<GOut> stream)
#endif
{




  if(s>0)
  {
    
        stream.Append(gin[0]);
        
    stream.Append(gin[1]);
    
    stream.Append(gin[2]);
    stream.RestartStrip();
    return ;
  }
  
num[0] = num[1] = num[2] = 0;
#if 1
  float4 p0 = gin[0].position/gin[0].position.w;
  float4 p1 = gin[1].position/gin[1].position.w;
  float4 p2 = gin[2].position/gin[2].position.w;
  float dx = 2.0/screen_size_x;
  float dy = 2.0/screen_size_y;
  float2 dxy = float2(dx,dy);



  float2 normal01 = cross(float3(p0.xy,1),float3(p1.xy,1));
  float2 normal12 = cross(float3(p1.xy,1),float3(p2.xy,1));
  float2 normal20 = cross(float3(p2.xy,1),float3(p0.xy,1));
  
  float2 e1 = p1.xy - p0.xy;
  float2 e2 = p2.xy - p1.xy;
  float2 e3 = p0.xy - p2.xy;

  if(cross(float3(-e1,0),float3(e2,0)).z<0)
  {
    normal01 *= -1;
    normal12 *= -1;
    normal20 *= -1;
  }
//gin[0].position.xy += normal01.xy;
/*
  float2 o01 = sign(normal01);//*dxy;
  float2 o12 = sign(normal12);//*dxy;
  float2 o20 = sign(normal20);//*dxy;
*/
//normal
#if POINT
{

  GOut vl;
  vl.position = float4((p0 + p1)/2 + 0.1*(normalize(normal01)),0.5,1);
  vl.normal = float3(1,1,1);
  vl.color = float3(0,1,0);
  vl.tex = float2(0,0);
  
  stream.Append(vl);
  
  
  vl.position = float4((p0 + p1).xy/2,0.5,1);

  stream.Append(vl);
  
  vl.position = float4((p2 + p1)/2 + 0.1*normalize(normal12),0.5,1);
  vl.normal = float3(1,1,1);
  vl.color = float3(0,1,0);
  vl.tex = float2(0,0);
  stream.Append(vl);

  vl.position = float4((p2 + p1).xy/2,0.5,1);
  stream.Append(vl);
  
  vl.position = float4((p0 + p2)/2 + 0.1*normalize(normal20),0.5,1);
  vl.normal = float3(1,1,1);
  vl.color = float3(0,1,0);
  vl.tex = float2(0,0);
  stream.Append(vl);

  vl.position = float4((p0 + p2).xy/2,0.5,1);
  stream.Append(vl);
  
}
#endif
  
  //2001
  processVertex(gin[0],normal20,normal01,0,stream);
  //0112
  processVertex(gin[1],normal01,normal12,1,stream);
  //1220
  processVertex(gin[2],normal12,normal20,2,stream);

  GOut fas[9];
  fas[0] = gv[0];
  fas[1] = gv[0];
  fas[2] = gv[0];
  fas[3] = gv[0];
  fas[4] = gv[0];
  fas[5] = gv[0];
  fas[6] = gv[0];
  fas[7] = gv[0];
  fas[8] = gv[0];

  int n = 0;


  GOut a = gv[0];

  int i = 0;
  for( i=1;i<num[0];i++)
  {
    fas[n++] = gv[i];
  }
  i = 0;
  for( i=0;i<num[1];++i)
  {
    fas[n++] = gv[3 + i];
  }
  i = 0;
  for( i=0;i<num[2];++i)
  {
    fas[n++] = gv[6 + i];
  }

  i = 0;

#if !POINT
  for( i=0;i<n-1;++i)
  {
    stream.Append(a);
    stream.Append(fas[i]);
    stream.Append(fas[i+1]);
    stream.RestartStrip();
  }
  #endif

#else

#endif
/*
gin[0].position.y = 1.01*gin[0].position.w;
  gin[0].position.y += s*dy*gin[0].position.w;
  gin[1].position.y = -1*25*dy*gin[1].position.w;
  gin[2].position.y = -0.9*25*dy*gin[2].position.w;
    stream.Append(gin[0]);
    stream.Append(gin[1]);
    stream.Append(gin[2]);
    stream.RestartStrip();
    
    */

}