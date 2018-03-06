//-----------------------------------------------------------------------------
// Configurable Defines
struct VOut
{
  float4 position : SV_POSITION;
  float2 tex : TEXCOORD;
};

cbuffer CB:register(b0)
{
    //用于重建深度
  matrix inv_projection;
  /**
 * SSSS_FOV must be set to the value used to render the scene.
 */
  float sss_fovy;
  /**
 * Light diffusion should occur on the surface of the object, not in a screen 
 * oriented plane. Setting SSSS_FOLLOW_SURFACE to 1 will ensure that diffusion
 * is more accurately calculated, at the expense of more memory accesses.
 */
  int sss_follow_surface;
  /**
  * This parameter specifies the global level of subsurface scattering
  * or, in other words, the width of the filter. It's specified in
  * world space units.
  */
  float sssWidth;

  /**
  * This parameter indicates whether the stencil buffer should be
  * initialized. Should be set to 'true' for the first pass if not
  * previously initialized, to enable optimization of the Second
  * pass.
  */
  int initStencil;
  /**
  * Direction of the blur:
  *   - First pass:   float2(1.0, 0.0)
  *   - Second pass:  float2(0.0, 1.0)
  */
  float2 dir;
  float cam_range;
};

/**
 * This define allows to specify a different source for the SSS strength
 * (instead of using the alpha channel of the color framebuffer). This is
 * useful when the alpha channel of the mian color buffer is used for something
 * else.
 */
#ifndef SSSS_STREGTH_SOURCE
#define SSSS_STREGTH_SOURCE (colorM.a)
#endif




float get_actual_linear_depth(float depth,float2 tex)
{
  float4 ndcpos = float4(tex.x*2-1,(1-tex.y)*2-1,depth,1);
  float4 viewpos = mul(ndcpos,inv_projection);
  viewpos = viewpos/viewpos.w;
  return viewpos.z;
}

//-----------------------------------------------------
SamplerState LinearSampler:register(s0);// { Filter = MIN_MAG_LINEAR_MIP_POINT; AddressU = Clamp; AddressV = Clamp; };
SamplerState PointSampler:register(s1);// { Filter = MIN_MAG_MIP_POINT; AddressU = Clamp; AddressV = Clamp; };
#define SSSSTexture2D Texture2D
#define SSSSSampleLevelZero(tex, coord) tex.SampleLevel(LinearSampler, coord, 0)
#define SSSSSampleLevelZeroPoint(tex, coord) tex.SampleLevel(PointSampler, coord, 0)
#define SSSSSample(tex, coord) SSSSSampleLevelZero(tex, coord)
#define SSSSSamplePoint(tex, coord) SSSSSampleLevelZeroPoint(tex, coord)
#define SSSSSampleLevelZeroOffset(tex, coord, offset) tex.SampleLevel(LinearSampler, coord, 0, offset)
#define SSSSSampleOffset(tex, coord, offset) SSSSSampleLevelZeroOffset(tex, coord, offset)
#define SSSSLerp(a, b, t) lerp(a, b, t)
#define SSSSSaturate(a) saturate(a)
#define SSSSMad(a, b, c) mad(a, b, c)
#define SSSSMul(v, m) mul(v, m)
#define SSSS_FLATTEN [flatten]
#define SSSS_BRANCH [branch]
#define SSSS_UNROLL [unroll]


//-----------------------------------------------------------------------------
// Separable SSS Transmittance Function

