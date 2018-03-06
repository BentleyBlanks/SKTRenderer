#pragma once
#include "FastApp.h"
#include "d3d11.h"
#include "D3DCompiler.h"
#include <vector>
#include "Vector3.h"
#include "Vector2.h"
#include "Matrix.h"
#include "d3d11fastrenderer.h"


using std::vector;
#define PAD16(x) (((x)+15)/16*16)

class DSFastApp : public FastApp
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

  DSFastApp(class AppBase* app);
  virtual ~DSFastApp();
  DSFastApp(const DSFastApp& o) = delete;
  DSFastApp& operator=(const DSFastApp& o) = delete;


protected:
  void load_models_generate_buffer(const char* filename);
  void load_assets();
  void create_rhires();
  void set_matrix(const RBMatrix& m, const RBMatrix& v, const RBMatrix& p);
  void set_screen_size();
  void set_gpass_vbuffer();
  void handle_input(f32 dt);

private:
  TextureID _bufferA;
  TextureID _bufferB;
  TextureID _bufferC;
  TextureID _buffer_depth;
  TextureID _buffer_lighting;
  TextureID _buffer_depth_stencil;

  TextureID _env_texture;
  TextureID _mat_texture;
  TextureID _normal_texture;
  TextureID _env_texture_single;
  TextureID _env_texture_hdr;

  /** models */
  IndexBufferID _index_buffer;
  VertexBufferID _vertex_buffer;
  TextureID _texture1;

  IndexBufferID _index_buffer_lighting;
  VertexBufferID _vertex_buffer_lighting;

  ShaderID _gbuffer_shader;
  ShaderID _lighting_shader;

  DepthStateID _depth_stencil_state;
  DepthStateID _depth_stencil_state_disable;
  BlendStateID _blending_state;
  BlendStateID _blending_state_disable;


  VertexFormatID _vf_gbuffer;
  VertexFormatID _vf_lighting;

  SamplerStateID _sample_state;
  RasterizerStateID _raster_state;


  ID3D11Device* _device;
  ID3D11DeviceContext* _context;
  IDXGIFactory1* _gi_factory;

  std::vector<Vertex_PNTT> _vecs;
  std::vector<u32> _idxs;
  Vertex_PT quad[4];
  uint16 quad_idx[6];

  class RBCamera* _cam;

  float _high_light;
  float _factor_l;
  float _metallic;
  int _shading_model;

  float _zplus;

  pD3DCompile compile_shader;
  HMODULE CompilerDLL;

  Direct3D11Renderer* _d3d11_renderer;
};
