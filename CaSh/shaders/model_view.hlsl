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
VOut main_vs(VSIn input)
{
  VOut output;
  float4 mo = float4(input.position,1);
  mo = mul(mo,m);
  mo = mul(mo,v);
  float4 vd = mo;
  mo = mul(mo,o);
  output.position = mo;
  output.normal = mul(input.normal,(float3x3)m);
  output.normal = mul(input.normal,(float3x3)v);
  output.normal = normalize(output.normal);
  output.tex = input.tex;
  output.color = float3(1,1,1);
  return output;
}
Texture2D dft;
SamplerState ss;
float4 main_ps(VOut pin) : SV_TARGET
{
  float3 d = dft.Sample(ss,pin.tex).xyz;
  float3 lighting = normalize(float3(1,1,-1));
  float4 ret = float4(0,0,0,1);
  float cost = saturate(dot(pin.normal,lighting));
  ret.xyz = pin.color*cost*d;
  return ret;
}