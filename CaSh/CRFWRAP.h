#pragma once

#include "FastApp.h"
#include "RHID3D11.h"
#include "D3DCompiler.h"
#include <vector>
#include "Vector3.h"
#include "Vector2.h"
#include "Matrix.h"


using std::vector;
#define PAD16(x) (((x)+15)/16*16)

class ConservativeRasterizationFastAppWrapped : public FastApp
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

  ConservativeRasterizationFastAppWrapped(class AppBase* app);
  virtual ~ConservativeRasterizationFastAppWrapped();
  ConservativeRasterizationFastAppWrapped(const ConservativeRasterizationFastAppWrapped& o) = delete;
  ConservativeRasterizationFastAppWrapped& operator=(const ConservativeRasterizationFastAppWrapped& o) = delete;


protected:
  void load_models_generate_buffer(const char* filename);
  void load_assets();
  void create_rhires();
  void set_matrix(const RBMatrix& m, const RBMatrix& v, const RBMatrix& p);
  void set_screen_size();
  void handle_input(f32 dt);

private:

  void show_error_message(ID3D10Blob* error, const char* filename);

  
  RBVertexBufferRHIRef _vb;
  RBIndexBufferRHIRef _ib;

  RBVertexShaderRHIRef _vs;
  RBGeometryShaderRHIRef _gs;
  RBPixelShaderRHIRef _ps;
  RBUniformBufferRHIRef _matrix_buffer;

  std::vector<Vertex_PNT> _vecs;
  std::vector<u32> _idxs;

  
  RBDepthStencilStateRHIRef _ds_enable;
  RBDepthStencilStateRHIRef _ds_disable;

  RBBlendStateRHIRef _bs_enable;
  RBBlendStateRHIRef _bs_disable;

  RBSamplerStateRHIRef _ss;

  RBRasterizerStateRHIRef _rs;

  RBTexture2DRHIRef _buffer_depth_stencil;
  RBUniformBufferRHIRef _uniform_screen_size;

  class RBCamera* _cam;
  RBVector2 _cam_move_speed;
  RBVector2 _cam_rotate_speed;
  int old_x, old_y;

  int dx;

  RBDynamicRHI* _rhi;
};
