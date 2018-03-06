Texture2D shaderTexture1 : register(t0);
Texture2D mat_tex : register(t1);
Texture2D normal_map : register(t2);
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

float3 get_view_normal(VOut input)
{
  //maybe no filtering
  float3 origin_n = normal_map.Sample(SampleType,input.tex).xyz*2 - float3(1,1,1);
  float3 vertex_n = normalize(input.normal);
#define A1
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
  float4 mat = mat_tex.Sample(SampleType, input.tex);
  //just for test
  output.rt0 = shaderTexture1.Sample(SampleType,input.tex);//float4(1,0,0,1);
  
#define NORMALMAP 1
#if NORMALMAP
  float3 normal = get_view_normal(input);
  normal = (normal + float3(1,1,1))*0.5;
  output.rt1 = float4(normal,1);
#else
  input.normal = (input.normal + float3(1,1,1))*0.5;
  output.rt1 = float4(input.normal,1);
#endif
  output.rt2 = mat;//float4(input.positionw,mat.g);
  output.rt3.r = input.rec.x / input.rec.y ;
  return output;
}