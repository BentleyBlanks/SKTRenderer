#include "SceneEditorFastApp.h"
#include "Camera.h"
#include "tiny_obj_loader.h"
#include <iostream>
#include "ResourceManager.h"
#include "Input.h"
#include "thirdpart/imgui/imgui.h"
#include "D3D11ShaderResources.h"
#include "SMAA/AreaTex.h"
#include "SMAA/SearchTex.h"

class SMAAHelper
{
public:
  static bool init(Direct3D11Renderer* render)
  {
    _render_ref = render;
    _device_ref = render->device;
    _context_ref = render->context;


    ID3D11Texture2D *areaTex;
    ID3D11ShaderResourceView *areaTexSRV;
    ID3D11Texture2D *searchTex;
    ID3D11ShaderResourceView *searchTexSRV;
    //load area texture
    {
    HRESULT hr;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = areaTexBytes;
    data.SysMemPitch = AREATEX_PITCH;
    data.SysMemSlicePitch = 0;

    D3D11_TEXTURE2D_DESC descTex;
    descTex.Width = AREATEX_WIDTH;
    descTex.Height = AREATEX_HEIGHT;
    descTex.MipLevels = descTex.ArraySize = 1;
    descTex.Format = DXGI_FORMAT_R8G8_UNORM;
    descTex.SampleDesc.Count = 1;
    descTex.SampleDesc.Quality = 0;
    descTex.Usage = D3D11_USAGE_DEFAULT;
    descTex.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    descTex.CPUAccessFlags = 0;
    descTex.MiscFlags = 0;
    CHECK(_device_ref->CreateTexture2D(&descTex, &data, &areaTex));

    D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
    descSRV.Format = descTex.Format;
    descSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    descSRV.Texture2D.MostDetailedMip = 0;
    descSRV.Texture2D.MipLevels = 1;
    CHECK(_device_ref->CreateShaderResourceView(areaTex, &descSRV, &areaTexSRV));

    _area_tex = _render_ref->addTexture1(areaTex, areaTexSRV);
    }
    //load search texture
    {
    HRESULT hr;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = searchTexBytes;
    data.SysMemPitch = SEARCHTEX_PITCH;
    data.SysMemSlicePitch = 0;

    D3D11_TEXTURE2D_DESC descTex;
    descTex.Width = SEARCHTEX_WIDTH;
    descTex.Height = SEARCHTEX_HEIGHT;
    descTex.MipLevels = descTex.ArraySize = 1;
    descTex.Format = DXGI_FORMAT_R8_UNORM;
    descTex.SampleDesc.Count = 1;
    descTex.SampleDesc.Quality = 0;
    descTex.Usage = D3D11_USAGE_DEFAULT;
    descTex.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    descTex.CPUAccessFlags = 0;
    descTex.MiscFlags = 0;
    CHECK(_device_ref->CreateTexture2D(&descTex, &data, &searchTex));

    D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
    descSRV.Format = descTex.Format;
    descSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    descSRV.Texture2D.MostDetailedMip = 0;
    descSRV.Texture2D.MipLevels = 1;
    CHECK(_device_ref->CreateShaderResourceView(searchTex, &descSRV, &searchTexSRV));

    _search_tex = _render_ref->addTexture1(searchTex, searchTexSRV);
    }
    //load shader

  }
private:
  static Direct3D11Renderer* _render_ref;
  static ID3D11Device *_device_ref;
  static ID3D11DeviceContext *_context_ref;
  static TextureID _area_tex;
  static TextureID _search_tex;
  static ShaderID _edge_detection;
  static ShaderID _blending_weight_calculation;
  static ShaderID _neighborhood_blending;
};

bool SceneEditorFastApp::preinit()
{
  dx = 0;
  for (int i = 0; i < max_model; ++i)
  {
	  _ib[i] = 0;
	  _vb[i] = 0;
	  _duffuse_texture[i] = 0;
	  _normal_texture[i] = 0;
	  _mra_texture[i] = 0;
	  _model_matrix[i] =  RBMatrix::identity ;
	  _draw_size[i] = 0 ;
  }
  return true;
}

bool SceneEditorFastApp::init()
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

  _cam->set_move_speed(RBVector2(15, 15));
  _cam->set_rotate_speed(RBVector2(50, 50));

  _d3d11_renderer = new Direct3D11Renderer(_rhi->GetDevice(), _rhi->GetDeviceContext());

  load_assets();
  create_rhires();

  _d3d11_renderer->setBlendState(_bs_enable, 0xffffffff);
  _d3d11_renderer->setDepthState(_ds_enable);
  _d3d11_renderer->setRasterizerState(_rs);


  _d3d11_renderer->setViewport(ww, wh);
  _d3d11_renderer->setFrameBuffer(back_buffer_view, _d3d11_renderer->getDSV(_buffer_depth_stencil));
  _d3d11_renderer->changeToMainFramebuffer();

  point_lights[0].position_ = RBVector3(-36, 50.457, -9);
  point_lights[1].position_ = RBVector3(-23.0, 82.622, -59.7);
  point_lights[2].position_ = RBVector3(-51.9, 76.6, -74.537);
  point_lights[3].position_ = RBVector3(-23.0, 76.622, -59.7);


  return true;
}

void SceneEditorFastApp::update(f32 dt)
{
  handle_input(dt);
  proccess_imgui(dt);
}

