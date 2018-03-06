Texture2D shaderTexture1 : register(t0);
Texture2D shaderTexture2 : register(t1);
Texture2D shaderTexture3 : register(t2);
Texture2D shaderTexture4 : register(t3);
Texture2D shaderTexture5 : register(t7);

SamplerState SampleType;

TextureCube	env_texture : register( t4);
TextureCube	env_texture_hi : register( t5);
Texture2D tex_mat : register(t6);
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

struct VOut
{
  float4 position : SV_POSITION;
  float2 tex : TEXCOORD;
};


float3 get_view_normal(float4 texture_ret)
{
    float z = sqrt(1-(texture_ret.x*texture_ret.x + texture_ret.y*texture_ret.y));
    return float3(texture_ret.x,texture_ret.y,-z);
}

float3 get_view_tangent(float4 texture_ret,float4 tex2)
{
    //float z = sqrt(1-(texture_ret.z*texture_ret.z + texture_ret.w*texture_ret.w));
    return float3(texture_ret.z,texture_ret.w,tex2.a*2-1);
}

#define PI 3.141592653

float4 diffuse;

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

float3 f(float3 x)
{
  x = max(0,x-0.004);
  return (x*(6.2*x+0.5))/(x*(6.2*x+1.7)+0.06);
}

float4 PsMain(VOut input) : SV_TARGET0
{
  float4 output;

  float4 point_lighting_color = float4(10,10,10.,1);
  float4 pposition = float4(150,140,0,1);
  pposition = mul(pposition,mv_mat);
  float3 point_posv = pposition.xyz;

#if 0
#define MAX_LIGHT 64
  float4 lights[MAX_LIGHT];
  for(int i =0;i<MAX_LIGHT;++i)
  {
    float w = 4;
    float h = 4;
    float inter = 16;
    int a = i/(w*h);
    int b = (i - a*(w*h))/w;
    int c = (i - a*(w*h)) - b*w;
    float4 p = float4(a-MAX_LIGHT/2/(w*h),b-h/2,c,1/inter)*inter;
    p = mul(p,mv_mat);
    lights[i] = p;
  }
#endif

  input.tex.y = 1.f - input.tex.y;
  int indeces = (input.tex.y * h * w) + input.tex.x * w;
  diffuse = shaderTexture1.Sample(SampleType, input.tex);
  diffuse = pow(diffuse,2.2);
  if(diffuse.x<0.00001) return float4(0.5,0.5,0.5,1);
  float4 normal1 = shaderTexture2.Load(int3(input.tex.x*w,input.tex.y*h,0))*2 - float4(1,1,1,0);
  float4 normal = float4(get_view_normal(normal1),1);
  float4 tangent = float4(get_view_tangent(normal1,diffuse),1);
  float4 powerc = shaderTexture3.Sample(SampleType, input.tex);
  float shift = powerc.a*2 - 1;
  powerc = pow(powerc,2.2);
  float depth = shaderTexture4.Load(int3(input.tex.x*w,input.tex.y*h,0));
  float4 noiset = shaderTexture5.Sample(SampleType, input.tex);

  float4 world = 0;
  float4 ndcpos = float4(input.tex.x*2-1,(1-input.tex.y)*2-1,depth,1);
  float4 viewpos = mul(ndcpos,inv_projection);
  viewpos = viewpos/viewpos.w;
  world.xyz = viewpos.xyz;
  //return float4(world.yyy,1)*hipow;
    
  float dist = distance(world.xyz,point_posv.xyz);
  float factor1 = 0;
  float factor2 = 0;
  float factorc = 1;
  float factor = dist*dist*factor2 + factor1*dist + factorc;
  float4 lightingp = point_lighting_color/factor*factor_l;
  float3 v = normalize(-world.xyz);
  float3 l = normalize(point_posv - world.xyz);
  //return (normal + float4(1,1,1,1))*0.5; 
  //return float4(normal.xyz,1)*float4(1,1,0,1); 
  //return pow(dot(normal.xyz,normalize(l)),hipow)*diffuse*lightingp + diffuse;
  output = 0;
  
  //return float4(kk,kk,kk,1);

  //float3 l = normalize(point_posv - world.xyz);
  float3 r = reflect( normalize(world.xyz),normalize(normal.xyz));
  //l = r;

  float3 h = normalize(v + l);
  //return float4(0,(normalize(tangent.xyz)).y,0,1);

  float3 specularc2 = powerc.xyz;
  float3 specularc1 = float3(1,1,1);
  float primary_shift = 0.1;
  float second_shift = -0.1;


  shift = shift*0.2;
  float3 t1 = normalize(tangent.xyz + (shift+primary_shift)*normal.xyz);
  float3 t2 = normalize(tangent.xyz + (shift+second_shift)*normal.xyz);

  float tdh1 = dot(t1,h);
  float tdh2 = dot(t2,h);

  //dirAtten:http://stackoverflow.com/questions/33369449/what-does-diratten-value-mean-in-kajiya-kay-model
  //用于衰减切向刮擦角以及背向，sin永远都是正直，所以即使背对也可见，这种着色方式也许更加适合于一根一根的头发，有透明度那种，所以可能涉及到排序透明度之类的
  float dir_atten1 = smoothstep(-1.0, 0.0, tdh1);
  float dir_atten2 = smoothstep(-1.0, 0.0, tdh2);

  float3 spec1 = specularc1* pow(sqrt(1.0-tdh1*tdh1),5000*hipow)*dir_atten1;
  float3 spec2 = abs(noiset.x*2)*specularc2* pow(sqrt(1.0-tdh2*tdh2),1000*hipow)*dir_atten2;  
   

  output.xyz += metallic*diffuse* saturate(lerp(0.25,1.0,dot(normalize(normal),l)))*lightingp + diffuse*(spec1 + spec2)*lightingp;
  output.a = 0;
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