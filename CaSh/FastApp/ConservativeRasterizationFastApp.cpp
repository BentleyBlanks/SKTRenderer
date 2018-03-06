#include "ConservativeRasterizationFastApp.h"
#include "d3d11.h"
#include "RHID3D11.h"
#include "ResourceManager.h"
#include <iostream>
#include <fstream>
#include "tiny_obj_loader.h"
#include "Camera.h"
#include "Input.h"
#include "InputManager.h"
#include "D3D11InputElementDesc.h"
#include "Matrix.h"
#include "App.h"
#include "Logger.h"

bool ConservativeRasterizationFastApp::preinit()
{
  compile_shader = nullptr;
  CompilerDLL = 0;
  g_res_manager->startup();
  dx = 0;
  return true;
}

bool ConservativeRasterizationFastApp::init()
{
  FD3D11DynamicRHI* _rhid3d11 = static_cast<FD3D11DynamicRHI*>(base_app->get_rhi());
  _device = _rhid3d11->GetDevice();
  _context = _rhid3d11->GetDeviceContext();
  _gi_factory = _rhid3d11->GetFactory();
  _rhi = _rhid3d11;

  CompilerDLL = LoadLibrary("d3dcompiler_47.dll");
  compile_shader = (pD3DCompile)(void*)GetProcAddress(CompilerDLL, "D3DCompile");

  _cam = new RBCamera();
  if (!_cam) return false;
  _cam->set_position(0, 0, 0);
  RBVector4 position(0, 0, 1);
  _cam->set_target(position);
  _cam->set_fov_y(60);
  _cam->set_ratio(16.f / 9.f);
  _cam->set_near_panel(0.01f);
  _cam->set_far_panel(2000.f);

  _cam_move_speed = RBVector2(50, 50);
  _cam_rotate_speed = RBVector2(5, 5);



  load_assets();
  create_rhires();


  D3D11_VIEWPORT screen_viewport;
  screen_viewport.TopLeftX = 0;
  screen_viewport.TopLeftY = 0;
  screen_viewport.Width = ww;
  screen_viewport.Height = wh;
  screen_viewport.MinDepth = 0.0f;
  screen_viewport.MaxDepth = 1.0f;

  _context->RSSetViewports(1, &screen_viewport);

  //体素化时禁用深度测试和背面剔除
  //_context->OMSetDepthStencilState(_depth_stencil_state_disable, 1);
  _context->OMSetDepthStencilState(_depth_stencil_state, 1);

  float factor[4] = { 1.f, 1.f, 1.f, 1.f };
  _context->OMSetBlendState(_blending_state, factor, 0xffffffff);


  _context->RSSetState(_raster_state);
  _context->OMSetRenderTargets(1, &back_buffer_view, _buffer_depth_stencil_ds_view);

  return true;
}

void ConservativeRasterizationFastApp::update(f32 dt)
{
  _cam->update(dt);
  handle_input(dt);
}