void SceneEditorFastApp::draw()
{
  _d3d11_renderer->setBlendState(_bs_enable, 0xffffffff);
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
  RBMatrix mm;
  RBMatrix mp;
  RBMatrix mv;

  _cam->get_view_matrix(mv);
  _cam->get_perspective_matrix(mp);


  _d3d11_renderer->changeRenderTarget(_rt1_float, _buffer_depth_stencil);


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
  static f32 light_color[3] = { 1, 1, 1 };
  ImGui::SliderFloat3("sun color", light_color, 0, 10);
  ImGui::SliderInt("sample_view", &sample_view, 1, 256);
  ImGui::SliderInt("sample_light", &sample_light, 1, 256);



  float view_dist = _cam->get_near_panel();
  float fov_x = _cam->get_tan_half_fovx();
  float fov_y = _cam->get_tan_half_fovy();
  static int sky = 1;
  ImGui::SliderInt("sky", &sky, 0, 3);

  _d3d11_renderer->setShaderConstant3f("sphere_pos_v", sphere_pos_v);
  _d3d11_renderer->setShaderConstant1f("view_dist", view_dist);
  _d3d11_renderer->setShaderConstant1f("tan_fov_x", fov_x);
  _d3d11_renderer->setShaderConstant1f("tan_fov_y", fov_y);
  _d3d11_renderer->setShaderConstant3f("sun_dir", sun_dir);
  _d3d11_renderer->setShaderConstant3f("sun_color", RBVector3(light_color));
  _d3d11_renderer->setShaderConstant1f("sacle_factor", sacle_factor);
  _d3d11_renderer->setShaderConstant1i("n_sample", sample_view);
  _d3d11_renderer->setShaderConstant1i("n_sample_light", sample_light);
  _d3d11_renderer->setShaderConstant4x4f("mv_mat", mv);
  _d3d11_renderer->setShaderConstant4x4f("imv_mat", mv.get_inverse_slow());
  _d3d11_renderer->setTexture("spec_map", _spec_texture[sky]);
  _d3d11_renderer->setSamplerState("ss", _ss);
  _d3d11_renderer->apply();
  _d3d11_renderer->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  _d3d11_renderer->context->Draw(3, 0);

  _d3d11_renderer->setShader(_view_shader);
  _d3d11_renderer->setDepthState(_ds_enable);
  _d3d11_renderer->setBlendState(_bs_enable);
  _d3d11_renderer->setShaderConstant4x4f("v", mv);
  _d3d11_renderer->setShaderConstant4x4f("o", mp);


  
  _d3d11_renderer->setTexture("irr_map", _irr_texture[sky]);
  _d3d11_renderer->setTexture("spec_map", _spec_texture[sky]);
  _d3d11_renderer->setTexture("brdf_lut", _spec_brdf_lut);
  _d3d11_renderer->setSamplerState("ss", _ss);
  _d3d11_renderer->setShaderConstant3f("sun_dir_m", sun_dir);
  _d3d11_renderer->setShaderConstant3f("sun_color_m", RBVector3(light_color));

   ImGui::SliderFloat3("point position 1", reinterpret_cast<float*>(&point_lights[0].position_), -100, 100);
   ImGui::SliderFloat3("point position 2", reinterpret_cast<float*>(&point_lights[1].position_), -100, 100);
   ImGui::SliderFloat3("point position 3", reinterpret_cast<float*>(&point_lights[2].position_), -100, 100);
   ImGui::SliderFloat3("point position 4", reinterpret_cast<float*>(&point_lights[3].position_), -100, 100);
   ImGui::SliderFloat3("point clq", reinterpret_cast<float*>(&point_lights[0].clq_), 0, 1);
  //4 byte aligned per vector
  _d3d11_renderer->setShaderConstantRaw("point_lights", point_lights, sizeof(PointLight) * ARRAY_COUNT(point_lights));
  static int nl = 1;
  ImGui::SliderInt("light_n", &nl, 1, 4);
  
  _d3d11_renderer->setShaderConstant1i("light_n", nl);
  _d3d11_renderer->setShaderConstant3f("view_world_pos",_cam->get_position());
  static int sm = 0;
  ImGui::SliderInt("SM", &sm, 0, 2);
  _d3d11_renderer->setShaderConstant1i("shading_model", sm);
  if (Input::get_key_up(WIP_C))
	  _cam->get_position().out();
  static f32 roughness = 0.1f;
  ImGui::SliderFloat("roughness", &roughness,0.01f,1.f);
  _d3d11_renderer->setShaderConstant1f("roughness", roughness);
  static f32 metallic = 0.1f;
  ImGui::SliderFloat("metallic", &metallic, 0.01f, 1.f);
  _d3d11_renderer->setShaderConstant1f("metallic", metallic);
  
  _d3d11_renderer->setVertexFormat(_vf);
  for (int i = 0; i < cur_model; ++i)
  {
	  _d3d11_renderer->setShaderConstant4x4f("m", _model_matrix[i]);
	  _d3d11_renderer->setTexture("dft", _duffuse_texture[i]);
	  _d3d11_renderer->setTexture("normal_map", _normal_texture[i]);
	  _d3d11_renderer->setTexture("mra", _mra_texture[i]);
	  _d3d11_renderer->setVertexBuffer(0, _vb[i]);
	  _d3d11_renderer->setIndexBuffer(_ib[i]);
	  _d3d11_renderer->apply();
	  _d3d11_renderer->drawElements(Primitives::PRIM_TRIANGLES, 0, _draw_size[i]);

  }

  

  _d3d11_renderer->setDepthState(_ds_disable);
  _d3d11_renderer->setBlendState(_bs_disable);

  _d3d11_renderer->setShader(_bound_shader_gen_lum);
  _d3d11_renderer->setSamplerState("sampler_gen_lum", _ss);
  _d3d11_renderer->setViewport(512, 512);
  _d3d11_renderer->changeRenderTarget(_lumin_half);
  _d3d11_renderer->clearRenderTarget(_lumin_half, RBVector4::zero_vector);
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
  _d3d11_renderer->setTexture("hdr_color_gen_lun", _rt1_float);
  _d3d11_renderer->apply();
  _d3d11_renderer->context->Draw(3, 0);

  _d3d11_renderer->generateMipMaps(_lumin_half);

  _d3d11_renderer->setShader(_bound_shader_gen_avg_lum);
  _d3d11_renderer->setSamplerState("samp", _ss);
  _d3d11_renderer->setViewport(16, 16);
  _d3d11_renderer->changeRenderTarget(_lumin_avg_16);
  _d3d11_renderer->clearRenderTarget(_lumin_avg_16, RBVector4::zero_vector);
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
  _d3d11_renderer->setTexture("lumin_buffer", _lumin_half);
  static f32 middleGrey = 0.18f;
  ImGui::SliderFloat("middleGrey", &middleGrey, 0, 0.8);
  _d3d11_renderer->setShaderConstant1f("middleGrey", middleGrey);
  _d3d11_renderer->apply();
  _d3d11_renderer->context->Draw(3, 0);

  _d3d11_renderer->setViewport(ww / 2, ww / 2);
  //read_back(_d3d11_renderer->textures[_lumin_avg_16].texture);

  // 重实现降采样，平均3x3像素！
  
  _d3d11_renderer->setShader(_bound_shader_bright);
  _d3d11_renderer->setSamplerState("bright_sampler", _ss);
  _d3d11_renderer->changeRenderTarget(_bright_half);
  _d3d11_renderer->clearRenderTarget(_bright_half, RBVector4::zero_vector);
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
  _d3d11_renderer->setTexture("color_texture", _rt1_float);
  static float bloomThreshold = 1;
  ImGui::SliderFloat("bloomThreshold", &bloomThreshold, 0, 20);
  _d3d11_renderer->setShaderConstant1f("bloomThreshold", bloomThreshold);
  _d3d11_renderer->apply();
  _d3d11_renderer->context->Draw(3, 0);



  //blur1
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

  _d3d11_renderer->setViewport(ww / 4, ww / 4);
  _d3d11_renderer->setShader(_bound_shader_downsample);
  _d3d11_renderer->changeRenderTarget(_bright_half2);
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
  _d3d11_renderer->setTexture("orign_tex", _bright_half);
  _d3d11_renderer->apply();
  _d3d11_renderer->context->Draw(3, 0);
  //blur2
  _d3d11_renderer->setShader(_bound_shader_blur_x);
  _d3d11_renderer->changeRenderTarget(_bright_half2_blur_x);
  _d3d11_renderer->clearRenderTarget(_bright_half2_blur_x, RBVector4::zero_vector);
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
  _d3d11_renderer->setTexture("g_Input0", _bright_half2);
  _d3d11_renderer->apply();
  _d3d11_renderer->context->Draw(3, 0);

  _d3d11_renderer->setShader(_bound_shader_blur_xy);
  _d3d11_renderer->changeRenderTarget(_bright_half2_blur_xy);
  _d3d11_renderer->clearRenderTarget(_bright_half2_blur_xy, RBVector4::zero_vector);
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
  _d3d11_renderer->setTexture("g_Input1", _bright_half2_blur_x);
  _d3d11_renderer->apply();
  _d3d11_renderer->context->Draw(3, 0);

  _d3d11_renderer->setViewport(ww / 8, ww / 8);
  _d3d11_renderer->setShader(_bound_shader_downsample);
  _d3d11_renderer->changeRenderTarget(_bright_half4);
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
  _d3d11_renderer->setTexture("orign_tex", _bright_half2);
  _d3d11_renderer->apply();
  _d3d11_renderer->context->Draw(3, 0);
  //blur3
  _d3d11_renderer->setShader(_bound_shader_blur_x);
  _d3d11_renderer->changeRenderTarget(_bright_half4_blur_x);
  _d3d11_renderer->clearRenderTarget(_bright_half4_blur_x, RBVector4::zero_vector);
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
  _d3d11_renderer->setTexture("g_Input0", _bright_half4);
  _d3d11_renderer->apply();
  _d3d11_renderer->context->Draw(3, 0);

  _d3d11_renderer->setShader(_bound_shader_blur_xy);
  _d3d11_renderer->changeRenderTarget(_bright_half4_blur_xy);
  _d3d11_renderer->clearRenderTarget(_bright_half4_blur_xy, RBVector4::zero_vector);
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
  _d3d11_renderer->setTexture("g_Input1", _bright_half4_blur_x);
  _d3d11_renderer->apply();
  _d3d11_renderer->context->Draw(3, 0);

  _d3d11_renderer->setViewport(ww / 16, ww / 16);
  _d3d11_renderer->setShader(_bound_shader_downsample);
  _d3d11_renderer->changeRenderTarget(_bright_half8);
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
  _d3d11_renderer->setTexture("orign_tex", _bright_half4);
  _d3d11_renderer->apply();
  _d3d11_renderer->context->Draw(3, 0);
  //blur4
  _d3d11_renderer->setShader(_bound_shader_blur_x);
  _d3d11_renderer->changeRenderTarget(_bright_half8_blur_x);
  _d3d11_renderer->clearRenderTarget(_bright_half8_blur_x, RBVector4::zero_vector);
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
  _d3d11_renderer->setTexture("g_Input0", _bright_half8);
  _d3d11_renderer->apply();
  _d3d11_renderer->context->Draw(3, 0);

  _d3d11_renderer->setShader(_bound_shader_blur_xy);
  _d3d11_renderer->changeRenderTarget(_bright_half8_blur_xy);
  _d3d11_renderer->clearRenderTarget(_bright_half8_blur_xy, RBVector4::zero_vector);
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
  _d3d11_renderer->setTexture("g_Input1", _bright_half8_blur_x);
  _d3d11_renderer->apply();
  _d3d11_renderer->context->Draw(3, 0);


  static  RBVector4 tint(0.7f, 0.2f, 0.9f, 1.0f);
  static  float tex_scale = 1.f;
  static  float blur_scale = 1.f;
  static  float threshold = 0.1f;
  static  int lens_sample = 8;
  {
    ImGui::Begin("LensParam");
    ImGui::SliderFloat4("Tint", (float*)&tint, 0.f, 1.f);
    ImGui::SliderFloat("Texture Scale", &tex_scale, 0.f, 10.f);
    ImGui::SliderFloat("Blur Scale", &blur_scale, 0.f, 10.f);
    ImGui::SliderFloat("Threshold", &threshold, 0.f, 10.f);
    ImGui::SliderInt("Samples", &lens_sample, 1, 64);
    ImGui::End();
    //lens
    _d3d11_renderer->setShader(_bound_shader_lensflare);
    _d3d11_renderer->changeRenderTarget(_lens_half);
    _d3d11_renderer->clearRenderTarget(_lens_half, RBVector4::zero_vector);
    _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
    _d3d11_renderer->setTexture("light_tex", _bright_half_blur_xy);
    _d3d11_renderer->setSamplerState("ls", _ss);
    _d3d11_renderer->setTexture("light_tex2", _bright_half2_blur_xy);
    _d3d11_renderer->setTexture("light_tex4", _bright_half4_blur_xy);
    _d3d11_renderer->setTexture("light_tex8", _bright_half8_blur_xy);
    _d3d11_renderer->setTexture("lensColor", _lens_blur);
    _d3d11_renderer->setShaderConstant1i("NumSamples", lens_sample);
    _d3d11_renderer->setShaderConstant4f("vTint", tint);
    _d3d11_renderer->setShaderConstant1f("fTexScale", tex_scale);
    _d3d11_renderer->setShaderConstant1f("fBlurScale", blur_scale);
    _d3d11_renderer->setShaderConstant1f("fThreshold", threshold);
    _d3d11_renderer->setShaderConstant2f("sslight_pos", _cam->world_to_screen(point_lights[0].position_));

    _d3d11_renderer->setShaderConstant2f("sxy", RBVector2(ww, wh));
    _d3d11_renderer->apply();
    _d3d11_renderer->context->Draw(3, 0);
  }

  static f32 exposure_adjustment = 1.f;
  static f32 gamma_ = 2.2f;
  static f32 bloomMultiplier = 1.f;
  static f32 op_sc[4] = { 1, 1, 0, 0 };
  static f32 dirt_density = 1.f;
  static int aa_type = 0;
  static int enable_debug = 0;
  ImGui::Begin("HDR Param");
  ImGui::SliderFloat("exposure_adjustment", &exposure_adjustment, 0, 10);
  ImGui::SliderFloat("gama", &gamma_, 1.f, 2.2f);
  ImGui::SliderFloat("bloomMultiplier", &bloomMultiplier, 0, 3);
  ImGui::SliderFloat("dirt_density", &dirt_density, 0, 10);
  
  ImGui::SliderFloat4("op_sc", op_sc, -1, 1);
  static int use_filimic = 0;
  ImGui::SliderInt("use filmic", &use_filimic, 0, 1);
  ImGui::SliderInt("avg_debug", &enable_debug, 0, 1);
  ImGui::SliderInt("AA(NOAA/FXAA/SMAA)", &aa_type, 0, 2);

  ImGui::End();

  _d3d11_renderer->setViewport(ww, wh);

  switch (aa_type)
  { 
  case (1):
  {

    _d3d11_renderer->setShader(_bound_shader_fxaa);
    _d3d11_renderer->changeRenderTarget(_rt1_float1);
    _d3d11_renderer->clearRenderTarget(_rt1_float1, RBVector4::zero_vector);
    _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
    _d3d11_renderer->setTexture("scene", _rt1_float);
    _d3d11_renderer->setSamplerState("fxss", _ss);
    _d3d11_renderer->setShaderConstant2f("screenxy", RBVector2(ww, wh));;
    _d3d11_renderer->apply();
    _d3d11_renderer->context->Draw(3, 0);
  }
          break;
  case 2:
    g_logger->debug_print("SMAA is not surpported now!");
    
  case 0:
  {
    _d3d11_renderer->setShader(_bound_shader_save_frame);
    _d3d11_renderer->changeRenderTarget(_rt1_float1);
    _d3d11_renderer->clearRenderTarget(_rt1_float1, RBVector4::zero_vector);
    _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
    _d3d11_renderer->setSamplerState("sam_copy", _ss);
    _d3d11_renderer->setTexture("tocopy", _rt1_float);
    _d3d11_renderer->apply();
    _d3d11_renderer->context->Draw(3, 0);

  }
  break;
  }
  //return;
  //save this frame for next frame
  
  _d3d11_renderer->setShader(_bound_shader_tone);
  _d3d11_renderer->changeRenderTarget(_last_frame[0]);
  _d3d11_renderer->clearRenderTarget(_last_frame[0], RBVector4::zero_vector);
  _d3d11_renderer->setSamplerState("s1", _ss);
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
  _d3d11_renderer->setTexture("color_buffer", _rt1_float1);
  _d3d11_renderer->setTexture("len_buffer", _lens_half);
  _d3d11_renderer->setTexture("blur_buffer", _bright_half_blur_xy);
  _d3d11_renderer->setTexture("blur_buffer2", _bright_half2_blur_xy);
  _d3d11_renderer->setTexture("blur_buffer4", _bright_half4_blur_xy);
  _d3d11_renderer->setTexture("blur_buffer8", _bright_half8_blur_xy);
  _d3d11_renderer->setTexture("dirt_mask", _text_dirt);
  _d3d11_renderer->setTexture("avg_buffer", _lumin_avg_16);

  //_d3d11_renderer->generateMipMaps(_last_frame[0]);

  // _d3d11_renderer->setTexture("last_buffer", _lumin_avg_16);
 
  

  if (use_filimic)
    proccess_tonemapping(1);

  _d3d11_renderer->setShaderConstant1i("use_filimic", use_filimic);
  _d3d11_renderer->setShaderConstant1f("exposure_adjustment", exposure_adjustment);
  _d3d11_renderer->setShaderConstant1f("gamma", gamma_);
  _d3d11_renderer->setShaderConstant1f("bloomMultiplier", bloomMultiplier);
  _d3d11_renderer->setShaderConstant1f("dirt_density", dirt_density);

  _d3d11_renderer->setShaderConstant1f("time", 1);
  _d3d11_renderer->setShaderConstant4f("op_sc", RBVector4(op_sc));
  _d3d11_renderer->setShaderConstant1i("avg_debug", enable_debug);

  _d3d11_renderer->apply();
  _d3d11_renderer->context->Draw(3, 0);

  //render to back buffer
  _d3d11_renderer->setShader(_bound_shader_save_frame);
  _d3d11_renderer->changeToMainFramebuffer();
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
  _d3d11_renderer->setTexture("tocopy", _last_frame[0]);
  _d3d11_renderer->setSamplerState("sam_copy", _ss);
  _d3d11_renderer->apply();
  _d3d11_renderer->context->Draw(3, 0);

  

}

