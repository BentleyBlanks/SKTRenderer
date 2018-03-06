#pragma once
#include "FastApp.h"
#include "RHID3D11.h"
#include "D3D11FastRenderer.h"
#include <vector>

class SceneEditorFastApp : public FastApp
{
public:
	virtual bool preinit() override;
	virtual bool init() override;
	virtual void update(f32 dt) override;
	virtual void draw() override;
	virtual void ter() override;
	virtual void postter() override;

	SceneEditorFastApp(class AppBase* app);
	virtual ~SceneEditorFastApp();
	SceneEditorFastApp(const SceneEditorFastApp& o) = delete;
	SceneEditorFastApp& operator=(const SceneEditorFastApp& o) = delete;

	struct Vertex_PNT
	{
		RBVector3 pos;
		RBVector3 normal;
		RBVector2 texcoord;
	};

	struct Vertex_PT
	{
		RBVector3 pos;
		RBVector2 texcoord;
	};
	struct ScreenSize
	{
		uint32 w; uint32 h;
		uint32 s; uint32 pad2;
		RBMatrix inv_projection;
	};
	struct MatrixBuffer
	{
		RBMatrix m;
		RBMatrix v;
		RBMatrix p;
	};

	typedef int MaterialID;
	struct Material
	{
		std::string name;
		f32 ns;
		f32 ni;
		f32 d;
		f32 tr;
		RBVector3 tf;
		f32 illum;
		RBVector3 ka;
		RBVector3 kd;
		RBVector3 ks;
		RBVector3 ke;
		TextureID map_metallic;
		TextureID map_diffuse;
		TextureID map_roughness;
		TextureID map_normal;
		TextureID map_mask;
		Material()
		{
			name = "";
			ns = ni = d = tr = illum = 0.f;
			tf = ka = kd = ks = ke = RBVector3::zero_vector;
			map_mask = map_diffuse = map_metallic = map_roughness = map_normal = -1;
		}
	};

	std::vector<Material> material_lib;

	struct MeshModel
	{
		std::vector<Vertex_PNT> vecs;
		std::vector<u32> idxs;
		MaterialID mat_id;
		VertexBufferID vb;
		IndexBufferID ib;
		RBMatrix model;

		MeshModel()
		{
			model = RBMatrix::identity;
			mat_id = -1;
			vb = ib = -1;
		}
	};

	std::vector<MeshModel> models;

protected:
	//degree
	void load_pbr_model(const char* achive_name, const RBVector3& pos, const RBVector3& rot);
	void load_models_generate_buffer(const char* filename);
	void load_assets();
	void create_rhires();
	void handle_input(f32 dt);
	void proccess_imgui(f32 dt);
	void proccess_tonemapping(f32 dt);
	void resize(int w, int h, bool bfullscreen);

	void draw1();
  void make_ibl();

private:
	DepthStateID _ds_enable;
	DepthStateID _ds_disable;

	BlendStateID _bs_enable;
	BlendStateID _bs_disable;

	SamplerStateID _ss;
  SamplerStateID _bs;

	RasterizerStateID _rs;
	int dx;
	TextureID _buffer_depth_stencil;
	TextureID _rt1_float;
  TextureID _rt1_float1;

	TextureID _texture_perlin_worley;
	TextureID _texture_worley;
	TextureID _texture_weather;

	ShaderID _bound_sky;
	TextureID _buffer_sky_32;


	ShaderID _bound_shader;

	class RBCamera* _cam;


	ShaderID _bound_shader_tone;
	ShaderID _bound_shader_gen_lum;
	ShaderID _bound_shader_gen_avg_lum;
	ShaderID _bound_shader_blur_x;
	ShaderID _bound_shader_blur_xy;
	ShaderID _bound_shader_bright;
	ShaderID _bound_shader_save_frame;
  ShaderID _bound_shader_downsample;
  ShaderID _bound_shader_fxaa;
  ShaderID _bound_shader_lensflare;


	TextureID _lumin_avg_16;
	TextureID _bright_half_blur_xy;
	TextureID _bright_half;
	TextureID _bright_half_blur_x;
  TextureID _bright_half2_blur_xy;
  TextureID _bright_half2;
  TextureID _bright_half2_blur_x;
  TextureID _bright_half4_blur_xy;
  TextureID _bright_half4;
  TextureID _bright_half4_blur_x;
  TextureID _bright_half8_blur_xy;
  TextureID _bright_half8;
  TextureID _bright_half8_blur_x;
	TextureID _lumin_half;
  TextureID _lens_half;
  TextureID _lens_blur;

	TextureID _last_frame[2];


	FD3D11DynamicRHI* _rhi;
	Direct3D11Renderer* _d3d11_renderer;

	//model
	std::vector<Vertex_PNT> _vecs;
	std::vector<u32> _idxs;
	static const int max_model = 1024;
	VertexBufferID _vb[max_model];
	IndexBufferID _ib[max_model];
	int cur_model = 0;
	VertexFormatID _vf = 0;
	ShaderID _view_shader;
	TextureID _duffuse_texture[max_model];
	TextureID _normal_texture[max_model];
	TextureID _mra_texture[max_model];
	RBMatrix _model_matrix[max_model];
	int _draw_size[max_model];
	TextureID _irr_texture[4];
	TextureID _spec_texture[4];
	TextureID _spec_brdf_lut;
  TextureID _text_dirt;

	struct PointLight
	{
		RBVector4 position_ = RBVector4::zero_vector;
		RBVector4 clq_ = RBVector4(0.f,0.00f,1.032f,0.f);
		RBVector4 ambient_ = RBColorf::white;
		RBVector4 diffuse_ = RBColorf(100000);
		RBVector4 specular_ = RBColorf::white;
	};

	PointLight point_lights[100];
};
