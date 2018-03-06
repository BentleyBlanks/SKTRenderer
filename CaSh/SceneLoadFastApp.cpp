#include "SceneLoadFastApp.h"
#include "Camera.h"
#include "tiny_obj_loader.h"
#include <iostream>
#include "ResourceManager.h"
#include "Input.h"
#include "thirdpart/imgui/imgui.h"
#include "D3D11ShaderResources.h"

bool SceneLoadFastApp::preinit()
{
  nlights = 128;
  lights.resize(nlights);
  dx = 0;
  A[1] = A[0] = 0.15;
  B[1] = B[0] = 0.50;
  C[1] = C[0] = 0.10;
  D[1] = D[0] = 0.20;
  E_[1] = E_[0] = 0.02;
  F[1] = F[0] = 0.30;
  W[1] = W[0] = 11.2;
  ExposureBias[1] = ExposureBias[0] = 2.f;
  ExposureAdjustment[1] = ExposureAdjustment[0] = 16.f;
  return true;
}

bool SceneLoadFastApp::init()
{
  _rhi = static_cast<FD3D11DynamicRHI*>(base_app->get_rhi());

  _cam = new RBCamera();
  if (!_cam) return false;
  _cam->set_position(0, 0, 0);
  RBVector4 position(0, 0, 1);
  _cam->set_target(position);
  _cam->set_fov_y(60);
  _cam->set_ratio(16.f / 9.f);
  _cam->set_near_panel(1.f);
  _cam->set_far_panel(500000.f);

  _cam->set_move_speed(RBVector2(150, 150));
  _cam->set_rotate_speed(RBVector2(50, 50));

  _d3d11_renderer = new Direct3D11Renderer(_rhi->GetDevice(), _rhi->GetDeviceContext());

  create_rhires();

  load_assets();


  _d3d11_renderer->setBlendState(_bs_enable, 0xffffffff);
  _d3d11_renderer->setDepthState(_ds_enable);
  _d3d11_renderer->setRasterizerState(_rs);


  _d3d11_renderer->setViewport(ww, wh);
  _d3d11_renderer->setFrameBuffer(back_buffer_view, _d3d11_renderer->getDSV(_buffer_depth_stencil));
  _d3d11_renderer->changeToMainFramebuffer();




  return true;
}

void SceneLoadFastApp::update(f32 dt)
{
  handle_input(dt);
  proccess_imgui(dt);
  ImGui::Checkbox("print v check", &p);
  render_forward(false, dt);
}

void SceneLoadFastApp::setup_lights(int n)
{
  bool update_ar = false;
  bool update_pos = false;
  bool update_color = false;
  bool update_ac = false;

  static f32 h[3] = { 0, 50, 0 };
  static f32 c[3] = { 10, 10, 10 };
  static f32 ap[4] = { 0, 0, 0.01, 0.01 };
  static f32 ar[] = { 0, 1 };
  ImGui::Begin("light");
  update_pos |= ImGui::SliderFloat3("light height", h, -1000, 1000);
  update_color |= ImGui::SliderFloat3("light color", c, 0, 50);
  bool use_white = ImGui::Button("Use White Color");
  update_color |= use_white;
  static float white_scale = 1;
  ImGui::SliderFloat("White Scale", &white_scale,0,1);

  if (use_white)
  {
    c[0] = c[1] = c[2] = 100*white_scale;
  }
  update_ac |= ImGui::InputFloat4("Attenuation coefficient", ap);
  update_ar |= ImGui::SliderFloat2("Attenuation range", ar, 0, 5000);
  ImGui::End();



  PointLight* light = lights.data();

  if (update_pos)
  {
    for (uint32 i = 0; i < n; ++i)
    {
      light[i].pos_view = RBVector3(h);
      light[i].pos_view.x += RBMath::get_rand_range_f(-1500, 1500);
      light[i].pos_view.y += RBMath::get_rand_range_f(-40, 800);
      light[i].pos_view.z += RBMath::get_rand_range_f(-500, 500);
    }
  }

  if (update_ac)
    for (uint32 i = 0; i < n; ++i)
    {
      light[i].attenuation_param = RBVector4(ap);
    }
  if (update_color)
    for (uint32 i = 0; i < n; ++i)
    {
      light[i].color = RBVector3(c);
      if (!use_white)
      { 
        light[i].color.x += RBMath::clamp(RBMath::get_rand_range_f(-10.f, 10.f), -light[i].color.x, 10.f);
        light[i].color.y += RBMath::clamp(RBMath::get_rand_range_f(-10.f, 10.f), -light[i].color.y, 10.f);
        light[i].color.z += RBMath::clamp(RBMath::get_rand_range_f(-10.f, 10.f), -light[i].color.z, 10.f);
      }
    }
  if (update_ar)
    for (uint32 i = 0; i < n; ++i)
    {
      light[i].end = ar[1];// +RBMath::clamp(RBMath::get_rand_range_f(-1000.f, 1000.f), -ar[1], 10.f);
      light[i].start = ar[0];// +RBMath::clamp(RBMath::get_rand_range_f(-1000.f, 1000.f), -ar[0], light[i].end - ar[0]);

    }

  if (update_ar || update_ac || update_color || update_pos)
  {
    PointLight* litptr = _d3d11_renderer->mapStructureBufferDiscard<PointLight>(_lights);
    memcpy(litptr, light, lights.size()*sizeof(PointLight));
    _d3d11_renderer->umapStructureBuffer(_lights);
  }
}

