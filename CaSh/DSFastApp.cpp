#include "DSFastApp.h"
#include "tiny_obj_loader.h"
#include "D3D11ShaderResources.h"
#include "RHID3D11.h"
#include "Camera.h"
#include "Input.h"
#include <iostream>

bool DSFastApp::preinit()
{
  compile_shader = nullptr;
  CompilerDLL = 0;
  g_res_manager->startup();
  _high_light = 0.5f;
  _factor_l = 1.0;
  _shading_model = 0;
  return true;
}

bool DSFastApp::init()
{
  FD3D11DynamicRHI* _rhid3d11 = dynamic_cast<FD3D11DynamicRHI*>(FD3D11DynamicRHI::GetRHI(ERHITYPES::RHI_D3D11));
  _device = _rhid3d11->GetDevice();
  _context = _rhid3d11->GetDeviceContext();
  _gi_factory = _rhid3d11->GetFactory();
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

  _d3d11_renderer = new Direct3D11Renderer(_rhid3d11->GetDevice(), _rhid3d11->GetDeviceContext());

  load_assets();
  create_rhires();

  _d3d11_renderer->setBlendState(_blending_state, 0xffffffff);
  _d3d11_renderer->setDepthState(_depth_stencil_state);
  _d3d11_renderer->setRasterizerState(_raster_state);

  _d3d11_renderer->setViewport(ww, wh);
  _d3d11_renderer->setFrameBuffer(back_buffer_view, _d3d11_renderer->getDSV(_buffer_depth_stencil));
  _d3d11_renderer->changeToMainFramebuffer();

  return true;
}

void DSFastApp::update(f32 dt)
{
  handle_input(dt);
}

void DSFastApp::draw()
{
  TextureID views[] = {_bufferA,_bufferB,_bufferC,_buffer_depth};
  for (int i = 0; i < 4; ++i) _d3d11_renderer->clearRenderTarget(views[i], RBColorf::blank);
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);

  _d3d11_renderer->changeRenderTargets(views, 4, _buffer_depth_stencil);
  _d3d11_renderer->setDepthState(_depth_stencil_state, 1);
  _d3d11_renderer->setShader(_gbuffer_shader);
  _d3d11_renderer->setTexture("shaderTexture1", _texture1);
  _d3d11_renderer->setTexture("mat_tex", _mat_texture);
  _d3d11_renderer->setTexture("normal_map", _normal_texture);
  _d3d11_renderer->setSamplerState("SampleType", _sample_state);
  RBMatrix mm, mv, mp;
  mm.set_identity();
  _cam->get_view_matrix(mv);
  _cam->get_perspective_matrix(mp);
  set_matrix(mm, mv, mp);
  //set_gpass_vbuffer();
  u32 stride = sizeof(Vertex_PNTT);
  u32 offset = 0;
  _d3d11_renderer->setVertexFormat(_vf_gbuffer);
  _d3d11_renderer->setVertexBuffer(0, _vertex_buffer);
  _d3d11_renderer->setIndexBuffer(_index_buffer);
  _d3d11_renderer->apply();
  _d3d11_renderer->drawElements(PRIM_TRIANGLES, 0, _idxs.size(), 0, 0);

  _d3d11_renderer->setDepthState(_depth_stencil_state_disable);
  _d3d11_renderer->setBlendState(_blending_state);
  _d3d11_renderer->changeToMainFramebuffer();
  _d3d11_renderer->setShader(_lighting_shader);
  stride = sizeof(Vertex_PT);
  offset = 0;
  _d3d11_renderer->setVertexFormat(_vf_lighting);
  _d3d11_renderer->setVertexBuffer(0, _vertex_buffer_lighting);
  _d3d11_renderer->setIndexBuffer(_index_buffer_lighting);
  _d3d11_renderer->setSamplerState("SampleType", _sample_state);
  _d3d11_renderer->setTexture("shaderTexture1", _bufferA);
  _d3d11_renderer->setTexture("shaderTexture2", _bufferB);
  _d3d11_renderer->setTexture("shaderTexture3", _bufferC);
  _d3d11_renderer->setTexture("shaderTexture4", _buffer_depth);
  _d3d11_renderer->setTexture("env_texture", _env_texture_single);
  _d3d11_renderer->setTexture("env_texture_hi", _env_texture_hdr);
  set_screen_size();
  _d3d11_renderer->apply();
  _d3d11_renderer->drawElements(PRIM_TRIANGLES, 0, 6, 0, 4);
  
}

