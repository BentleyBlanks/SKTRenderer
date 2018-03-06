cbuffer MatrixBuffer:register(b0)
{
  matrix m;

};

struct VOut
{
  float4 position : SV_POSITION;
  float3 normal:NORMAL;
  float2 tex : TEXCOORD;
};

struct VSIn
{
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 tex : TEXCOORD;
};


VOut main(VSIn input)
{
  VOut output;
  float4 mo = float4(input.position,1);
  mo = mul(mo,m);
  output.position = mo;
  output.tex = input.tex;
  output.normal = mul(input.normal, (float3x3) m);
  output.normal = normalize(output.normal);
  return output;
}