void SceneLoadFastApp::render_forward(bool bpreZ, f32 dt)
{
  /*
  _d3d11_renderer->changeRenderTarget(_rt1, FB_DEPTH);
  _d3d11_renderer->clearRenderTarget(_rt1, RBColorf::blank);
  */



  RBMatrix mp;
  RBMatrix mv;

  _cam->get_view_matrix(mv);
  _cam->get_perspective_matrix(mp);



  //prez
  {
    //todo
  }
  
  _d3d11_renderer->setDepthState(_ds_disable);
  _d3d11_renderer->changeRenderTarget(_rt1_float, _buffer_depth_stencil_low);


  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
  
  //render sky
  _d3d11_renderer->setShader(_bound_sky);
  //_d3d11_renderer->changeRenderTarget(_buffer_sky_32);
  //_d3d11_renderer->clearRenderTarget(_buffer_sky_32, RBVector4::zero_vector);
  //_d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);

  static RBVector3 sun_dir = RBVector3(0, 5, 1);
  ImGui::SliderFloat3("sun dir", reinterpret_cast<float*>(&sun_dir), -1, 1);
  sun_dir = sun_dir.get_normalized();

  RBVector3 sphere_pos_v(0, -6360.001, 0);
  static RBVector3 space_pos_(0, 0, 0);
  ImGui::SliderFloat("space move x(60km)", (&space_pos_[0]), -12000, 12000);
  ImGui::SliderFloat("space move y(60km)", (&space_pos_[1]), -100, 6400);
  ImGui::SliderFloat("space move z(60km)", (&space_pos_[2]), -500, 14000);
  sphere_pos_v += space_pos_;

  static float sacle_factor;
  ImGui::SliderFloat("scale factor", &sacle_factor, 0, 50);
  static int sample_view = 16;
  static int sample_light = 8;
  ImGui::SliderInt("sample_view", &sample_view, 1, 256);
  ImGui::SliderInt("sample_light", &sample_light, 1, 256);



  float view_dist = _cam->get_near_panel();
  float fov_x = _cam->get_tan_half_fovx();
  float fov_y = _cam->get_tan_half_fovy();


  _d3d11_renderer->setShaderConstant3f("sphere_pos_v", sphere_pos_v);
  _d3d11_renderer->setShaderConstant1f("view_dist", view_dist);
  _d3d11_renderer->setShaderConstant1f("tan_fov_x", fov_x);
  _d3d11_renderer->setShaderConstant1f("tan_fov_y", fov_y);
  _d3d11_renderer->setShaderConstant3f("sun_dir", sun_dir);
  _d3d11_renderer->setShaderConstant1f("sacle_factor", sacle_factor);
  _d3d11_renderer->setShaderConstant1i("n_sample", sample_view);
  _d3d11_renderer->setShaderConstant1i("n_sample_light", sample_light);
  _d3d11_renderer->setShaderConstant4x4f("mv_mat", mv);

  _d3d11_renderer->apply();
  _d3d11_renderer->context->Draw(3, 0);
  
  _d3d11_renderer->setShader(_bound_cloud);
  _d3d11_renderer->setShaderConstant4x4f("mv_mat", mv);
  _d3d11_renderer->setShaderConstant4x4f("inv_mv_mat", mv.get_inverse_slow());

  _d3d11_renderer->setShaderConstant1f("view_dist", _cam->get_near_panel());
  _d3d11_renderer->setShaderConstant1f("tan_fov_x", _cam->get_tan_half_fovx());
  _d3d11_renderer->setShaderConstant1f("tan_fov_y", _cam->get_tan_half_fovy());
  ImGui::Begin("fbm factor");
  static float density = 1;
  static float basis = 0;
  static float lod = 1;
  static float lod1 = 1;
  ImGui::SliderFloat("density", &density, 0, 2);
  ImGui::SliderFloat("basis", &basis, -2, 2);
  ImGui::SliderFloat("lod", &lod, 1, 100);
  ImGui::SliderFloat("lod1", &lod1, 1, 100);
  ImGui::End();
  _d3d11_renderer->setShaderConstant1f("density", density);
  _d3d11_renderer->setShaderConstant1f("basis", basis);
  _d3d11_renderer->setShaderConstant1f("lod", lod);
  _d3d11_renderer->setShaderConstant1f("lod1", lod1);
  _d3d11_renderer->apply();
  _d3d11_renderer->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  _d3d11_renderer->context->Draw(3, 0);

  //render object
  _d3d11_renderer->setDepthState(_ds_enable);
  _d3d11_renderer->clear(false, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);

  _d3d11_renderer->setShader(_bound_shader);
  _d3d11_renderer->setShaderConstant4x4f("v", mv);
  _d3d11_renderer->setShaderConstant4x4f("o", mp);
  _d3d11_renderer->setShaderConstant4x4f("v1", mv);
  /*
  _d3d11_renderer->setShaderConstant1i("screen_size_x", ww);
  _d3d11_renderer->setShaderConstant1i("screen_size_y", wh);
  _d3d11_renderer->setShaderConstant1i("s", dx);
  */
  //proccess_tonemapping(0);
  _d3d11_renderer->setSamplerState("texture_state", _ss);

  _d3d11_renderer->setVertexFormat(_vf);
  _d3d11_renderer->setRasterizerState(_rs);
  setup_lights(nlights);

  for (auto& model : models)
  {

    _d3d11_renderer->setShaderConstant4x4f("m", model.model);
    _d3d11_renderer->setShaderConstant4x4f("m1", model.model);
    _d3d11_renderer->setVertexBuffer(0, model.vb);
    _d3d11_renderer->setIndexBuffer(model.ib);
    _d3d11_renderer->setTexture("diffuse", material_lib[model.mat_id].map_diffuse);
    _d3d11_renderer->setTexture("alpha_mask", material_lib[model.mat_id].map_mask);
    _d3d11_renderer->setTexture("metallic", material_lib[model.mat_id].map_metallic);
    _d3d11_renderer->setTexture("normal", material_lib[model.mat_id].map_normal);
    _d3d11_renderer->setTexture("roughness", material_lib[model.mat_id].map_roughness);




    _d3d11_renderer->apply();
    //放在前面会被覆盖，需要检查,maybe applytexture

    _d3d11_renderer->setStructreBufferAndApply("gLight", _lights);
    _d3d11_renderer->drawElements(Primitives::PRIM_TRIANGLES, 0, model.idxs.size(), 0, 0);
  }
  _d3d11_renderer->setRasterizerState(_rs_double_face);

  for (auto& model : models_alpha)
  {

    _d3d11_renderer->setShaderConstant4x4f("m", model.model);
    _d3d11_renderer->setShaderConstant4x4f("m1", model.model);
    _d3d11_renderer->setVertexBuffer(0, model.vb);
    _d3d11_renderer->setIndexBuffer(model.ib);
    _d3d11_renderer->setTexture("diffuse", material_lib[model.mat_id].map_diffuse);
    _d3d11_renderer->setTexture("alpha_mask", material_lib[model.mat_id].map_mask);
    _d3d11_renderer->setTexture("metallic", material_lib[model.mat_id].map_metallic);
    _d3d11_renderer->setTexture("normal", material_lib[model.mat_id].map_normal);
    _d3d11_renderer->setTexture("roughness", material_lib[model.mat_id].map_roughness);




    _d3d11_renderer->apply();
    //放在前面会被覆盖，需要检查,maybe applytexture

    _d3d11_renderer->setStructreBufferAndApply("gLight", _lights);
    _d3d11_renderer->drawElements(Primitives::PRIM_TRIANGLES, 0, model.idxs.size(), 0, 0);
  }
  _d3d11_renderer->setRasterizerState(_rs);

  _d3d11_renderer->setDepthState(_ds_disable);





  //return;

  //read_back_color_and_check(2);
  _d3d11_renderer->setShader(_bound_shader_gen_lum);
  _d3d11_renderer->setSamplerState("sampler_gen_lum", _linear_sampler);
  _d3d11_renderer->setViewport(512, 512);
  _d3d11_renderer->changeRenderTarget(_lumin_half);
  _d3d11_renderer->clearRenderTarget(_lumin_half, RBVector4::zero_vector);
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
  _d3d11_renderer->setTexture("hdr_color_gen_lun", _rt1_float);
  _d3d11_renderer->apply();
  _d3d11_renderer->context->Draw(3, 0);

  _d3d11_renderer->generateMipMaps(_lumin_half);

  _d3d11_renderer->setShader(_bound_shader_gen_avg_lum);
  _d3d11_renderer->setSamplerState("samp", _linear_sampler);
  _d3d11_renderer->setViewport(16, 16);
  _d3d11_renderer->changeRenderTarget(_lumin_avg_16);
  _d3d11_renderer->clearRenderTarget(_lumin_avg_16, RBVector4::zero_vector);
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
  _d3d11_renderer->setTexture("lumin_buffer", _lumin_half);
  static f32 middleGrey = 0.1;
  ImGui::SliderFloat("middleGrey", &middleGrey, 0, 0.8);
  _d3d11_renderer->setShaderConstant1f("middleGrey", middleGrey);
  _d3d11_renderer->apply();
  _d3d11_renderer->context->Draw(3, 0);


  //read_back(_d3d11_renderer->textures[_lumin_avg_16].texture);
  

  _d3d11_renderer->setViewport(ww / 2, ww / 2);
  _d3d11_renderer->setShader(_bound_shader_bright);
  _d3d11_renderer->setSamplerState("bright_sampler", _linear_sampler);
  _d3d11_renderer->changeRenderTarget(_bright_half);
  _d3d11_renderer->clearRenderTarget(_bright_half, RBVector4::zero_vector);
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
  _d3d11_renderer->setTexture("color_texture", _rt1_float);
  static float bloomThreshold = 1;
  ImGui::SliderFloat("bloomThreshold", &bloomThreshold, 0, 1);
  _d3d11_renderer->setShaderConstant1f("bloomThreshold", bloomThreshold);
  _d3d11_renderer->apply();
  _d3d11_renderer->context->Draw(3, 0);

  //blurxy
  _d3d11_renderer->setShader(_bound_shader_blur_x);
  _d3d11_renderer->changeRenderTarget(_bright_half_blur_x);
  _d3d11_renderer->clearRenderTarget(_bright_half_blur_x, RBVector4::zero_vector);
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
  _d3d11_renderer->setTexture("g_Input0", _bright_half);
  _d3d11_renderer->apply();
  _d3d11_renderer->context->Draw(3, 0);

  _d3d11_renderer->setShader(_bound_shader_blur_xy);
  _d3d11_renderer->changeRenderTarget(_bright_half_blur_xy);
  _d3d11_renderer->clearRenderTarget(_bright_half_blur_xy, RBVector4::zero_vector);
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
  _d3d11_renderer->setTexture("g_Input1", _bright_half_blur_x);
  _d3d11_renderer->apply();
  _d3d11_renderer->context->Draw(3, 0);

  //save this frame for next frame
  _d3d11_renderer->setViewport(ww, wh);
  _d3d11_renderer->setShader(_bound_shader_tone);
  _d3d11_renderer->changeRenderTarget(_last_frame[_last_use]);
  _d3d11_renderer->clearRenderTarget(_last_frame[_last_use], RBVector4::zero_vector);
  _d3d11_renderer->setSamplerState("s1", _linear_sampler);
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
  _d3d11_renderer->setTexture("color_buffer", _rt1_float);
  _d3d11_renderer->setTexture("blur_buffer", _bright_half_blur_xy);
  _d3d11_renderer->setTexture("avg_buffer", _lumin_avg_16);

  // _d3d11_renderer->setTexture("last_buffer", _lumin_avg_16);
  static f32 exposure_adjustment = 1.f;
  static f32 gamma_ = 2.2f;
  static f32 bloomMultiplier = 1.f;
  static f32 op_sc[4] = { 1, 1, 0, 0 };


  ImGui::Begin("HDR Param");
  ImGui::SliderFloat("exposure_adjustment", &exposure_adjustment, 0, 10);
  ImGui::SliderFloat("gama", &gamma_, 1.f, 2.2f);
  ImGui::SliderFloat("bloomMultiplier", &bloomMultiplier, 0, 3);
  ImGui::SliderFloat4("op_sc", op_sc, -1, 1);
  static int use_filimic = 0;
  ImGui::SliderInt("use filmic", &use_filimic, 0, 1);
  ImGui::End();

  if (use_filimic)
    proccess_tonemapping(dt);

  _d3d11_renderer->setShaderConstant1i("use_filimic",use_filimic);
  _d3d11_renderer->setShaderConstant1f("exposure_adjustment", exposure_adjustment);
  _d3d11_renderer->setShaderConstant1f("gamma", gamma_);
  _d3d11_renderer->setShaderConstant1f("bloomMultiplier", bloomMultiplier);
  _d3d11_renderer->setShaderConstant1f("time", dt);
  _d3d11_renderer->setShaderConstant4f("op_sc", RBVector4(op_sc));


  _d3d11_renderer->apply();
  _d3d11_renderer->context->Draw(3, 0);


  //render to back buffer
  _d3d11_renderer->setShader(_bound_shader_save_frame);
  _d3d11_renderer->changeToMainFramebuffer();
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
  _d3d11_renderer->setTexture("tocopy", _last_frame[_last_use]);
  _d3d11_renderer->apply();
  _d3d11_renderer->context->Draw(3, 0);



  /*
  _d3d11_renderer->setViewport(ww, wh);
  _d3d11_renderer->changeToMainFramebuffer();
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);

  _d3d11_renderer->setShader(_bound_shader_bilt);
  _d3d11_renderer->setSamplerState("sam", _ss);
  _d3d11_renderer->setTexture("origin_buffer", _bright_half_blur_xy);
  _d3d11_renderer->apply();
  _d3d11_renderer->context->Draw(3, 0);

  _d3d11_renderer->setViewport(128, 128);
  _d3d11_renderer->changeToMainFramebuffer();
  //_d3d11_renderer->clear(false, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
  _d3d11_renderer->setTexture("origin_buffer", _bright_half);

  _d3d11_renderer->apply();
  _d3d11_renderer->context->Draw(3, 0);

  */

  _d3d11_renderer->setDepthState(_ds_enable);

  //ping-pong
  _last_use = 1 - _last_use;
  /*
  _d3d11_renderer->setShader(_bound_shader_tone);
  _d3d11_renderer->setSamplerState("sampler", _ss);
  _d3d11_renderer->setTexture("color_buffer", _rt1_float);
  static f32 exposure_adjustment = 1;
  static f32 gamma_ = 2.2f;
  ImGui::Begin("HDR Param");
  ImGui::SliderFloat("exposure_adjustment", &exposure_adjustment,0,10);
  ImGui::SliderFloat("gama", &gamma_, 1.f, 2.2f);
  ImGui::End();
  _d3d11_renderer->setShaderConstant1f("exposure_adjustment", exposure_adjustment);
  _d3d11_renderer->setShaderConstant1f("gamma",gamma_);
  _d3d11_renderer->apply();
  _d3d11_renderer->context->Draw(3, 0);
  */


}

