Texture2D color_buffer;
Texture2D len_buffer;
Texture2D blur_buffer;
Texture2D blur_buffer2;
Texture2D blur_buffer4;
Texture2D blur_buffer8;
Texture2D avg_buffer;
Texture2D last_buffer;
Texture2D dirt_mask;

SamplerState s1;
cbuffer hdr_param
{
  float exposure_adjustment;
  float gamma;
  float bloomMultiplier;
  float dirt_density;
  float time;
  float4 op_sc;

  float4 ABCD;
  float3 EFW;
  float ExposureBias;

  int use_filimic;
  int avg_debug;

};


struct VS_Output
{
  float4 Pos : SV_POSITION;
  float2 Tex : TEXCOORD0;
};

VS_Output main_vs(uint id : SV_VertexID)
{
  VS_Output Output;
  Output.Tex = float2((id << 1) & 2, id & 2);
  Output.Pos = float4(Output.Tex * float2(2, -2) + float2(-1, 1), 0, 1);
  return Output;
}

float3 Uncharted2Tonemap(float3 x)
{
  return ((x * (ABCD.x * x + ABCD.z * ABCD.y) + ABCD.w * EFW.x) / (x * (ABCD.x * x + ABCD.y) + ABCD.w * EFW.y)) - EFW.x / EFW.y;
}

// Approximates luminance from an RGB value
float CalcLuminance(float3 color)
{
  return max(dot(color, float3(0.299f, 0.587f, 0.114f)), 0.0001f);
}
float3 accurateLinearToSRGB(in float3 linearCol)
{
  float3 sRGBLo = linearCol * 12.92;
    float3 sRGBHi = (pow(abs(linearCol), 1.0 / 2.4) * 1.055) - 0.055;
    float3 sRGB = (linearCol <= 0.0031308) ? sRGBLo : sRGBHi;
    return sRGB;
}
float3 tonemap(float3 col, float exposure) {
  static const float A = 0.15; //shoulder strength
  static const float B = 0.50; //linear strength
  static const float C = 0.10; //linear angle
  static const float D = 0.20; //toe strength
  static const float E = 0.02; //toe numerator
  static const float F = 0.30; //toe denominator
  static const float W = 11.2; //linear white point value

  col *= exposure;

  col = ((col * (A * col + C * B) + D * E) / (col * (A * col + B) + D * F)) - E / F;
  static const float white = 1.0 / (((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F);
  col *= white;
  return col;
}
float4 main_ps(VS_Output pin) : SV_TARGET0
{

  float4 cl = last_buffer.Sample(s1, pin.Tex);
  float4 lc = len_buffer.Sample(s1, pin.Tex);


  float4 c = color_buffer.Sample(s1, pin.Tex);
  if (avg_debug)
    if (pin.Tex.x < 0.2&&pin.Tex.y < 0.2)
      return  float4(avg_buffer.Sample(s1, pin.Tex * 5).rrr, 1);
  float4 bloom = (
    (blur_buffer.Sample(s1, pin.Tex) + blur_buffer2.Sample(s1, pin.Tex) + blur_buffer4.Sample(s1, pin.Tex) + blur_buffer8.Sample(s1, pin.Tex))
    *0.25
    )*bloomMultiplier;

  float4 dirt = dirt_mask.Sample(s1, pin.Tex)*dirt_density;
    //c += 1 - (1 - bloom)*(1-dirt*bloom);
    bloom.xyz = tonemap(bloom.xyz, 1);

  float scale = avg_buffer.Sample(s1, float2(0.5, 0.5)).r;

  //if(scale==0) return float4(1,0,0,1);
  //if(isnan(scale)) return float4(0,0,1,1);

  scale = scale*op_sc[0] + op_sc[2];
  c = c*op_sc[1] + op_sc[3];


  float3 l = c*scale;

    if (use_filimic)
    {
      //l = Uncharted2Tonemap(ExposureBias*l);
      //float3 whiteScale = 1.f / Uncharted2Tonemap(float3(EFW.zzz));
      //l*=whiteScale;
      l.rgb = tonemap(l.rgb, exposure_adjustment);
    }
    else

      l = (l / (l + 1.f));

  lc.xyz = tonemap(lc.xyz, 1);



  l += 1 - (1 - bloom)*(1 - dirt*bloom);
  l += 1 - (1 - lc)*(1 - dirt*lc);

  l = pow(l, 1 / 2.2);



  //if(l.x<0.01||l.y<0.01||l.z<0.01)
  // return float4(0,1,0,1);

  //l = lerp(l,cl,time);
  return float4(l, 1);
  /*
  float3 toned = float3(1,1,1) - exp(-c*exposure_adjustment);
  toned = pow(toned,1/gamma);
  return float4(toned,1);
  */

}