float3 SSSSTransmittance(
        /**
         * This parameter allows to control the transmittance effect. Its range
         * should be 0..1. Higher values translate to a stronger effect.
         */
        float translucency,

        /**
         * This parameter should be the same as the 'SSSSBlurPS' one. See below
         * for more details.
         */
        float sssWidth,

        /**
         * Position in world space.
         */
        float3 worldPosition,

        /**
         * Normal in world space.
         */
        float3 worldNormal,

        /**
         * Light vector: lightWorldPosition - worldPosition.
         */
        float3 light,

        /**
         * Linear 0..1 shadow map.
         */
        SSSSTexture2D shadowMap,

        /**
         * Regular world to light space matrix.
         */
        float4x4 lightViewProjection,

        /**
         * Far plane distance used in the light projection matrix.
         */
        float lightFarPlane) 
{
    /**
     * Calculate the scale of the effect.
     */
    float scale = 8.25 * (1.0 - translucency) / sssWidth;
       
    /**
     * First we shrink the position inwards the surface to avoid artifacts:
     * (Note that this can be done once for all the lights)
     */
    float4 shrinkedPos = float4(worldPosition - 0.005 * worldNormal, 1.0);

    /**
     * Now we calculate the thickness from the light point of view:
     */
    float4 shadowPosition = SSSSMul(shrinkedPos, lightViewProjection);
    float d1 = SSSSSample(shadowMap, shadowPosition.xy / shadowPosition.w).r; // 'd1' has a range of 0..1
    float d2 = shadowPosition.z; // 'd2' has a range of 0..'lightFarPlane'
    d1 *= lightFarPlane; // So we scale 'd1' accordingly:
    float d = scale * abs(d1 - d2);

    /**
     * Armed with the thickness, we can now calculate the color by means of the
     * precalculated transmittance profile.
     * (It can be precomputed into a texture, for maximum performance):
     */
    float dd = -d * d;
    float3 profile = float3(0.233, 0.455, 0.649) * exp(dd / 0.0064) +
                     float3(0.1,   0.336, 0.344) * exp(dd / 0.0484) +
                     float3(0.118, 0.198, 0.0)   * exp(dd / 0.187)  +
                     float3(0.113, 0.007, 0.007) * exp(dd / 0.567)  +
                     float3(0.358, 0.004, 0.0)   * exp(dd / 1.99)   +
                     float3(0.078, 0.0,   0.0)   * exp(dd / 7.41);

    /** 
     * Using the profile, we finally approximate the transmitted lighting from
     * the back of the object:
     */
    return profile * SSSSSaturate(0.3 + dot(light, -worldNormal));
}

/**
* This is a SRGB or HDR color input buffer, which should be the final
* color frame, resolved in case of using multisampling. The desired
* SSS strength should be stored in the alpha channel (1 for full
* strength, 0 for disabling SSS). If this is not possible, you an
* customize the source of this value using SSSS_STREGTH_SOURCE.
*
* When using non-SRGB buffers, you
* should convert to linear before processing, and back again to gamma
* space before storing the pixels (see Chapter 24 of GPU Gems 3 for
* more info)
*
* IMPORTANT: WORKING IN A NON-LINEAR SPACE WILL TOTALLY RUIN SSS!
*/
SSSSTexture2D colorTex:register(t0);
 /**
* The linear depth buffer of the scene, resolved in case of using
* multisampling. The resolve should be a simple average to avoid
* artifacts in the silhouette of objects.
*/
SSSSTexture2D depthTex:register(t1);

float3 gaussian(float variance, float r) 
{
    /**
     * We use a falloff to modulate the shape of the profile. Big falloffs
     * spreads the shape making it wider, while small falloffs make it
     * narrower.
     */

     //temp 
float falloff[3];
falloff[0] = 0.5;
 falloff[0] = 0.3;
 falloff[0] = 0.3;

    float g[3];
    for (int i = 0; i < 3; i++) {
        float rr = r / (0.001f + falloff[i]);
        g[i] = exp((-(rr * rr)) / (2.0f * variance)) / (2.0f * 3.14f * variance);
    }
    return float3(g[0],g[1],g[2]);
}


float3 profile(float r) {
    /**
     * We used the red channel of the original skin profile defined in
     * [d'Eon07] for all three channels. We noticed it can be used for green
     * and blue channels (scaled using the falloff parameter) without
     * introducing noticeable differences and allowing for total control over
     * the profile. For example, it allows to create blue SSS gradients, which
     * could be useful in case of rendering blue creatures.
     */
    return  // 0.233f * gaussian(0.0064f, r) + /* We consider this one to be directly bounced light, accounted by the strength parameter (see @STRENGTH) */
               0.100f * gaussian(0.0484f, r) +
               0.118f * gaussian( 0.187f, r) +
               0.113f * gaussian( 0.567f, r) +
               0.358f * gaussian(  1.99f, r) +
               0.078f * gaussian(  7.41f, r);
} 