void SceneLoadFastApp::read_back_color_and_check(int ii)
{
  //g_logger->debug(WIP_INFO, "loading color back...");
  std::vector<RBVector4> v;
  v.resize(ww  * wh);
  // Copy the data to a staging resource.
  uint32 Subresource = 0;
  D3D11_BOX	Rect;
  Rect.left = 0;
  Rect.top = 0;
  Rect.right = ww;
  Rect.bottom = wh;
  Rect.back = 1;
  Rect.front = 0;
  _d3d11_renderer->context->CopySubresourceRegion(_d3d11_renderer->textures[_read_back_buffer_full].texture, 0, 0, 0, 0, _d3d11_renderer->textures[_rt1_float].texture, Subresource, &Rect);
  // Lock the staging resource.
  D3D11_MAPPED_SUBRESOURCE LockedRect;
  HRESULT hs = (_d3d11_renderer->context->Map(_d3d11_renderer->textures[_read_back_buffer_full].texture, 0, D3D11_MAP_READ, 0, &LockedRect));
  CHECKF(hs == S_OK, "read lumin_half failed!");
  memcpy(&v[0], LockedRect.pData, v.size()*sizeof(RBVector4));

  ii = (ii * 2 / ww) * ww * 2 + ii % (ww / 2) * 2;
  int k = 0;
  for (auto i : v)
  {
    /*
    if (k == ii||k==ii-2)
    g_logger->debug(WIP_ERROR, "color:%f,%f,%f,%f  | ref lum:%f", i.x, i.y, i.z, i.w, RBVector3::dot_product(i, RBVector3(0.299f, 0.587f, 0.114f)));
    */
    if ((RBMath::is_NaN_f32(i.x) || RBMath::is_NaN_f32(i.y) || RBMath::is_NaN_f32(i.z) || RBMath::is_NaN_f32(i.w))
      || ((i.x < 0) || (i.y < 0) || (i.z < 0) || (i.w < 0)))
    {

      g_logger->debug(WIP_ERROR, "fack pixel: %f,%f,%f,%f! |  id:%d", i.x, i.y, i.z, i.w, k / ww / 2 * ww / 2 + k%ww / 2);
      getchar();
      break;
    }

    k++;

  }
  _d3d11_renderer->context->Unmap(_d3d11_renderer->textures[_read_back_buffer_full].texture, 0);
}

