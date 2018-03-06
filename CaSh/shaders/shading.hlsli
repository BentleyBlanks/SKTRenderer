#define PI 3.141592653

//Phong
float4 tranditional_phong(float3 diffuse, float3 spec,float fact,float3 n, float3 v, float3 l, float3 clight)
{
    float3 cdiff = diffuse;
    float3 cspec = spec;
    float factor = fact;
    float a = dot(n, l);
    v = reflect(-v, n);
    if (a > 0)
        return (cdiff * saturate(a) + cspec * pow(saturate(dot(v, l)), factor)) * clight;
    else
        return (cdiff * saturate(a)) * clight;
}

//Modified Phong
float3 modified_tranditional_phong(float3 diffuse, float3 spec,float fact,float3 n, float3 v, float3 l, float3 clight)
{
  //mutiple a dot(n,l) to the hight-light term to make the dot(n,l) a part of the rendering equation.
    float3 cdiff = diffuse;
    float3 cspec = fact;
    float factor = fact;
    float a = dot(n, l);
    v = reflect(-v, n);
    return (cdiff + cspec * pow(saturate(dot(v, l)), factor)) * clight * saturate(a);
}

//Blinn-Phong
float3 blinn_phong(float3 diffuse, float3 spec,float fact,float3 n, float3 v, float3 l, float3 clight)
{
    float3 cdiff = diffuse;
    float3 cspec = spec;
    float factor = fact;
    float3 h = normalize(v + l);
    float a = dot(n, l);
    return (cdiff + cspec * pow(saturate(dot(n, h)), factor)) * clight * saturate(a);
}

float3 f_schlick(float3 cspec, float3 v, float3 h)
{
    return cspec + (1 - cspec) * pow((1 - dot(v, h)), 5);
}

float3 f_schlick_ue4(float3 cspec, float3 v, float3 h)
{
    float voh = saturate(dot(v, h));
    float fc = pow(1 - voh, 5);
  // Anything less than 2% is physically impossible and is instead considered to be shadowing
    return saturate(50.0 * cspec.g) * fc + (1 - fc) * cspec;

}

float3 f_fresnel(float3 cspec, float3 v, float3 h)
{
    float voh = saturate(dot(v, h));
    float3 SpecularColorSqrt = sqrt(clamp(float3(0, 0, 0), float3(0.99, 0.99, 0.99), cspec.xyz));
    float3 n = (1 + SpecularColorSqrt) / (1 - SpecularColorSqrt);
    float3 g = sqrt(n * n + voh * voh - 1);
    return 0.5 * pow((g - voh) / (g + voh), 2) * (1 + pow(((g + voh) * voh - 1) / ((g - voh) * voh + 1), 2));
}

float3 diffuse_lambert(float3 cdiff)
{
    return cdiff / PI;
}

float3 diffuse_burley_ue4(float3 cdiff, float roughness, float3 n, float3 v, float3 l)
{
    float3 h = normalize(v + l);
    float voh = saturate(dot(v, h));
    float nov = saturate(dot(n, v));
    float nol = saturate(dot(n, l));

    float fd90 = 0.5 + 2 * voh * voh * roughness;
    float fdv = 1 + (fd90 - 1) * pow(1 - nov, 5);
    float fdl = 1 + (fd90 - 1) * pow(1 - nol, 5);
    return cdiff * (1 / PI * fdv * fdl);
}

float d_blinn_ue4(float roughness, float3 n, float3 v, float3 l)
{
    float3 h = normalize(v + l);
    float noh = saturate(dot(n, h));
    float a = roughness * roughness;
    float a2 = a * a;
    float nn = 2 / a2 - 2;
    return (nn + 2) / (2 * PI) * pow(noh, nn);
}

float d_beckmann_ue4(float roughness, float3 n, float3 v, float3 l)
{
    float3 h = normalize(v + l);
    float noh = saturate(dot(n, h));
    float a = roughness * roughness;
    float a2 = a * a;
    float noh2 = noh * noh;
    return exp((noh2 - 1) / (a2 * noh2)) / (PI * a2 * noh2 * noh2);
}

float d_ggx_ue4(float roughness, float3 n, float3 v, float3 l)
{
    float3 h = normalize(v + l);
    float noh = saturate(dot(n, h));
    float a = roughness * roughness;
    float a2 = a * a;
    float d = (noh * a2 - noh) * noh + 1;
    return a2 / (PI * d * d);
}

float d_ggx_aniso_ue4(float rx, float ry, float3 v, float l, float n)
{
    return 0;
}

float vis_implicit_ue4()
{
    return 0.25;
}

float vis_kelemen_ue4(float3 v, float3 l)
{
    float3 h = normalize(v + l);
    float voh = saturate(dot(v, h));
    return rcp(4 * voh * voh + 1e-5);
}

float vis_neumann_ue4(float3 v, float3 l, float3 n)
{
    float nol = saturate(dot(n, l));
    float nov = saturate(dot(n, v));
    return 1 / (4 * max(nol, nov));
}

