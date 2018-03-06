#include "FastAppTemplate.h"
#include "Camera.h"

bool TemplateFastApp::preinit()
{
  return true;
}

bool TemplateFastApp::init()
{
  _rhi = static_cast<FD3D11DynamicRHI*>(base_app->get_rhi());

  _cam = new RBCamera();
  if (!_cam) return false;
  _cam->set_position(0, 0, 0);
  RBVector4 position(0, 0, 1);
  _cam->set_target(position);
  _cam->set_fov_y(60);
  _cam->set_ratio(16.f / 9.f);
  _cam->set_near_panel(0.01f);
  _cam->set_far_panel(2000.f);

  _cam->set_move_speed(RBVector2(50, 50));
  _cam->set_rotate_speed(RBVector2(5, 5));

  _d3d11_renderer = new Direct3D11Renderer(_rhi->GetDevice(), _rhi->GetDeviceContext());

  load_assets();
  create_rhires();

  _d3d11_renderer->setViewport(ww, wh);
  _d3d11_renderer->setFrameBuffer(back_buffer_view, _d3d11_renderer->getDSV(_buffer_depth_stencil));
  _d3d11_renderer->changeToMainFramebuffer();

  return true;
}

void TemplateFastApp::update(f32 dt)
{
  handle_input(dt);
}

void TemplateFastApp::draw()
{
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);

}

void TemplateFastApp::ter()
{
  _d3d11_renderer->finish();
  delete _d3d11_renderer;
}

void TemplateFastApp::postter()
{

}

TemplateFastApp::TemplateFastApp(class AppBase* app) : FastApp(app)
{

}

TemplateFastApp::~TemplateFastApp()
{

}

void TemplateFastApp::load_models_generate_buffer(const char* filename)
{

}

void TemplateFastApp::load_assets()
{

  load_models_generate_buffer("res/p.obj");
}

void TemplateFastApp::create_rhires()
{
  _buffer_depth_stencil = _d3d11_renderer->addRenderDepth(ww, wh, 1, FORMAT::FORMAT_D24S8);
}

void TemplateFastApp::handle_input(f32 dt)
{
  _cam->control_update(dt);
}