void SceneLoadFastApp::read_back_lumin_half()
{
  g_logger->debug(WIP_INFO, "loading back...");
  std::vector<RBVector4> v;
  v.resize(ww / 2 * wh / 2);
  // Copy the data to a staging resource.
  uint32 Subresource = 0;
  D3D11_BOX	Rect;
  Rect.left = 0;
  Rect.top = 0;
  Rect.right = ww / 2;
  Rect.bottom = wh / 2;
  Rect.back = 1;
  Rect.front = 0;
  _d3d11_renderer->context->CopySubresourceRegion(_d3d11_renderer->textures[_read_back_buffer_half].texture, 0, 0, 0, 0, _d3d11_renderer->textures[_lumin_half].texture, Subresource, &Rect);
  // Lock the staging resource.
  D3D11_MAPPED_SUBRESOURCE LockedRect;
  HRESULT hs = (_d3d11_renderer->context->Map(_d3d11_renderer->textures[_read_back_buffer_half].texture, 0, D3D11_MAP_READ, 0, &LockedRect));
  CHECKF(hs == S_OK, "read lumin_half failed!");
  memcpy(&v[0], LockedRect.pData, v.size()*sizeof(RBVector4));

  int k = 0;
  if (p)
    for (auto i : v)
    {

      if (i.x < 0)
      {
        g_logger->debug(WIP_ERROR, "note lumince is less than 0: %f", i.x);
        read_back_color_and_check(k);
      }
      if (RBMath::is_NaN_f32(i.x))
      {
        g_logger->debug(WIP_ERROR, "note %d lumince is nan : %f %f %f", k, i.x, i.y, i.z);
        g_logger->debug(WIP_ERROR, "note %d lumince is nan : %f %f %f", k - 1, v[k - 1].x, v[k - 1].y, v[k - 1].z);
        read_back_color_and_check(k);
      }
      k++;
    }
  _d3d11_renderer->context->Unmap(_d3d11_renderer->textures[_read_back_buffer_half].texture, 0);
}