void calculate_kernel(inout float4 kernel[25],float3 strength)
{
    int nSamples = 25;
    float RANGE = 3.0;
    float EXPONENT = 2.0f;

    // Calculate the offsets:
    float step = 2.0f * RANGE / (nSamples - 1);
    for (int i = 0; i < nSamples; i++) {
        float o = -RANGE + float(i) * step;
        float sign1 = o < 0.0f? -1.0f : 1.0f;
        kernel[i].w = RANGE * sign1 * abs(pow(o, EXPONENT)) / pow(RANGE, EXPONENT);
    }

    // Calculate the weights:
    for (int i = 0; i < nSamples; i++) {
        float w0;
        if(i > 0)
         w0 = abs(kernel[i].w - kernel[i - 1].w);
        else
          w0 = 0.0f;
        float w1; 
        if( i < nSamples - 1)
        w1 = abs(kernel[i].w - kernel[i + 1].w) ;
        else
        w1 =  0.0f;
        float area = (w0 + w1) / 2.0f;
        float3 t = area * profile(kernel[i].w);
        kernel[i].x = t.x;
        kernel[i].y = t.y;
        kernel[i].z = t.z;
    }

    // We want the offset 0.0 to come first:
    float4 t = kernel[nSamples / 2];
    for (int i = nSamples / 2; i > 0; i--)
        kernel[i] = kernel[i - 1];
    kernel[0] = t;
    
    // Calculate the sum of the weights, we will need to normalize them below:
    float3 sum = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < nSamples; i++)
        sum += float3(kernel[i].xyz);

    // Normalize the weights:
    for (int i = 0; i < nSamples; i++) {
        kernel[i].x /= sum.x;
        kernel[i].y /= sum.y;
        kernel[i].z /= sum.z;
    }

    // Tweak them using the desired strength. The first one is:
    //     lerp(1.0, kernel[0].rgb, strength)
    kernel[0].x = (1.0f - strength.x) * 1.0f + strength.x * kernel[0].x;
    kernel[0].y = (1.0f - strength.y) * 1.0f + strength.y * kernel[0].y;
    kernel[0].z = (1.0f - strength.z) * 1.0f + strength.z * kernel[0].z;

    // The others:
    //     lerp(0.0, kernel[0].rgb, strength)
    for (int i = 1; i < nSamples; i++) {
        kernel[i].x *= strength.x;
        kernel[i].y *= strength.y;
        kernel[i].z *= strength.z;
    }
}

//-----------------------------------------------------------------------------
// Separable SSS Reflectance Pixel Shader