void DSFastApp::ter(){}
void DSFastApp::postter(){}
DSFastApp::DSFastApp(class AppBase* app) : FastApp(app){}

DSFastApp::~DSFastApp()
{
  delete _d3d11_renderer;
}

void DSFastApp::load_models_generate_buffer(const char* filename)
{


  Image img_texture1;
  img_texture1.loadImage("Res/gun/Cerberus_A.png");
  _texture1 = _d3d11_renderer->addTexture(img_texture1);

  Image img_normal;
  img_normal.loadImage("res/gun/Cerberus_N.dds");
  _normal_texture = _d3d11_renderer->addTexture(img_normal);

  Image img_mat;
  img_mat.loadImage("Res/gun/Cerberus_RMAC.png");
  _mat_texture = _d3d11_renderer->addTexture(img_mat);

  Image img_env_single;
  img_env_single.loadImage("res/env/1DiffuseHDR.dds");
  _env_texture_single = _d3d11_renderer->addTexture(img_env_single);

  Image img_env_hdr;
  img_env_hdr.loadImage("res/env/1SpecularHDR.dds");
  _env_texture_hdr = _d3d11_renderer->addTexture(img_env_hdr);


  /** load file */
  {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    std::string pth = RBPathTool::get_file_directory(filename);
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename, pth.c_str());

    if (!err.empty())
    {
      std::cerr << err << std::endl;
    }

    if (!ret)
    {
      getchar();
    }

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
          Vertex_PNTT vv;
          vv.pos = RBVector3(vx, vy, vz);
          vv.normal = RBVector3(nx, ny, nz);
          vv.texcoord = RBVector2(tx, ty);



          {
            tinyobj::index_t v0i = shapes[s].mesh.indices[index_offset];
            RBVector3 v0p = RBVector3(attrib.vertices[3 * v0i.vertex_index + 0], attrib.vertices[3 * v0i.vertex_index + 1], attrib.vertices[3 * v0i.vertex_index + 2]);
            RBVector2 v0t = RBVector2(attrib.texcoords[2 * v0i.texcoord_index], attrib.texcoords[2 * v0i.texcoord_index + 1]);
            v0i = shapes[s].mesh.indices[index_offset + 1];
            RBVector3 v1p = RBVector3(attrib.vertices[3 * v0i.vertex_index + 0], attrib.vertices[3 * v0i.vertex_index + 1], attrib.vertices[3 * v0i.vertex_index + 2]);
            RBVector2 v1t = RBVector2(attrib.texcoords[2 * v0i.texcoord_index], attrib.texcoords[2 * v0i.texcoord_index + 1]);
            v0i = shapes[s].mesh.indices[index_offset + 2];
            RBVector3 v2p = RBVector3(attrib.vertices[3 * v0i.vertex_index + 0], attrib.vertices[3 * v0i.vertex_index + 1], attrib.vertices[3 * v0i.vertex_index + 2]);
            RBVector2 v2t = RBVector2(attrib.texcoords[2 * v0i.texcoord_index], attrib.texcoords[2 * v0i.texcoord_index + 1]);


            RBVector3 dp1 = v1p - v0p;
            RBVector3 dp2 = v2p - v0p;
            RBVector2 duv1 = v1t - v0t;
            RBVector2 duv2 = v2t - v0t;

            float f = 1.0f / (duv1.x * duv2.y - duv2.x * duv1.y);

            RBVector3 tangent1;
            tangent1.x = f * (duv2.y * dp1.x - duv1.y * dp2.x);
            tangent1.y = f * (duv2.y * dp1.y - duv1.y * dp2.y);
            tangent1.z = f * (duv2.y * dp1.z - duv1.y * dp2.z);
            tangent1 = tangent1.get_normalized();

            vv.tangent = tangent1;
            //vv.tangent.out();
          }

          _vecs.push_back(vv);
          _idxs.push_back(v + index_offset);// idx.vertex_index);
        }

        index_offset += fv;

        // per-face material
        shapes[s].mesh.material_ids[f];
      }
    }
  }

  FormatDesc fd[4];
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
  fd[3].format = AttributeFormat::FORMAT_FLOAT;
  fd[3].size = 3;
  fd[3].stream = 0;
  fd[3].type = AttributeType::TYPE_TANGENT;

  _vf_gbuffer = _d3d11_renderer->addVertexFormat(fd, 4, _gbuffer_shader);
  _vertex_buffer = _d3d11_renderer->addVertexBuffer(_vecs.size()*sizeof(Vertex_PNTT), BufferAccess::STATIC, _vecs.data());
  _index_buffer = _d3d11_renderer->addIndexBuffer(_idxs.size(), sizeof(u32), BufferAccess::STATIC, _idxs.data());
}

