#define PI 3.141592653

//Phong
float3 tranditional_phong(float3 diffuse, float3 spec, float fact, float3 n, float3 v, float3 l, float3 clight)
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
float3 modified_tranditional_phong(float3 diffuse, float3 spec, float fact, float3 n, float3 v, float3 l, float3 clight)
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
float3 blinn_phong(float3 diffuse, float3 spec, float fact, float3 n, float3 v, float3 l, float3 clight)
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
    return a2 / ((PI * d * d) + 0.0001);
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
    return rcp(vis_simithv * vis_simithl + 0.001);
}

float vis_smith_joint_approx_ue4(float roughness, float3 v, float3 l, float3 n)
{
    float nol = saturate(dot(n, l));
    float nov = saturate(dot(n, v));
    float a = roughness * roughness;
    float vis_simithv = nol * (nov * (1 - a) + a);
    float vis_simithl = nov * (nol * (1 - a) + a);
    //avoid to calculate a rcp for zero so add a 0.001
    return 0.5 * rcp(vis_simithv + vis_simithl + 0.001);
}

float3 f_none_ue4(float3 cspec)
{
    return cspec;
}

float3 derive_diffuse_color(float3 base_color, float metallic)
{
    return base_color - base_color * metallic;
}

float3 derive_specular_color(float3 base_color, float3 specular, float metallic)
{
  //0.08*specular.xxx
    return lerp(0.08 * specular.xxx, base_color, metallic);
}

float3 micro_phong(float metallic, float3 diffuse, float3 cspec, float fact, float3 n, float3 v, float3 l, float3 clight)
{
    float3 cdiff = derive_diffuse_color(diffuse, metallic);
   
    cspec = derive_specular_color(diffuse, cspec /*float4(0.56, 0.57, 0.58, 1)*/, metallic);
    
    float factor = fact;
    float3 h = normalize(v + l);
    float a = dot(n, l);
    return (cdiff) + (factor + 2) / 8 * pow(saturate(dot(n, h)), factor) * f_fresnel(cspec, v, h) * clight * saturate(a);

}

float3 standard_shading_ue4(float roughness, float kd, float3 cspec, float3 cdiff, float kdiff, float3 l, float3 v, float3 n)
{
    float d = d_ggx_ue4(roughness, n, v, l) * kd;
    float vis = vis_smith_joint_approx_ue4(roughness, v, l, n);
    float3 h = normalize(v + l);
    float3 f = f_schlick_ue4(cspec, v, h);

    //we dont use burley,diff is small
    float3 diffuse = //diffuse_burley_ue4(cdiff, roughness, n, v, l);
    diffuse_lambert(cdiff);
    float3 specular = d * vis * f;

    return diffuse * kdiff * (1 - specular) +
     specular;

}

float3 standard_shading_ue4_wrap(float rs, float metallic, float3 diffuse, float3 cspec, float3 n, float3 v, float3 l, float3 clight)
{
    float3 base = diffuse; //float3(0.5,0.5,0.5);//191.0/255.0,173.0/255.0,111.0/255.0);
    float3 cdiff = derive_diffuse_color(base, metallic);
    cspec = derive_specular_color(base, cspec /*float4(0.56, 0.57, 0.58, 1)*/, metallic);
    float a = dot(n, l);
    float roughness = rs;
    float kd = 1.0;
    float kdiff = 1.0;
    float3 brdf = standard_shading_ue4(roughness, kd, cspec, cdiff, kdiff, l, v, n);
    //��������clamp���кڰߵ�
    //use hdr
    float3 ret = clamp(brdf, float3(0, 0, 0), float3(1000, 1000, 1000))
     * clight.xyz * saturate(a);
    return  brdf* clight.xyz * saturate(a);
}

float3 diffuse_ibl(float rs, float metallic, float3 diffuse, float cspec, float3 n, float3 v, float3 l, float3 clight)
{

    float3 base = diffuse; //float3(0.5,0.5,0.5);//191.0/255.0,173.0/255.0,111.0/255.0);
    float3 cdiff = derive_diffuse_color(base, metallic);
    float roughness = rs;
    float3 diff = diffuse_lambert(cdiff);
    cspec = derive_specular_color(base, cspec /*float4(0.56, 0.57, 0.58, 1)*/, metallic);
    return diff * clight / PI;
  
}

float3 specular_ibl(float rs, float metallic, float3 diffuse, float3 cspec, float3 n, float3 v, float3 l, float3 clight)
{
    float3 base = diffuse; //float3(0.5,0.5,0.5);//191.0/255.0,173.0/255.0,111.0/255.0);
    cspec = derive_specular_color(base, cspec /*float4(0.56, 0.57, 0.58, 1)*/, metallic);
    float3 h = normalize(v + l);
    //��ʱ����һ��base��ǿ������blendЧ��
    return cspec * clight /*f_schlick_ue4(cspec,v,h)*/ * (1 - rs) * metallic;
}

struct PointLight
{
    float3 positionView;
    float attenuationBegin;
    float3 color;
    float attenuationEnd;
    float4 AttenuationParam;
};

struct VOut
{
  float4 position : SV_POSITION;
  float3 positionw : POSITION1;
  float3 normal:NORMAL;
  float2 tex : TEXCOORD;
};

cbuffer ToneMapping
{
    float4 ABCD;
    float3 EFW;
    float ExposureBias;
    float ExposureAdjustment;
};

cbuffer Mat
{
    matrix m1;
    matrix v1;
};

SamplerState texture_state;
Texture2D diffuse;
Texture2D normal;
Texture2D metallic;
Texture2D roughness;
Texture2D alpha_mask;
StructuredBuffer<PointLight> gLight;