void ConservativeRasterizationFastApp::draw()
{
  _context->ClearDepthStencilView(_buffer_depth_stencil_ds_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
  _context->ClearRenderTargetView(back_buffer_view, reinterpret_cast<const float*>(&RBColorf::blank));



  RBMatrix mm;
  RBMatrix mp;
  RBMatrix mv;

  mm.set_identity();
  _cam->get_view_matrix(mv);
  _cam->get_perspective_matrix(mp);
  set_matrix(mm, mv, mp);
  set_screen_size();
  
  _context->VSSetShader(_vs, 0, 0);
  _context->GSSetShader(_gs, 0, 0);
  _context->PSSetShader(_ps, 0, 0);
  _context->IASetInputLayout(_layout);
  u32 stride = sizeof(Vertex_PNT);
  u32 offset = 0;
  _context->IASetVertexBuffers(0, 1, &_vertex_buffer, &stride, &offset);
  _context->IASetIndexBuffer(_index_buffer, DXGI_FORMAT_R32_UINT, 0);
  _context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  _context->DrawIndexed(6/*_idxs.size()*/, 0, 0);



}

void ConservativeRasterizationFastApp::ter()
{

}

void ConservativeRasterizationFastApp::postter()
{
  g_res_manager->shutdown();
}

ConservativeRasterizationFastApp::ConservativeRasterizationFastApp(AppBase* app) :FastApp(app)
{
  
}

ConservativeRasterizationFastApp::~ConservativeRasterizationFastApp()
{

}

void ConservativeRasterizationFastApp::load_models_generate_buffer(const char* filename)
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
      g_logger->debug_print("读取文件%s失败\n",filename);
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
          Vertex_PNT vv;
          vv.pos = RBVector3(vx, vy, vz);
          vv.normal = RBVector3(nx, ny, nz);
          vv.texcoord = RBVector2(tx, ty);
          _vecs.push_back(vv);
          _idxs.push_back(v + index_offset);// idx.vertex_index);

        }
        index_offset += fv;

        // per-face material
        shapes[s].mesh.material_ids[f];
      }
    }
  }


  Vertex_PNT quad[4];

  quad[0] = Vertex_PNT{ RBVector3(-1.f, -1.f, 0.02f), RBVector3(0, 0, -1), RBVector2(0, 0) };
  quad[1] = Vertex_PNT{ RBVector3(1.f, -1.f, 0.02f), RBVector3(0, 0, -1), RBVector2(1, 0) };
  quad[2] = Vertex_PNT{ RBVector3(-1.f, 1.f, 0.02f), RBVector3(0, 0, -1), RBVector2(0, 1) };
  quad[3] = Vertex_PNT{ RBVector3(1.f, 1.f, 0.02f), RBVector3(0, 0, -1), RBVector2(1, 1) };

  uint32 quad_idx[6];

  quad_idx[0] = 0;
  quad_idx[1] = 3;
  quad_idx[2] = 1;
  quad_idx[3] = 0;
  quad_idx[4] = 2;
  quad_idx[5] = 3;



  {
    /** create vertex buffer with vertex data */
    D3D11_BUFFER_DESC des_def;
    ZeroMemory(&des_def, sizeof(des_def));
    des_def.Usage = D3D11_USAGE_IMMUTABLE;
    des_def.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    des_def.ByteWidth = (4 * sizeof(Vertex_PNT));
    des_def.MiscFlags = 0;
    des_def.StructureByteStride = 0;
    des_def.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA init_dat;
    init_dat.pSysMem = quad;
    init_dat.SysMemPitch = (4 * sizeof(Vertex_PNT));
    init_dat.SysMemSlicePitch = 0;

    HR(_device->CreateBuffer(&des_def, &init_dat, &_vertex_buffer));
  }

  {
    /** create index buffer with index data */
    D3D11_BUFFER_DESC des_def;
    ZeroMemory(&des_def, sizeof(des_def));
    des_def.Usage = D3D11_USAGE_IMMUTABLE;
    des_def.BindFlags = D3D11_BIND_INDEX_BUFFER;
    des_def.ByteWidth = (6*sizeof(u32));
    des_def.MiscFlags = 0;
    des_def.StructureByteStride = 0;
    des_def.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA init_dat;
    init_dat.pSysMem = quad_idx;
    init_dat.SysMemPitch = (6*sizeof(u32));
    init_dat.SysMemSlicePitch = 0;

    HR(_device->CreateBuffer(&des_def, &init_dat, &_index_buffer));
  }

#if 0
  {
    /** create vertex buffer with vertex data */
    D3D11_BUFFER_DESC des_def;
    ZeroMemory(&des_def, sizeof(des_def));
    des_def.Usage = D3D11_USAGE_IMMUTABLE;
    des_def.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    des_def.ByteWidth = (_vecs.size()*sizeof(Vertex_PNT))+1;
    des_def.MiscFlags = 0;
    des_def.StructureByteStride = 0;
    des_def.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA init_dat;
    init_dat.pSysMem = _vecs.data();
    init_dat.SysMemPitch = (_vecs.size()*sizeof(Vertex_PNT)) + 1;
    init_dat.SysMemSlicePitch = 0;

    HR(_device->CreateBuffer(&des_def, &init_dat, &_vertex_buffer));
  }

  {
    /** create index buffer with index data */
    D3D11_BUFFER_DESC des_def;
    ZeroMemory(&des_def, sizeof(des_def));
    des_def.Usage = D3D11_USAGE_IMMUTABLE;
    des_def.BindFlags = D3D11_BIND_INDEX_BUFFER;
    des_def.ByteWidth = (_vecs.size()*sizeof(u32));
    des_def.MiscFlags = 0;
    des_def.StructureByteStride = 0;
    des_def.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA init_dat;
    init_dat.pSysMem = _idxs.data();
    init_dat.SysMemPitch = (_vecs.size()*sizeof(u32));
    init_dat.SysMemSlicePitch = 0;

    HR(_device->CreateBuffer(&des_def, &init_dat, &_index_buffer));
  }
