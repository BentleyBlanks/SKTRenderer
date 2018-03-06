cbuffer CB
{
  matrix v;
  matrix p;
};

struct VSOut
{
  float4 color : COLOR;  
  float4 Pos : POSITION;
};

struct GOut 
{
  float4 pos : POSITION;
  float3 color : COLOR;
};


[maxvertexcount(36)]
void main(point VSOut gin[1],inout TriangleStream<GOut> stream)
{
  int3 vp = gin[0].Pos.xyz;
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

  op.color = gin[0].color;
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