struct PointLight 
{
    float4 position_;
    float4 clq_;
    float4 ambient;
    float4 diffuse;
    float4 specular;
};

#define MAX_POINTLIGHT 100

cbuffer MatrixBuffer:register(b0)
{
  matrix m;
  matrix v;
  matrix o;
  float3 sun_dir_m;
  float3 sun_color_m;
  PointLight point_lights[MAX_POINTLIGHT];
  int light_n;
  float3 view_world_pos;
  float roughness;
  float metallic;
  int shading_model;

};
struct VSIn
{
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 tex : TEXCOORD;
};
struct VOut
{
  float4 position : SV_POSITION;
  float3 view_dir : POSITION1;
  float3 world_pos : POSITION2;
  float3 normal : NORMAL;
  float3 normal_w : NORMAL1;
  float3 color : COLOR;
  float2 tex : TEXCOORD;
};
VOut main_vs(VSIn input)
{
  VOut output;
  float4 mo = float4(input.position,1);
  mo = mul(mo,m);
  output.world_pos = mo.xyz;
  mo = mul(mo,v);
  output.view_dir = -normalize(mo.xyz);
  float4 vd = mo;
  mo = mul(mo,o);
  output.position = mo;
  //if you have a scale you must let Normal = mat3(transpose(inverse(model))) * aNormal;that is to say - use inverse transpose
  output.normal = mul(input.normal,(float3x3)m);
  output.normal_w = output.normal;
  output.normal = mul(input.normal,(float3x3)v);
  output.normal = normalize(output.normal);
  output.tex = input.tex;
  output.color = float3(1,1,1);
  
  return output;
}
Texture2D dft;
Texture2D mra;
Texture2D normal_map;
TextureCube irr_map;
TextureCube spec_map;
Texture2D brdf_lut;
SamplerState ss;

#define PI 3.14159265
//#define SIMPLE
#define FROST
//From Frostbite.
//f0 is specular color
float3 F_Schlick(float3 f0,float f90,float ldh)
{
  return f0 + (f90-f0)*pow(1.f-ldh,5.f);
}

float V_SmithGGXCorrelated(float ndl,float ndv,float alphaG)
{
  //V_SmithGGXCorrelated = G_SmithGGXCorrelated/(4.f*ndl*ndv);
  //This is the optimize version
  float alphaG2 = alphaG*alphaG;
  //Caution:ref to notes
  float Lambda_GGXV = ndl*sqrt((-ndv*alphaG2+ndv)*ndv+alphaG2);
  float Lambda_GGXL = ndv*sqrt((-ndl*alphaG2+ndl)*ndl+alphaG2);
  return 0.5f/max((Lambda_GGXL+Lambda_GGXV),0);
}

float D_GGX(float ndh,float m)
{
  float m2 = m*m;
  float f = (ndh*m2-ndh)*ndh+1;
  return m2/(f*f);
}

//disney diffuse brdf with renomalization of its energy.
float Fr_DisneyDiffuse(float ndv,float ndl,float ldh,float linearRoughness)
{
  float energy_bias = lerp(0,0.5f,linearRoughness);
  float energy_factor = lerp(1.0,1.f/1.51f,linearRoughness);
  float fd90 = energy_bias+2.f*ldh*ldh*linearRoughness;
  float3 f0 = float3(1.f,1.f,1.f);
  float light_scatter = F_Schlick(f0,fd90,ndl).r;
  float view_scatter = F_Schlick(f0,fd90,ndv).r;
  return light_scatter*view_scatter*energy_factor;
}

float3 frostbite_specular_brdf(float3 normal,float3 view,float3 light,float3 cspecular,float f90,float linearroughness)
{
  float roughness = linearroughness*linearroughness;
  float ndv = abs(dot(normal,view))+1e-5f;//avoid 0
  float3 h = normalize(view+light);
  float ldh = saturate(dot(light,h));
  float ndh = saturate(dot(normal,h));
  float ndl = saturate(dot(normal,light));

  //specular
  float3 f = F_Schlick(cspecular,f90,ldh);
  float vis = V_SmithGGXCorrelated(ndv,ndl,roughness);
  float d = D_GGX(ndh,roughness);
  float3 fs = d*f*vis/PI;
  return fs;

}

float3 frostbite_diffuse_brdf(float3 normal,float3 view,float3 light,float3 cdiffuse,float linearroughness)
{
  float ndv = abs(dot(normal,view))+1e-5f;//avoid 0
  float3 h = normalize(view+light);
  float ldh = saturate(dot(light,h));
  float ndh = saturate(dot(normal,h));
  float ndl = saturate(dot(normal,light));
  //diffuse
  float3 fd = Fr_DisneyDiffuse(ndv,ndl,ldh,linearroughness)*cdiffuse;
  return fd;
}