#endif
}

void ConservativeRasterizationFastApp::load_assets()
{
  load_models_generate_buffer("res/p.obj");
  /** load shaders */
  {
    uint32 CompileFlags = D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY
      | D3D10_SHADER_PACK_MATRIX_ROW_MAJOR;
    if (1)//是否开启Debug
      CompileFlags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
    else
      if (1)//是否标准优化
        CompileFlags |= D3D10_SHADER_OPTIMIZATION_LEVEL1;
      else
        CompileFlags |= D3D10_SHADER_OPTIMIZATION_LEVEL3;
    if (0)//CFLAG_PreferFlowControl or CFLAG_AvoidFlowControl
      if (1)
        CompileFlags |= D3D10_SHADER_PREFER_FLOW_CONTROL;
      else
        CompileFlags |= D3D10_SHADER_AVOID_FLOW_CONTROL;

    //CompileFlags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY  ;

    /** vertex shader */
    {
      auto res_handle = g_res_manager->load_resource("shaders/conservation_rasterization_v.hlsl", WIPResourceType::TEXT);
      TRefCountPtr<ID3DBlob> Shader;
      TRefCountPtr<ID3DBlob> Errors;

      HRESULT Result;

      if (compile_shader)
      {
        bool bException = false;

        Result = compile_shader(
          ((std::string*)res_handle->ptr)->data(),
          ((std::string*)res_handle->ptr)->length(),
          "shaders/conservation_rasterization_v.hlsl",
          /*pDefines=*/ NULL,
          /*pInclude=*/ NULL,
          "main",
          "vs_5_0",
          CompileFlags,
          0,
          Shader.GetInitReference(),
          Errors.GetInitReference()
          );
        if (FAILED(Result))
        {
          if (Errors)
          {
            show_error_message(Errors, "shaders/conservation_rasterization_v.hlsl");
          }
          assert(0);
        }
        else
        {
          _device->CreateVertexShader(Shader->GetBufferPointer(), Shader->GetBufferSize(), NULL, &_vs);
          /** constant buffer */
          {
            D3D11_BUFFER_DESC mbd;

            mbd.Usage = D3D11_USAGE_DYNAMIC;
            mbd.ByteWidth = sizeof(RBMatrix) * 3;
            mbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            mbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            mbd.MiscFlags = 0;
            mbd.StructureByteStride = 0;

            HR(_device->CreateBuffer(&mbd, 0, &_matrix_buffer));

          }
          _device->CreateInputLayout(RBD3D11InputElementDesc::vertexDescPNT, 3, Shader->GetBufferPointer(), Shader->GetBufferSize(), &_layout);

        }
      }
      else
      {
        printf("compile_shader failed!\n");
      }
      g_res_manager->free(res_handle, res_handle->size);
    }
    /** Geometry Shader */
    {
      auto res_handle1 = g_res_manager->load_resource("shaders/conservation_rasterization_g.hlsl", WIPResourceType::TEXT);
      TRefCountPtr<ID3DBlob> Shader;
      TRefCountPtr<ID3DBlob> Errors;

      HRESULT Result;

      if (compile_shader)
      {
        bool bException = false;

        Result = compile_shader(
          ((std::string*)res_handle1->ptr)->data(),
          ((std::string*)res_handle1->ptr)->length(),
          "shaders/conservation_rasterization_g.hlsl",
          /*pDefines=*/ NULL,
          /*pInclude=*/ NULL,
          "main",
          "gs_5_0",
          CompileFlags,
          0,
          Shader.GetInitReference(),
          Errors.GetInitReference()
          );
        if (FAILED(Result))
        {
          if (Errors)
          {
            show_error_message(Errors, "shaders/conservation_rasterization_g.hlsl");
          }
          assert(0);
        }
        else
        {
          _device->CreateGeometryShader(Shader->GetBufferPointer(), Shader->GetBufferSize(), NULL, &_gs);
        }
        g_res_manager->free(res_handle1, res_handle1->size);
      }
      else
      {
        printf("compile_shader failed!\n");
      }
    }

    /** Pixel Shader */
    {
      auto res_handle1 = g_res_manager->load_resource("shaders/conservation_rasterization_p.hlsl", WIPResourceType::TEXT);
      TRefCountPtr<ID3DBlob> Shader;
      TRefCountPtr<ID3DBlob> Errors;

      HRESULT Result;

      if (compile_shader)
      {
        bool bException = false;

        Result = compile_shader(
          ((std::string*)res_handle1->ptr)->data(),
          ((std::string*)res_handle1->ptr)->length(),
          "shaders/conservation_rasterization_p.hlsl",
          /*pDefines=*/ NULL,
          /*pInclude=*/ NULL,
          "main",
          "ps_5_0",
          CompileFlags,
          0,
          Shader.GetInitReference(),
          Errors.GetInitReference()
          );
        if (FAILED(Result))
        {
          if (Errors)
          {
            show_error_message(Errors, "shaders/conservation_rasterization_p.hlsl");
          }
          assert(0);
        }
        else
        {
          _device->CreatePixelShader(Shader->GetBufferPointer(), Shader->GetBufferSize(), NULL, &_ps);
        }
        //_device->CreateInputLayout(RBD3D11InputElementDesc::vertexDescPT, 2, Shader->GetBufferPointer(), Shader->GetBufferSize(), &_layout_lighting);
        g_res_manager->free(res_handle1, res_handle1->size);
      }
      else
      {
        printf("compile_shader failed!\n");
      }
    }
  }
}

