struct VSIn
{
  float4 color : COLOR;
  float3 position : POSITION;
};

struct VSOut
{
  float4 color : COLOR;  
  float4 Pos : POSITION;
};

cbuffer CB
{
  matrix m;
};

VSOut main1(VSIn input)
{
    VSOut Output;
    float3 pos = input.position.xyz + 0.5;
    Output.Pos = float4(pos,1);
    Output.Pos = mul(Output.Pos,m);
    Output.color = input.color;
    return Output;
}