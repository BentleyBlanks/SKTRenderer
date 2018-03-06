struct GOut 
{
  float4 pos : POSITION;
  float3 color : COLOR;
};


float4 main(GOut pin) : SV_TARGET0
{
  return float4(pin.color,1);
}