struct VSIn
{
  float3 position : POSITION;
  float2 tex : TEXCOORD;
};
struct VOut
{
  float4 position : SV_POSITION;
  float2 tex : TEXCOORD;
};
VOut VsMain(VSIn input)
{
  VOut output;
  output.position = float4(input.position,1);
  output.tex = input.tex;
  return output;
}