void SceneEditorFastApp::ter()
{
  _d3d11_renderer->finish();
  delete _d3d11_renderer;
}

void SceneEditorFastApp::postter()
{

}

SceneEditorFastApp::SceneEditorFastApp(class AppBase* app) : FastApp(app)
{

}

SceneEditorFastApp::~SceneEditorFastApp()
{

}

void SceneEditorFastApp::load_models_generate_buffer(const char* filename)
{
	/** load file */
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;
		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename);

		if (!err.empty())
		{
			std::cerr << err << std::endl;
		}

		if (!ret)
		{
			g_logger->debug_print("读取文件%s失败\n", filename);
			getchar();
		}

		_vecs.clear();
		_idxs.clear();
		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces(polygon)
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				int fv = shapes[s].mesh.num_face_vertices[f];

				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++) {
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
					vv.texcoord = RBVector2(tx, 1 - ty);
					_vecs.push_back(vv);
					_idxs.push_back(v + index_offset);// idx.vertex_index);

				}
				index_offset += fv;

				// per-face material
				shapes[s].mesh.material_ids[f];
			}
		}
	}

	if (!_vf)
	{
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

		_vf = _d3d11_renderer->addVertexFormat(fd, 3, _view_shader);
	}

	{
		_vb[cur_model] = _d3d11_renderer->addVertexBuffer(_vecs.size()*sizeof(Vertex_PNT), BufferAccess::STATIC, _vecs.data());
		_ib[cur_model] = _d3d11_renderer->addIndexBuffer(_idxs.size(), sizeof(u32), BufferAccess::STATIC, _idxs.data());
		_draw_size[cur_model] = _idxs.size();
	}
}

