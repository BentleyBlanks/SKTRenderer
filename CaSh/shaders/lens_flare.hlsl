Texture2D light_tex;
Texture2D light_tex2;
Texture2D light_tex4;
Texture2D light_tex8;
Texture1D lensColor;
SamplerState ls;
cbuffer lems_param
{
  float4 vTint;
  float fTexScale;
  float fBlurScale;
  float fThreshold;// = 0.1f;
  float2 sxy;
  float2 sslight_pos;
  int NumSamples;
};


struct VS_Output
{
  float4 Pos : SV_POSITION;
  noperspective float2 Tex : TEXCOORD0;
};

VS_Output main_vs(uint id : SV_VertexID)
{
  VS_Output Output;
  Output.Tex = float2((id << 1) & 2, id & 2);
  Output.Pos = float4(Output.Tex * float2(2, -2) + float2(-1, 1), 0, 1);
  return Output;
}


const static float4 vPurple = float4(0.7f, 0.2f, 0.9f, 1.0f);
const static float4 vOrange = float4(0.7f, 0.4f, 0.2f, 1.0f);

//flare
float4 main_ps1(VS_Output pin) : SV_TARGET0
{
  float2 in_vTexCoord = pin.Tex;
  // The flare should appear on the opposite side of the screen as the
  // source of the light, so first we mirror the texture coordinate.
  // Then we normalize so we can apply a scaling factor.
  float2 vMirrorCoord = float2(1.0f, 1.0f) - in_vTexCoord;
  float2 vNormalizedCoord = vMirrorCoord * 2.0f - 1.0f;
  vNormalizedCoord *= fTexScale;

  // We'll blur towards the center of screen, and also away from it.
  float2 vTowardCenter = normalize(-vNormalizedCoord);
  float2 fBlurDist = fBlurScale * NumSamples;
  float2 vStartPoint = vNormalizedCoord + ((vTowardCenter / sxy) * fBlurDist);
  float2 vStep = -(vTowardCenter / sxy) * 2 * fBlurDist;
  // Do the blur and sum the samples
  float4 vSum = 0;
  float2 vSamplePos = vStartPoint;
  for (int i = 0; i < NumSamples; i++)
  {
    float2 vSampleTexCoord = vSamplePos * 0.5f + 0.5f;
    // Don't add in samples past texture border
    if (vSampleTexCoord.x >= 0 && vSampleTexCoord.x <= 1.0f
        && vSampleTexCoord.y >= 0 && vSampleTexCoord.y <= 1.0f)
    {
        float4 vSample = (light_tex.Sample(ls, vSampleTexCoord)+light_tex2.Sample(ls, vSampleTexCoord)+light_tex4.Sample(ls, vSampleTexCoord)+light_tex8.Sample(ls, vSampleTexCoord))*0.25f;
        vSum += max(0, vSample - fThreshold) * vTint;
    }
    vSamplePos += vStep;
    //vStep/=1.1;
  }
  return vSum / (NumSamples);
}

//float3 Distortion = float3(0.94f, 0.97f, 1.00f) * 1.5;
float4 sample_light(float2 vSampleTexCoord)
{
  return (light_tex.Sample(ls, vSampleTexCoord)+light_tex2.Sample(ls, vSampleTexCoord)+light_tex4.Sample(ls, vSampleTexCoord)+light_tex8.Sample(ls, vSampleTexCoord))*0.25f;
}
float3 texture2DDistorted(float2 TexCoord, float2 Direction, float3 Distortion)
{
    return float3(
         sample_light( TexCoord + Direction * Distortion.r).r,
         sample_light( TexCoord + Direction * Distortion.g).g,
         sample_light( TexCoord + Direction * Distortion.b).b
    );
}

float3 texture2DDistortedLensColor(float2 TexCoord, float2 Direction, float3 Distortion)
{
    return float3(
        lensColor.Sample(ls, TexCoord + Direction * Distortion.r).r,
        lensColor.Sample(ls, TexCoord + Direction * Distortion.r).g,
        lensColor.Sample(ls, TexCoord + Direction * Distortion.r).b
    );
}

