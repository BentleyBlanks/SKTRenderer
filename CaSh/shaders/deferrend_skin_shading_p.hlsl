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

//Blinn-Phong
float4 blinn_phong(float3 n,float3 v,float3 l,float4 clight)
{
  float4 cdiff = diffuse;
  float4 cspec = float4(1,1,1,1);
  float factor = 2000*hipow;
  float3 h = normalize(v + l);
  float a = dot(n,l);
  return (cdiff+ cspec*pow(saturate(dot(n,h)),factor))*clight*saturate(a);
}

float3 f_schlick_ue4(float3 cspec,float3 v,float3 h)
{
  float voh = saturate(dot(v,h));
  float fc = pow(1-voh,5);
  // Anything less than 2% is physically impossible and is instead considered to be shadowing
  return saturate(50.0*cspec.g)*fc + (1-fc)*cspec;

}

float d_beckmann_ue4(float roughness,float3 n,float3 v,float3 l)
{
  float3 h = normalize(v+l);
  float noh = saturate(dot(n,h));
  float a = roughness*roughness;
  float a2 = a*a;
  float noh2 = noh*noh;
  return exp((noh2-1)/(a2*noh2))/(PI*a2*noh2*noh2);
}

float d_ggx_ue4(float roughness,float3 n,float3 v,float3 l)
{
  float3 h = normalize(v+l);
  float noh = saturate(dot(n,h));
  float a = roughness*roughness;
  float a2 = a*a;
  float d = (noh*a2 - noh)*noh + 1;
  return a2/(PI*d*d);
}

float d_blinn_ue4(float roughness,float3 n,float3 v,float3 l)
{
  float3 h = normalize(v+l);
  float noh = saturate(dot(n,h));
  float a = roughness*roughness;
  float a2 = a*a;
  float nn = 2/a2 - 2;
  return (nn+2)/(2*PI)*pow(noh,nn);
}

float3 ks_skin_specular(float3 n,float3 l,float3 v,float roughness,float3 powerk/*高光系数*/)
{
  float nol = dot(n,l);
  if(nol>0)
  {
    float3 h = normalize(l+v);
    float noh = dot(n,h);
    float ph = d_beckmann_ue4(roughness,n,v,l);
    float3 fresnel = f_schlick_ue4(float3(0.028,0.028,0.028),v,h);
    float3 frspec = max(fresnel*ph /  dot(h,h),0);
    return nol*powerk*frspec;
  }
  return float3(0,0,0);
}

float3 diffuse_ibl(float3 clight)
{
  float3 base = diffuse;//float3(0.5,0.5,0.5);//191.0/255.0,173.0/255.0,111.0/255.0);
  float4 cdiff = float4(base,1);
  float3 diff = cdiff/PI;
  return diff*clight/PI;
}


float4 PsMain(VOut input) : SV_TARGET0
{
  float4 output;

  float4 point_lighting_color = float4(1,1,1.f,1);
  float4 pposition = float4(100,0,0,1);
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
  if(diffuse.x<0.00001&&diffuse.y<0.00001&&diffuse.z<0.00001) return float4(0,0,0,0);
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
    
  float dist = distance(world.xyz,point_posv.xyz);
  float factor1 = 0;
  float factor2 = 0;
  float factorc = 1.0;
  float factor = dist*dist*factor2 + factor1*dist + factorc;
  float4 lightingp = point_lighting_color/factor*factor_l;
  
  float3 v = normalize(-world.xyz);
  float3 l = normalize(point_posv - world.xyz);
  //return (normal + float4(1,1,1,1))*0.5; 

  //return float4(normalize(normal.xyz),1)*float4(1,1,1,1); 
  //return pow(dot(normal.xyz,normalize(l)),hipow)*diffuse*lightingp + diffuse;
  output = 0;
  
  //return float4(kk,kk,kk,1);

  //float3 l = normalize(point_posv - world.xyz);
  float3 r = reflect( normalize(world.xyz),normalize(normal.xyz));
  //l = r;

  float3 h = normalize(v + l);
  //return float4(0,(normalize(tangent.xyz)).y,0,1);
  float3 lighting = pow(env_texture.SampleLevel(SampleType,normalize(normal.xyz),6*hipow),1);
  output.xyz += diffuse_ibl(lighting);
  float3 lighting_hi = pow(env_texture_hi.SampleLevel(SampleType,r,6*hipow),1);


  output.xyz += metallic*diffuse* saturate(lerp(0.25,1.0,saturate(dot(normalize(normal),l))))*lightingp + (100.0*lightingp+200*lighting_hi)*ks_skin_specular(normal.xyz,l,v,hipow,powerc)*saturate(dot(normalize(normal),l));
  output.a = 0.1;

  return output;

}