void SceneEditorFastApp::load_pbr_model(const char* path,  const RBVector3& pos, const RBVector3& rot)
{
	std::string path1 = "res/model/";
	path1 += path;
	path1 += "/";
	std::string path_obj = path1+"model.obj";
	std::string path_d = path1+"BaseColor.png";
	std::string path_n = path1+"NormalMap.png";
	std::string path_m = path1+"MRA.png";
	if (cur_model < max_model)
	{
	load_models_generate_buffer(path_obj.c_str());
		{
			Image img4;
			img4.loadImage(path_d.c_str());
			bool succ = img4.createMipMaps();
			if (!succ)
				printf("generate mip map failed!\n");
			_duffuse_texture[cur_model] = _d3d11_renderer->addTexture(img4);
			img4.free();
		}
  {
	  Image img4;
	  img4.loadImage(path_m.c_str());
	  bool succ = img4.createMipMaps();
	  if (!succ)
		  printf("generate mip map failed!\n");
	  _mra_texture[cur_model] = _d3d11_renderer->addTexture(img4);
	  img4.free();
  }
	{
		Image img4;
		img4.loadImage(path_n.c_str());
		_normal_texture[cur_model] = _d3d11_renderer->addTexture(img4);
		img4.free();
	}
	}
	_model_matrix[cur_model].set_translation(pos);
	_model_matrix[cur_model].set_rotation(rot.x,rot.y,rot.z);

	cur_model++;
}