//sun shraft
float4 main_ps11(VS_Output pin) : SV_TARGET0
{
  float2 uv = pin.Tex;
	// Calculate Sun rays
	int samples = 100;
	float intensity = 0.125, decay = 0.97375;
  //sslight_pos+=float2(0.5,-0.5);
	float2 direction = sslight_pos - uv, texCoord = uv;
	direction /= samples;
	float3 color =   sample_light(texCoord).rgb;

	for (int Sample = 0; Sample < samples; Sample++)
	{
		color +=  sample_light(texCoord).rgb * intensity;
		intensity *= decay;
		texCoord += direction;
	}

	// Calculate Lens flare ghosts
	float dispersal = 0.15, distortion = 1.25;
	int ghosts = 6;

	texCoord = float2(1.0,1.0) - uv;
	float2 texelSize = 1.0 / sxy;
	float3 Distortion = float3(-texelSize.x * distortion, -texelSize.y * distortion, texelSize.x * distortion);
	float2 ghostfloat = (float2(0.5,0.5) - texCoord) * dispersal;
	direction = normalize(ghostfloat);
	float3 result = float3(0.0,0,0);
	for (int i = 0; i < ghosts; i++)
	{
		float2 offset = frac(texCoord + ghostfloat * float(i));
		float weight = length(float2(0.5,0.5) - offset) / length(float2(0.5,0.5));
		result += texture2DDistorted( offset, direction, Distortion) * weight;
	}

	// Radial gradient of 1D rainbow color texture
	result *= lensColor.Sample(ls, length(float2(0.5,0.5) - texCoord) / length(float2(0.5,0.5))).rgb;

	float3 finalColor = color + result;
/*
if(length(sslight_pos-uv)<=0.01)
{
  return float4(1,0,0,1);
}
else
*/
	return float4(finalColor, 1.0);
}

float4 main_ps111(VS_Output pin) : SV_TARGET0
{
  float2 uv = pin.Tex;
	float dispersal = 0.2, haloWidth = 0.45;
	float2 texCoord =  uv;
	float2 ghostVec = (float2(0.5,0.5) - texCoord) * dispersal;
	float2 haloVec = normalize(ghostVec) * haloWidth;
	float2 direction = normalize(ghostVec);
	float3 Distortion = float3(0.94f, 0.97f, 1.00f);
	float weight = length(float2(0.5,0.5) - frac(texCoord + haloVec)) / length(float2(0.5,0.5));
	weight = pow(1.0 - weight, 10.0);
	float3 halo = texture2DDistorted(
		texCoord,
		direction * haloWidth,
		Distortion
	);
  halo = max(0, halo - fThreshold) * vTint;
	//halo /= 5.0;

	return float4(halo*fBlurScale, 1.0);
}

float3 halo(float2 uv,float dispersal)
{
    //halo screen radius
    float haloWidth = 0.5;
    float2 texCoord =  uv;
    float2 ghostVec = (float2(0.5,0.5) - texCoord) * dispersal;
    float2 haloVec = normalize(ghostVec) * haloWidth;
    float2 direction = normalize(ghostVec);
    float3 Distortion = float3(0.94f, 0.97f, 1.00f);
    float weight = length(float2(0.5,0.5) - frac(texCoord + haloVec)) / length(float2(0.5,0.5));
    weight = pow(1.0 - weight, 10);
    float3 halo = texture2DDistorted(
        texCoord,
        direction * haloWidth,
        Distortion
    );

    halo = max(0, halo*weight - fThreshold) * vTint;
    //halo /= 5.0;
    //return weight;
    return halo*fBlurScale;
}

float4 main_ps(VS_Output pin):SV_TARGET0
{
    float2 uv = pin.Tex;
    float2 texcoord = float2(1.0,1.0)-uv;
    float dispersal = 0.3;
    int uGhosts = 6;
    float uDistortion = 1;
    float2 ghostVec = (float2(0.5,0.5) - texcoord) * dispersal;
    float2 texelSize = 1.0 / sxy;
    float3 distortion = float3(-texelSize.x * uDistortion, -texelSize.y * uDistortion, texelSize.x * uDistortion);
    float2 direction = normalize(ghostVec);
    float4 res = float4(0,0,0,0);
    for(int i = 0; i < uGhosts; ++i)
    {
        float2 offset = frac(texcoord + ghostVec * float(i));
        float weight = length(float2(0.5,0.5) - offset) / length(float2(0.5,0.5));
        weight = pow(1.0 - weight, 10.0);
        res.rgb += texture2DDistorted(offset,direction,distortion)* weight;
    }
    //float3 lenc = texture2DDistortedLensColor(length(float2(0.5,0.5) - texcoord)/length(float2(0.5,0.5)),direction,distortion);
    float3 lenc = lensColor.Sample(ls, length(float2(0.5,0.5) - texcoord)/length(float2(0.5,0.5)));
    res.rgb*= lenc ;

    //halo
    res.rgb += halo(uv,dispersal)*lenc;
    res.w = 1;
    return res;

}

//http://john-chapman-graphics.blogspot.com/2013/02/pseudo-lens-flare.html