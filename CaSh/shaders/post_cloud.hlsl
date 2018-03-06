Texture3D texture_perlin_worley;
Texture3D texture_worley;
Texture2D texture_wheather;
SamplerState sampler_linear_;

cbuffer CB
{
    matrix mv_mat;
    matrix inv_mav_mat;
    float3 earth_pos;
    float sample_scale;
    float sample_scale_detail;
    float cloud_scale;
    float view_dist;
    float tan_fov_x;
    float tan_fov_y;
    float depth_scale;
    float beer_scale;
    float ao_scale;
    float3 light_dir;
    float start_height;
    float3 light_color;
    float end_height;
    float noise_threshold;
    float noise_max;
    float max_dist;
    float sun_scale;

};

struct VS_Output
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

VS_Output main_vs(uint id : SV_VertexID)
{
    VS_Output Output;
    Output.Tex = float2((id << 1) & 2, id & 2);
    Output.Pos = float4(Output.Tex * float2(2, -2) + float2(-1, 1), 0, 1);
    return Output;
}

#define M_PI 3.141592653

int isc_sphere(float3 ro, float3 rd, float3 pos, float r, inout float t1, inout float t2)
{
    float t;
    float3 temp = ro - pos;
    float a = dot(rd, rd);
    float b = 2.f * dot(temp, rd);
    float c = dot(temp, temp) - r * r;
    float ds = b * b - 4 * a * c;
    int isc_n = 0;
    if (ds < 0.f)
        return isc_n;
    else
    {
        float e = sqrt(ds);
        float de = 2.f * a;
        t = (-b - e) / de;
        if (t > 0)
        {
            t1 = t;
            isc_n++;
        }
        t = (-b + e) / de;
        if (t > 0)
        {
            t2 = t;
            isc_n++;
        }
    }
    return isc_n;
}

float remap(float orig, float origin_min, float origin_max, float new_min, float new_max)
{
    return new_min + ((orig - origin_min) / (origin_max - origin_min)) * (new_max - new_min);
}

float SampleCloudDensity(float3 pos, float height, bool sampledetail)
{
    const float base_freq1 = 0.0225 / 100;
    const float base_freq2 = 0.035 / 100;
    
    pos = mul(float4(pos, 1), inv_mav_mat);

    float4 coord = float4(pos * base_freq1 * sample_scale, 0);
    float4 base_cloud = texture_perlin_worley.SampleLevel(sampler_linear_, coord.xyz, 0);

    float base_fbm = dot(base_cloud.gba, float3(0.625, 0.25, 0.125));
    float cloud_desity = base_cloud.r;

    cloud_desity = remap(cloud_desity, -base_fbm, 1, 0, 1);
    cloud_desity = remap(cloud_desity, noise_threshold, noise_max, 0, 1);

    //to texture center
    float2 wc = (pos.xz / 3000 + 0.5);

    float wd = texture_wheather.SampleLevel(sampler_linear_, wc, 0).b;
    
    cloud_desity = remap(cloud_desity, saturate(height / wd), 1, 0, wd);
    cloud_desity *= wd;

    if (sampledetail)
    {
        coord = float4(pos * base_freq2 * sample_scale_detail, 0);
        float3 detail_cloud = texture_worley.SampleLevel(sampler_linear_, coord.xyz, 0).xyz;
        float detail_fbm = dot(detail_cloud, float3(0.625, 0.25, 0.125));
        float detail_cloud_density = lerp(detail_fbm, 1 - detail_fbm, saturate(height * 10));
        cloud_desity = remap(cloud_desity, detail_cloud_density, 1, 0, 1);
    }
    
    return saturate(cloud_desity * cloud_scale);
}


float3 ambientLighting(float height)
{
    float3 cloud_color_bt = float3(98 / 255.f, 120 / 255.f, 134 / 255.f);
    float3 cloud_color_tp = float3(54 / 255.f, 68 / 255.f, 76 / 255.f);
    return lerp(cloud_color_bt, cloud_color_tp, height);
}

