#include "CRFWRAP.h"
#include "ResourceManager.h"
#include "Camera.h"
#include "tiny_obj_loader.h"
#include <iostream>
#include "D3D11ShaderResources.h"
#include "Color32.h"

bool ConservativeRasterizationFastAppWrapped::preinit()
{
  g_res_manager->startup();
  dx = 0;
  return true;
}

bool ConservativeRasterizationFastAppWrapped::init()
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

  _cam_move_speed = RBVector2(50, 50);
  _cam_rotate_speed = RBVector2(5, 5);

  load_assets();

  return true;
}

void ConservativeRasterizationFastAppWrapped::load_assets()
{
  //load_models_generate_buffer("res/mz1.obj");
  //auto sh1 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/conservation_rasterization_v.hlsl", "VsMain", "vs_5_0");
  //auto sh2 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/conservation_rasterization_g.hlsl", "GsMain", "gs_5_0");
  auto sh3 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/deferred_compass_p.hlsl", "PsMain", "ps_5_0");

  //_vs = _rhi->RHICreateVertexShader(sh1->GetBufferPointer(), sh1->GetBufferSize());
  //_gs = _rhi->RHICreateGeometryShader(sh2->GetBufferPointer(), sh2->GetBufferSize());
  _ps = _rhi->RHICreatePixelShader(sh3->GetBufferPointer(), sh3->GetBufferSize());

  

}

void ConservativeRasterizationFastAppWrapped::load_models_generate_buffer(const char* filename)
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

  RBVertexDeclarationElementList vdlist;
  RBVertexElement eposition;
  eposition.StreamIndex = 0;
  eposition.Offset = 0;//12 24
  eposition.Type = VET_Float3;//3 2
  eposition.AttributeIndex = 0;
  eposition.bUseInstanceIndex = false;
  eposition.Stride = 12;
  RBVertexElement enormal;
  enormal.StreamIndex = 0;
  enormal.Offset = 12;
  enormal.Type = VET_Float3;
  enormal.AttributeIndex = 0;
  enormal.bUseInstanceIndex = false;
  enormal.Stride = 12;
  RBVertexElement etex;
  etex.StreamIndex = 0;
  etex.Offset = 24;
  etex.Type = VET_Float2;
  etex.AttributeIndex = 0;
  etex.bUseInstanceIndex = false;
  etex.Stride = 8;
  
  vdlist.push_back(eposition);
  vdlist.push_back(enormal);
  vdlist.push_back(etex);

  RBVertexDeclarationRHIRef vd = _rhi->RHICreateVertexDeclaration(vdlist);

  {
    RBRHIResourceCreateInfo info1_vertex;
    TResourceArray<Vertex_PNT> ra(false);
    ra.resize(_vecs.size()*sizeof(Vertex_PNT));
    memcpy(&ra[0], &_vecs[0], _vecs.size()*sizeof(Vertex_PNT));
    info1_vertex.ResourceArray = &ra;
    _vb = _rhi->RHICreateVertexBuffer(_vecs.size()*sizeof(Vertex_PNT), BUF_Static, info1_vertex);
  }
  {
    RBRHIResourceCreateInfo info1_index;
    TResourceArray<u32> ra(false);
    ra.resize(_idxs.size()*sizeof(u32));
    memcpy(&ra[0], &_idxs[0], _idxs.size()*sizeof(u32));
    info1_index.ResourceArray = &ra;
    _ib = _rhi->RHICreateIndexBuffer(sizeof(u32) * 3, _idxs.size()*sizeof(u32), BUF_Static, info1_index);
  }

}