float vis_schlick_ue4(float roughness, float3 v, float3 l, float3 n)
{
    float nol = saturate(dot(n, l));
    float nov = saturate(dot(n, v));
    float k = pow(roughness, 2) * 0.5;
    float vis_schlickv = nov * (1 - k) + k;
    float vis_schlickl = nol * (1 - k) + k;
    return 0.25 / (vis_schlickl * vis_schlickv);
}

float vis_smith_ue4(float roughness, float3 v, float3 l, float3 n)
{
    float nol = saturate(dot(n, l));
    float nov = saturate(dot(n, v));
    float a = pow(roughness, 2);
    float a2 = a * a;
    float vis_simithv = nov + sqrt(nov * (nov - nov * a2) + a2);
    float vis_simithl = nol + sqrt(nol * (nol - nol * a2) + a2);
    return rcp(vis_simithv * vis_simithl);
}

float vis_smith_joint_approx_ue4(float roughness, float3 v, float3 l, float3 n)
{
    float nol = saturate(dot(n, l));
    float nov = saturate(dot(n, v));
    float a = roughness * roughness;
    float vis_simithv = nol * (nov * (1 - a) + a);
    float vis_simithl = nov * (nol * (1 - a) + a);
    return 0.5 * rcp(vis_simithv + vis_simithl);
}

float3 f_none_ue4(float3 cspec)
{
    return cspec;
}

float3 derive_diffuse_color(float3 base_color,float metallic)
{
    return base_color - base_color * metallic;
}

float3 derive_specular_color(float3 base_color, float3 specular,float metallic)
{
  //0.08*specular.xxx
    return lerp(0.08 * specular.xxx, base_color, metallic);
}

float3 micro_phong(float metallic,float3 diffuse,float3 cspec,float fact,float3 n, float3 v, float3 l, float3 clight)
{
    float3 cdiff = derive_diffuse_color(diffuse,metallic);
    float3 cspec = derive_specular_color(diffuse, cspec/*float4(0.56, 0.57, 0.58, 1)*/,metallic);
    float factor = fact;
    float3 h = normalize(v + l);
    float a = dot(n, l);
    return (cdiff + (factor + 2) / 8 * pow(saturate(dot(n, h)), factor) * f_fresnel(cspec, v, h)) * clight * saturate(a);

}

float3 standard_shading_ue4(float roughness, float kd, float3 cspec, float3 cdiff, float kdiff, float3 l, float3 v, float3 n)
{
    float d = d_ggx_ue4(roughness, n, v, l) * kd;
    float vis = vis_smith_joint_approx_ue4(roughness, v, l, n);
    float3 h = normalize(v + l);
    float3 f = f_schlick_ue4(cspec, v, h);

    float3 diffuse = diffuse_burley_ue4(cdiff, roughness, n, v, l);
  //diffuse = diffuse_lambert(cdiff);
    float3 specular = d * vis * f;

    return diffuse * kdiff * (1 - specular) + specular;

}

float3 standard_shading_ue4_wrap(float rs,float metallic,float3 diffuse,float3 cspec,float3 n, float3 v, float3 l, float3 clight)
{
    float3 base = diffuse; //float3(0.5,0.5,0.5);//191.0/255.0,173.0/255.0,111.0/255.0);
    float3 cdiff = derive_diffuse_color(base,metallic);
    float3 cspec = derive_specular_color(base, cspec/*float4(0.56, 0.57, 0.58, 1)*/,metallic);
    float a = dot(n, l);
    float roughness = rs;
    float kd = 1.0;
    float kdiff = 1.0;
    float3 brdf = standard_shading_ue4(roughness, kd, cspec, cdiff, kdiff, l, v, n);
    return brdf * clight.xyz * saturate(a);
}

float3 diffuse_ibl(float rs,float metallic,float3 diffuse,float cspec,float3 n, float3 v, float3 l, float3 clight)
{

    float3 base = diffuse; //float3(0.5,0.5,0.5);//191.0/255.0,173.0/255.0,111.0/255.0);
    float3 cdiff = derive_diffuse_color(base,metallic);
    float roughness = rs;
    float3 diff = diffuse_lambert(cdiff);
    float3 cspec = derive_specular_color(base, cspec/*float4(0.56, 0.57, 0.58, 1)*/,metallic);
    return diff * clight / PI;
  
}

float3 specular_ibl(float rs,float metallic,float3 diffuse,float3 cspec,float3 n, float3 v, float3 l, float3 clight)
{
    float3 base = diffuse; //float3(0.5,0.5,0.5);//191.0/255.0,173.0/255.0,111.0/255.0);
    float3 cspec = derive_specular_color(base, cspec/*float4(0.56, 0.57, 0.58, 1)*/,metallic);
    float3 h = normalize(v + l);
    //暂时增加一个base增强漫反射blend效果
    return cspec * clight /*f_schlick_ue4(cspec,v,h)*/ * (1 - rs) * metallic;
}