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

class ConservativeRasterizationFastApp : public FastApp
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

  ConservativeRasterizationFastApp(class AppBase* app);
  virtual ~ConservativeRasterizationFastApp();
  ConservativeRasterizationFastApp(const ConservativeRasterizationFastApp& o) = delete;
  ConservativeRasterizationFastApp& operator=(const ConservativeRasterizationFastApp& o) = delete;


protected:
  void load_models_generate_buffer(const char* filename);
  void load_assets();
  void create_rhires();
  void set_matrix(const RBMatrix& m, const RBMatrix& v, const RBMatrix& p);
  void set_screen_size();
  void handle_input(f32 dt);

private:

  void show_error_message(ID3D10Blob* error, const char* filename);

  ID3D11Texture2D* _buffer_depth_stencil;
  ID3D11ShaderResourceView* _buffer_depth_stencil_shader_view;
  ID3D11DepthStencilView* _buffer_depth_stencil_ds_view;

  /** models */
  ID3D11Buffer* _index_buffer;
  ID3D11Buffer* _vertex_buffer;
  ID3D11Texture2D* _texture1;
  ID3D11ShaderResourceView* _texture1_sview;


  ID3D11DepthStencilState* _depth_stencil_state;
  ID3D11DepthStencilState* _depth_stencil_state_disable;
  ID3D11BlendState* _blending_state;
  ID3D11BlendState* _blending_state_disable;

  ID3D11Buffer* _matrix_buffer;
  ID3D11Buffer* _screen_size_buffer;
  ID3D11InputLayout* _layout;
  ID3D11InputLayout* _layout_lighting;
  ID3D11SamplerState* _sample_state;

  ID3D11RasterizerState* _raster_state;

  ID3D11Device* _device;
  ID3D11DeviceContext* _context;
  IDXGIFactory1* _gi_factory;

  std::vector<Vertex_PNT> _vecs;
  std::vector<u32> _idxs;

  class RBCamera* _cam;
  RBVector2 _cam_move_speed;
  RBVector2 _cam_rotate_speed;
  int old_x, old_y;

  ID3D11GeometryShader* _gs;
  ID3D11VertexShader* _vs;
  ID3D11PixelShader* _ps;

  int dx;

  pD3DCompile compile_shader;
  HMODULE CompilerDLL;

  class RBDynamicRHI* _rhi;
};