float3 Uncharted2Tonemap(float3 x)
{
    return ((x * (ABCD.x * x + ABCD.z * ABCD.y) + ABCD.w * EFW.x) / (x * (ABCD.x * x + ABCD.y) + ABCD.w * EFW.y)) - EFW.x / EFW.y;
}

float3 get_view_normal(VOut input)
{
  //maybe no filtering
    float2 text1 = input.tex;
    text1.y = 1 - text1.y;
    float4 origin_n = normal.Sample(texture_state, text1);
    origin_n.b = 1 - origin_n.b;
    origin_n.r = origin_n.a;
    origin_n.xyz = origin_n.xyz * 2 - float3(1, 1, 1);
    origin_n.w = 1;
    float3 vertex_n = normalize(input.normal);
#define A2
#ifdef A1
    //������
//�ο���http://pan.baidu.com/s/1jHH3wZs
    float3 dpdx = ddx(input.positionw);
    float3 dpdy = ddy(input.positionw);
    float dudx = ddx(input.tex.x);
    float dudy = ddy(input.tex.x);

    float3 tangent = normalize(dpdx * dudy - dudx * dpdy);
    float3 binormal = cross(vertex_n, tangent);
    float3x3 tbn;
    tbn[0] = tangent;
    tbn[1] = normalize(binormal);
    tbn[2] = vertex_n;
    return normalize(mul(origin_n, (tbn)));
#endif

#ifdef A2
   // get edge vectors of the pixel triangle
    float3 dp1 = ddx( input.positionw );
    float3 dp2 = ddy( input.positionw );
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
return normalize(mul(origin_n,tbn));
#endif

#ifdef A3
    //������
    float3 dp1 = ddx( input.positionw );
    float3 dp2 = ddy( input.positionw );
    float2 duv1 = ddx( text1 );
    float2 duv2 = ddy( text1 );
float f = 1.0f / (duv1.x * duv2.y - duv2.x * duv1.y);

float3 tangent1;
tangent1.x = f * (duv2.y * dp1.x - duv1.y * dp2.x);
tangent1.y = f * (duv2.y * dp1.y - duv1.y * dp2.y);
tangent1.z = f * (duv2.y * dp1.z - duv1.y * dp2.z);
tangent1 = normalize(tangent1);

float3 binormal = cross((vertex_n),tangent1);

    float3x3 tbn;
    tbn[0] = tangent1;
    tbn[1] = normalize(binormal);
    tbn[2] = (vertex_n);
    float k = dot(tbn[0],tbn[2]);

  return normalize(mul(origin_n,tbn));
#endif
}

float3 red(float3 ccc,float mtc,float rs)
{
    float3 l = ccc * 2 / PI;
    l.b += mtc;
    l.g += rs;
    return l;
}

float3 render_forward(float3 color, float3 n, float mtc, float rs, float3 v, float3 vpos,float mask)
{
    uint totalLights, dummy;
    gLight.GetDimensions(totalLights, dummy);
    float3 ac = float3(0,0,0);
    PointLight light;
    float3 l;
    for (int i = 0; i < totalLights; i++)
    {
        light = gLight[i];
        float3 cc = light.color;
        float4 vp = mul(float4(light.positionView, 1), m1);
        vp = mul(vp, v1);
        float3 l = normalize(vp.xyz - vpos);
        float dist = length(vp.xyz - vpos);
        if (dist < light.attenuationBegin)
            //ac += micro_phong(mtc, color, float3(0.56, 0.57, 0.58), 200, n, v, l, cc);
            ac += standard_shading_ue4_wrap(rs, mtc, color, float3(0.56, 0.57, 0.58), n, v, l, cc );
        else 
           if (dist < light.attenuationEnd)
            //ac += micro_phong(mtc, color, float3(0.56, 0.57, 0.58), 200, n, v, l, cc / (light.AttenuationParam.x * dist * dist * dist + light.AttenuationParam.y * dist * dist + light.AttenuationParam.z * dist + light.AttenuationParam.w));
            ac += standard_shading_ue4_wrap(rs, mtc, color, float3(0.56, 0.57, 0.58), n, v, l, cc / (light.AttenuationParam.x*dist*dist*dist+light.AttenuationParam.y*dist*dist+light.AttenuationParam.z * dist + light.AttenuationParam.w));
    }
    ac /= totalLights;
    return ac;
}


float4 main(VOut input) : SV_TARGET
{
  float2 tex1 = input.tex;
    
  tex1.y = 1 - tex1.y;
    float4 a1 = diffuse.Sample(texture_state, tex1);
    a1.xyz = pow(a1.xyz, 2.2);
    float3 n = get_view_normal(input);
    float mtc = metallic.Sample(texture_state, tex1).r;
    float rs = roughness.Sample(texture_state, tex1).r;
    float msk = alpha_mask.Sample(texture_state,tex1).r;

    float3 v = -normalize(input.positionw);



    float3 c = a1.rgb;
    a1.rgb = render_forward(c, n, mtc, rs, v, input.positionw,msk);
    a1.a = msk;

    //if(isnan(a1.r)||isnan(a1.g)||isnan(a1.b))
      //  return float4(3,0,0,1);
    
    /*
    a.xyz *= ExposureAdjustment;
    a.xyz = Uncharted2Tonemap(ExposureBias * a.xyz);
    float3 whiteScale = 1.f / Uncharted2Tonemap(float3(EFW.zzz));
    a.xyz = a.xyz * whiteScale;
*/
    //a1.xyz = pow(a1.xyz,1.f/2.2f);
  return a1;
}