void SceneLoadFastApp::read_back(ID3D11Resource* res)
{
  static f32 r1 = 0, g1 = 0, b1 = 0, a1 = 0;
  // Copy the data to a staging resource.
  uint32 Subresource = 0;
  D3D11_BOX	Rect;
  Rect.left = 0;
  Rect.top = 0;
  Rect.right = 16;
  Rect.bottom = 16;
  Rect.back = 1;
  Rect.front = 0;
  _d3d11_renderer->context->CopySubresourceRegion(_d3d11_renderer->textures[_read_back_buffer_16].texture, 0, 0, 0, 0, res, Subresource, &Rect);
  // Lock the staging resource.
  D3D11_MAPPED_SUBRESOURCE LockedRect;
  HRESULT hs = (_d3d11_renderer->context->Map(_d3d11_renderer->textures[_read_back_buffer_16].texture, 0, D3D11_MAP_READ, 0, &LockedRect));
  CHECK(hs == S_OK);
  RBVector4* out;
  out = (RBVector4*)LockedRect.pData;
  f32 r = (*out)[0];
  f32 g = (*out)[1];
  f32 b = ((*out)[2]);
  f32 a = ((*out)[3]);
  if (RBMath::abs(r - r1) > 0.1 || RBMath::abs(b - b1) > 0.1 || RBMath::abs(g - g1) > 0.1)
  {
    g_logger->debug(WIP_INFO, "%f,%f,%f,%f", (*out)[0], g, b, a);
    r1 = r; g1 = g; b1 = b; a1 = a;
  }
  if (RBMath::is_NaN_f32(g) || r > 400)
    read_back_lumin_half();

  _d3d11_renderer->context->Unmap(_d3d11_renderer->textures[_read_back_buffer_16].texture, 0);

}

void SceneLoadFastApp::draw()
{


}

void SceneLoadFastApp::ter()
{
  _d3d11_renderer->finish();
  delete _d3d11_renderer;
}

void SceneLoadFastApp::postter()
{

}

SceneLoadFastApp::SceneLoadFastApp(class AppBase* app) : FastApp(app)
{

}

SceneLoadFastApp::~SceneLoadFastApp()
{

}

