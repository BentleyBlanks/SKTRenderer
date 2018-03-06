cbuffer MatrixBuffer:register(b0)
{
  matrix m;
  matrix v;
  matrix o;
};
struct VSIn
{
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 tex : TEXCOORD;
  float3 tangent : TANGENT;
};
struct VOut
{
  float4 position : SV_POSITION;
  float3 positionw:POSITIONW;
  float3 normal:NORMAL;
  float3 tangent : TANGENT;
  float3 viewu : VIEWU;
  float3 viewv : VIEWV;
  float2 tex : TEXCOORD;
  float2 rec : POSITIONP;

};
cbuffer CB:register(b1)
{
  float zplus;
};

VOut VsMain(VSIn input)
{
  VOut output;
  float4 mo = float4(input.position,1);
  
  mo = mul(mo,m);
  //output.positionw = mo.xyz;
  mo = mul(mo,v);
  //mo.z  = mo.z + zplus;
  output.positionw = mo.xyz;
  mo = mul(mo,o);
  output.position = mo;
  output.normal = mul(input.normal,(float3x3)m);
    output.normal = mul(output.normal, (float3x3) v);
  output.normal = normalize(output.normal);
  output.tex = input.tex;
  //
  //https://mynameismjp.wordpress.com/2009/03/10/reconstructing-position-from-depth/
  //http://www.cnblogs.com/graphics/archive/2012/07/25/2582119.html
  //do not divide first because of the interpolation
  //output.positionw.w = output.position.z / output.position.w;
  //这个值最后写出在叫做深度buffer的buffer上，但本身并不是深度，w才是深度
  //z是fn/(n-f)+Pz*f/(f-n),w是Pz,两个值相除是非线性深度，而且越远差距越大
  output.rec.xy = output.position.zw;
  output.tangent = mul(input.tangent,(float3x3)m);
  output.tangent = mul(input.tangent,(float3x3)v);
  output.tangent = normalize(output.tangent);
  float3 model_u = float3(1,0,0);
  float3 model_v = float3(0,-1,0);
  model_u = mul(model_u,(float3x3)m);
  output.viewu = mul(model_u,(float3x3)v);
  model_v = mul(model_v,(float3x3)m);
  output.viewv = mul(model_v,(float3x3)v);
  return output;
}