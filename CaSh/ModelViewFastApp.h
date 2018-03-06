#pragma once
#include "FastApp.h"
#include "RHID3D11.h"
#include "D3D11FastRenderer.h"

class ModelViewFastApp : public FastApp
{
public:
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

  struct MatrixBuffer
  {
    RBMatrix m;
    RBMatrix v;
    RBMatrix p;
  };
  struct ScreenSize
  {
    uint32 w; uint32 h;
    uint32 s; uint32 pad2;
    RBMatrix inv_projection;
  };
  virtual bool preinit() override;
  virtual bool init() override;
  virtual void update(f32 dt) override;
  virtual void draw() override;
  virtual void ter() override;
  virtual void postter() override;

  ModelViewFastApp(class AppBase* app);
  virtual ~ModelViewFastApp();
  ModelViewFastApp(const ModelViewFastApp& o) = delete;
  ModelViewFastApp& operator=(const ModelViewFastApp& o) = delete;


protected:
  void load_models_generate_buffer(const char* filename);
  void load_assets();
  void create_rhires();
  void handle_input(f32 dt);

private:
  std::vector<Vertex_PNT> _vecs;
  std::vector<u32> _idxs;

  VertexBufferID _vb;
  IndexBufferID _ib;
  VertexFormatID _vf;

  ShaderID _view_shader;
  TextureID _buffer_depth_stencil;
  TextureID _duffuse_texture;
  class RBCamera* _cam;
  FD3D11DynamicRHI* _rhi;
  Direct3D11Renderer* _d3d11_renderer;

  BlendStateID _bs_enable;
  DepthStateID _ds_enable;
  SamplerStateID _ss;
  RasterizerStateID _rs;

};
