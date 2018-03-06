void FAST32_hash_2D( float2 gridcell, out float4 hash_0, out float4 hash_1 )  //  generates 2 random numbers for each of the 4 cell corners
{
    //    gridcell is assumed to be an integer coordinate
    const float2 OFFSET = float2( 26.0, 161.0 );
    const float DOMAIN = 71.0;
    const float2 SOMELARGEFLOATS = float2( 951.135664, 642.949883 );
    float4 P = float4( gridcell.xy, gridcell.xy + 1.0 );
    P = P - floor(P * ( 1.0 / DOMAIN )) * DOMAIN;
    P += OFFSET.xyxy;
    P *= P;
    P = P.xzxz * P.yyww;
    hash_0 = frac( P * ( 1.0 / SOMELARGEFLOATS.x ) );
    hash_1 = frac( P * ( 1.0 / SOMELARGEFLOATS.y ) );
}

void FAST32_hash_3D(  float3 gridcell,
                        out float4 lowz_hash_0,
                        out float4 lowz_hash_1,
                        out float4 lowz_hash_2,
                        out float4 highz_hash_0,
                        out float4 highz_hash_1,
                        out float4 highz_hash_2 )   //  generates 3 random numbers for each of the 8 cell corners
{
    //    gridcell is assumed to be an integer coordinate

    //  TODO:   these constants need tweaked to find the best possible noise.
    //      probably requires some kind of brute force computational searching or something....
    const float2 OFFSET = float2( 50.0, 161.0 );
    const float DOMAIN = 69.0;
    const float3 SOMELARGEFLOATS = float3( 635.298681, 682.357502, 668.926525 );
    const float3 ZINC = float3( 48.500388, 65.294118, 63.934599 );

    //  truncate the domain
    gridcell.xyz = gridcell.xyz - floor(gridcell.xyz * ( 1.0 / DOMAIN )) * DOMAIN;
    float3 gridcell_inc1 = step( gridcell, float3( DOMAIN - 1.5,DOMAIN - 1.5,DOMAIN - 1.5 ) ) * ( gridcell + 1.0 );

    //  calculate the noise
    float4 P = float4( gridcell.xy, gridcell_inc1.xy ) + OFFSET.xyxy;
    P *= P;
    P = P.xzxz * P.yyww;
    float3 lowz_mod = float3( 1.0 / ( SOMELARGEFLOATS.xyz + gridcell.zzz * ZINC.xyz ) );
    float3 highz_mod = float3( 1.0 / ( SOMELARGEFLOATS.xyz + gridcell_inc1.zzz * ZINC.xyz ) );
    lowz_hash_0 = frac( P * lowz_mod.xxxx );
    highz_hash_0 = frac( P * highz_mod.xxxx );
    lowz_hash_1 = frac( P * lowz_mod.yyyy );
    highz_hash_1 = frac( P * highz_mod.yyyy );
    lowz_hash_2 = frac( P * lowz_mod.zzzz );
    highz_hash_2 = frac( P * highz_mod.zzzz );
}

//  convert a 0.0->1.0 sample to a -1.0->1.0 sample weighted towards the extremes
float4 Cellular_weight_samples( float4 samples )
{
    samples = samples * 2.0 - 1.0;
    //return (1.0 - samples * samples) * sign(samples); // square
    return (samples * samples * samples) - sign(samples); // cubic (even more variance)
}