float3 lambert_brdf(float3 cdiffuse)
{
  return cdiffuse/PI;
}

float3 blinn_phong_brdf(float3 normal,float3 lighting,float3 view_dir,float power,float3 cspecular)
{
  float3 half_dir = normalize(lighting + view_dir);  
  float fs = pow(saturate(dot(normal, half_dir)), power)*(8.f+power)/8.f;
  return cspecular*fs/PI;
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return nom / max(denom,0.f);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / max(denom,0.f);
}
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}
float3 ref_brdf(float3 normal,float3 view,float3 light,float3 cspecular,float f90,float linearroughness,float3 obj_color)
{
  //float roughness = linearroughness*linearroughness;
  float ndv = max(dot(normal,view),0.f);//avoid 0
  float3 h = normalize(view+light);
  float ldh = max(dot(light,h),0.f);
  //float ndh = saturate(dot(normal,h));
  float ndl = max(dot(normal,light),0.f);

  //specular
  float3 f = F_Schlick(cspecular,f90,ldh);
  float g = GeometrySmith(normal,view,light,linearroughness);
  float d = DistributionGGX(normal,h,linearroughness);
  float3 fs = 0.25*f*d*g/max( ndv * ndl,0.001f);
  float3 fd = Fr_DisneyDiffuse(ndv,ndl,ldh,linearroughness)*obj_color;
  
  float3 kd = 1.f - f;
  kd *= 1.f-metallic;
  fd=kd*obj_color/PI;
  return fs+fd;

}

//object color and specular color both are feature of object
float3 lighting(float3 light_dir,float3 view_dir,float3 normal,float3 object_color,
float3 light_color,float3 ambient_color,float3 spec_color,float roughness1)
{
  float3 lighting = normalize(light_dir);
  float cost = saturate(dot(normal,lighting));

float3 cbrdf;
if(shading_model==0)
  cbrdf = blinn_phong_brdf(normal,lighting,view_dir,1/pow(roughness1,3),spec_color) + lambert_brdf(object_color);
else if(shading_model==1)
  cbrdf = frostbite_diffuse_brdf(normal,view_dir,lighting,object_color,roughness1)
  +frostbite_specular_brdf(normal,view_dir,lighting,spec_color,1.f,roughness1);
else
  cbrdf = ref_brdf(normal,view_dir,lighting,spec_color,1.f,roughness1,object_color);


  float3 ambient = ambient_color;

  return cbrdf*light_color*cost;
}

float4 compute_direct_lighting(float3 obj_pos_w, float3 normal_w, float3 ambient_color,
float3 object_color,float3 spec_color, float3 view_pos_w,float3 light_color,float3 light_w,float roughness1)
{
  return float4(lighting(light_w,normalize(view_pos_w-obj_pos_w),normal_w,object_color,light_color,ambient_color,spec_color,roughness1),1);
}

float4 compute_point_lighting( float3 obj_pos_w, float3 normal_w, float3 object_color, float3 view_pos_w,float roughness1,float3 spec_color,float3 ambient_color)
{
  float4 ret = float4(0,0,0,1);
  for(int i=0;i<light_n;++i)
  {
    PointLight lit = point_lights[i];
    float dist = length(lit.position_.xyz - obj_pos_w);
    float attenuation = 1.0 / (lit.clq_.x + lit.clq_.y * dist + lit.clq_.z * (dist * dist));
    ret.rgb = ret.rgb + lighting(normalize( lit.position_.xyz - obj_pos_w),
      normalize(view_pos_w-obj_pos_w),normal_w,object_color,
      lit.diffuse.rgb*attenuation*0.1,
      ambient_color,spec_color,roughness1);
  }
  ret.rgb = ret.rgb/light_n;
  return ret;
}

float3 fresnelSchlickRoughness(float ndv, float3 f0, float roughness)
{
    return f0 + (max(float3(1.0 - roughness,1.0 - roughness,1.0 - roughness), f0) - f0) * pow(1.0 - ndv, 5.0);
}

float4 compute_sphere_lighting( float3 obj_pos_w, float3 normal_w, float3 object_color, float3 view_pos_w)
{
  return float4(0,0,0,0);
}

