Texture2D shaderTexture1 : register(t0);
Texture2D opacity_tex : register(t1);
Texture2D normal_map : register(t2);
Texture2D power_tex : register(t3);

SamplerState SampleType;

struct VOut
{
  float4 position : SV_POSITION;
  float3 positionw:POSITIONW;
  float3 normal:NORMAL;
  float3 tangent : TANGENT;
  float3 viewu : VIEWU;
  float3 viewv : VIEWV;
  float2 tex : TEXCOORD;
  float2 rec : POSITIONP;
};

struct POut
{
    float4 rt0 : SV_TARGET0;
    float4 rt1 : SV_TARGET1;
    float4 rt2 : SV_TARGET2;
	  float4 rt3 : SV_TARGET3;
};

cbuffer CBuffer:register(b0)
{
    int tw, th;
};

float3 calc_tangent(float3 dp1,float3 dp2,float2 duv1,float2 duv2)
{
  float f = 1.0f / (duv1.x * duv2.y - duv2.x * duv1.y);

float3 tangent1;
tangent1.x = f * (duv2.y * dp1.x - duv1.y * dp2.x);
tangent1.y = f * (duv2.y * dp1.y - duv1.y * dp2.y);
tangent1.z = f * (duv2.y * dp1.z - duv1.y * dp2.z);

return normalize(tangent1);
}

float3 get_view_tangent(VOut input)
{
//maybe no filtering
  float3 origin_n = normal_map.Sample(SampleType,input.tex).xyz*2 - float3(1,1,1);
  float3 vertex_n = normalize(input.normal);
#define A44


#ifdef A22
   // get edge vectors of the pixel triangle
    float3 dp1 = ddx( input.positionw );
    float3 dp2 = ddy( input.positionw );
    float2 duv1 = ddx( input.tex );
    float2 duv2 = ddy( input.tex );
 
    // solve the linear system
    float3 dp2perp = cross( dp2, vertex_n );
    float3 dp1perp = cross( vertex_n, dp1 );
    float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    float3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
    // construct a scale-invariant frame 
    float invmax = 1/sqrt( max( dot(T,T), dot(B,B) ) );
    return T * invmax;

#endif

#ifdef A33
  float3 dp1 = ddx( input.positionw );
    float3 dp2 = ddy( input.positionw );
    float2 duv1 = ddx( input.tex );
    float2 duv2 = ddy( input.tex );

    float3 t1 = calc_tangent(dp1,dp2,duv1,duv2);
    float3 t2 = calc_tangent(-dp2,dp1,-duv2,duv1);
    float3 t3 = calc_tangent(-dp1,-dp2,-duv1,-duv2);
    float3 t4 = calc_tangent(dp2,-dp1,duv2,-duv1);
    return normalize(t1 + t2 + t3 + t4);


#endif

//#define INTANGENT
#ifdef A44
float3 tangent1 = input.tangent;
float kk = dot(normalize(input.viewv),tangent1);
return normalize(tangent1 - dot((vertex_n) , tangent1) * (vertex_n));


#endif

}

float3 get_view_normal(VOut input)
{
  //maybe no filtering
  float3 origin_n;
  //不要插值法线！会出现很烂的效果。。。
  
  
  //origin_n = normal_map.Load(int3(input.tex.x*tw,(input.tex.y)*th,0)).xyz*2 - float3(1,1,1);
  
 origin_n = normal_map.Sample(SampleType,input.tex).xyz*2 - float3(1,1,1);
  float3 vertex_n = normalize(input.normal);
#define A4
#ifdef A1
//参考：http://pan.baidu.com/s/1jHH3wZs
  float3 dpdx = ddx(input.positionw);
  float3 dpdy = ddy(input.positionw);
  float dudx = ddx(input.tex.x);
  float dudy = ddy(input.tex.x);

  float3 tangent = normalize(dpdx*dudy-dudx*dpdy);
  float3 binormal = cross(vertex_n,tangent);
  float3x3 tbn;
  tbn[0] = tangent;
  tbn[1] = normalize(binormal);
    tbn[2] = vertex_n;
return normalize(mul(origin_n,(tbn)));
#endif


#ifdef A2
   // get edge vectors of the pixel triangle
    float3 dp1 = ddx( input.positionw );
    float3 dp2 = ddy( input.positionw );
    float2 duv1 = ddx( input.tex );
    float2 duv2 = ddy( input.tex );
 
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
    float3 dp1 = ddx( input.positionw );
    float3 dp2 = ddy( input.positionw );
    float2 duv1 = ddx( input.tex );
    float2 duv2 = ddy( input.tex );
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

//#define INTANGENT
#ifdef A4
float3 tangent1 = input.tangent;
float kk = dot(normalize(input.viewv),tangent1);
tangent1 = normalize(tangent1 - dot((vertex_n) , tangent1) * (vertex_n));

float3 binormal = cross((vertex_n),tangent1);

    float3x3 tbn;
    tbn[0] = tangent1;
    tbn[1] = normalize(binormal);
    tbn[2] = (vertex_n);
    float k = dot(tbn[0],tbn[2]);

  return normalize(mul(origin_n,tbn));
#endif

}

POut PsMain(VOut input)
{
  POut output;
  //power
  float4 power = power_tex.Sample(SampleType, input.tex);
  //opacity
  float opacity = opacity_tex.Sample(SampleType,input.tex).r;
  //diffuse
  output.rt0 = shaderTexture1.Sample(SampleType,input.tex);//float4(1,0,0,1);
  //output.rt0.xyz = float3(208/255.f,158/255.f,130/255.f);
  //normal and tangent

#define NORMALMAP 1
#if NORMALMAP
  float3 normal = get_view_normal(input);
  normal = (normal + float3(1,1,1))*0.5;
  float3 tangent = get_view_tangent(input);
  tangent = (tangent + float3(1,1,1))*0.5;
  output.rt1 = float4(normal.x,normal.y,tangent.x,tangent.y);
  output.rt0.a = (tangent.z + 1)*0.5;
#else
  input.normal = normalize(input.normal);
  input.normal = (input.normal + float3(1,1,1))*0.5;
  output.rt1 = float4(input.normal.x,input.normal.y,input.tangent.x,input.tangent.y);
  output.rt0.a = (input.tangent.z + 1)*0.5;
#endif
  output.rt2 = power;//float4(input.positionw,mat.g);
  output.rt3 = input.rec.x / input.rec.y;
  //output.rt3.b = 1;
  
  return output;
}