//
//  Cellular Noise 2D
//  Based off Stefan Gustavson's work at http://www.itn.liu.se/~stegu/GLSL-cellular
//  http://briansharpe.files.wordpress.com/2011/12/cellularsample.jpg
//
//  Speed up by using 2x2 search window instead of 3x3
//  produces a range of 0.0->1.0
//
float Cellular2D(float2 P)
{
    //  establish our grid cell and unit position
    float2 Pi = floor(P);
    float2 Pf = P - Pi;

    //  calculate the hash.
    //  ( various hashing methods listed in order of speed )
    float4 hash_x, hash_y;
    FAST32_hash_2D( Pi, hash_x, hash_y );
    //SGPP_hash_2D( Pi, hash_x, hash_y );

    //  generate the 4 random points
#if 1
    //  restrict the random point offset to eliminate artifacts
    //  we'll improve the variance of the noise by pushing the points to the extremes of the jitter window
    const float JITTER_WINDOW = 0.25; // 0.25 will guarentee no artifacts.  0.25 is the intersection on x of graphs f(x)=( (0.5+(0.5-x))^2 + (0.5-x)^2 ) and f(x)=( (0.5+x)^2 + x^2 )
    hash_x = Cellular_weight_samples( hash_x ) * JITTER_WINDOW + float4(0.0, 1.0, 0.0, 1.0);
    hash_y = Cellular_weight_samples( hash_y ) * JITTER_WINDOW + float4(0.0, 0.0, 1.0, 1.0);
#else
    //  non-weighted jitter window.  jitter window of 0.4 will give results similar to Stefans original implementation
    //  nicer looking, faster, but has minor artifacts.  ( discontinuities in signal )
    const float JITTER_WINDOW = 0.4;
    hash_x = hash_x * JITTER_WINDOW * 2.0 + float4(-JITTER_WINDOW, 1.0-JITTER_WINDOW, -JITTER_WINDOW, 1.0-JITTER_WINDOW);
    hash_y = hash_y * JITTER_WINDOW * 2.0 + float4(-JITTER_WINDOW, -JITTER_WINDOW, 1.0-JITTER_WINDOW, 1.0-JITTER_WINDOW);
#endif

    //  return the closest squared distance
    float4 dx = Pf.xxxx - hash_x;
    float4 dy = Pf.yyyy - hash_y;
    float4 d = dx * dx + dy * dy;
    d.xy = min(d.xy, d.zw);
    return min(d.x, d.y) * ( 1.0 / 1.125 ); //  scale return value from 0.0->1.125 to 0.0->1.0  ( 0.75^2 * 2.0  == 1.125 )
}
//
//  Cellular Noise 3D
//  Based off Stefan Gustavson's work at http://www.itn.liu.se/~stegu/GLSL-cellular
//  http://briansharpe.files.wordpress.com/2011/12/cellularsample.jpg
//
//  Speed up by using 2x2x2 search window instead of 3x3x3
//  produces range of 0.0->1.0
//
float Cellular3D(float3 P)
{
    //  establish our grid cell and unit position
    float3 Pi = floor(P);
    float3 Pf = P - Pi;

    //  calculate the hash.
    //  ( various hashing methods listed in order of speed )
    float4 hash_x0, hash_y0, hash_z0, hash_x1, hash_y1, hash_z1;
    FAST32_hash_3D( Pi, hash_x0, hash_y0, hash_z0, hash_x1, hash_y1, hash_z1 );
    //SGPP_hash_3D( Pi, hash_x0, hash_y0, hash_z0, hash_x1, hash_y1, hash_z1 );

    //  generate the 8 random points
#if 1
    //  restrict the random point offset to eliminate artifacts
    //  we'll improve the variance of the noise by pushing the points to the extremes of the jitter window
    const float JITTER_WINDOW = 0.166666666;  // 0.166666666 will guarentee no artifacts. It is the intersection on x of graphs f(x)=( (0.5 + (0.5-x))^2 + 2*((0.5-x)^2) ) and f(x)=( 2 * (( 0.5 + x )^2) + x * x )
    hash_x0 = Cellular_weight_samples( hash_x0 ) * JITTER_WINDOW + float4(0.0, 1.0, 0.0, 1.0);
    hash_y0 = Cellular_weight_samples( hash_y0 ) * JITTER_WINDOW + float4(0.0, 0.0, 1.0, 1.0);
    hash_x1 = Cellular_weight_samples( hash_x1 ) * JITTER_WINDOW + float4(0.0, 1.0, 0.0, 1.0);
    hash_y1 = Cellular_weight_samples( hash_y1 ) * JITTER_WINDOW + float4(0.0, 0.0, 1.0, 1.0);
    hash_z0 = Cellular_weight_samples( hash_z0 ) * JITTER_WINDOW + float4(0.0, 0.0, 0.0, 0.0);
    hash_z1 = Cellular_weight_samples( hash_z1 ) * JITTER_WINDOW + float4(1.0, 1.0, 1.0, 1.0);
#else
    //  non-weighted jitter window.  jitter window of 0.4 will give results similar to Stefans original implementation
    //  nicer looking, faster, but has minor artifacts.  ( discontinuities in signal )
    const float JITTER_WINDOW = 0.4;
    hash_x0 = hash_x0 * JITTER_WINDOW * 2.0 + float4(-JITTER_WINDOW, 1.0-JITTER_WINDOW, -JITTER_WINDOW, 1.0-JITTER_WINDOW);
    hash_y0 = hash_y0 * JITTER_WINDOW * 2.0 + float4(-JITTER_WINDOW, -JITTER_WINDOW, 1.0-JITTER_WINDOW, 1.0-JITTER_WINDOW);
    hash_x1 = hash_x1 * JITTER_WINDOW * 2.0 + float4(-JITTER_WINDOW, 1.0-JITTER_WINDOW, -JITTER_WINDOW, 1.0-JITTER_WINDOW);
    hash_y1 = hash_y1 * JITTER_WINDOW * 2.0 + float4(-JITTER_WINDOW, -JITTER_WINDOW, 1.0-JITTER_WINDOW, 1.0-JITTER_WINDOW);
    hash_z0 = hash_z0 * JITTER_WINDOW * 2.0 + float4(-JITTER_WINDOW, -JITTER_WINDOW, -JITTER_WINDOW, -JITTER_WINDOW);
    hash_z1 = hash_z1 * JITTER_WINDOW * 2.0 + float4(1.0-JITTER_WINDOW, 1.0-JITTER_WINDOW, 1.0-JITTER_WINDOW, 1.0-JITTER_WINDOW);
#endif

    //  return the closest squared distance
    float4 dx1 = Pf.xxxx - hash_x0;
    float4 dy1 = Pf.yyyy - hash_y0;
    float4 dz1 = Pf.zzzz - hash_z0;
    float4 dx2 = Pf.xxxx - hash_x1;
    float4 dy2 = Pf.yyyy - hash_y1;
    float4 dz2 = Pf.zzzz - hash_z1;
    float4 d1 = dx1 * dx1 + dy1 * dy1 + dz1 * dz1;
    float4 d2 = dx2 * dx2 + dy2 * dy2 + dz2 * dz2;
    d1 = min(d1, d2);
    d1.xy = min(d1.xy, d1.wz);
    return min(d1.x, d1.y) * ( 9.0 / 12.0 );  //  scale return value from 0.0->1.333333 to 0.0->1.0   (2/3)^2 * 3  == (12/9) == 1.333333
}