float4 PsMain(VOut input) : SV_TARGET0
{

/**
 * Here you have ready-to-use kernels for quickstarters. Three kernels are 
 * readily available, with varying quality.
 * To create new kernels take a look into SSS::calculateKernel, or simply
 * push CTRL+C in the demo to copy the customized kernel into the clipboard.
 *
 * Note: these preset kernels are not used by the demo. They are calculated on
 * the fly depending on the selected values in the interface, by directly using
 * SSS::calculateKernel.
 *
 * Quality ranges from 0 to 2, being 2 the highest quality available.
 * The quality is with respect to 1080p; for 720p Quality=0 suffices.
 */
#define SSSS_QUALITY 2

#if SSSS_QUALITY == 2
#define SSSS_N_SAMPLES 25
float4 kernel[] = {
    float4(0.530605, 0.613514, 0.739601, 0),
    float4(0.000973794, 1.11862e-005, 9.43437e-007, -3),
    float4(0.00333804, 7.85443e-005, 1.2945e-005, -2.52083),
    float4(0.00500364, 0.00020094, 5.28848e-005, -2.08333),
    float4(0.00700976, 0.00049366, 0.000151938, -1.6875),
    float4(0.0094389, 0.00139119, 0.000416598, -1.33333),
    float4(0.0128496, 0.00356329, 0.00132016, -1.02083),
    float4(0.017924, 0.00711691, 0.00347194, -0.75),
    float4(0.0263642, 0.0119715, 0.00684598, -0.520833),
    float4(0.0410172, 0.0199899, 0.0118481, -0.333333),
    float4(0.0493588, 0.0367726, 0.0219485, -0.1875),
    float4(0.0402784, 0.0657244, 0.04631, -0.0833333),
    float4(0.0211412, 0.0459286, 0.0378196, -0.0208333),
    float4(0.0211412, 0.0459286, 0.0378196, 0.0208333),
    float4(0.0402784, 0.0657244, 0.04631, 0.0833333),
    float4(0.0493588, 0.0367726, 0.0219485, 0.18750),
    float4(0.0410172, 0.0199899, 0.0118481, 0.3333330),
    float4(0.0263642, 0.0119715, 0.00684598, 0.5208330),
    float4(0.017924, 0.00711691, 0.00347194, 0.750),
    float4(0.0128496, 0.00356329, 0.00132016, 1.020830),
    float4(0.0094389, 0.00139119, 0.000416598, 1.333330),
    float4(0.00700976, 0.00049366, 0.000151938, 1.68750),
    float4(0.00500364, 0.00020094, 5.28848e-005, 2.083330),
    float4(0.00333804, 7.85443e-005, 1.2945e-005, 2.52083),
    float4(0.000973794, 1.11862e-005, 9.43437e-007, 3),
};
#elif SSSS_QUALITY == 1
#define SSSS_N_SAMPLES 17
float4 kernel[] = {
    float4(0.536343, 0.624624, 0.748867, 0),
    float4(0.00317394, 0.000134823, 3.77269e-005, -2),
    float4(0.0100386, 0.000914679, 0.000275702, -1.53125),
    float4(0.0144609, 0.00317269, 0.00106399, -1.125),
    float4(0.0216301, 0.00794618, 0.00376991, -0.78125),
    float4(0.0347317, 0.0151085, 0.00871983, -0.5),
    float4(0.0571056, 0.0287432, 0.0172844, -0.28125),
    float4(0.0582416, 0.0659959, 0.0411329, -0.125),
    float4(0.0324462, 0.0656718, 0.0532821, -0.03125),
    float4(0.0324462, 0.0656718, 0.0532821, 0.03125),
    float4(0.0582416, 0.0659959, 0.0411329, 0.125),
    float4(0.0571056, 0.0287432, 0.0172844, 0.28125),
    float4(0.0347317, 0.0151085, 0.00871983, 0.5),
    float4(0.0216301, 0.00794618, 0.00376991, 0.78125),
    float4(0.0144609, 0.00317269, 0.00106399, 1.125),
    float4(0.0100386, 0.000914679, 0.000275702, 1.53125),
    float4(0.00317394, 0.000134823, 3.77269e-005, 2),
};
#elif SSSS_QUALITY == 0
#define SSSS_N_SAMPLES 11
float4 kernel[] = {
    float4(0.560479, 0.669086, 0.784728, 0),
    float4(0.00471691, 0.000184771, 5.07566e-005, -2),
    float4(0.0192831, 0.00282018, 0.00084214, -1.28),
    float4(0.03639, 0.0130999, 0.00643685, -0.72),
    float4(0.0821904, 0.0358608, 0.0209261, -0.32),
    float4(0.0771802, 0.113491, 0.0793803, -0.08),
    float4(0.0771802, 0.113491, 0.0793803, 0.08),
    float4(0.0821904, 0.0358608, 0.0209261, 0.32),
    float4(0.03639, 0.0130999, 0.00643685, 0.72),
    float4(0.0192831, 0.00282018, 0.00084214, 1.28),
    float4(0.00471691, 0.000184771, 5.07565e-005, 2),
};
#else
#error Quality must be one of {0,1,2}
#endif




    input.tex.y = 1.0 - input.tex.y;
    float2 texcoord = input.tex;

    // Fetch color of current pixel:
    float4 colorM = SSSSSamplePoint(colorTex, texcoord);
    //return float4(colorM.a*colorM.rgb,1);
    // Initialize the stencil buffer in case it was not already available:
    if (initStencil) // (Checked in compile time, it's optimized away)
        if (SSSS_STREGTH_SOURCE == 0.0) return (0,0,0,0);


    // Fetch linear depth of current pixel:
    //我们的深度必须通过重建来得到
    float depthM = get_actual_linear_depth(SSSSSamplePoint(depthTex, texcoord).r,input.tex);

    // Calculate the sssWidth scale (1.0 for a unit plane sitting on the
    // projection window):
    float distanceToProjectionWindow = 1.0 / tan(0.5 * radians(sss_fovy));
    float scale = distanceToProjectionWindow / depthM;

    
    // Calculate the final step to fetch the surrounding pixels:
    float2 finalStep = sssWidth * scale * dir;
    finalStep *= SSSS_STREGTH_SOURCE; // Modulate it using the alpha channel.
    
    finalStep *= 1.0 / 3.0; // Divide by 3 as the kernels range from -3 to 3.

    // Accumulate the center sample:
    float4 colorBlurred = colorM;
    //colorBlurred.a *= 0.5;
    //calculate_kernel(kernel,float3(SSSS_STREGTH_SOURCE,SSSS_STREGTH_SOURCE,SSSS_STREGTH_SOURCE));
    colorBlurred.rgb *= kernel[0].rgb;
    //return float4(kernel[0].rgb,1);
    // Accumulate the other samples:
    SSSS_UNROLL
    for (int i = 1; i < SSSS_N_SAMPLES; i++) {
        // Fetch color and depth for current sample:
        float2 offset = texcoord + kernel[i].a * finalStep;
        float4 color = SSSSSample(colorTex, offset);
        #if  1
        // If the difference in depth is huge, we lerp color back to "colorM":
        float depth = get_actual_linear_depth(SSSSSample(depthTex, offset).r,offset);
        float s = SSSSSaturate(cam_range * distanceToProjectionWindow *
                               sssWidth * abs(depthM - depth));
        color.rgb = SSSSLerp(color.rgb, colorM.rgb, s);
        #endif

        // Accumulate:
        colorBlurred.rgb += kernel[i].rgb * color.rgb;
    }

    return colorBlurred;
}

//-----------------------------------------------------------------------------