void ConservativeRasterizationFastApp::create_rhires()
{
  {
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    // Set up the description of the stencil state.
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

    depthStencilDesc.StencilEnable = true;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0xFF;

    // Stencil operations if pixel is front-facing.
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing.
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    _device->CreateDepthStencilState(&depthStencilDesc, &_depth_stencil_state);
  }
  {
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    // Set up the description of the stencil state.
    depthStencilDesc.DepthEnable = false;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

    depthStencilDesc.StencilEnable = false;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0xFF;

    // Stencil operations if pixel is front-facing.
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing.
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    _device->CreateDepthStencilState(&depthStencilDesc, &_depth_stencil_state_disable);
  }
  {
    D3D11_BLEND_DESC blendStateDescription;
    ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));
    // Create an alpha enabled blend state description.
    blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
    //blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendStateDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    _device->CreateBlendState(&blendStateDescription, &_blending_state);
  }
  {
    D3D11_BLEND_DESC blendStateDescription;
    ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));
    // Create an alpha enabled blend state description.
    blendStateDescription.RenderTarget[0].BlendEnable = FALSE;
    //blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendStateDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    _device->CreateBlendState(&blendStateDescription, &_blending_state_disable);
  }

  D3D11_SAMPLER_DESC spdes;
  spdes.Filter = D3D11_FILTER_ANISOTROPIC;// D3D11_FILTER_ANISOTROPIC;// D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  spdes.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  spdes.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  spdes.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  spdes.MipLODBias = 0.0f;
  spdes.MaxAnisotropy = 16;
  spdes.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
  spdes.BorderColor[0] = 0;
  spdes.BorderColor[1] = 0;
  spdes.BorderColor[2] = 0;
  spdes.BorderColor[3] = 0;
  spdes.MinLOD = 0;
  spdes.MaxLOD = D3D11_FLOAT32_MAX;

  HR(_device->CreateSamplerState(&spdes, &_sample_state));

  {
    D3D11_RASTERIZER_DESC resd;
    ZeroMemory(&resd, sizeof(D3D11_RASTERIZER_DESC));
    resd.FillMode = D3D11_FILL_SOLID;
	resd.CullMode = D3D11_CULL_NONE;
  resd.FrontCounterClockwise = FALSE;
    resd.DepthClipEnable = TRUE;

    HR(_device->CreateRasterizerState(&resd, &_raster_state));
  }
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = ww;
		desc.Height = wh;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		HR(_device->CreateTexture2D(&desc, NULL, &_buffer_depth_stencil));
		D3D11_SHADER_RESOURCE_VIEW_DESC srdes;
		memset(&srdes, 0, sizeof(srdes));
		srdes.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		srdes.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srdes.Texture2D.MostDetailedMip = 0;
		srdes.Texture2D.MipLevels = 1;
		//HR(_device->CreateShaderResourceView(_buffer_depth_stencil, &srdes, &_buffer_depth_stencil_shader_view));
		HR(_device->CreateDepthStencilView(_buffer_depth_stencil, 0, &_buffer_depth_stencil_ds_view));
	}
	{
		D3D11_BUFFER_DESC mbd;

		mbd.Usage = D3D11_USAGE_DYNAMIC;
		mbd.ByteWidth = PAD16(sizeof(ScreenSize));
		mbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		mbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		mbd.MiscFlags = 0;
		mbd.StructureByteStride = 0;

		HR(_device->CreateBuffer(&mbd, 0, &_screen_size_buffer));
	}
}

