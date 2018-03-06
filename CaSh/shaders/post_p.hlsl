Texture2D render_buffer : register(t0);
Texture2D depth_buffer : register(t1);
TextureCube	env_texture : register( t2);
Texture2D noise_buffer : register(t3);

SamplerState SampleType;
SamplerState env_sample : register( s1 );

cbuffer ScreenSize:register(b0)
{
  int w;
  int h;
  float hipow;float factor_l;
  matrix inv_projection;
  matrix mv_mat;
  float metallic;int shading_model;
};

cbuffer PostBuffer:register(b1)
{
    float2 step;
};

struct VOut
{
  float4 position : SV_POSITION;
  float2 tex : TEXCOORD;
};

#define PI 3.141592653

float3 Uncharted2Tonemap(float3 x)
{
float A = 0.15;
float B = 0.50;
float C = 0.10;
float D = 0.20;
float E = 0.02;
float F = 0.30;

   return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

float4 blur(float2 tex, float2 step) 
{
    // Gaussian weights for the six samples around the current pixel:
    //   -3 -2 -1 +1 +2 +3
    float w[6] = { 0.006,   0.061,   0.242,  0.242,  0.061, 0.006 };
    float o[6] = {  -1.0, -0.6667, -0.3333, 0.3333, 0.6667,   1.0 };

    // Fetch color and linear depth for current pixel:
    float4 colorM = render_buffer.Sample(SampleType, tex);
    float depthM = depth_buffer.Sample(SampleType, tex);

    // Accumulate center sample, multiplying it with its gaussian weight:
    float4 colorBlurred = colorM;
    colorBlurred.rgb *= 0.382;

    // Calculate the step that we will use to fetch the surrounding pixels,
    // where "step" is:
    //     step = sssStrength * gaussianWidth * pixelSize * dir
    // The closer the pixel, the stronger the effect needs to be, hence
    // the factor 1.0 / depthM.
    float2 finalStep = colorM.a * step / depthM;

    // Accumulate the other samples:
    [unroll]
    for (int i = 0; i < 6; i++) {
        // Fetch color and depth for current sample:
        float2 offset = tex + o[i] * finalStep;
        float3 color = render_buffer.SampleLevel(SampleType, offset, 0).rgb;
        float depth = depth_buffer.SampleLevel(SampleType, offset, 0);

        float correction = 1;
        // If the difference in depth is huge, we lerp color back to "colorM":
        float s = min(0.0125 * correction * abs(depthM - depth), 1.0);
        color = lerp(color, colorM.rgb, s);

        // Accumulate:
        colorBlurred.rgb += w[i] * color;
    }

    // The result will be alpha blended with current buffer by using specific
    // RGB weights. For more details, I refer you to the GPU Pro chapter :)
    return colorBlurred;
}

float4 PsMain(VOut input) : SV_TARGET0
{
  float4 output;

  input.tex.y = 1.f - input.tex.y;
  int indeces = (input.tex.y * h * w) + input.tex.x * w;
  float4 diffuse = render_buffer.Sample(SampleType, input.tex);
 // if(diffuse.x<=0) return float4(0.5,0.5,0.5,1);
  float depth = depth_buffer.Load(int3(input.tex.x*w,input.tex.y*h,0));
  float4 noiset = noise_buffer.Sample(SampleType, input.tex);

  float4 world = 0;
  float4 ndcpos = float4(input.tex.x*2-1,(1-input.tex.y)*2-1,depth,1);
  float4 viewpos = mul(ndcpos,inv_projection);
  viewpos = viewpos/viewpos.w;
  world.xyz = viewpos.xyz;

  output = diffuse;

  //
  //return  depth_buffer.Sample(SampleType, input.tex)/2;
  return pow(output,1/2.2);

  output.xyz *= 16;
  float ExposureBias = 2.0f;
  float3 curr = Uncharted2Tonemap(ExposureBias*output);
  float W = 11.2;
  float3 whiteScale = 1.0f/Uncharted2Tonemap(W);
  float3 color = curr*whiteScale;

  float3 retColor = pow(color,1/2.2);
  return float4(retColor,1);
}