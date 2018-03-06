Texture2D shaderTexture1 : register(t0);
Texture2D shaderTexture2 : register(t1);
Texture2D shaderTexture3 : register(t2);
Texture2D shaderTexture4 : register(t3);

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

Texture2D ts[2];
SamplerState ss[2];

cbuffer AAA:register(b1)
{
  int a;int b;int c[4];
};

float4 sss;


struct VOut
{
  float4 position : SV_POSITION;
  float2 tex : TEXCOORD;
};

#define PI 3.141592653

float4 diffuse;


//Phong
float4 tranditional_phong(float3 n,float3 v,float3 l,float4 clight)
{
  float4 cdiff = diffuse;
  float4 cspec = float4(1,1,1,1);
  float factor = 200;
  float a = dot(n,l);
  v = reflect(-v,n);
  if(a>0)
    return (cdiff*saturate(a) + cspec*pow(saturate(dot(v,l)),factor))*clight;
  else
    return (cdiff*saturate(a))*clight;
}

//Modified Phong
float4 modified_tranditional_phong(float3 n,float3 v,float3 l,float4 clight)
{
  //mutiple a dot(n,l) to the hight-light term to make the dot(n,l) a part of the rendering equation.
  float4 cdiff = diffuse;
  float4 cspec = float4(1,1,1,1);
  float factor = 200;
  float a = dot(n,l);
  v = reflect(-v,n);
  return (cdiff+ cspec*pow(saturate(dot(v,l)),factor))*clight*saturate(a);
}

//Blinn-Phong
float4 blinn_phong(float3 n,float3 v,float3 l,float4 clight)
{
  float4 cdiff = diffuse;
  float4 cspec = float4(1,1,1,1);
  float factor = 200;
  float3 h = normalize(v + l);
  float a = dot(n,l);
  return (cdiff+ cspec*pow(saturate(dot(n,h)),factor))*clight*saturate(a);
}

float4 f_schlick(float4 cspec,float3 v,float3 h)
{
  return cspec + (1-cspec)*pow((1-dot(v,h)),5); 
}

float4 f_schlick_ue4(float4 cspec,float3 v,float3 h)
{
  float voh = saturate(dot(v,h));
  float fc = pow(1-voh,5);
  // Anything less than 2% is physically impossible and is instead considered to be shadowing
  return saturate(50.0*cspec.g)*fc + (1-fc)*cspec;

}

float4 f_fresnel( float4 cspec, float3 v,float3 h )
{
  float voh = saturate(dot(v,h));
	float3 SpecularColorSqrt = sqrt( clamp( float3(0, 0, 0), float3(0.99, 0.99, 0.99), cspec.xyz ) );
	float3 n = ( 1 + SpecularColorSqrt ) / ( 1 - SpecularColorSqrt );
	float3 g = sqrt( n*n + voh*voh - 1 );
	return float4(0.5 * pow( (g - voh) / (g + voh) ,2) * ( 1 + pow( ((g+voh)*voh - 1) / ((g-voh)*voh + 1) ,2) ),1);
}

float4 diffuse_lambert(float3 cdiff)
{
  return float4(cdiff/PI,1);
}