float3 compute_ibl(float3 normal,float3 view,float3 cspec,float3 light_irr,float3 obj_color,float linearroughness)
{
  float3 r = reflect(-view,normal);
  float3 color = pow(spec_map.SampleLevel(ss,r,linearroughness*4.f).rgb,1.0);
  
  //ibl diffuse counting fresnel
  //because of irradiance, there is no single half vector so use view instead
  //to avoid strong specular on non-metal, we count roughness.
  //float3 fr = F_Schlick(cspec,1.f,max(dot(normal,view)));
  float3 fr = fresnelSchlickRoughness(max(dot(normal,view),0.f),cspec,linearroughness);
  float3 kd = 1.0 - fr;//substract specular
  kd*=1-metallic;
  float3 diffuse = light_irr * obj_color;

  float2 brdf  = brdf_lut.Sample(ss,float2(max(dot(normal, view),0.f),1-linearroughness)).rg;
  float3 specular = color * (fr * brdf.x + brdf.y);

  float3 ambient = (kd * diffuse)+specular;
  return ambient;

}


float3 accurateSRGBToLinear(in float3 sRGBCol)
{
  float3 linearRGBLo = sRGBCol / 12.92;
  float3 linearRGBHi = pow((sRGBCol + 0.055) / 1.055, 2.4);
  float3 linearRGB = (sRGBCol <= 0.04045) ? linearRGBLo : linearRGBHi;
  return linearRGB;
}

//wrong!
float3 unpack_normal(VOut input)
{
  //maybe no filtering
    float2 text1 = input.tex;
    //text1.y = 1 - text1.y;
    float4 origin_n = normal_map.Sample(ss, text1);
    //origin_n.b = 1 - origin_n.b;
    //origin_n.r = origin_n.a;
    origin_n.xyz = origin_n.xyz * 2 - float3(1, 1, 1);
    origin_n.w = 1;
    float3 vertex_n = normalize(input.normal_w);

   // get edge vectors of the pixel triangle
    float3 dp1 = ddx( input.world_pos );
    float3 dp2 = ddy( input.world_pos );
    float2 duv1 = ddx( text1 );
    float2 duv2 = ddy( text1 );
 
    // solve the linear system
    float3 dp2perp = cross( dp2, vertex_n );
    float3 dp1perp = cross( vertex_n, dp1 );
    float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    float3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
    // construct a scale-invariant frame 
    float invmax = 1/sqrt( max( dot(T,T), dot(B,B) ) );
    float3x3 tbn;
    tbn[0] = T * invmax;
    tbn[1] = B * invmax;
    tbn[2] = vertex_n;
    float3 rn = normalize(mul(origin_n,tbn));
    //float temp = rn.y;
    //rn.y = rn.z;
    //rn.z = temp;
    return rn;
}

//???how it works?
float3 unpack_normal1(VOut input)
{
    float2 text1 = input.tex;
    //float3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;
    float3 tangentNormal = normal_map.Sample(ss, text1).xyz*2-1;
    float3 Q1  = ddx(input.world_pos);
    float3 Q2  = ddy(input.world_pos);
    float2 st1 = ddx(text1);
    float2 st2 = ddy(text1);

    float3 N   = normalize(input.normal_w);
    float3 T  = normalize(Q1*st2.y - Q2*st1.y);
    float3 B  = -normalize(cross(N, T));
    float3x3 tbn;
    tbn[0] = T;
    tbn[1] = B;
    tbn[2] = N;
    float3 rn = normalize(mul(tangentNormal,tbn));
    return rn;
}
float4 main_ps(VOut pin) : SV_TARGET
{
  float3 k = irr_map.Sample(ss,normalize(pin.normal_w));
  //Ambient component
  float ambient_factor = 1;
  float3 ambient_color = k;

  //difuse component
  float s = step(0.01f, sun_dir_m.y);
  //we dont use DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,we use a custom transform
  float3 d = pow(dft.Sample(ss,pin.tex).xyz,2.2);
  //d.rgb = float3(1,0,0);
  float3 mrat = mra.Sample(ss,pin.tex)+0.0001f;
  roughness = pow(mrat.g,1.0)*roughness;
  metallic = pow(mrat.r,1.0)*metallic;
  //d = d*(1.f-metallic);
  float3 specular = lerp(0.04,d,metallic);
  float3 lighting = normalize(sun_dir_m);
  sun_color_m = sun_color_m*s;

  float3 ambient = ambient_color*ambient_factor;
  float3 n = unpack_normal1(pin);

  float4 ret = float4(0,0,0,1);
  ret.xyz = compute_direct_lighting(pin.world_pos,n,ambient,d,specular,view_world_pos,sun_color_m,lighting,roughness).rgb;
  ret.xyz =  ret.xyz+compute_point_lighting(pin.world_pos,n,d,view_world_pos,roughness,specular,ambient).rgb;
  ret.xyz += compute_ibl(n,normalize(view_world_pos-pin.world_pos),specular,ambient,d,roughness);
  //ret.xyz =  brdf_lut.Sample(ss,float2(0.01,1-0.01)).rgb;//ret.xyz+compute_sphere_lighting(pin.world_pos,n,d,view_world_pos).rgb;
  return ret;
}