void DSFastApp::load_assets()
{
  auto sh1 = D3D11ShaderCompiler::compile(_device, "shaders/deferred_gbuffer_v.hlsl", "VsMain", "vs_5_0");
  auto sh2 = D3D11ShaderCompiler::compile(_device, "shaders/deferred_gbuffer_g.hlsl", "GsMain", "gs_5_0");
  auto sh3 = D3D11ShaderCompiler::compile(_device, "shaders/deferred_gbuffer_p.hlsl", "PsMain", "ps_5_0");
  _gbuffer_shader = _d3d11_renderer->addShader(sh1.GetReference(), /*sh2.GetReference()*/nullptr, sh3.GetReference(),D3D11ShaderCompiler::reflect_shader);

  auto sh4 = D3D11ShaderCompiler::compile(_device, "shaders/deferred_compass_v.hlsl", "VsMain", "vs_5_0");
  auto sh5 = D3D11ShaderCompiler::compile(_device, "shaders/deferred_compass_p.hlsl", "PsMain", "ps_5_0");
  _lighting_shader = _d3d11_renderer->addShader(sh4.GetReference(), nullptr, sh5.GetReference(), D3D11ShaderCompiler::reflect_shader);

  _bufferA = _d3d11_renderer->addRenderTarget(ww, wh, 1, 1, 1, FORMAT::FORMAT_RGBA8);
  _bufferB = _d3d11_renderer->addRenderTarget(ww,wh,1,1,1,FORMAT::FORMAT_RGBA16F);
  _bufferC = _d3d11_renderer->addRenderTarget(ww, wh, 1, 1, 1, FORMAT::FORMAT_RGBA8);
  _buffer_lighting = _d3d11_renderer->addRenderTarget(ww, wh, 1, 1, 1, FORMAT::FORMAT_RGBA8);
  _buffer_depth = _d3d11_renderer->addRenderTarget(ww,wh,1,1,1,FORMAT::FORMAT_R32F);
  _buffer_depth_stencil = _d3d11_renderer->addRenderDepth(ww, wh, 1, FORMAT::FORMAT_D24S8);

  /** Create quad for composition pass */
  quad[0] = Vertex_PT{ RBVector3(-1.f, -1.f, 0.02f), RBVector2(0, 0) };
  quad[1] = Vertex_PT{ RBVector3(1.f, -1.f, 0.02f), RBVector2(1, 0) };
  quad[2] = Vertex_PT{ RBVector3(-1.f, 1.f, 0.02f), RBVector2(0, 1) };
  quad[3] = Vertex_PT{ RBVector3(1.f, 1.f, 0.02f), RBVector2(1, 1) };

  quad_idx[0] = 0;
  quad_idx[1] = 3;
  quad_idx[2] = 1;
  quad_idx[3] = 0;
  quad_idx[4] = 2;
  quad_idx[5] = 3;

  FormatDesc fd[2];
  fd[0].format = AttributeFormat::FORMAT_FLOAT;
  fd[0].size = 3;
  fd[0].stream = 0;
  fd[0].type = AttributeType::TYPE_VERTEX;
  fd[1].format = AttributeFormat::FORMAT_FLOAT;
  fd[1].size = 2;
  fd[1].stream = 0;
  fd[1].type = AttributeType::TYPE_TEXCOORD;

  _vf_lighting = _d3d11_renderer->addVertexFormat(fd, 2, _lighting_shader);
  _vertex_buffer_lighting = _d3d11_renderer->addVertexBuffer(4 * sizeof(Vertex_PT), BufferAccess::STATIC, quad);
  _index_buffer_lighting = _d3d11_renderer->addIndexBuffer(6, sizeof(u16), BufferAccess::STATIC, quad_idx);

  load_models_generate_buffer("Res/gun/pistol.obj");
  //load_models_generate_buffer("Res/sponza/pbr/sponza.obj");

}