#include <fstream>

void SceneEditorFastApp::load_assets()
{

  auto sh1 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/model_view_lighting.hlsl", "main_vs", "vs_5_0");
  auto sh2 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/model_view_lighting.hlsl", "main_ps", "ps_5_0");
  _view_shader = _d3d11_renderer->addShader(sh1, 0, sh2, D3D11ShaderCompiler::reflect_shader);
  
  //!!!recalculate ibl especilly specular(include brdf!)
  std::fstream fs;
  fs.open("res/scene.sce",std::ios::in);
  string name;
  RBVector3 pos;
  RBVector3 rot;
  int count = 0;
  while (fs >> name)
  {
    if (++count > max_model) break;
    fs >> pos.x >> pos.y >> pos.z;
    fs >> rot.x >> rot.y >> rot.z;
    load_pbr_model(name.c_str(),pos,rot);
  }
  fs.close();
  
}

void SceneEditorFastApp::create_rhires()
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
  _bs_enable = _d3d11_renderer->addBlendState(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA,
    D3D11_BLEND_ONE, D3D11_BLEND_ZERO,
    D3D11_BLEND_OP_ADD, D3D11_BLEND_OP_ADD,
    D3D11_COLOR_WRITE_ENABLE_ALL);
  _bs_disable = _d3d11_renderer->addBlendState(D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL);
  float a[] = { 0, 0, 0, 0 };
  _ss = _d3d11_renderer->addSamplerState(Filter::BILINEAR, AddressMode::CLAMP, AddressMode::CLAMP, AddressMode::CLAMP, 0, 0, 0, a);
  _bs = _d3d11_renderer->addSamplerState(Filter::BILINEAR, AddressMode::WRAP, AddressMode::WRAP, AddressMode::CLAMP, 0, 0, 0, a);
  _rs = _d3d11_renderer->addRasterizerState(CULL_BACK, SOLID, false, false, 0, 0);
  _buffer_depth_stencil = _d3d11_renderer->addRenderDepth(ww, wh, 1, FORMAT::FORMAT_D24S8);




  _rt1_float = _d3d11_renderer->addRenderTarget(ww, wh, 1, 1, 1, FORMAT::FORMAT_RGBA32F);
  _rt1_float1 = _d3d11_renderer->addRenderTarget(ww, wh, 1, 1, 1, FORMAT::FORMAT_RGBA32F);

  _buffer_sky_32 = _d3d11_renderer->addRenderTarget(ww, wh, 1, 1, 1, FORMAT::FORMAT_RGBA32F);
  {
    auto sh7 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/sky_v.hlsl", "main_vs", "vs_5_0");
    auto sh8 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/sky_v.hlsl", "main_ps", "ps_5_0");
    _bound_sky = _d3d11_renderer->addShader(sh7, 0, sh8, D3D11ShaderCompiler::reflect_shader);
  }

  auto sh3 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/hdr_post.hlsl", "main_vs", "vs_5_0");
  auto sh4 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/hdr_post.hlsl", "main_ps", "ps_5_0");
  _bound_shader_tone = _d3d11_renderer->addShader(sh3, 0, sh4, D3D11ShaderCompiler::reflect_shader);

  /*
  auto sh5 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/post_bilt.hlsl", "main_vs", "vs_5_0");
  auto sh6 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/post_bilt.hlsl", "main_ps", "ps_5_0");
  _bound_shader_bilt = _d3d11_renderer->addShader(sh5, 0, sh6, D3D11ShaderCompiler::reflect_shader);
  */

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
      auto sh7 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/downsample.hlsl", "main_vs", "vs_5_0");
      auto sh8 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/downsample.hlsl", "main_ps", "ps_5_0");
      _bound_shader_downsample = _d3d11_renderer->addShader(sh7, 0, sh8, D3D11ShaderCompiler::reflect_shader);
    }
    {
      auto sh7 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/fxaa.hlsl", "main_vs", "vs_5_0");
      auto sh8 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/fxaa.hlsl", "main_ps", "ps_5_0");
      _bound_shader_fxaa = _d3d11_renderer->addShader(sh7, 0, sh8, D3D11ShaderCompiler::reflect_shader);
    }
    {
      auto sh7 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/lens_flare.hlsl", "main_vs", "vs_5_0");
      auto sh8 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/lens_flare.hlsl", "main_ps", "ps_5_0");
      _bound_shader_lensflare = _d3d11_renderer->addShader(sh7, 0, sh8, D3D11ShaderCompiler::reflect_shader);
    }
  _bright_half = _d3d11_renderer->addRenderTarget(ww / 2, wh / 2, 1, 1, 1, FORMAT::FORMAT_RGBA16F);
  _lens_half = _d3d11_renderer->addRenderTarget(ww / 2, wh / 2, 1, 1, 1, FORMAT::FORMAT_RGBA16F);
  _bright_half_blur_x = _d3d11_renderer->addRenderTarget(ww / 2, wh / 2, 1, 1, 1, FORMAT::FORMAT_RGBA16F);
  _bright_half_blur_xy = _d3d11_renderer->addRenderTarget(ww / 2, wh / 2, 1, 1, 1, FORMAT::FORMAT_RGBA16F);
  int s = 4;
  _bright_half2 = _d3d11_renderer->addRenderTarget(ww / s, wh / s, 1, 1, 1, FORMAT::FORMAT_RGBA16F);
  _bright_half2_blur_x = _d3d11_renderer->addRenderTarget(ww / s, wh / s, 1, 1, 1, FORMAT::FORMAT_RGBA16F);
  _bright_half2_blur_xy = _d3d11_renderer->addRenderTarget(ww / s, wh / s, 1, 1, 1, FORMAT::FORMAT_RGBA16F);
  s = 8;
  _bright_half4 = _d3d11_renderer->addRenderTarget(ww / s, wh / s, 1, 1, 1, FORMAT::FORMAT_RGBA16F);
  _bright_half4_blur_x = _d3d11_renderer->addRenderTarget(ww / s, wh / s, 1, 1, 1, FORMAT::FORMAT_RGBA16F);
  _bright_half4_blur_xy = _d3d11_renderer->addRenderTarget(ww / s, wh / s, 1, 1, 1, FORMAT::FORMAT_RGBA16F);
  s = 16;
  _bright_half8 = _d3d11_renderer->addRenderTarget(ww / s, wh / s, 1, 1, 1, FORMAT::FORMAT_RGBA16F);
  _bright_half8_blur_x = _d3d11_renderer->addRenderTarget(ww / s, wh / s, 1, 1, 1, FORMAT::FORMAT_RGBA16F);
  _bright_half8_blur_xy = _d3d11_renderer->addRenderTarget(ww / s, wh / s, 1, 1, 1, FORMAT::FORMAT_RGBA16F);

  _lumin_half = _d3d11_renderer->addRenderTarget(ww / 2, wh / 2, 1, 10, 1, FORMAT::FORMAT_RGBA32F, 1, -1, USE_MIPGEN);
  _lumin_avg_16 = _d3d11_renderer->addRenderTarget(16, 16, 1, 1, 1, FORMAT::FORMAT_RGBA32F, 1, -1);

  _d3d11_renderer->clearRenderTarget(_bright_half_blur_x, RBVector4::zero_vector);
  _d3d11_renderer->clearRenderTarget(_bright_half_blur_xy, RBVector4::zero_vector);

  _last_frame[0] = _d3d11_renderer->addRenderTarget(ww, wh, 1, 4, 1, FORMAT::FORMAT_RGBA8);
  _last_frame[1] = _d3d11_renderer->addRenderTarget(ww, wh, 1, 4, 1, FORMAT::FORMAT_RGBA8);

  {
	  const char* filenames[] = { "res/cube/2/output_iem_posx.hdr", "res/cube/2/output_iem_negx.hdr", "res/cube/2/output_iem_posy.hdr", "res/cube/2/output_iem_negy.hdr", "res/cube/2/output_iem_posz.hdr", "res/cube/2/output_iem_negz.hdr" };
	  const char* filenames1[] = { "res/cube/2/output_pmrem_posx.hdr", "res/cube/2/output_pmrem_negx.hdr", "res/cube/2/output_pmrem_posy.hdr", "res/cube/2/output_pmrem_negy.hdr", "res/cube/2/output_pmrem_posz.hdr", "res/cube/2/output_pmrem_negz.hdr" };

	  //_d3d11_renderer->addSamplerState(Filter::LINEAR, AddressMode::CLAMP, AddressMode::CLAMP, AddressMode::CLAMP);
	  _irr_texture[0] = _d3d11_renderer->addCubemap(filenames, true);
	  _spec_texture[0] = _d3d11_renderer->addCubemap(filenames1, true);
  }
  {
	  Image image;
	  image.loadImage("res/model/ibl_brdf_lut.png");
	  _spec_brdf_lut = _d3d11_renderer->addTexture(image);
	  image.free();
  }
  {
    Image image;
    image.loadImage("res/DirtMask.png");
    _text_dirt = _d3d11_renderer->addTexture(image);
    image.free();
  }
  {
    Image image;
    image.loadImage("res/lens.png");
    _lens_blur = _d3d11_renderer->addTexture(image);
    image.free();
  }
   {
	   const char* filenames[] = { "res/cube/1/output_iem_posx.hdr", "res/cube/1/output_iem_negx.hdr", "res/cube/1/output_iem_posy.hdr", "res/cube/1/output_iem_negy.hdr", "res/cube/1/output_iem_posz.hdr", "res/cube/1/output_iem_negz.hdr" };
	   const char* filenames1[] = { "res/cube/1/output_pmrem_posx.hdr", "res/cube/1/output_pmrem_negx.hdr", "res/cube/1/output_pmrem_posy.hdr", "res/cube/1/output_pmrem_negy.hdr", "res/cube/1/output_pmrem_posz.hdr", "res/cube/1/output_pmrem_negz.hdr" };

	   //_d3d11_renderer->addSamplerState(Filter::LINEAR, AddressMode::CLAMP, AddressMode::CLAMP, AddressMode::CLAMP);
	   _irr_texture[1] = _d3d11_renderer->addCubemap(filenames, true);
	   _spec_texture[1] = _d3d11_renderer->addCubemap(filenames1, true);
   }
	{
		const char* filenames[] = { "res/cube/3/output_iem_posx.hdr", "res/cube/3/output_iem_negx.hdr", "res/cube/3/output_iem_posy.hdr", "res/cube/3/output_iem_negy.hdr", "res/cube/3/output_iem_posz.hdr", "res/cube/3/output_iem_negz.hdr" };
		const char* filenames1[] = { "res/cube/3/output_pmrem_posx.hdr", "res/cube/3/output_pmrem_negx.hdr", "res/cube/3/output_pmrem_posy.hdr", "res/cube/3/output_pmrem_negy.hdr", "res/cube/3/output_pmrem_posz.hdr", "res/cube/3/output_pmrem_negz.hdr" };

		//_d3d11_renderer->addSamplerState(Filter::LINEAR, AddressMode::CLAMP, AddressMode::CLAMP, AddressMode::CLAMP);
		_irr_texture[2] = _d3d11_renderer->addCubemap(filenames, true);
		_spec_texture[2] = _d3d11_renderer->addCubemap(filenames1, true);
	}
	 {
		 const char* filenames[] = { "res/cube/4/output_iem_posx.hdr", "res/cube/4/output_iem_negx.hdr", "res/cube/4/output_iem_posy.hdr", "res/cube/4/output_iem_negy.hdr", "res/cube/4/output_iem_posz.hdr", "res/cube/4/output_iem_negz.hdr" };
		 const char* filenames1[] = { "res/cube/4/output_pmrem_posx.hdr", "res/cube/4/output_pmrem_negx.hdr", "res/cube/4/output_pmrem_posy.hdr", "res/cube/4/output_pmrem_negy.hdr", "res/cube/4/output_pmrem_posz.hdr", "res/cube/4/output_pmrem_negz.hdr" };

		 //_d3d11_renderer->addSamplerState(Filter::LINEAR, AddressMode::CLAMP, AddressMode::CLAMP, AddressMode::CLAMP);
		 _irr_texture[3] = _d3d11_renderer->addCubemap(filenames, true);
		 _spec_texture[3] = _d3d11_renderer->addCubemap(filenames1, true);
	 }
  
}