void ConservativeRasterizationFastApp::set_matrix(const RBMatrix& m, const RBMatrix& v, const RBMatrix& p)
{
  HRESULT hr;
  D3D11_MAPPED_SUBRESOURCE mpr;
  
  hr = _context->Map(_matrix_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mpr);
  if (FAILED(hr))
  {
    ::MessageBox(0, "Map mat buffer failed!", "ERROR", MB_OK);
  }
  MatrixBuffer* mb;
  mb = (MatrixBuffer*)mpr.pData;
  mb->m = m;
  mb->v = v;
  mb->p = p;
  _context->Unmap(_matrix_buffer, 0);
  
  _context->VSSetConstantBuffers(1, 1, &_matrix_buffer);
  /*
  MatrixBuffer mb;
  mb.m = m;
  mb.v = v;
  mb.p = p;
  */
  //_context->UpdateSubresource(_matrix_buffer,0,0,&mb,0,0);
}

void ConservativeRasterizationFastApp::handle_input(f32 dt)
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
	
  auto cam = _cam;
  RBVector3 v = cam->get_forward();
  v.normalize();
  v *= dt * _cam_move_speed.y;
  RBVector3 vr = cam->get_right();
  vr.normalize();
  vr *= dt * _cam_move_speed.x;
  if (Input::get_key_pressed(WIP_W))
  {
    cam->translate(v.x, v.y, v.z);
  }
  if (Input::get_key_pressed(WIP_S))
  {
    cam->translate(-v.x, -v.y, -v.z);
  }
  if (Input::get_key_pressed(WIP_A))
  {
    cam->translate(-vr.x, -vr.y, -vr.z);
  }
  if (Input::get_key_pressed(WIP_D))
  {
    cam->translate(vr.x, vr.y, vr.z);
  }

  int x = Input::get_mouse_x();
  int y = Input::get_mouse_y();
  int dx = 0, dy = 0;
  RBVector2 mouse_m;
  if (Input::get_sys_key_pressed(WIP_MOUSE_RBUTTON))
  {
    if (Input::is_move())
    {
      dx = x - old_x;
      dy = y - old_y;
    }
  }
  mouse_m.set(dx, dy);
  old_x = x;
  old_y = y;

  //cam->rotate(-mouse_m.y*dt*_cam_rotate_speed.y, -mouse_m.x*dt*_cam_rotate_speed.x,0);
  float val = mouse_m.y*_cam_rotate_speed.y / (dt*1000.f);
  RBVector3 down = RBVector3(0, -1, 0);
  cam->rotate_by_axis(val, cam->get_right());
  RBVector3 f = cam->get_forward();
  if (down.is_parallel(f.get_normalized(), down))
  {
    cam->rotate_by_axis(-val, cam->get_right());
  }
  cam->rotate_by_axis(mouse_m.x*_cam_rotate_speed.x / (dt*1000.f), cam->get_up());
}

void ConservativeRasterizationFastApp::show_error_message(ID3D10Blob* error, const char* filename)
{
  char* e = (char*)(error->GetBufferPointer());
  unsigned long bs = error->GetBufferSize();
  std::ofstream fout;

  fout.open("error.txt");


  fout << filename << " compile error : " << std::endl;
  for (unsigned long i = 0; i < bs; ++i)
  {
    fout << e[i];
  }

  fout << std::endl << "==================" << std::endl;

  error->Release();
  error = 0;

}

void ConservativeRasterizationFastApp::set_screen_size()
{
  HRESULT hr;
  D3D11_MAPPED_SUBRESOURCE mpr;

  hr = _context->Map(_screen_size_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mpr);
  if (FAILED(hr))
  {
    ::MessageBox(0, "Map screen size buffer failed!", "ERROR", MB_OK);
  }

  ScreenSize* mb;
  mb = (ScreenSize*)mpr.pData;
  mb->w = ww;
  mb->h = wh;
  mb->s = dx;
  mb->pad2 = 0;
  RBMatrix m;
  _cam->get_perspective_matrix(m);
  m = m.get_inverse_slow();
  mb->inv_projection = m;
  _context->Unmap(_screen_size_buffer, 0);
  _context->GSSetConstantBuffers(1, 1, &_screen_size_buffer);
}