float Interpolation_C2( float x ) { return x * x * x * (x * (x * 6.0 - 15.0) + 10.0); }   //  6x^5-15x^4+10x^3  ( Quintic Curve.  As used by Perlin in Improved Noise.  http://mrl.nyu.edu/~perlin/paper445.pdf )
float2 Interpolation_C2( float2 x ) { return x * x * x * (x * (x * 6.0 - 15.0) + 10.0); }
float3 Interpolation_C2( float3 x ) { return x * x * x * (x * (x * 6.0 - 15.0) + 10.0); }
float4 Interpolation_C2( float4 x ) { return x * x * x * (x * (x * 6.0 - 15.0) + 10.0); }
float4 Interpolation_C2_InterpAndDeriv( float2 x ) { return x.xyxy * x.xyxy * ( x.xyxy * ( x.xyxy * ( x.xyxy * float2( 6.0, 0.0 ).xxyy + float2( -15.0, 30.0 ).xxyy ) + float2( 10.0, -60.0 ).xxyy ) + float2( 0.0, 30.0 ).xxyy ); }
float3 Interpolation_C2_Deriv( float3 x ) { return x * x * (x * (x * 30.0 - 60.0) + 30.0); }

float Interpolation_C2_Fast( float x ) { float x3 = x*x*x; return ( 7.0 + ( x3 - 7.0 ) * x ) * x3; }   //  7x^3-7x^4+x^7   ( Faster than Perlin Quintic.  Not quite as good shape. )
float2 Interpolation_C2_Fast( float2 x ) { float2 x3 = x*x*x; return ( 7.0 + ( x3 - 7.0 ) * x ) * x3; }
float3 Interpolation_C2_Fast( float3 x ) { float3 x3 = x*x*x; return ( 7.0 + ( x3 - 7.0 ) * x ) * x3; }
float4 Interpolation_C2_Fast( float4 x ) { float4 x3 = x*x*x; return ( 7.0 + ( x3 - 7.0 ) * x ) * x3; }

