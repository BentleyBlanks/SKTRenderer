#pragma once
#include "FastApp.h"
#include "RHID3D11.h"
#include "D3D11FastRenderer.h"

class TemplateFastApp : public FastApp
{
public:
  virtual bool preinit() override;
  virtual bool init() override;
  virtual void update(f32 dt) override;
  virtual void draw() override;
  virtual void ter() override;
  virtual void postter() override;

  TemplateFastApp(class AppBase* app);
  virtual ~TemplateFastApp();
  TemplateFastApp(const TemplateFastApp& o) = delete;
  TemplateFastApp& operator=(const TemplateFastApp& o) = delete;


protected:
  void load_models_generate_buffer(const char* filename);
  void load_assets();
  void create_rhires();
  void handle_input(f32 dt);

private:
  TextureID _buffer_depth_stencil;
  class RBCamera* _cam;
  FD3D11DynamicRHI* _rhi;
  Direct3D11Renderer* _d3d11_renderer;
};