void ConservativeRasterizationFastAppWrapped::create_rhires()
{
  {
  RBDepthStencilStateInitializerRHI initer(true, CF_Less);
  _ds_enable = _rhi->RHICreateDepthStencilState(initer);
  RBDepthStencilStateInitializerRHI initer1(false,CF_Less);
  _ds_disable = _rhi->RHICreateDepthStencilState(initer);
  }
  {
    RBBlendStateInitializerRHI::FRenderTarget rt;
    rt.AlphaBlendOp = BO_Add;
    rt.AlphaDestBlend = BF_Zero;
    rt.AlphaSrcBlend = BF_One;
    rt.ColorBlendOp = BO_Add;
    rt.ColorDestBlend = BF_InverseSourceAlpha;
    rt.ColorSrcBlend = BF_SourceAlpha;
    rt.ColorWriteMask = CW_RGBA;
    RBBlendStateInitializerRHI initer(rt);
    initer.bUseIndependentRenderTargetBlendStates = false;
    _bs_enable = _rhi->RHICreateBlendState(initer);

    rt.AlphaBlendOp = BO_Add;
    rt.AlphaDestBlend = BF_Zero;
    rt.AlphaSrcBlend = BF_One;
    rt.ColorBlendOp = BO_Add;
    rt.ColorDestBlend = BF_InverseSourceAlpha;
    rt.ColorSrcBlend = BF_SourceAlpha;
    rt.ColorWriteMask = CW_RGBA;
    RBBlendStateInitializerRHI initer1(rt);
    initer1.bUseIndependentRenderTargetBlendStates = false;
    _bs_disable = _rhi->RHICreateBlendState(initer1);
  }
  {
    RBSamplerStateInitializerRHI initer;
    initer.AddressU = AM_Wrap;
    initer.AddressV = AM_Wrap;
    initer.AddressW = AM_Wrap;
    initer.BorderColor = (RBColor32(0,0,0,0)).dw_color();
    initer.Filter = SF_AnisotropicLinear;
    initer.MaxAnisotropy = 16;
    initer.MaxMipLevel = MAX_F32;
    initer.MinMipLevel = 0;
    initer.MipBias = 0;
    initer.SamplerComparisonFunction = SCF_Less;
    _ss = _rhi->RHICreateSamplerState(initer);
  }
  {
    RBRasterizerStateInitializerRHI initer;
    initer.bAllowMSAA = false;
    initer.bEnableLineAA = false;
    initer.CullMode = CM_None;
    initer.DepthBias = 0;
    initer.FillMode = FM_Solid;
    initer.SlopeScaleDepthBias = 0;
    _rs = _rhi->RHICreateRasterizerState(initer);
  }
  {
    RBRHIResourceCreateInfo info;
    info.BulkData = nullptr;
    info.ClearValueBinding = RBClearValueBinding::None;
    info.ResourceArray = nullptr;
    _rhi->RHICreateTexture2D(ww, wh, PF_DepthStencil, 1, 1, TexCreate_DepthStencilTargetable, info);
  }
  {
    
    RBRHIUniformBufferLayout _layout("_uniform_screen_size");
    //资源大小
    _layout.ConstantBufferSize = PAD16(sizeof(ScreenSize));
    //Contents中的起始偏移量
    _layout.ResourceOffset = 1;
    //每个资源的类型
    _layout.Resources.push_back(1);
    RBRHIUniformBufferLayout ubl(RBRHIUniformBufferLayout::EInit::Zero);
    ScreenSize* contents = (ScreenSize*)rb_align_malloc(_layout.ConstantBufferSize, 16);
    _uniform_screen_size = _rhi->RHICreateUniformBuffer(contents, ubl, EUniformBufferUsage::UniformBuffer_SingleFrame);
  }


}

ConservativeRasterizationFastAppWrapped::ConservativeRasterizationFastAppWrapped(AppBase* app)
  :FastApp(app)
{

}

void ConservativeRasterizationFastAppWrapped::update(f32 dt)
{

}

void ConservativeRasterizationFastAppWrapped::draw()
{

}

void ConservativeRasterizationFastAppWrapped::ter()
{

}

void ConservativeRasterizationFastAppWrapped::postter()
{

}

void ConservativeRasterizationFastAppWrapped::handle_input(f32 dt)
{

}

void ConservativeRasterizationFastAppWrapped::show_error_message(ID3D10Blob* error, const char* filename)
{

}

void ConservativeRasterizationFastAppWrapped::set_screen_size()
{

}

void ConservativeRasterizationFastAppWrapped::set_matrix(const RBMatrix& m, const RBMatrix& v, const RBMatrix& p)
{

}

ConservativeRasterizationFastAppWrapped::~ConservativeRasterizationFastAppWrapped()
{

}
