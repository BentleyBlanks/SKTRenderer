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
	//km
	float3 sphere_pos_v;
	float view_dist;
	float3 sun_dir;
	float sacle_factor;
	float tan_fov_x;
	float tan_fov_y;
	int n_sample;
	int n_sample_light;
	matrix mv_mat;
	matrix imv_mat;
	float3 sun_color;
};
TextureCube spec_map;
SamplerState ss;
float4 main_ps(VS_Output pin): SV_TARGET0
{
	//!unit is cm not m to fix!
	sphere_pos_v = mul(sphere_pos_v*1000,mv_mat);
	sphere_pos_v/=1000;
	bool a = false;
	if(sun_dir.y<0)
	{
		a=!a;
		
	}
	sun_dir = mul(sun_dir,mv_mat);
	
	//km
	float er = 6360;
	float ar = 6420;
	//m
	float hr = 7994;
	float hm = 1200;
	
	//mm
	float3 betar= float3(0.0058,0.0135,0.0331);
	float3 betam = float3(0.021,0.021,0.021);
	
	float tx = 2*pin.Tex.x - 1;
	float ty = 2*(1-pin.Tex.y) - 1;
	float vx = tx*view_dist* tan_fov_x;
	float vy = ty*view_dist*tan_fov_y;
	
	//cm
	float3 view_dir = float3(vx,vy,view_dist);
	view_dir = normalize(view_dir);

	if(a)
		return float4(spec_map.Sample(ss,mul(view_dir,imv_mat)).rgb,1);
	
	float t1 = -1;
	float t2 = -1;
	float t11 = -1;
	float t12 = -1;
	int ik = isc_sphere(float3(0,0,0),view_dir,sphere_pos_v,ar,t1,t2);
	//检测内球交点，提升采样率
	int ic = isc_sphere(float3(0,0,0),view_dir,sphere_pos_v,er,t11,t12);
	float3 pa,pb;
	if(ik==0)
		discard;
		//return float4(1,0,0,1);
	if(ik==1)
	{
		pa = float3(0,0,0);
		pb = view_dir*t2;
		//return float4(0,0,1,1);
	}
	else if(ik==2)
	{
		
		if(ic<=1)
		{
			pa = view_dir*t1;
			pb = view_dir*t2;
		}
		else if(ic==2 )
		{
			pa = view_dir*t1;
			pb = view_dir*t11;
		}
		//return float4(0,1,0,1);
	}
	
	
	//km
	float seg_len = length(pb-pa)/n_sample;
	float3 sumr=0,summ=0;
	float optiacal_depth_r=0,optical_depth_m=0;
	float theta = dot(view_dir,sun_dir);
	float phaser = 3.f / (16.f * M_PI) * (1 + theta * theta);
	float g = 0.76;
	float phasem = 3.f / (8.f * M_PI) * ((1.f - g * g) * (1.f + theta * theta)) / ((2.f + g * g) * pow(1.f + g * g - 2.f * g * theta, 1.5f));
		
	for(uint i=0;i<n_sample;++i)
	{
		//km
		float3 sample_pos = pa + seg_len*(i+0.5)*view_dir;
		float samlpe_height = length(sample_pos - sphere_pos_v) - er;
		//km to m
		samlpe_height*=1000;
		float h_r = exp(-samlpe_height/hr)*seg_len;
		float h_m = exp(-samlpe_height/hm)*seg_len;
		//to km
		optiacal_depth_r += h_r;
		optical_depth_m += h_m;
		float t0Light;
		float t1Light;
		int sn = isc_sphere(sample_pos,sun_dir,sphere_pos_v,ar,t0Light,t1Light);

		//km
		float seg_len_sun = t1Light/n_sample_light;
		float optiacal_depth_sun_r=0,optical_depth_sun_m=0;
		uint j = 0;
		for(j=0;j<n_sample_light;++j)
		{
			float3 sample_pos_sun = sample_pos + seg_len_sun*(j+0.5)*sun_dir;
			float sample_height_sun = length(sample_pos_sun - sphere_pos_v) - er;
			//km to m
			sample_height_sun*=1000;
			if(sample_height_sun<0) 
			{
				//return float4(0.5,0.5,0.5,1);
				//break;
			}	
			//km
			optiacal_depth_sun_r += exp(-sample_height_sun/hr)*seg_len_sun;
			optical_depth_sun_m += exp(-sample_height_sun/hm)*seg_len_sun;
			
		}
		//if(j==n_sample_light)
		{
			//mm*km=m^2
			float3 tau = betar*(optiacal_depth_r+optical_depth_m) + betam*1.1f*(optiacal_depth_sun_r+optical_depth_sun_m);
			float3 attenuation = float3(exp(-tau.x),exp(-tau.y),exp(-tau.z));

			
			sumr += attenuation*h_r;
			summ += attenuation*h_m;
		}
	}
	
	float3 sky_color = clamp(sun_color*(sumr*betar*phaser + summ*betam*phasem)*sacle_factor,0.0001,100000000);
	
	return float4(sky_color,1);
}