void SceneLoadFastApp::load_models_generate_buffer(const char* filename)
{
  /** load file */
  {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    std::string path;
    path = RBPathTool::get_file_directory(filename);
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename, path.data());

    if (!err.empty())
    {
      std::cerr << err << std::endl;
    }

    if (!ret)
    {
      g_logger->debug_print("读取文件%s失败\n", filename);
      getchar();
    }

    //load material libary
    for (auto mat : materials)
    {

      Material mat_;
      mat_.name = mat.name;
      mat_.ka = mat.ambient;
      mat_.kd = mat.diffuse;
      mat_.ks = mat.specular;
      mat_.ke = mat.emission;
      mat_.tf = mat.transmittance;
      mat_.ns = mat.shininess;
      mat_.ni = mat.ior;
      mat_.d = mat.dissolve;
      mat_.tr = 0;
      mat_.illum = mat.illum;
      {
        //metallic
        Image img;
        if (!img.loadImage((mat.ambient_texname != "") ?
          (path + mat.ambient_texname).c_str() :
          (g_logger->debug(WIP_INFO, "%s no metallic", mat_.name.c_str()), (path + "textures_pbr_dds/Dielectric_metallic.dds").c_str())))
        {
          g_logger->debug(WIP_ERROR, "Read image %s failed!", mat.ambient_texname.c_str());
          getchar();
        }
        mat_.map_metallic = _d3d11_renderer->addTexture(img);
      }
    {
      //diffuse
      Image img;
      if (!img.loadImage((mat.diffuse_texname != "") ?
        ((path + mat.diffuse_texname).c_str()) :
        (g_logger->debug(WIP_INFO, "%s no diffuse", mat_.name.c_str()), (path + "textures_pbr_dds/Metallic_metallic.dds").c_str())
        ))
      {
        g_logger->debug(WIP_ERROR, "Read image %s failed!", mat.diffuse_texname.c_str());
        getchar();
      }
      mat_.map_diffuse = _d3d11_renderer->addTexture(img);
    }
    {
      //roughness
      Image img;
      if (!img.loadImage(mat.specular_highlight_texname != "" ?
        (path + mat.specular_highlight_texname).c_str() :
        (g_logger->debug(WIP_INFO, "%s no roughness", mat_.name.c_str()), (path + "textures_pbr_dds/Metallic_metallic.dds").c_str())
        ))
      {
        g_logger->debug(WIP_ERROR, "Read image %s failed!", mat.specular_highlight_texname.c_str());
        getchar();
      }
      mat_.map_roughness = _d3d11_renderer->addTexture(img);
    }
    {
      //normal
      //G不变/B翻转/R移动到Alpha

      Image img;
      if (!img.loadImage(mat.bump_texname != "" ?
        (path + mat.bump_texname).c_str() :
        (g_logger->debug(WIP_INFO, "%s no normal", mat_.name.c_str()), (path + "textures_pbr_dds/Dielectric_metallic.dds").c_str())
        ))
      {
        g_logger->debug(WIP_ERROR, "Read image %s failed!", mat.bump_texname.c_str());
        getchar();
      }
      mat_.map_normal = _d3d11_renderer->addTexture(img);
    }
    {
      //mask
      Image img;
      if (!img.loadImage(mat.alpha_texname != "" ?
        (path + mat.alpha_texname).c_str() :
        (g_logger->debug(WIP_INFO, "%s no mask", mat_.name.c_str()), (path + "textures_pbr_dds/Metallic_metallic.dds").c_str())
        ))
      {
        g_logger->debug(WIP_ERROR, "Read image %s failed!", mat.alpha_texname.c_str());
        getchar();
      }
      mat_.map_mask = _d3d11_renderer->addTexture(img);
    }

    material_lib.push_back(mat_);
    }



    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++)
    {
      // Loop over faces(polygon)
      size_t index_offset = 0;
      MeshModel model;
      model.mat_id = shapes[s].mesh.material_ids[0];
      if (model.mat_id < 0)
        printf("%s\n", shapes[s].name.c_str());
      for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
      {
        int fv = shapes[s].mesh.num_face_vertices[f];

        // Loop over vertices in the face.
        for (size_t v = 0; v < fv; v++)
        {
          // access to vertex
          tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
          float vx = attrib.vertices[3 * idx.vertex_index + 0];
          float vy = attrib.vertices[3 * idx.vertex_index + 1];
          float vz = attrib.vertices[3 * idx.vertex_index + 2];
          float nx = attrib.normals[3 * idx.normal_index + 0];
          float ny = attrib.normals[3 * idx.normal_index + 1];
          float nz = attrib.normals[3 * idx.normal_index + 2];
          float tx = attrib.texcoords[2 * idx.texcoord_index + 0];
          float ty = attrib.texcoords[2 * idx.texcoord_index + 1];
          Vertex_PNT vv;
          vv.pos = RBVector3(vx, vy, vz);
          vv.normal = RBVector3(nx, ny, nz);
          vv.texcoord = RBVector2(tx, ty);
          model.vecs.push_back(vv);
          model.idxs.push_back(v + index_offset);// idx.vertex_index);

        }
        index_offset += fv;

        // per-face material
        //shapes[s].mesh.material_ids[f];
      }
      if (!(strcmp(materials[shapes[s].mesh.material_ids[0]].name.c_str(), "_sponza_sponzaleaf") && strcmp(materials[shapes[s].mesh.material_ids[0]].name.c_str(), "_sponza_sponzaMaterial__57")))
        models_alpha.push_back(model);
      else
        models.push_back(model);
    }

    for (auto& model : models)
    {
      model.ib = _d3d11_renderer->addIndexBuffer(model.idxs.size(), sizeof(u32), BufferAccess::STATIC, model.idxs.data());
      model.vb = _d3d11_renderer->addVertexBuffer(model.vecs.size()*sizeof(Vertex_PNT), BufferAccess::STATIC, model.vecs.data());
    }
    for (auto& model : models_alpha)
    {
      model.ib = _d3d11_renderer->addIndexBuffer(model.idxs.size(), sizeof(u32), BufferAccess::STATIC, model.idxs.data());
      model.vb = _d3d11_renderer->addVertexBuffer(model.vecs.size()*sizeof(Vertex_PNT), BufferAccess::STATIC, model.vecs.data());
    }
  }

  FormatDesc fd[3];
  fd[0].format = AttributeFormat::FORMAT_FLOAT;
  fd[0].size = 3;
  fd[0].stream = 0;
  fd[0].type = AttributeType::TYPE_VERTEX;
  fd[1].format = AttributeFormat::FORMAT_FLOAT;
  fd[1].size = 3;
  fd[1].stream = 0;
  fd[1].type = AttributeType::TYPE_NORMAL;
  fd[2].format = AttributeFormat::FORMAT_FLOAT;
  fd[2].size = 2;
  fd[2].stream = 0;
  fd[2].type = AttributeType::TYPE_TEXCOORD;

  _vf = _d3d11_renderer->addVertexFormat(fd, 3, _bound_shader);
}

