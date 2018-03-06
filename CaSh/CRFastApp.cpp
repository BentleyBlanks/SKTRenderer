#include "CRFastApp.h"
#include "ResourceManager.h"
#include "Camera.h"
#include "tiny_obj_loader.h"
#include <iostream>
#include "D3D11ShaderResources.h"
#include "Color32.h"
#include "Input.h"

bool CRFastApp::preinit()
{
	g_res_manager->startup();
	dx = 0;
	return true;
}

void CRFastApp::draw()
{
  _d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);

  RBMatrix mm;
  RBMatrix mp;
  RBMatrix mv;

  mm.set_identity();
  _cam->get_view_matrix(mv);
  _cam->get_perspective_matrix(mp);

  _d3d11_renderer->setShader(_bound_shader);
  set_matrix(mm, mv, mp);
  set_screen_size();

  _d3d11_renderer->setVertexFormat(_vf);
  _d3d11_renderer->setVertexBuffer(0, _vb);
  _d3d11_renderer->setIndexBuffer(_ib);

  _d3d11_renderer->apply();

  _d3d11_renderer->drawElements(Primitives::PRIM_TRIANGLES, 0, _idxs.size(), 0, 0);

}

bool CRFastApp::init()
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

  _cam->set_move_speed(RBVector2(50,50));
  _cam->set_rotate_speed(RBVector2(5,5));

	_d3d11_renderer = new Direct3D11Renderer(_rhi->GetDevice(),_rhi->GetDeviceContext());

	load_assets();
  create_rhires();

  _d3d11_renderer->setBlendState(_bs_enable, 0xffffffff);
  _d3d11_renderer->setDepthState(_ds_enable);
  _d3d11_renderer->setRasterizerState(_rs);

  _d3d11_renderer->setViewport(ww, wh);
  _d3d11_renderer->setFrameBuffer(back_buffer_view, _d3d11_renderer->getDSV(_buffer_depth_stencil));
  _d3d11_renderer->changeToMainFramebuffer();
  
  return true;
}

void CRFastApp::load_assets()
{
#ifdef COMPLEBYHANDE
  auto sh1 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/conservation_rasterization_v.hlsl", "main", "vs_5_0");
  auto sh2 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/conservation_rasterization_g.hlsl", "main", "gs_5_0");
  auto sh3 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/conservation_rasterization_p.hlsl", "main", "ps_5_0");
  _bound_shader = _d3d11_renderer->addShader(sh1.GetReference(), sh2.GetReference(), sh3.GetReference(),D3D11ShaderCompiler::reflect_shader);
#else
  auto res_handle = g_res_manager->load_resource("shaders/conservation_rasterization_v.hlsl", WIPResourceType::TEXT);
  auto res_handle1 = g_res_manager->load_resource("shaders/conservation_rasterization_g.hlsl", WIPResourceType::TEXT);
  auto res_handle2 = g_res_manager->load_resource("shaders/conservation_rasterization_p.hlsl", WIPResourceType::TEXT);
  _bound_shader =  _d3d11_renderer->addShader(((std::string*)res_handle->ptr)->c_str(),
      ((std::string*)res_handle1->ptr)->c_str(), ((std::string*)res_handle2->ptr)->c_str(), 0, 0, 0);
  printf("%d", _bound_shader);
#endif

  load_models_generate_buffer("res/pot.obj");
}

void CRFastApp::load_models_generate_buffer(const char* filename)
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
			g_logger->debug_print("¶ÁÈ¡ÎÄ¼þ%sÊ§°Ü\n", filename);
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

  /*
  Vertex_PNT quad[4];

  quad[0] = Vertex_PNT{ RBVector3(-1.f, -1.f, 0.02f),RBVector3(0,0,-1), RBVector2(0, 0) };
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

  _vb = _d3d11_renderer->addVertexBuffer(4*sizeof(Vertex_PNT), BufferAccess::STATIC, quad);
  _ib = _d3d11_renderer->addIndexBuffer(6, sizeof(u32), BufferAccess::STATIC, quad_idx);
  */
  
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
  
  _vf = _d3d11_renderer->addVertexFormat(fd, 3 , _bound_shader);
  _vb = _d3d11_renderer->addVertexBuffer(_vecs.size()*sizeof(Vertex_PNT), BufferAccess::STATIC,_vecs.data());
  _ib = _d3d11_renderer->addIndexBuffer(_idxs.size(), sizeof(u32), BufferAccess::STATIC, _idxs.data());
}

void CRFastApp::create_rhires()
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
  _bs_disable = _d3d11_renderer->addBlendState(D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL);
  float a[] = {0,0,0,0};
  _ss = _d3d11_renderer->addSamplerState(Filter::BILINEAR_ANISO, AddressMode::WRAP, AddressMode::WRAP, AddressMode::WRAP, 0, 16, D3D11_COMPARISON_ALWAYS, a);
  _rs = _d3d11_renderer->addRasterizerState(CULL_NONE,SOLID,false,false,0,0);
  _buffer_depth_stencil = _d3d11_renderer->addRenderDepth(ww, wh, 1, FORMAT::FORMAT_D24S8);
}

void CRFastApp::update(f32 dt)
{
  handle_input(dt);
}

void CRFastApp::ter()
{
  _d3d11_renderer->finish();
	delete _d3d11_renderer;
}

void CRFastApp::handle_input(f32 dt)
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

void CRFastApp::set_screen_size()
{
  _d3d11_renderer->setShaderConstant1i("screen_size_x",ww);
  _d3d11_renderer->setShaderConstant1i("screen_size_y",wh);
  _d3d11_renderer->setShaderConstant1i("s",dx);

}

void CRFastApp::set_matrix(const RBMatrix& m, const RBMatrix& v, const RBMatrix& p)
{
  _d3d11_renderer->setShaderConstant4x4f("m", m);
  _d3d11_renderer->setShaderConstant4x4f("v", v);
  _d3d11_renderer->setShaderConstant4x4f("o", p);
}

CRFastApp::CRFastApp(AppBase* app) :FastApp(app){}

void CRFastApp::postter(){}

CRFastApp::~CRFastApp(){}