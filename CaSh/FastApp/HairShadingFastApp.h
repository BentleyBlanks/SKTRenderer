#pragma once
#include "FastApp.h"
#include "d3d11.h"
#include "D3DCompiler.h"
#include <vector>
#include "Vector3.h"
#include "Vector2.h"
#include "Matrix.h"


using std::vector;
#define PAD16(x) (((x)+15)/16*16)

class HairShadingFastApp : public FastApp
{
public:

  struct Vertex_PNT
  {
    RBVector3 pos;
    RBVector3 normal;
    RBVector2 texcoord;
  };

  struct Vertex_PNTT
  {
    RBVector3 pos;
    RBVector3 normal;
    RBVector2 texcoord;
    RBVector3 tangent;
  };

  struct Vertex_PT
  {
    RBVector3 pos;
    RBVector2 texcoord;
  };

  struct MatrixBuffer
  {
    RBMatrix m;
    RBMatrix v;
    RBMatrix p;
  };
  struct ScreenSize
  {
    uint32 w; uint32 h;
    f32 hipow; float factor_l;
    RBMatrix inv_projection;
    RBMatrix mv;
    float metallic; int shading_model;
  };
  virtual bool preinit() override;
  virtual bool init() override;
  virtual void update(f32 dt) override;
  virtual void draw() override;
  virtual void ter() override;
  virtual void postter() override;

  HairShadingFastApp(class AppBase* app);
  virtual ~HairShadingFastApp();
  HairShadingFastApp(const HairShadingFastApp& o) = delete;
  HairShadingFastApp& operator=(const HairShadingFastApp& o) = delete;


protected:
  bool create_texture_from_file(const char* filename, ID3D11Texture2D** tex, ID3D11ShaderResourceView** tex_view);
  void load_deffered_pipe();
  void load_shaders();
  void load_models_generate_buffer(const char* filename);
  void load_assets();
  void create_rhires();
  void set_matrix(const RBMatrix& m, const RBMatrix& v, const RBMatrix& p);
  void set_screen_size();
  void set_gpass_vbuffer();
  void handle_input(f32 dt);

private:

  void show_error_message(ID3D10Blob* error, const char* filename);

  ID3D11Texture2D* _bufferA;
  ID3D11RenderTargetView* _bufferA_target_view;
  ID3D11ShaderResourceView* _bufferA_shader_view;

  ID3D11Texture2D* _bufferB;
  ID3D11RenderTargetView* _bufferB_target_view;
  ID3D11ShaderResourceView* _bufferB_shader_view;

  ID3D11Texture2D* _bufferC;
  ID3D11RenderTargetView* _bufferC_target_view;
  ID3D11ShaderResourceView* _bufferC_shader_view;

  ID3D11Texture2D* _bufferDepth;
  ID3D11RenderTargetView* _bufferDepth_target_view;
  ID3D11ShaderResourceView* _bufferDepth_shader_view;

  ID3D11Texture2D* _buffer_lighting;
  ID3D11RenderTargetView* _buffer_lighting_target_view;
  ID3D11ShaderResourceView* _buffer_lighting_shader_view;

  ID3D11Texture2D* _buffer_depth_stencil;
  ID3D11ShaderResourceView* _buffer_depth_stencil_shader_view;
  ID3D11DepthStencilView* _buffer_depth_stencil_ds_view;

  

  ID3D11Texture2D* _env_texture;
  ID3D11ShaderResourceView* _env_texture_shader_view;

  ID3D11Texture2D* _mat_texture;
  ID3D11ShaderResourceView* _mat_texture_shader_view;

  ID3D11Texture2D* _noise_texture;
  ID3D11ShaderResourceView* _noise_texture_shader_view;

  ID3D11Texture2D* _env_texture_single;
  ID3D11ShaderResourceView* _env_texture_single_shader_view;

  ID3D11Texture2D* _env_texture_hdr;
  ID3D11ShaderResourceView* _env_texture_hdr_shader_view;

  /** models */
  ID3D11Buffer* _index_buffer;
  ID3D11Buffer* _vertex_buffer;
  /** model textures */
  ID3D11Texture2D* _texture1;
  ID3D11ShaderResourceView* _texture1_sview;

  ID3D11Texture2D* _power_texture;
  ID3D11ShaderResourceView* _power_texture_shader_view;

  ID3D11Texture2D* _opacity_texture;
  ID3D11ShaderResourceView* _opacity_texture_shader_view;

  ID3D11Texture2D* _normal_texture;
  ID3D11ShaderResourceView* _normal_texture_shader_view;


  ID3D11Buffer* _index_buffer_lighting;
  ID3D11Buffer* _vertex_buffer_lighting;

  ID3D11VertexShader* _def_vertex_shader;
  ID3D11GeometryShader* _def_geometry_shader;
  ID3D11PixelShader* _def_pixel_shader;
  ID3D11VertexShader* _def_vertex_shader_light;
  ID3D11PixelShader* _def_pixel_shader_light;

  ID3D11DepthStencilState* _depth_stencil_state;
  ID3D11DepthStencilState* _depth_stencil_state_disable;
  ID3D11BlendState* _blending_state;
  ID3D11BlendState* _blending_state_disable;

  ID3D11Buffer* _matrix_buffer;
  ID3D11Buffer* _screen_size_buffer;
  ID3D11Buffer* _dv_cbuffer;
  ID3D11InputLayout* _layout;
  ID3D11InputLayout* _layout_lighting;
  ID3D11SamplerState* _sample_state;

  ID3D11RasterizerState* _raster_state;

  ID3D11Device* _device;
  ID3D11DeviceContext* _context;
  IDXGIFactory1* _gi_factory;

  std::vector<Vertex_PNTT> _vecs;
  std::vector<u32> _idxs;
  Vertex_PT quad[4];
  uint16 quad_idx[6];

  class RBCamera* _cam;
  RBVector2 _cam_move_speed;
  RBVector2 _cam_rotate_speed;
  int old_x, old_y;

  float _high_light;
  float _factor_l;
  float _metallic;
  int _shading_model;

  float _zplus;

  pD3DCompile compile_shader;
  HMODULE CompilerDLL;
};
