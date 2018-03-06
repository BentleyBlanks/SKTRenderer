struct GOut
{
  float4 position : SV_POSITION;
  float3 normal : NORMAL;
  float3 color : COLOR;
  float2 tex : TEXCOORD;
  noperspective float2 vp : POSITION1;
  nointerpolation float4 aabb : AABB;
};

#define USE_TEXTURE
#ifdef USE_TEXTURE
Texture2D diffuse_tex : register(t0);
SamplerState tex_sampler : register(s0);
#endif

float4 main(GOut pin) : SV_TARGET
{
    //return float4(pin.color,1);
    float4 aabb = pin.aabb;
    float2 pos = pin.vp;
    /*
    if(aabb.z-aabb.x<0||aabb.w-aabb.y<0||aabb.z-aabb.x>0.2||aabb.w-aabb.y>0.2||aabb.x>0.99||aabb.y>0.99||aabb.z<-0.99||aabb.w<-0.99)
        discard;
    */
    if(pos.x<aabb.x||pos.x>aabb.z||pos.y<aabb.y||pos.y>aabb.w) discard;//return float4(1,0,0,1);
#ifdef USE_TEXTURE
    float4 color = diffuse_tex.Sample(tex_sampler,pin.tex);
    color.a = 1;
    return color;
#else
    float3 n = pin.normal;
    float3 cc = abs(dot(n,normalize(float3(1,1,1))));
    return float4(cc,1);
#endif

}