//
//  Perlin Noise 2D  ( gradient noise )
//  Return value range of -1.0->1.0
//  http://briansharpe.files.wordpress.com/2011/11/perlinsample.jpg
//
float Perlin2D( float2 P )
{
    //  establish our grid cell and unit position
    float2 Pi = floor(P);
    float4 Pf_Pfmin1 = P.xyxy - float4( Pi, Pi + 1.0 );

#if 1
    //
    //  classic noise looks much better than improved noise in 2D, and with an efficent hash function runs at about the same speed.
    //  requires 2 random numbers per point.
    //

    //  calculate the hash.
    //  ( various hashing methods listed in order of speed )
    float4 hash_x, hash_y;
    FAST32_hash_2D( Pi, hash_x, hash_y );
    //SGPP_hash_2D( Pi, hash_x, hash_y );

    //  calculate the gradient results
    float4 grad_x = hash_x - 0.49999;
    float4 grad_y = hash_y - 0.49999;
    float4 grad_results = rsqrt( grad_x * grad_x + grad_y * grad_y ) * ( grad_x * Pf_Pfmin1.xzxz + grad_y * Pf_Pfmin1.yyww );

#if 1
    //  Classic Perlin Interpolation
    grad_results *= 1.4142135623730950488016887242097;    //  (optionally) scale things to a strict -1.0->1.0 range    *= 1.0/sqrt(0.5)
    float2 blend = Interpolation_C2( Pf_Pfmin1.xy );
    float4 blend2 = float4( blend, float2( 1.0 - blend ) );
    return dot( grad_results, blend2.zxzx * blend2.wwyy );
#else
    //  Classic Perlin Surflet
    //  http://briansharpe.wordpress.com/2012/03/09/modifications-to-classic-perlin-noise/
    grad_results *= 2.3703703703703703703703703703704;    //  (optionally) scale things to a strict -1.0->1.0 range    *= 1.0/cube(0.75)
    float4 floats_len_sq = Pf_Pfmin1 * Pf_Pfmin1;
    floats_len_sq = floats_len_sq.xzxz + floats_len_sq.yyww;
    return dot( Falloff_Xsq_C2( min( float4( 1.0 ), floats_len_sq ) ), grad_results );
#endif

#else
    //
    //  2D improved perlin noise.
    //  requires 1 random value per point.
    //  does not look as good as classic in 2D due to only a small number of possible cell types.  But can run a lot faster than classic perlin noise if the hash function is slow
    //

    //  calculate the hash.
    //  ( various hashing methods listed in order of speed )
    float4 hash = FAST32_hash_2D( Pi );
    //float4 hash = BBS_hash_2D( Pi );
    //float4 hash = SGPP_hash_2D( Pi );
    //float4 hash = BBS_hash_hq_2D( Pi );

    //
    //  evaulate the gradients
    //  choose between the 4 diagonal gradients.  ( slightly slower than choosing the axis gradients, but shows less grid artifacts )
    //  NOTE:  diagonals give us a nice strict -1.0->1.0 range without additional scaling
    //  [1.0,1.0] [-1.0,1.0] [1.0,-1.0] [-1.0,-1.0]
    //
    hash -= 0.5;
    float4 grad_results = Pf_Pfmin1.xzxz * sign( hash ) + Pf_Pfmin1.yyww * sign( abs( hash ) - 0.25 );

    //  blend the results and return
    float2 blend = Interpolation_C2( Pf_Pfmin1.xy );
    float4 blend2 = float4( blend, float2( 1.0 - blend ) );
    return dot( grad_results, blend2.zxzx * blend2.wwyy );

#endif

}