void SceneLoadFastApp::load_assets()
{
#ifdef USEFRAMEWORK3
  auto res_handle = g_res_manager->load_resource("shaders/simple_v.hlsl", WIPResourceType::TEXT);
  //auto res_handle1 = g_res_manager->load_resource("shaders/conservation_rasterization_g.hlsl", WIPResourceType::TEXT);
  auto res_handle2 = g_res_manager->load_resource("shaders/simple_p.hlsl", WIPResourceType::TEXT);
  _bound_shader = _d3d11_renderer->addShader(((std::string*)res_handle->ptr)->c_str(),
    0/*((std::string*)res_handle1->ptr)->c_str()*/, ((std::string*)res_handle2->ptr)->c_str(), 0, 0, 0);
#else    
  auto sh1 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/simple_v.hlsl", "main", "vs_5_0");
  auto sh2 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/simple_p.hlsl", "main", "ps_5_0");
  _bound_shader = _d3d11_renderer->addShader(sh1, 0, sh2, D3D11ShaderCompiler::reflect_shader);
#endif

  auto sh3 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/hdr_post.hlsl", "main_vs", "vs_5_0");
  auto sh4 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/hdr_post.hlsl", "main_ps", "ps_5_0");
  _bound_shader_tone = _d3d11_renderer->addShader(sh3, 0, sh4, D3D11ShaderCompiler::reflect_shader);


  auto sh5 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/post_bilt.hlsl", "main_vs", "vs_5_0");
  auto sh6 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/post_bilt.hlsl", "main_ps", "ps_5_0");
  _bound_shader_bilt = _d3d11_renderer->addShader(sh5, 0, sh6, D3D11ShaderCompiler::reflect_shader);

  auto sh7 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/post_gen_lumin.hlsl", "main_vs", "vs_5_0");
  auto sh8 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/post_gen_lumin.hlsl", "main_ps", "ps_5_0");
  _bound_shader_gen_lum = _d3d11_renderer->addShader(sh7, 0, sh8, D3D11ShaderCompiler::reflect_shader);

  {
    auto sh7 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/post_calc_avg_lum.hlsl", "main_vs", "vs_5_0");
    auto sh8 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/post_calc_avg_lum.hlsl", "main_ps", "ps_5_0");
    _bound_shader_gen_avg_lum = _d3d11_renderer->addShader(sh7, 0, sh8, D3D11ShaderCompiler::reflect_shader);
  }

  {
    auto sh7 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/post_blur_x.hlsl", "main_vs", "vs_5_0");
    auto sh8 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/post_blur_x.hlsl", "main_ps", "ps_5_0");
    _bound_shader_blur_x = _d3d11_renderer->addShader(sh7, 0, sh8, D3D11ShaderCompiler::reflect_shader);
  }

  {
    auto sh7 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/post_blur_xy.hlsl", "main_vs", "vs_5_0");
    auto sh8 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/post_blur_xy.hlsl", "main_ps", "ps_5_0");
    _bound_shader_blur_xy = _d3d11_renderer->addShader(sh7, 0, sh8, D3D11ShaderCompiler::reflect_shader);
  }

  {
    auto sh7 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/post_bright.hlsl", "main_vs", "vs_5_0");
    auto sh8 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/post_bright.hlsl", "main_ps", "ps_5_0");
    _bound_shader_bright = _d3d11_renderer->addShader(sh7, 0, sh8, D3D11ShaderCompiler::reflect_shader);
  }


  {
    auto sh7 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/post_save_frame.hlsl", "main_vs", "vs_5_0");
    auto sh8 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/post_save_frame.hlsl", "main_ps", "ps_5_0");
    _bound_shader_save_frame = _d3d11_renderer->addShader(sh7, 0, sh8, D3D11ShaderCompiler::reflect_shader);
  }

  {
    auto sh7 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/sky_v.hlsl", "main_vs", "vs_5_0");
    auto sh8 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/sky_v.hlsl", "main_ps", "ps_5_0");
    _bound_sky = _d3d11_renderer->addShader(sh7, 0, sh8, D3D11ShaderCompiler::reflect_shader);
  }
  {
	  auto sh1 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/gpu_noise.hlsl", "main_vs", "vs_5_0");
	  auto sh2 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/gpu_noise.hlsl", "main_ps", "ps_5_0");
	  _bound_cloud = _d3d11_renderer->addShader(sh1, 0, sh2, D3D11ShaderCompiler::reflect_shader);
  }
  load_models_generate_buffer("res/sponza/pbr/sp.obj");
}

void SceneLoadFastApp::create_rhires()
{
  _ds_enable = _d3d11_renderer->addDepthState(true, true, D3D11_COMPARISON_LESS, true, 0xFF, 0xFF,
    D3D11_COMPARISON_ALWAYS, D3D11_COMPARISON_ALWAYS,
    D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP,
    D3D11_STENCIL_OP_INCR, D3D11_STENCIL_OP_DECR,
    D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP);
  _ds_disable = _d3d11_renderer->addDepthState(false, true, D3D11_COMPARISON_LESS, false, 0xFF, 0xFF,
    D3D11_COMPARISON_ALWAYS, D3D11_COMPARISON_ALWAYS,
    D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP,
    D3D11_STENCIL_OP_INCR, D3D11_STENCIL_OP_DECR,
    D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP);
  _ds_write_disable = _d3d11_renderer->addDepthState(true, false, D3D11_COMPARISON_LESS_EQUAL, false, 0xFF, 0xFF,
    D3D11_COMPARISON_ALWAYS, D3D11_COMPARISON_ALWAYS,
    D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP,
    D3D11_STENCIL_OP_INCR, D3D11_STENCIL_OP_DECR,
    D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP);
  _bs_enable = _d3d11_renderer->addBlendState(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA,
    D3D11_BLEND_ONE, D3D11_BLEND_ZERO,
    D3D11_BLEND_OP_ADD, D3D11_BLEND_OP_ADD,
    D3D11_COLOR_WRITE_ENABLE_ALL);
  _bs_disable = _d3d11_renderer->addBlendState(D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL);
  float a[] = { 0, 0, 0, 0 };
  _ss = _d3d11_renderer->addSamplerState(Filter::BILINEAR_ANISO, AddressMode::WRAP, AddressMode::WRAP, AddressMode::WRAP, 0, 16, 0, a);
  _rs = _d3d11_renderer->addRasterizerState(CULL_BACK, SOLID, false, false, 0, 0);
  _rs_double_face = _d3d11_renderer->addRasterizerState(CULL_NONE, SOLID, false, false, 0, 0);

  _buffer_depth_stencil = _d3d11_renderer->addRenderDepth(ww, wh, 1, FORMAT::FORMAT_D24S8);
  _buffer_depth_stencil_low = _d3d11_renderer->addRenderDepth(ww, wh, 1, FORMAT::FORMAT_D24S8);


  _lights = _d3d11_renderer->addStructureBuffer<PointLight>(nlights, true);

  _linear_sampler = _d3d11_renderer->addSamplerState(Filter::BILINEAR, AddressMode::WRAP, AddressMode::WRAP, AddressMode::WRAP, 0, 0, 0, a);

  _rt1_float = _d3d11_renderer->addRenderTarget(ww, wh, 1, 1, 1, FORMAT::FORMAT_RGBA32F);
  _bright_half = _d3d11_renderer->addRenderTarget(ww / 2, wh / 2, 1, 1, 1, FORMAT::FORMAT_RGBA16F);
  _bright_half_blur_x = _d3d11_renderer->addRenderTarget(ww / 2, wh / 2, 1, 1, 1, FORMAT::FORMAT_RGBA16F);
  _bright_half_blur_xy = _d3d11_renderer->addRenderTarget(ww / 2, wh / 2, 1, 1, 1, FORMAT::FORMAT_RGBA16F);
  _lumin_half = _d3d11_renderer->addRenderTarget(ww / 2, wh / 2, 1, 10, 1, FORMAT::FORMAT_RGBA32F, 1, -1, USE_MIPGEN);
  _lumin_avg_16 = _d3d11_renderer->addRenderTarget(16, 16, 1, 1, 1, FORMAT::FORMAT_RGBA32F, 1, -1);

  _d3d11_renderer->clearRenderTarget(_bright_half_blur_x, RBVector4::zero_vector);
  _d3d11_renderer->clearRenderTarget(_bright_half_blur_xy, RBVector4::zero_vector);

  _last_frame[0] = _d3d11_renderer->addRenderTarget(ww, wh, 1, 1, 1, FORMAT::FORMAT_RGBA8);
  _last_frame[1] = _d3d11_renderer->addRenderTarget(ww, wh, 1, 1, 1, FORMAT::FORMAT_RGBA8);

  _buffer_sky_32 = _d3d11_renderer->addRenderTarget(ww, wh, 1, 1, 1, FORMAT::FORMAT_RGBA32F);


  {
    D3D11_TEXTURE2D_DESC desc;
    rb_memzero(&desc, sizeof(desc));
    desc.Width = 16;
    desc.Height = 16;
    desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.ArraySize = 1;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    HRESULT res = _d3d11_renderer->device->CreateTexture2D(&desc, 0, &tex);
    if (res != S_OK)
    {
      g_logger->debug(WIP_ERROR, "Couldn't create staging texture!");
    }

    _read_back_buffer_16 = _d3d11_renderer->addTexture(tex, 0);
  }
  //read_back(_d3d11_renderer->textures[_lumin_avg_16].texture);
  {
    D3D11_TEXTURE2D_DESC desc;
    rb_memzero(&desc, sizeof(desc));
    desc.Width = ww / 2;
    desc.Height = wh / 2;
    desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.ArraySize = 1;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    HRESULT res = _d3d11_renderer->device->CreateTexture2D(&desc, 0, &tex);
    if (res != S_OK)
    {
      g_logger->debug(WIP_ERROR, "Couldn't create staging texture!");
    }

    _read_back_buffer_half = _d3d11_renderer->addTexture(tex, 0);
  }
    {
      D3D11_TEXTURE2D_DESC desc;
      rb_memzero(&desc, sizeof(desc));
      desc.Width = ww;
      desc.Height = wh;
      desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
      desc.MipLevels = 1;
      desc.SampleDesc.Count = 1;
      desc.SampleDesc.Quality = 0;
      desc.Usage = D3D11_USAGE_STAGING;
      desc.BindFlags = 0;
      desc.ArraySize = 1;
      desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
      HRESULT res = _d3d11_renderer->device->CreateTexture2D(&desc, 0, &tex);
      if (res != S_OK)
      {
        g_logger->debug(WIP_ERROR, "Couldn't create staging texture!");
      }

      _read_back_buffer_full = _d3d11_renderer->addTexture(tex, 0);
    }

}

