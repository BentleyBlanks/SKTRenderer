#pragma once
#include "FastApp.h"
#include "RHID3D11.h"
#include "D3D11FastRenderer.h"
#include <vector>

class SceneLoadFastApp : public FastApp
{
public:
  virtual bool preinit() override;
  virtual bool init() override;
  virtual void update(f32 dt) override;
  virtual void draw() override;
  virtual void ter() override;
  virtual void postter() override;

  SceneLoadFastApp(class AppBase* app);
  virtual ~SceneLoadFastApp();
  SceneLoadFastApp(const SceneLoadFastApp& o) = delete;
  SceneLoadFastApp& operator=(const SceneLoadFastApp& o) = delete;

  struct PointLight
  {
	  RBVector3 pos_view;
	  f32 start;
	  RBVector3 color;
	  f32 end;
	  RBVector4 attenuation_param;
  };

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
  //todo:change shader
  std::vector<MeshModel> models_alpha;

  std::vector<PointLight> lights;

protected:
  void load_models_generate_buffer(const char* filename);
  void load_assets();
  void create_rhires();
  void handle_input(f32 dt);
  void proccess_imgui(f32 dt);
  void proccess_tonemapping(f32 dt);
  void resize(int w, int h, bool bfullscreen);
  void setup_lights(int n);
  void render_forward(bool bpreZ,f32 dt);
  void read_back(ID3D11Resource* res);
  void read_back_lumin_half();
  void read_back_color_and_check(int i);

private:
	
	StructureBufferID _lights;

	DepthStateID _ds_enable;
	DepthStateID _ds_disable;
	DepthStateID _ds_write_disable;

	BlendStateID _bs_enable;
	BlendStateID _bs_disable;

	SamplerStateID _ss;

	RasterizerStateID _rs;
	RasterizerStateID _rs_double_face;

	int dx;
	TextureID _buffer_depth_stencil;
  TextureID _buffer_depth_stencil_low;

  TextureID _buffer_sky_32;

  //º∆À„avg_lumin
  /*
  TextureID _lumin_512;
  TextureID _scale_downsample_128;
  TextureID _scale_downsample_32;
  TextureID _scale_downsample_4;
  TextureID _scale_downsample_1;
  */
  TextureID _read_back_buffer_16;
  TextureID _read_back_buffer_half;
  TextureID _read_back_buffer_full;
  ID3D11Texture2D* tex;

  TextureID _bright_half;
  TextureID _bright_half_blur_x;
  TextureID _lumin_half;


  //post effect
	TextureID _rt1_float;
	TextureID _lumin_avg_16;
	TextureID _bright_half_blur_xy;



	TextureID _last_frame[2];

	SamplerStateID _linear_sampler;
  /*
  TextureID _rt1_float_lum_avg[];
  TextureID _rt1_float_
  */

	VertexFormatID _vf;
	ShaderID _bound_shader_bilt;
  ShaderID _bound_shader;
  ShaderID _bound_shader_tone;
  ShaderID _bound_shader_gen_lum;
  ShaderID _bound_shader_gen_avg_lum;
  ShaderID _bound_shader_blur_x;
  ShaderID _bound_shader_blur_xy;
  ShaderID _bound_shader_bright;
  ShaderID _bound_shader_save_frame;

  ShaderID _bound_sky;
  ShaderID _bound_cloud;

  uint _last_use = 0;

  class RBCamera* _cam;
  bool p = false;
  float A[2];
  float B[2];
  float C[2];
  float D[2];
  float E_[2];
  float F[2];
  float W[2];
  float ExposureBias[2];
  float ExposureAdjustment[2];
  uint32 nlights;

  FD3D11DynamicRHI* _rhi;
  Direct3D11Renderer* _d3d11_renderer;
};