float PhaseHenyeyGreenStein(float inScatteringAngle, float g)
{
    return ((1.0 - g * g) / pow((1.0 + g * g - 2.0 * g * inScatteringAngle), 3.0 / 2.0)) / (4.0 * 3.14159);
}


float Beer(float opticalDepth)
{
    return exp(-beer_scale * 0.01f * opticalDepth);
}

float3 sample_light(float3 pos, float3 erath_pos, float3 light_dir, float3 light_color, float costheta, float er, float sr)
{
    const float3 RandomUnitSphere[6] =
    {
        { 0.3f, -0.8f, -0.5f },
        { 0.9f, -0.3f, -0.2f },
        { -0.9f, -0.3f, -0.1f },
        { -0.5f, 0.5f, 0.7f },
        { -1.0f, 0.3f, 0.0f },
        { -0.3f, 0.9f, 0.4f }
    };
    const int steps = 6;
    float sunRayStepLength = 100 / ((float) steps);
    float3 sunRayStep = normalize(light_dir) * sunRayStepLength;
    pos += 0.5 * sunRayStep;
    float opticalDepth = 0.0;
    [loop]
    for (int i = 0; i < steps; i++)
    {
        float3 randomOffset = RandomUnitSphere[i] * sunRayStepLength * 0.25 * ((float) (i + 1));
        float3 samplePos = pos + randomOffset;
        float height = (length(samplePos - erath_pos) - sr) / (er - sr);
        float3 weatherData = float3(1, 1, 1); 
        float cloudDensity = SampleCloudDensity(samplePos, height, true);
        opticalDepth += cloudDensity * sunRayStepLength;
        pos += sunRayStep;
    }
    float hg = PhaseHenyeyGreenStein(costheta, 0.76);
    float extinct = Beer(opticalDepth);
    return light_color * extinct * hg;
}

