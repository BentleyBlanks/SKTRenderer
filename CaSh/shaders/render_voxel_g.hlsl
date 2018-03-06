cbuffer CB
{
  matrix v;
  matrix p;
};

struct VOut
{  
    float4 Pos : SV_POSITION;
};

Texture3D<uint> voxel_tex;

struct GOut 
{
  float4 pos : SV_POSITION;
  float3 color : COLOR;
};

float4 toFloat4(uint value)
{
  return float4(float((value & 0x000000FF)), float((value & 0x0000FF00) >> 8U), float((value & 0x00FF0000) >> 16U), float((value & 0xFF000000) >> 24U)) / 255.0f;
}

[maxvertexcount(36)]
void main(point VOut gin[1],inout TriangleStream<GOut> stream)
{
  int3 vp = gin[0].Pos.xyz;
  uint a = voxel_tex.Load(int4(vp,0));
  if(a==0) return;
  const float3 offset[] = {
    {0.5,0.5,0.5},
    {0.5,-0.5,0.5},
    {0.5,-0.5,-0.5},
    {0.5,0.5,-0.5},
    {-0.5,0.5,0.5},
    {-0.5,-0.5,0.5},
    {-0.5,-0.5,-0.5},
    {-0.5,0.5,-0.5}
  };

  float3 verts[8];

  [unroll]
  for(int i=0;i<8;i++)
  {
    verts[i] = vp + offset[i];
  }

  GOut op;

  op.color = toFloat4(a).xyz;
  op.pos = float4(verts[6],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
    op.pos = float4(verts[7],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
    op.pos = float4(verts[3],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);


  stream.RestartStrip();

    op.pos = float4(verts[6],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
    op.pos = float4(verts[3],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
    op.pos = float4(verts[2],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);

  stream.RestartStrip();

    op.pos = float4(verts[2],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
    op.pos = float4(verts[3],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
    op.pos = float4(verts[0],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
  stream.RestartStrip();

    op.pos = float4(verts[2],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
    op.pos = float4(verts[0],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
    op.pos = float4(verts[1],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
  stream.RestartStrip();

    op.pos = float4(verts[7],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
    op.pos = float4(verts[4],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
    op.pos = float4(verts[0],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
  stream.RestartStrip();

    op.pos = float4(verts[7],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
    op.pos = float4(verts[0],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
    op.pos = float4(verts[3],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
  stream.RestartStrip();

    op.pos = float4(verts[5],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
    op.pos = float4(verts[4],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
    op.pos = float4(verts[7],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
  stream.RestartStrip();


    op.pos = float4(verts[5],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
    op.pos = float4(verts[7],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
    op.pos = float4(verts[6],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);

  stream.RestartStrip();

    op.pos = float4(verts[0],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
    op.pos = float4(verts[4],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
    op.pos = float4(verts[5],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
  stream.RestartStrip();


    op.pos = float4(verts[0],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
    op.pos = float4(verts[5],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
    op.pos = float4(verts[1],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
  stream.RestartStrip();


    op.pos = float4(verts[6],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
    op.pos = float4(verts[2],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
    op.pos = float4(verts[1],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
  stream.RestartStrip();


    op.pos = float4(verts[6],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
    op.pos = float4(verts[1],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);
    op.pos = float4(verts[5],1);
  op.pos = mul(op.pos,v);
  op.pos = mul(op.pos,p);
  stream.Append(op);

  stream.RestartStrip();

}