void DSFastApp::create_rhires()
{
  _depth_stencil_state = _d3d11_renderer->addDepthState(true, true, D3D11_COMPARISON_LESS, true, 0xFF, 0xFF,
    D3D11_COMPARISON_ALWAYS, D3D11_COMPARISON_ALWAYS,
    D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP,
    D3D11_STENCIL_OP_INCR, D3D11_STENCIL_OP_DECR,
    D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP);
  _depth_stencil_state_disable = _d3d11_renderer->addDepthState(false, true, D3D11_COMPARISON_LESS, false, 0xFF, 0xFF,
    D3D11_COMPARISON_ALWAYS, D3D11_COMPARISON_ALWAYS,
    D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP,
    D3D11_STENCIL_OP_INCR, D3D11_STENCIL_OP_DECR,
    D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP);
  _blending_state = _d3d11_renderer->addBlendState(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA,
    D3D11_BLEND_ONE, D3D11_BLEND_ZERO,
    D3D11_BLEND_OP_ADD, D3D11_BLEND_OP_ADD,
    D3D11_COLOR_WRITE_ENABLE_ALL);
  _blending_state_disable = _d3d11_renderer->addBlendState(D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL);
  float a[] = { 0, 0, 0, 0 };
  _sample_state = _d3d11_renderer->addSamplerState(Filter::BILINEAR_ANISO, AddressMode::WRAP, AddressMode::WRAP, AddressMode::WRAP, 0, 16, 0, a);
  _raster_state = _d3d11_renderer->addRasterizerState(CULL_NONE, SOLID, false, false, 0, 0);
}

void DSFastApp::set_matrix(const RBMatrix& m, const RBMatrix& v, const RBMatrix& p)
{
  _d3d11_renderer->setShaderConstant4x4f("m", m);
  _d3d11_renderer->setShaderConstant4x4f("v", v);
  _d3d11_renderer->setShaderConstant4x4f("o", p);
}

void DSFastApp::set_screen_size()
{
  _d3d11_renderer->setShaderConstant1i("w",ww);
  _d3d11_renderer->setShaderConstant1i("h", wh);
  _d3d11_renderer->setShaderConstant1f("hipow", _high_light);
  _d3d11_renderer->setShaderConstant1f("factor_l", _factor_l);
  RBMatrix m;
  _cam->get_perspective_matrix(m);
  m = m.get_inverse_slow();
  _d3d11_renderer->setShaderConstant4x4f("inv_projection", m);
  _cam->get_view_matrix(m);
  _d3d11_renderer->setShaderConstant4x4f("mv_mat", m);
  _d3d11_renderer->setShaderConstant1f("metallic", _metallic);
  _d3d11_renderer->setShaderConstant1i("shading_model", _shading_model);
}

void DSFastApp::set_gpass_vbuffer()
{
  _d3d11_renderer->setShaderConstant1f("zplus", _zplus);
}

void DSFastApp::handle_input(f32 dt)
{
  if (Input::get_key_pressed(WIP_Q))
  {
    _high_light += 0.01f;
    _high_light = RBMath::clamp(_high_light, 0.f, 1.f);
    printf("%f\n", _high_light);

  }
  if (Input::get_key_pressed(WIP_E))
  {
    _high_light -= 0.01f;
    _high_light = RBMath::clamp(_high_light, 0.f, 1.f);
    printf("%f\n", _high_light);

  }
  if (Input::get_key_pressed(WIP_Z))
    _factor_l += 0.01f;
  if (Input::get_key_pressed(WIP_X))
  {
    _factor_l -= 0.01;
    _factor_l = RBMath::clamp(_factor_l, 0.01f, 99999.f);
  }
  if (Input::get_key_pressed(WIP_M))
  {
    _metallic += 0.01f;
    _metallic = RBMath::clamp(_metallic, 0.f, 1.f);
    printf("m:%f\n", _metallic);

  }
  if (Input::get_key_pressed(WIP_N))
  {
    _metallic -= 0.01f;
    _metallic = RBMath::clamp(_metallic, 0.f, 1.f);
    printf("m:%f\n", _metallic);

  }
  if (Input::get_key_pressed(WIP_H))
  {
    _shading_model = 1;
    printf("blinn phong");
  }
  if (Input::get_key_pressed(WIP_J))
  {
    _shading_model = 2;
    printf("tranditional phong");

  }
  if (Input::get_key_pressed(WIP_K))
  {
    _shading_model = 3;
    printf("modify tranditional phong");

  }
  if (Input::get_key_pressed(WIP_L))
  {
    _shading_model = 4;
    printf("micro phong");

  }
  if (Input::get_key_pressed(WIP_G))
  {
    _shading_model = 0;
    printf("micofact");

  }
  if (Input::get_key_pressed(WIP_V))
    _zplus += 0.1;
  _cam->control_update(dt);
}