float4 diffuse_burley_ue4(float3 cdiff,float roughness,float3 n,float3 v,float3 l)
{
  float3 h = normalize(v + l);
  float voh = saturate(dot(v,h));
  float nov = saturate(dot(n,v));
  float nol = saturate(dot(n,l));

  float fd90 = 0.5 + 2*voh*voh*roughness;
  float fdv = 1 + (fd90 - 1)*pow(1-nov,5);
  float fdl = 1 + (fd90 - 1)*pow(1-nol,5);
  return float4(cdiff*(1/PI * fdv*fdl),1); 
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

float d_ggx_aniso_ue4(float rx,float ry,float3 v,float l,float n)
{
  return 0;
}

float vis_implicit_ue4()
{
  return 0.25;
}

float vis_kelemen_ue4(float3 v,float3 l)
{
  float3 h = normalize(v+l);
  float voh = saturate(dot(v,h));
  return rcp(4*voh*voh+1e-5);
}

float vis_neumann_ue4(float3 v,float3 l,float3 n)
{
  float nol = saturate(dot(n,l));
  float nov = saturate(dot(n,v));
  return 1/(4*max(nol,nov));
}

float vis_schlick_ue4(float roughness,float3 v,float3 l,float3 n)
{
  float nol = saturate(dot(n,l));
  float nov = saturate(dot(n,v));
  float k = pow(roughness,2)*0.5;
  float vis_schlickv = nov*(1-k) + k;
  float vis_schlickl = nol*(1-k) + k;
  return 0.25/(vis_schlickl*vis_schlickv);
}

float vis_smith_ue4(float roughness,float3 v,float3 l,float3 n)
{
  float nol = saturate(dot(n,l));
  float nov = saturate(dot(n,v));
  float a = pow(roughness,2);
  float a2 = a*a;
  float vis_simithv = nov + sqrt(nov*(nov - nov*a2)+a2);
  float vis_simithl = nol + sqrt(nol*(nol - nol*a2)+a2);
  return rcp(vis_simithv*vis_simithl);
}

float vis_smith_joint_approx_ue4(float roughness,float3 v,float3 l,float3 n)
{
  float nol = saturate(dot(n,l));
  float nov = saturate(dot(n,v));
  float a = roughness*roughness;
  float vis_simithv = nol * (nov*(1-a)+a);
  float vis_simithl = nov * (nol*(1-a)+a);
  return 0.5*rcp(vis_simithv + vis_simithl);
}

float3 f_none_ue4(float3 cspec)
{
  return cspec;
}

float3 derive_diffuse_color(float3 base_color)
{
  return base_color - base_color*metallic;
}

float3 derive_specular_color(float3 base_color,float3 specular)
{
  //0.08*specular.xxx
  return lerp(0.08*specular.xxx,base_color,metallic);
}

float4 micro_phong(float3 n,float3 v,float3 l,float4 clight)
{
  float4 cdiff = float4(derive_diffuse_color(diffuse),1);
  float4 cspec = float4(derive_specular_color(diffuse,float4(0.56,0.57,0.58,1)),1);
  float factor = 200;
  float3 h = normalize(v + l);
  float a = dot(n,l);
  return (cdiff + (factor + 2)/8*pow(saturate(dot(n,h)),factor)*f_fresnel(cspec,v,h))*clight*saturate(a);

}

float3 standard_shading_ue4(float roughness,float kd,float3 cspec,float3 cdiff,float kdiff,float3 l,float3 v,float3 n)
{
  float d = d_ggx_ue4(roughness,n,v,l)*kd;
  float vis = vis_smith_joint_approx_ue4(roughness,v,l,n);
  float3 h = normalize(v+l);
  float3 f = f_schlick_ue4(float4(cspec,1),v,h);

  float3 diffuse = diffuse_burley_ue4(cdiff,roughness,n,v,l);
  //diffuse = diffuse_lambert(cdiff);
  float3 specular = d*vis*f;

  return diffuse* kdiff*(1-specular) + specular;

}

float4 standard_shading_ue4_wrap(float3 n,float3 v,float3 l,float4 clight)
{
  float3 base = diffuse;//float3(0.5,0.5,0.5);//191.0/255.0,173.0/255.0,111.0/255.0);
  float4 cdiff = float4(derive_diffuse_color(base),1);
  float4 cspec = float4(derive_specular_color(base,float4(0.56,0.57,0.58,1)),1);
  float a = dot(n,l);
  float roughness = hipow;
  float kd = 1.0;
  float kdiff = 1.0;
  float3 brdf = standard_shading_ue4(roughness,kd,cspec,cdiff,kdiff,l,v,n);
  return float4(brdf*clight.xyz*saturate(a),1);
}

float3 diffuse_ibl(float3 n,float3 v,float3 l,float3 clight)
{

  float3 base = diffuse;//float3(0.5,0.5,0.5);//191.0/255.0,173.0/255.0,111.0/255.0);
  float4 cdiff = float4(derive_diffuse_color(base),1);
  float roughness = hipow;
  float3 diff = diffuse_lambert(cdiff);
  float4 cspec = float4(derive_specular_color(base,float4(0.56,0.57,0.58,1)),1);   
  return diff*clight/PI;
  
}

float3 specular_ibl(float3 n,float3 v,float3 l,float3 clight)
{
  float3 base = diffuse;//float3(0.5,0.5,0.5);//191.0/255.0,173.0/255.0,111.0/255.0);
  float4 cspec = float4(derive_specular_color(base,float4(0.56,0.57,0.58,1)),1);
  float3 h = normalize(v+l);    
  //暂时增加一个base增强漫反射blend效果
  return cspec*clight/*f_schlick_ue4(cspec,v,h)*/*(1-hipow)*metallic;
}




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
  float4 pposition = float4(30,0,0,1);
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
  /*
  float3 cc = diffuse.xyz*16*hipow;
  float ExposureBias1 = 2.0*metallic;
  float3 curr1 = Uncharted2Tonemap(ExposureBias1*cc);
  //return float4(curr1,1);
  float W = 11.2;
  float3 whiteScale1 = 1.0f/Uncharted2Tonemap(W);
  float3 color1 = curr1*whiteScale1;

  float3 retColor1 = pow(color1,1/2.2);
  
  //if(!(retColor1.x<1000000||retColor1.y<1000000||retColor1.z<1000000))

  
  return float4((retColor1.xyz),1);
*/
  if(diffuse.x<0.1) return float4(0,0,0,1);
  //diffuse = pow(diffuse,2.2);
  //float4 normal = shaderTexture2.Sample(SampleType, input.tex)*2 - float4(1,1,1,0);
  float4 normal = shaderTexture2.Load(int3(input.tex.x*w,input.tex.y*h,0))*2 - float4(1,1,1,0);
  //if(normal.x==0.5&&normal.z==0.5&&normal.y==0.5) dicard();
  float4 world = shaderTexture3.Sample(SampleType, input.tex);//Load(int3(input.tex.x*w,input.tex.y*h,0)));//*2 - float4(1,1,1,0))*128;
  //深度到底能不能采样？！上述哪些可以采样哪些不能？！
  //PS中的position到底是什么？能不能用？
  float depth = shaderTexture4.Load(int3(input.tex.x*w,input.tex.y*h,0));
  //depth = shaderTexture4.Sample(SampleType, input.tex);
  
  //return float4(normal.xyz,1)*float4(0,1,0,2)*0.5;
  //if(normal.z>0) return float4(0,0,0,1);
  
  float kk = pow(1 - world.r,hipow);
  //拉开模糊区域差距
  hipow = kk;
  float tt = world.g*metallic;
  metallic = tt;
  float ao = world.b;
  
  //return float4(hipow,hipow,hipow,1);


  float4 ndcpos = float4(input.tex.x*2-1,(1-input.tex.y)*2-1,depth,1);
  float4 viewpos = mul(ndcpos,inv_projection);
  viewpos = viewpos/viewpos.w;
  world.xyz = viewpos.xyz;
  
  /*
  for(int j =0;j<MAX_LIGHT;++j)
  {
    point_posv = lights[j];
    */
    
  float dist = distance(world.xyz,point_posv.xyz);
  float factor1 = 0.0;
  float factor2 = 0.0;
  float factorc = 1.0;
  float factor = dist*dist*factor2 + factor1*dist + factorc;
  float4 lightingp = point_lighting_color/factor*factor_l;

float3 v = normalize(-world.xyz);
float3 l = normalize(point_posv - world.xyz);
//return (normal + float4(1,1,1,1))*0.5; 
//return float4(normal.xyz,1)*float4(0,0,1,1); 
//return dot(normal.xyz,normalize(l))*diffuse;
output = 0;

//return float4(kk,kk,kk,1);

switch(shading_model)
{
  case 0:
  output = standard_shading_ue4_wrap(normalize(normal.xyz),v,l,lightingp);// + float4(0.2,0.2,0.2,1.0)*normal;
break;
  case 1:
output = blinn_phong(normalize(normal.xyz),v,l,lightingp);
break;

  case 2:
output = tranditional_phong(normalize(normal.xyz),v,l,lightingp);
break;

  case 3:
output = modified_tranditional_phong(normalize(normal.xyz),v,l,lightingp);
break;

  case 4:
output = micro_phong(normalize(normal.xyz),v,l,lightingp);
break;

}

  
  //float3 l = normalize(point_posv - world.xyz);
  float3 r = reflect( normalize(world.xyz),normalize(normal.xyz));
  l = r;
  float3 lighting = pow(env_texture.SampleLevel(SampleType,normalize(normal.xyz),6*hipow),1);
  //float3 h = normalize(v + l);

  float3 ibld= 1*ao*diffuse_ibl(normalize(normal.xyz),v,l,float4(lighting,1));// + float4(0.2,0.2,0.2,1.0)*normal;

  float3 lighting_hi = pow(env_texture_hi.SampleLevel(SampleType,r,6*hipow),1);
  
 
  float3 ibls = /*2.0*diffuse*/specular_ibl(normalize(normal.xyz),v,l,float4(lighting_hi,1));// + float4(0.2,0.2,0.2,1.0)*normal;
  
  output.xyz += ibld*abs(1-ibls) + ibls;

//output += f_schlick_ue4(float4(10,10,10,1),v,h)*diffuse/30;

 // }
  //output = output / MAX_LIGHT;
  //return float4(pow((output.xyz),1/2.2),1);
  output.xyz *= 16;
  float ExposureBias = 2.0f;
  float3 curr = Uncharted2Tonemap(ExposureBias*output);
  float W = 11.2;
  float3 whiteScale = 1.0f/Uncharted2Tonemap(W);
  float3 color = curr*whiteScale;

  float3 retColor = pow(color,1/2.2);
  return float4(retColor,1);

  //return pow(output,1/2.2f);
}