struct GOut
{
  float4 position : SV_POSITION;
  float3 normal : NORMAL;
  float3 color : COLOR;
  float2 tex : TEXCOORD;
  noperspective float2 vp : POSITION1;
  float3 vpw : POSITION2;  
  nointerpolation uint main_axis_index : INDEX;
  nointerpolation float4 aabb : AABB;

};

#define SVO

#ifdef SVO
struct Voxel
{
    float4 color;
    float3 pos;
};
RWStructuredBuffer<Voxel> voxel_list : register(u1);
RWStructuredBuffer<uint> voxel_total_number : register(u2);
#else
RWTexture3D<uint> voxel_volum : register(u1);
#endif



Texture2D diffuse_tex : register(t0);
SamplerState tex_sampler : register(s0);


uint convVec4ToRGBA8(float4 val)
{
	return (uint (val.w) & 0x000000FF) << 24U | (uint(val.z) & 0x000000FF) << 16U | (uint(val.y) & 0x000000FF) << 8U | (uint(val.x) & 0x000000FF);
}

float4 convRGBA8ToVec4(uint val)
{
	return float4(float((val & 0x000000FF)), float((val & 0x0000FF00) >> 8U), float((val & 0x00FF0000) >> 16U), float((val & 0xFF000000) >> 24U));
}

//Averages the color stored in a specific texel
void imageAtomicRGBA8Avg(RWTexture3D<uint> imgUI, uint3 coords, float4 val)
{
	val.rgb *= 255.0f; // Optimize following calculations
	uint newVal = convVec4ToRGBA8(val);
	uint prevStoredVal = 0;
	uint curStoredVal = 0;

	// Loop as long as destination value gets changed by other threads
	
	[allow_uav_condition] do //While loop does not work and crashes the graphics driver, but do while loop that does the same works; compiler error?
	{
		InterlockedCompareExchange(imgUI[coords], prevStoredVal, newVal, curStoredVal);

		if (curStoredVal == prevStoredVal)
			break;

		prevStoredVal = curStoredVal;
		float4 rval = convRGBA8ToVec4(curStoredVal);
			rval.xyz = (rval.xyz* rval.w); // Denormalize
		float4 curValF = rval + val; // Add new value
			curValF.xyz /= (curValF.w); // Renormalize
		newVal = convVec4ToRGBA8(curValF); 
		
		
	} while (true);
}

void main(GOut pin)
{
    //return float4(pin.color,1);
    float4 aabb = pin.aabb;
    float tx = 2*pin.position.x - 1;
	float ty = 2*(1-pin.position.y) - 1;
    float2 pos = float2(tx,ty);
    pos = pin.vp;
    /*
    if(aabb.z-aabb.x<0||aabb.w-aabb.y<0||aabb.z-aabb.x>0.2||aabb.w-aabb.y>0.2||aabb.x>0.99||aabb.y>0.99||aabb.z<-0.99||aabb.w<-0.99)
        discard;
    */

    if(pos.x<aabb.x||pos.x>aabb.z||pos.y<aabb.y||pos.y>aabb.w) discard;//return float4(1,0,0,1);
    
    int3 texDimensions;
    #ifdef SVO
    texDimensions = 512;
    #else
	   voxel_volum.GetDimensions(texDimensions.x, texDimensions.y, texDimensions.z);
    #endif

     float3 temp = pin.vpw.xyz;
     
     if(pin.main_axis_index==1)
     {
      temp = pin.vpw.zyx;
      //temp.x*=2;
     }
     else if(pin.main_axis_index==2)
     {
      temp = pin.vpw.xzy;
      //temp.z*=-1;
     }
     else if(pin.main_axis_index==3)
     {
      temp = pin.vpw.xyz;
     }
     else
      temp = abs(pin.main_axis_index);
      

     //else
     //voxel_volum[int3(temp.x*texDimensions.x,temp.y*texDimensions.y,temp.z*texDimensions.z)] = 0x58565;
     float3 size = 1.f;
    //temp = clamp(temp,0.51,1);
    int3 tex_loc = int3(temp.x*texDimensions.x*size.x,temp.y*texDimensions.y*size.y,temp.z*texDimensions.z*size.z);
    float2 tex_coord = pin.tex;
    tex_coord.y = 1 - tex_coord.y;
    float4 color = diffuse_tex.Sample(tex_sampler,tex_coord);
    
    //voxel_volum[uint3(pin.tex*128,pin.tex.x*128)].r = (color.r + color.g + color.b)*0.333 ;
#ifdef SVO
[branch]
if(all(tex_loc<texDimensions)&&all(tex_loc>0))
  {
        //debug
//        voxel_total_number[0] = 1;
    //    voxel_list[0].color = float4(1,0,0,1);
        //voxel_list[0].pos = int3(100,100,100);

        //high resolution and override mesh must use imageAtomicRGBA8Avg
      InterlockedAdd(voxel_total_number[0],1);
        uint idx = voxel_total_number.IncrementCounter();
        voxel_list[idx].color = color;
        voxel_list[idx].pos = tex_loc;

    }


#else
    if(all(tex_loc<texDimensions)&&all(tex_loc>0))
        voxel_volum[tex_loc] = convVec4ToRGBA8(color*=255);
      //imageAtomicRGBA8Avg(voxel_volum, tex_loc, (color));
#endif

    //float3 n = pin.normal;
    //float3 cc = abs(dot(n,normalize(float3(1,1,1))));
    //return float4(pin.vpw,1);
}