void SceneEditorFastApp::handle_input(f32 dt)
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


void SceneEditorFastApp::proccess_imgui(f32 dt)
{
  make_ibl();
}

void SceneEditorFastApp::proccess_tonemapping(f32 dt)
{
};


void SceneEditorFastApp::resize(int w, int h, bool bfullscreen)
{
  //当前不支持resize
  /*
  _d3d11_renderer->setViewport(ww, wh);
  _d3d11_renderer->setFrameBuffer(back_buffer_view, _d3d11_renderer->getDSV(_buffer_depth_stencil));
  _d3d11_renderer->changeToMainFramebuffer();
  _d3d11_renderer->resizeRenderTarget
  */
}

void SceneEditorFastApp::draw1()
{

  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);
  RBMatrix mm;
  RBMatrix mp;
  RBMatrix mv;

  mm.set_identity();
  _cam->get_view_matrix(mv);
  _cam->get_perspective_matrix(mp);


  _d3d11_renderer->setShader(_bound_shader);
  _d3d11_renderer->setShaderConstant4x4f("mv_mat", mv);
  _d3d11_renderer->setShaderConstant4x4f("inv_mv_mat", mv.get_inverse_slow());

  _d3d11_renderer->setShaderConstant1f("view_dist", _cam->get_near_panel());
  _d3d11_renderer->setShaderConstant1f("tan_fov_x", _cam->get_tan_half_fovx());
  _d3d11_renderer->setShaderConstant1f("tan_fov_y", _cam->get_tan_half_fovy());
  ImGui::Begin("fbm factor");
  static float density = 1;
  static float basis = 0;
  static float density1 = 1;
  static float basis1 = 0;
  static float density2 = 1;
  static float basis2 = 0;
  static float lod = 1;
  static float lod1 = 1;
  static float lod_1 = 1;
  static float lod_2 = 1;
  static float lod_3 = 1;
  static float height_range[2] = { 0, 1 };
  ImGui::SliderFloat("density", &density, 0, 2);

  ImGui::SliderFloat("basis", &basis, -2, 2);
  ImGui::SliderFloat("density worley", &density1, 0, 2);
  ImGui::SliderFloat("basis worley", &basis1, -2, 2);
  ImGui::SliderFloat("density perlin-worley", &density2, 0, 2);
  ImGui::SliderFloat("basis perlin-worley", &basis2, -2, 2);
  ImGui::SliderFloat("lod", &lod, 0, 100);
  ImGui::SliderFloat("lod1", &lod1, 0, 5);
  ImGui::SliderFloat("lod_1", &lod_1, 0, 100);
  ImGui::SliderFloat("lod_2", &lod_2, 0, 100);
  ImGui::SliderFloat("lod_3", &lod_3, 0, 100);
  ImGui::SliderFloat("min height", &height_range[0], -1, 3);
  ImGui::SliderFloat("max height", &height_range[1], -1, 3);

  ImGui::End();
  _d3d11_renderer->setShaderConstant1f("density", density);
  _d3d11_renderer->setShaderConstant1f("density1", density1);
  _d3d11_renderer->setShaderConstant1f("density2", density2);

  _d3d11_renderer->setShaderConstant1f("basis", basis);
  _d3d11_renderer->setShaderConstant1f("basis1", basis1);

  _d3d11_renderer->setShaderConstant1f("basis2", basis2);
  _d3d11_renderer->setShaderConstant1f("lod", lod);
  _d3d11_renderer->setShaderConstant1f("lod1", lod1);
  _d3d11_renderer->setShaderConstant1f("lod_1", lod_1);
  _d3d11_renderer->setShaderConstant1f("lod_2", lod_2);
  _d3d11_renderer->setShaderConstant1f("lod_3", lod_3);
  _d3d11_renderer->setShaderConstant2f("height_range", RBVector2(height_range[0], height_range[1]));
  _d3d11_renderer->apply();
  _d3d11_renderer->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  _d3d11_renderer->context->Draw(3, 0);
}


void SceneEditorFastApp::make_ibl()
{
  ImGui::Begin("IBL Tools");
  static bool v = false;
  ImGui::Checkbox("Specular", &v);
  if(ImGui::Button("Generate"))
  { 
    if (v)
    {
     g_logger->debug(WIP_INFO, "generate Specular");

    }
    else
    {
      g_logger->debug(WIP_INFO, "generate Diffuse");
    }
  }
  ImGui::End();
}