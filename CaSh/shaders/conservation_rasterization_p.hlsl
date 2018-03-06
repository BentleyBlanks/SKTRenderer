struct VOut
{
  float4 position : SV_POSITION;
  float3 normal:NORMAL;
  float3 color : COLOR;
  float2 tex : TEXCOORD;
};

float4 main(VOut input) : SV_TARGET
{
  float factor = abs(dot(normalize(input.normal),normalize(float3(0,0,-1))));
  //if(factor<0) return float4(0,0,0,1);
  //return float4((normalize(input.normal) + 1)*0.5,1);
  //当前没用到uv所以y用来暂时储存view深度
  //float w = input.tex.y/50;
  //return float4(w,w,w,1);
  return float4(float3(1,1,1)*factor,1);
 // return float4(1,1,1,1);
}