//
//  Perlin Noise 3D  ( gradient noise )
//  Return value range of -1.0->1.0
//  http://briansharpe.files.wordpress.com/2011/11/perlinsample.jpg
//
float Perlin3D( float3 P )
{
    //  establish our grid cell and unit position
    float3 Pi = floor(P);
    float3 Pf = P - Pi;
    float3 Pf_min1 = Pf - 1.0;

#if 1
    //
    //  classic noise.
    //  requires 3 random values per point.  with an efficent hash function will run faster than improved noise
    //

    //  calculate the hash.
    //  ( various hashing methods listed in order of speed )
    float4 hashx0, hashy0, hashz0, hashx1, hashy1, hashz1;
    FAST32_hash_3D( Pi, hashx0, hashy0, hashz0, hashx1, hashy1, hashz1 );
    //SGPP_hash_3D( Pi, hashx0, hashy0, hashz0, hashx1, hashy1, hashz1 );

    //  calculate the gradients
    float4 grad_x0 = hashx0 - 0.49999;
    float4 grad_y0 = hashy0 - 0.49999;
    float4 grad_z0 = hashz0 - 0.49999;
    float4 grad_x1 = hashx1 - 0.49999;
    float4 grad_y1 = hashy1 - 0.49999;
    float4 grad_z1 = hashz1 - 0.49999;
    float4 grad_results_0 = rsqrt( grad_x0 * grad_x0 + grad_y0 * grad_y0 + grad_z0 * grad_z0 ) * ( float2( Pf.x, Pf_min1.x ).xyxy * grad_x0 + float2( Pf.y, Pf_min1.y ).xxyy * grad_y0 + Pf.zzzz * grad_z0 );
    float4 grad_results_1 = rsqrt( grad_x1 * grad_x1 + grad_y1 * grad_y1 + grad_z1 * grad_z1 ) * ( float2( Pf.x, Pf_min1.x ).xyxy * grad_x1 + float2( Pf.y, Pf_min1.y ).xxyy * grad_y1 + Pf_min1.zzzz * grad_z1 );

#if 1
    //  Classic Perlin Interpolation
    float3 blend = Interpolation_C2( Pf );
    float4 res0 = lerp( grad_results_0, grad_results_1, blend.z );
    float4 blend2 = float4( blend.xy, float2( 1.0 - blend.xy ) );
    float final = dot( res0, blend2.zxzx * blend2.wwyy );
    final *= 1.1547005383792515290182975610039;   //  (optionally) scale things to a strict -1.0->1.0 range    *= 1.0/sqrt(0.75)
    return final;
#else
    //  Classic Perlin Surflet
    //  http://briansharpe.wordpress.com/2012/03/09/modifications-to-classic-perlin-noise/
    Pf *= Pf;
    Pf_min1 *= Pf_min1;
    float4 floats_len_sq = float4( Pf.x, Pf_min1.x, Pf.x, Pf_min1.x ) + float4( Pf.yy, Pf_min1.yy );
    float final = dot( Falloff_Xsq_C2( min( float4( 1.0 ), floats_len_sq + Pf.zzzz ) ), grad_results_0 ) + dot( Falloff_Xsq_C2( min( float4( 1.0 ), floats_len_sq + Pf_min1.zzzz ) ), grad_results_1 );
    final *= 2.3703703703703703703703703703704;   //  (optionally) scale things to a strict -1.0->1.0 range    *= 1.0/cube(0.75)
    return final;
#endif

#else
    //
    //  improved noise.
    //  requires 1 random value per point.  Will run faster than classic noise if a slow hashing function is used
    //

    //  calculate the hash.
    //  ( various hashing methods listed in order of speed )
    float4 hash_lowz, hash_highz;
    FAST32_hash_3D( Pi, hash_lowz, hash_highz );
    //BBS_hash_3D( Pi, hash_lowz, hash_highz );
    //SGPP_hash_3D( Pi, hash_lowz, hash_highz );

    //
    //  "improved" noise using 8 corner gradients.  Faster than the 12 mid-edge point method.
    //  Ken mentions using diagonals like this can cause "clumping", but we'll live with that.
    //  [1,1,1]  [-1,1,1]  [1,-1,1]  [-1,-1,1]
    //  [1,1,-1] [-1,1,-1] [1,-1,-1] [-1,-1,-1]
    //
    hash_lowz -= 0.5;
    float4 grad_results_0_0 = float2( Pf.x, Pf_min1.x ).xyxy * sign( hash_lowz );
    hash_lowz = abs( hash_lowz ) - 0.25;
    float4 grad_results_0_1 = float2( Pf.y, Pf_min1.y ).xxyy * sign( hash_lowz );
    float4 grad_results_0_2 = Pf.zzzz * sign( abs( hash_lowz ) - 0.125 );
    float4 grad_results_0 = grad_results_0_0 + grad_results_0_1 + grad_results_0_2;

    hash_highz -= 0.5;
    float4 grad_results_1_0 = float2( Pf.x, Pf_min1.x ).xyxy * sign( hash_highz );
    hash_highz = abs( hash_highz ) - 0.25;
    float4 grad_results_1_1 = float2( Pf.y, Pf_min1.y ).xxyy * sign( hash_highz );
    float4 grad_results_1_2 = Pf_min1.zzzz * sign( abs( hash_highz ) - 0.125 );
    float4 grad_results_1 = grad_results_1_0 + grad_results_1_1 + grad_results_1_2;

    //  blend the gradients and return
    float3 blend = Interpolation_C2( Pf );
    float4 res0 = lerp( grad_results_0, grad_results_1, blend.z );
    float4 blend2 = float4( blend.xy, float2( 1.0 - blend.xy ) );
    return dot( res0, blend2.zxzx * blend2.wwyy ) * (2.0 / 3.0);  //  (optionally) mult by (2.0/3.0) to scale to a strict -1.0->1.0 range
#endif

}


//
// Description : Array and textureless GLSL 2D/3D/4D simplex 
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
// 