float4 main_ps(VS_Output pin) : SV_TARGET0
{
    float3 temp_pos = float3(0, 0, 5);
    float temp_r = 15;

    float tx = 2 * pin.Tex.x - 1;
    float ty = 2 * (1 - pin.Tex.y) - 1;
    float vx = tx * view_dist * tan_fov_x;
    float vy = ty * view_dist * tan_fov_y;
	
    float3 view_dir = float3(vx, vy, view_dist);
    view_dir = normalize(view_dir);

    light_dir = mul(light_dir, (float3x3) mv_mat);
    light_dir = normalize(light_dir);

#ifdef _MARCHSPHERE
    temp_pos = mul(float4(temp_pos,1),mv_mat).xyz;

    float tt1 = -1;
    float tt2 = -1;
    int iscn = isc_sphere(float3(0,0,0),view_dir,temp_pos,temp_r,tt1,tt2);

    if(iscn==0) return float4(0,1,0,1);
    else
    {
        float3 tsp = float3(0,0,0);
        float3 tep;
        if(iscn==2)
            tsp = view_dir*tt1;
        tep = view_dir*tt2;          
        int sample_n = 100;
        int sample_l = 100;
        float cost = dot(light_dir,view_dir);
        float3 pos = float3(0,0,0);
        float seg_len = length(tep - tsp)/sample_n;
        float opd = 0;
        float3 cc = float3(0,0,0);

        float noise_v = 0;
        [loop]
        for(int i=0;i<sample_n;++i)
        {
            pos = tsp+(i+0.5)*view_dir*seg_len;
    
            float3 spos = mul(float4(pos,1),inv_mav_mat).xyz;
            float4 coord = float4(spos*sample_scale*0.01,0);
            float4 base_cloud = texture_perlin_worley.SampleLevel(sampler_linear_,coord.xyz,0);
            noise_v += base_cloud.r;
            opd +=  depth_scale*seg_len*base_cloud.r;
            float ext = exp(-0.01*opd);
            float3 lit1 = light_color;
            float lt1=-1,lt2=-1;
            int ltk = isc_sphere(pos,light_dir,temp_pos,temp_r,lt1,lt2);
            if(ltk==0) {
                 continue;
            }

            float seg_light = lt2/sample_l;

            float dp = 0;
            [loop]
            for(int j=0;j<sample_l;++j)
            {
                float3 pl = pos + (j+0.5)*light_dir*seg_light;

            float3 spl = mul(float4(pl,1),inv_mav_mat).xyz;

                 float4 coord1 = float4(spl*sample_scale_detail*0.01,0);
            float4 base_cloud1 = texture_perlin_worley.SampleLevel(sampler_linear_,coord.xyz,0);

                dp +=  depth_scale*seg_light*base_cloud1.r;
                //float extl = exp(-dp);
                //lit1 += lit1*extl;
            } 
            float hg1 = PhaseHenyeyGreenStein(cost, 0.76);
            float extl = exp(-dp*0.01);
            lit1 = lit1*extl*hg1;
            cc += lit1*ext;
        }
        noise_v*=depth_scale*0.1;
        //return float4(noise_v,noise_v,noise_v,1);
        return float4(cc,1-exp(-opd*0.01));
        
    }
    return float4(1,0,0,1);
#endif

    float4 ev_pos_ = mul(float4(earth_pos, 1), mv_mat);
    float3 ev_pos = ev_pos_.xyz;
        
    float sr = -earth_pos.y + start_height / 10; //km
    float er = -earth_pos.y + end_height / 10; //km

    float t1 = -1, t2 = -1, t11 = -1, t12 = -1;
    int ik = isc_sphere(float3(0, 0, 0), view_dir, ev_pos, sr, t1, t2);
    int ik1 = isc_sphere(float3(0, 0, 0), view_dir, ev_pos, er, t11, t12);
    
    float3 sp = float3(0, 0, 0);
    float3 ep;
    float vheight = length(ev_pos);
    if (vheight < sr)
    {
        if (t2 < max_dist)
        {
            sp = view_dir * t2;
            ep = view_dir * t12;
        }
        else
            discard;
    }
    else if (vheight > er)
    {
        if (ik1 == 0)
            discard;
        sp = view_dir * t11;
        ep = view_dir * t1;
    }
    else
    {
        if (ik == 2)
            ep = view_dir * t11;
        else
            ep = view_dir * t12;
    }
    float costheta = dot(view_dir, normalize(light_dir));
    int steps = 128;
    float3 cpos = sp;
    float extinct = 1;
    float opticaldepth = 0;
    float4 color = float4(0, 0, 0, 0);
    float step_len = length(ep - sp) / steps;
    //return float4(er/length(ep-ev_pos),0,0,1);
    [loop]
    for (int i = 0; i <= steps; ++i)
    {
        cpos += view_dir * step_len;
        //calculate relative height to cloud layer
        float cheight = (length(cpos - ev_pos) - sr) / (er - sr);
        if (extinct < 0.01 || cheight > 1.1 || cheight < 0)
            break; 
        float cloud_density = SampleCloudDensity(cpos, cheight, true);
        if (cloud_density <= 0)
            continue;
        float3 al = ambientLighting(cheight);
        float3 sunlight = sample_light(cpos, ev_pos, light_dir, light_color, costheta, er, sr);
        float currentOpticalDepth = cloud_density * step_len * depth_scale;
        al *= ao_scale * 0.1f * currentOpticalDepth;
        sunlight *= sun_scale * currentOpticalDepth;
        opticaldepth += currentOpticalDepth;
        extinct = Beer(opticaldepth);
        color.rgb += (sunlight + al) * extinct;
    }
    color.a = 1 - Beer(opticaldepth);
    ep = mul(float4(ep, 1), inv_mav_mat).xyz;
    float fade = 1 - saturate(length(ep.xz) / max_dist);
    return color * fade;
}