void SceneLoadFastApp::handle_input(f32 dt)
{
  if (Input::get_key_up(WIP_Q))
  {
    dx--;
    printf("%d", dx);
  }
  if (Input::get_key_up(WIP_E))
  {
    dx++;
    printf("%d", dx);
  }
  _cam->control_update(dt);
}


void SceneLoadFastApp::proccess_imgui(f32 dt)
{
  ImGui::SetNextWindowSize(ImVec2(100, 50));
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::Begin("Another Window");
  ImGui::Value("FPS", 1.f / dt);
  ImGui::End();
}

void SceneLoadFastApp::proccess_tonemapping(f32 dt)
{
  //Order: render scene in HDR, Histogram from HDR color, Do EyeAdaptation based on historgram and eye adaptation settings, SceneColorTint, tonemapper maps the HDR color to an LDR color, Color grading is applied to map the LDR color to another LDR color (think of photoshop color adjustments - they happen in LDR)
  //https://answers.unrealengine.com/questions/10564/questiontrying-to-understand-hdr-ldr-eye-adaptatio.html
  //http://www.gamedev.net/topic/670001-calculate-hdr-exposure-from-luminance-histogram/


  ImGui::Begin("Tonemaping Test");
  ImGui::SliderFloat("A", A, 0.001f, 1);
  ImGui::SliderFloat("B", B, 0.001f, 1);
  ImGui::SliderFloat("C", C, 0.001f, 1);
  ImGui::SliderFloat("D", D, 0.001f, 1);
  ImGui::SliderFloat("E", E_, 0.001f, 1);
  ImGui::SliderFloat("F", F, 0.001f, 1);
  ImGui::SliderFloat("W", W, 0.001f, 100);
  ImGui::SliderFloat("ExposureBias", ExposureBias, 0.001f, 10);
  //ImGui::SliderFloat("ExposureAdjustment", ExposureAdjustment, 0.001f, 50);

  if (ImGui::Button("Restore", ImVec2(100, 50)))
  {
#define __(V) (V[0]) = (V[1])
    __(A); __(B); __(C); __(D);
    __(E_); __(F); __(W);
    __(ExposureBias);
    __(ExposureAdjustment);
#undef __
  }

  ImGui::End();



  RBVector4 ABCD;
  ABCD[0] = A[0];
  ABCD[1] = B[0];
  ABCD[2] = C[0];
  ABCD[3] = D[0];
  RBVector3 EFW;
  EFW[0] = E_[0];
  EFW[1] = F[0];
  EFW[2] = W[0];

  _d3d11_renderer->setShaderConstant4f("ABCD", ABCD);
  _d3d11_renderer->setShaderConstant3f("EFW", EFW);
  _d3d11_renderer->setShaderConstant1f("ExposureBias", ExposureBias[0]);
  //_d3d11_renderer->setShaderConstant1f("ExposureAdjustment", ExposureAdjustment[0]);
};


void SceneLoadFastApp::resize(int w, int h, bool bfullscreen)
{
  //当前不支持resize
  /*
  _d3d11_renderer->setViewport(ww, wh);
  _d3d11_renderer->setFrameBuffer(back_buffer_view, _d3d11_renderer->getDSV(_buffer_depth_stencil));
  _d3d11_renderer->changeToMainFramebuffer();
  _d3d11_renderer->resizeRenderTarget
  */
}

