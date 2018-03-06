
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
};
struct VOut
{
  float4 position : SV_POSITION;
  float3 normal:NORMAL;
  float3 color : COLOR;
  float2 tex : TEXCOORD;
};
VOut main(VSIn input)
{
  VOut output;
  float4 mo = float4(input.position,1);
  mo = mul(mo,m);
  mo = mul(mo,v);
  mo = mul(mo,o);
  output.position = mo;
  output.normal = mul(input.normal,(float3x3)m);
  output.normal = mul(input.normal,(float3x3)v);
  output.normal = normalize(output.normal);
  output.tex = input.tex; 
  output.color = float3(1,1,1);
  return output;
}