float3 mod289(float3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

float4 mod289(float4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

float4 permute(float4 x) {
     return mod289(((x*34.0)+1.0)*x);
}

float4 taylorInvSqrt(float4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(float3 v)
  { 
  const float2  C = float2(1.0/6.0, 1.0/3.0) ;
  const float4  D = float4(0.0, 0.5, 1.0, 2.0);

// First corner
  float3 i  = floor(v + dot(v, C.yyy) );
  float3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  float3 g = step(x0.yzx, x0.xyz);
  float3 l = 1.0 - g;
  float3 i1 = min( g.xyz, l.zxy );
  float3 i2 = max( g.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
  float3 x1 = x0 - i1 + C.xxx;
  float3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  float3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289(i); 
  float4 p = permute( permute( permute( 
             i.z + float4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + float4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + float4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  float3  ns = n_ * D.wyz - D.xzx;

  float4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

  float4 x_ = floor(j * ns.z);
  float4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  float4 x = x_ *ns.x + ns.yyyy;
  float4 y = y_ *ns.x + ns.yyyy;
  float4 h = 1.0 - abs(x) - abs(y);

  float4 b0 = float4( x.xy, y.xy );
  float4 b1 = float4( x.zw, y.zw );

  //float4 s0 = float4(lessThan(b0,0.0))*2.0 - 1.0;
  //float4 s1 = float4(lessThan(b1,0.0))*2.0 - 1.0;
  float4 s0 = floor(b0)*2.0 + 1.0;
  float4 s1 = floor(b1)*2.0 + 1.0;
  float4 sh = -step(h, float4(0.0,0.0,0.0,0.0));

  float4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  float4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  float3 p0 = float3(a0.xy,h.x);
  float3 p1 = float3(a0.zw,h.y);
  float3 p2 = float3(a1.xy,h.z);
  float3 p3 = float3(a1.zw,h.w);

//Normalise gradients
  float4 norm = taylorInvSqrt(float4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// lerp final noise value
  float4 m = max(0.6 - float4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, float4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
  }


  float3 snoisefloat3( float3 x ){

  float s  = snoise(float3( x ));
  float s1 = snoise(float3( x.y - 19.1 , x.z + 33.4 , x.x + 47.2 ));
  float s2 = snoise(float3( x.z + 74.2 , x.x - 124.5 , x.y + 99.4 ));
  float3 c = float3( s , s1 , s2 );
  return c;

}


float3 curlNoise( float3 p ){
  
  const float e = .1;
  float3 dx = float3( e   , 0.0 , 0.0 );
  float3 dy = float3( 0.0 , e   , 0.0 );
  float3 dz = float3( 0.0 , 0.0 , e   );

  float3 p_x0 = snoisefloat3( p - dx );
  float3 p_x1 = snoisefloat3( p + dx );
  float3 p_y0 = snoisefloat3( p - dy );
  float3 p_y1 = snoisefloat3( p + dy );
  float3 p_z0 = snoisefloat3( p - dz );
  float3 p_z1 = snoisefloat3( p + dz );

  float x = p_y1.z - p_y0.z - p_z1.y + p_z0.y;
  float y = p_z1.x - p_z0.x - p_x1.z + p_x0.z;
  float z = p_x1.y - p_x0.y - p_y1.x + p_y0.x;

  const float divisor = 1.0 / ( 2.0 * e );
  return normalize( float3( x , y , z ) * divisor );

}


/*
out float4 FragColor;
uniform sampler2D uPtex;
uniform float2 dims;
uniform float gradScale;

float2
Grad(float x, float y)
{
    float eps = 1.0/dims.x;
    float h = 1/dims.x * 0.5;
    float n1, n2, dx, dy;
    float2 dx0, dx1, dy0, dy1;

    dx0 = float2(x, y - eps) + h;
    dx1 = float2(x, y + eps) + h;
    dy0 = float2(x - eps, y) + h;
    dy1 = float2(x + eps, y) + h;
    
    n1 = texture(uPtex, dx0).r;
    n2 = texture(uPtex, dx1).r;
    dx = n1 - n2;

    n1 = texture(uPtex, dy0).r;
    n2 = texture(uPtex, dy1).r;
    dy = n1 - n2;
    float2 g = float2(dx, dy) / (2 * eps);
    g *= gradScale;

    return g;
}

void
main()
{
    float2 loc = gl_FragCoord.xy / dims;

    float2 g = Grad(loc.x, loc.y);
    FragColor = float4(g.x, 0.0, g.y, 1.0);
    
}


out float4 FragColor;
uniform sampler2D uPtex;
uniform float2 dims;

float2
Grad(float x, float y)
{
    float eps = 1.0/dims.x;
    float h = 1.0/dims.x * 0.5;
    
    float n1, n2, dx, dy;
    float2 dx0, dx1, dy0, dy1;
    dx0 = float2(x - eps, y);
    dx1 = float2(x + eps, y);
    dy0 = float2(x, y - eps);
    dy1 = float2(x, y + eps);
    
    n1 = texture(uPtex, dx0 + h).r;
    n2 = texture(uPtex, dx1 + h).r;
    dx = n1 - n2;
    
    n1 = texture(uPtex, dy0 + h).r;
    n2 = texture(uPtex, dy1 + h).r;
    dy = n1 - n2;

    float2 g = float2(dx, dy) * 10.0f;
    return g;
}


float2
Curl(float x, float y)
{
    float2 g = Grad(x, y);
    return float2(g.y, -g.x);
}

void
main()
{
    float2 loc = gl_FragCoord.xy;
    loc /= dims;

    float2 g = Curl(loc.s, loc.t);
    //i use the blue channel just cause I like blue more than green
    FragColor = float4(g.x, 0.0, g.y, 1.0);
    
}

*/




struct VS_Output
{  
    float4 Pos : SV_POSITION;              
    float2 Tex : TEXCOORD0;
};

VS_Output main_vs(uint id : SV_VertexID)
{
    VS_Output Output;
    Output.Tex = float2((id << 1) & 2, id & 2);
    Output.Pos = float4(Output.Tex * float2(2,-2) + float2(-1,1), 0, 1);
    return Output;
}

#define M_PI 3.141592653

int isc_sphere(float3 ro,float3 rd,float3 pos,float r,inout float t1,inout float t2)
{
	float t;
	float3 temp = ro - pos;
	float a = dot(rd, rd);
	float b = 2.f*dot(temp, rd);
	float c = dot(temp, temp) - r*r;
	float ds = b*b - 4 * a*c;

		
	int isc_n = 0;
	if (ds < 0.f)
	{
		return isc_n;
	}
	else
	{
		float e = sqrt(ds);
		float de = 2.f * a;
		t = (-b - e) / de;

		if (t > 0.00001)
		{
			t1 = t;
			isc_n++;
			//return isc_n;
		}

		t = (-b + e) / de;

		if (t>0.00001)
		{
			t2 = t;
			isc_n++;
			//return isc_n;
		}
	}
	return isc_n;
}

cbuffer CB
{
    matrix mv_mat;
    matrix inv_mv_mat;
	float view_dist;
	float tan_fov_x;
	float tan_fov_y;
	float density;
    float basis;
    float density1;
    float basis1;
    float density2;
    float basis2;
    float lod;
    float lod1;
    float lod_1;
    float lod_2;
    float lod_3;
    float2 height_range;
};

float perlin(float3 pos)
{
    return (Perlin3D(pos) + 1)*0.5;
}

float worley(float3 pos)
{
    return (Cellular3D(pos));
}

float sample_worley(float3 nv,float lod)
{
    return 1- worley(nv*lod);
}

float sample_perlin(float3 nv,float lod)
{
    return perlin(nv*lod);
}

float fbm_perlin(float3 pos,float lod)
{
    const float AMF = 0.507;
    const float FRF = 2.0789;
    //pos *= 0.1;
    //pos = normalize(pos);
    float A = 1;
    float V = A*(sample_perlin((pos),lod));A*= AMF;pos*=FRF;
    V += A*(sample_perlin((pos),lod));A*= AMF;pos*=FRF;
    V += A*(sample_perlin((pos),lod));A*= AMF;pos*=FRF;
    V += A*(sample_perlin((pos),lod));A*= AMF;pos*=FRF;

    return clamp(density*V+basis,0,1000);
}

float fbm_worley(float3 pos,float lod)
{
    const float AMF = 0.507;
    const float FRF = 2.0789;
    //pos = normalize(pos);
    float A = 1;
    float V = A*sample_worley(pos,lod);A*= AMF;pos*=FRF;
    V += A*sample_worley(pos,lod);A*= AMF;pos*=FRF;
    V += A*sample_worley(pos,lod);A*= AMF;pos*=FRF;
    V += A*sample_worley(pos,lod);A*= AMF;pos*=FRF;

    float ret = density1*V+basis1;
    ret = ret / (ret + 1);
    return ret;
    //return clamp(density1*V+basis1,0,1000);
}



float remap(float orig,float origin_min,float origin_max,float new_min,float new_max)
{
    return new_min + ((orig - origin_min)/(origin_max-origin_min))*(new_max-new_min);
}

float sample_perlin_worley(float3 pos,float lod)
{
	float fw = fbm_worley(pos,lod);
    
	float p = sample_perlin(pos,lod1)*5;
    //protect p>fw ALWAYS and 1>fw ALWAYS
	float res = remap(p,fw,1,0,1);
	return clamp(density2*res+basis2,0,1000);

}

float fbm_perlin_worley(float3 pos,float lod)
{
    const float AMF = 0.507;
    const float FRF = 2.0789;
    //pos = normalize(pos);
    float A = 1;
    float V = A*sample_perlin_worley(pos,lod);A*= AMF;pos*=FRF;
    V += A*sample_perlin_worley(pos,lod);A*= AMF;pos*=FRF;
    V += A*sample_perlin_worley(pos,lod);A*= AMF;pos*=FRF;
    V += A*sample_perlin_worley(pos,lod);A*= AMF;pos*=FRF;

    return clamp(density*V+basis,0,1000);
}


float get_height_fraction_for_point(float3 pos,float2 cloudrange)
{
    float ret = (pos.y - cloudrange.x)/(cloudrange.y - cloudrange.x);
    return saturate(ret);
}

float sample_cloud_density(float3 p)
{
    float fbmpw = sample_perlin_worley(p,lod);
    //return fbmpw;///(fbmpw+10);
    float worley1 = fbm_worley(p,3.69);
    float worley2 = fbm_worley(p,7.2);
    float worley3 = fbm_worley(p,15);
    float low_freq_FBM = worley1*0.625 + worley2*0.25 + worley3*0.125;
    //return low_freq_FBM;
    float base_cloud = remap(fbmpw*3,(low_freq_FBM),1,0,1);


    float w1 = fbm_worley(p*0.1,3.69);
    float w2 = fbm_worley(p*0.1,7.2);
    float w3 = fbm_worley(p*0.1,15);
    float high_freq_FBM = w1*0.625 + w2*0.25 + w3*0.125;

    float hf = get_height_fraction_for_point(p,height_range);
    float high_mo = lerp(high_freq_FBM,1-high_freq_FBM,hf);
    
    float ret = remap(base_cloud,high_mo*0.2,1,0,1);

    return ret;

    
}




float4 main_ps(VS_Output pin): SV_TARGET0
{	

    float4 sphere_pos = float4(0,0,0,1);
    sphere_pos = mul(sphere_pos,mv_mat);
    
    float4 forward = float4(0,0,1,0);
    float4 up = float4(0,1,0,0);
    forward = mul(forward,mv_mat);
    //up = mul(up,mv_mat);
    forward = normalize(forward);
    up = normalize(up);
    float sr = 1;

	float tx = 2*pin.Tex.x - 1;
	float ty = 2*(1-pin.Tex.y) - 1;
	float vx = tx*view_dist* tan_fov_x;
	float vy = ty*view_dist*tan_fov_y;
	
	
	float3 view_dir = float3(vx,vy,view_dist);
	view_dir = normalize(view_dir);

    view_dir = mul(float4(view_dir,0),inv_mv_mat).xyz;

	/*
	float t1 = -1;
	float t2 = -1;
	int ik = isc_sphere(float3(0,0,0),view_dir,sphere_pos.xyz,sr,t1,t2);
	*/
    
    if(view_dir.y<0)
        discard;

    float cost = dot(up.xyz,view_dir);
    if(cost==0) discard;
    float t = sr/cost;
    if(t>10)
        discard;



        float3 po;
        /*
        if(ik==2)
         po = t1*view_dir;
        else
         po = t2 * view_dir;
        */
        po = view_dir*t;
        float step = 0.1;
        float r = 0;
        for(int i=0;i<5;i++)
        {
            po = po + view_dir*i*step;
            float hf = get_height_fraction_for_point(po,height_range);
            //v.r = 1-abs(Cellular3D(po));
        
            r += sample_cloud_density(po)*hf;
        }
        r=r/(r+10);
        float3 c = float3(1,1,0.25)*2*exp(-r)*(1-exp(-2*r))*0.25/3.14159*((1-0.75*0.75)/(1+0.75*0.75-2*0.75*pow(dot(view_dir,normalize(float3(1,1,1))),1.5)));
        c= c/(c+1);
        return float4(c,1);
        //r=r/(r+10);
        //return float4(r,r,r,1);
    

}