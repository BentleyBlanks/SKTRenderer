#include "HairShadingFastApp.h"
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
#include "DDSTextureLoader.h"

bool HairShadingFastApp::preinit()
{
  compile_shader = nullptr;
  CompilerDLL = 0;
  g_res_manager->startup();
  _high_light = 0.5f;
  _factor_l = 1.0;
  _shading_model = 0;
  return true;
}

bool HairShadingFastApp::init()
{
  FD3D11DynamicRHI* _rhid3d11 = dynamic_cast<FD3D11DynamicRHI*>(FD3D11DynamicRHI::GetRHI(ERHITYPES::RHI_D3D11));
  _device = _rhid3d11->GetDevice();
  _context = _rhid3d11->GetDeviceContext();
  _gi_factory = _rhid3d11->GetFactory();

  CompilerDLL = LoadLibrary("d3dcompiler_47.dll");
  compile_shader = (pD3DCompile)(void*)GetProcAddress(CompilerDLL, "D3DCompile");

  _cam = new RBCamera();
  if (!_cam) return false;
  _cam->set_position(0, 150, -50);
  RBVector4 position(0, 150, 1);
  _cam->set_target(position);
  _cam->set_fov_y(60);
  _cam->set_ratio(16.f / 9.f);
  _cam->set_near_panel(0.01f);
  _cam->set_far_panel(2000.f);

  _cam_move_speed = RBVector2(50, 50);
  _cam_rotate_speed = RBVector2(1, 1);



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

  _context->OMSetDepthStencilState(_depth_stencil_state, 1);

  float factor[4] = { 1.f, 1.f, 1.f, 1.f };
  _context->OMSetBlendState(_blending_state, factor, 0xffffffff);


  _context->RSSetState(_raster_state);


  return true;
}

void HairShadingFastApp::update(f32 dt)
{
  //_cam->pan(RBVector3(0, 0, 1), 1);
  _cam->update(dt);
  handle_input(dt);

}

void HairShadingFastApp::draw()
{

  ID3D11ShaderResourceView* null[] = { nullptr, nullptr, nullptr, nullptr };

  ID3D11RenderTargetView* views[4] = { _bufferA_target_view, _bufferB_target_view, _bufferC_target_view, _bufferDepth_target_view };
  _context->OMSetRenderTargets(4, views, _buffer_depth_stencil_ds_view);
  _context->ClearRenderTargetView(_bufferA_target_view, reinterpret_cast<const float*>(&RBColorf::black));
  _context->ClearRenderTargetView(_bufferB_target_view, reinterpret_cast<const float*>(&RBColorf::black));
  _context->ClearRenderTargetView(_bufferC_target_view, reinterpret_cast<const float*>(&RBColorf::black));
  _context->ClearRenderTargetView(_bufferDepth_target_view, reinterpret_cast<const float*>(&RBColorf::black));

  _context->OMSetDepthStencilState(_depth_stencil_state, 1);
  _context->ClearDepthStencilView(_buffer_depth_stencil_ds_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);



  RBMatrix mm;
  RBMatrix mp;
  RBMatrix mv;

  mm.set_identity();
  _cam->get_view_matrix(mv);
  _cam->get_perspective_matrix(mp);
  set_matrix(mm, mv, mp);
  set_gpass_vbuffer();
  _context->VSSetShader(_def_vertex_shader, 0, 0);
  //_context->GSSetShader(_def_geometry_shader, 0, 0);
  _context->PSSetShader(_def_pixel_shader, 0, 0);
  _context->PSSetShaderResources(0, 1, &_texture1_sview);
  _context->PSSetShaderResources(1, 1, &_opacity_texture_shader_view);
  _context->PSSetShaderResources(2, 1, &_normal_texture_shader_view);
  _context->PSSetShaderResources(3, 1, &_power_texture_shader_view);
  _context->PSSetSamplers(0, 1, &_sample_state);
  _context->IASetInputLayout(_layout);
  u32 stride = sizeof(Vertex_PNTT);
  u32 offset = 0;
  _context->IASetVertexBuffers(0, 1, &_vertex_buffer, &stride, &offset);
  _context->IASetIndexBuffer(_index_buffer, DXGI_FORMAT_R32_UINT, 0);
  _context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  _context->DrawIndexed(_idxs.size(), 0, 0);

  //_context->OMSetRenderTargets(0, 0, 0);
  _context->OMSetDepthStencilState(_depth_stencil_state_disable, 1);
  float factor[4] = { 1.f, 1.f, 1.f, 1.f };
  _context->OMSetBlendState(_blending_state_disable, factor, 0xffffffff);
  _context->OMSetRenderTargets(1, &back_buffer_view, _buffer_depth_stencil_ds_view);
  //_context->ClearDepthStencilView(_buffer_depth_stencil_ds_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
  _context->ClearRenderTargetView(back_buffer_view, reinterpret_cast<const float*>(&RBColorf::black));

  _context->VSSetShader(_def_vertex_shader_light, 0, 0);
  _context->PSSetShader(_def_pixel_shader_light, 0, 0);

  stride = sizeof(Vertex_PT);
  offset = 0;
  _context->IASetInputLayout(_layout_lighting);
  _context->IASetVertexBuffers(0, 1, &_vertex_buffer_lighting, &stride, &offset);
  _context->IASetIndexBuffer(_index_buffer_lighting, DXGI_FORMAT_R16_UINT, 0);
  _context->PSSetSamplers(0, 1, &_sample_state);
  _context->PSSetShaderResources(4, 1, &_env_texture_single_shader_view);
  _context->PSSetShaderResources(5, 1, &_env_texture_hdr_shader_view);
  _context->PSSetShaderResources(7, 1, &_noise_texture_shader_view);
  ID3D11ShaderResourceView* sviews[4] = { _bufferA_shader_view, _bufferB_shader_view, _bufferC_shader_view, _bufferDepth_shader_view };
  //ID3D11ShaderResourceView* sviews[4] = { _texture1_sview, _bufferB_shader_view, _bufferC_shader_view, _bufferDepth_shader_view };
  _context->PSSetShaderResources(0, 4, sviews);
  set_screen_size();
  _context->DrawIndexed(6, 0, 0);

  _context->PSSetShaderResources(0, 4, null);

}

void HairShadingFastApp::ter()
{
  FreeLibrary(CompilerDLL);
  CompilerDLL = 0;
}

void HairShadingFastApp::postter()
{
  g_res_manager->shutdown();
}

HairShadingFastApp::HairShadingFastApp(AppBase* a) :FastApp(a)
{

}

HairShadingFastApp::~HairShadingFastApp()
{

}

void HairShadingFastApp::load_assets()
{
  //g_res_manager->load_resource("shaders/d.vs", WIPResourceType::TEXT);
  load_shaders();
  load_deffered_pipe();
  /** load models */
  load_models_generate_buffer("Res/hair/hair.obj");
}



void HairShadingFastApp::show_error_message(ID3D10Blob* error, const char* filename)
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


void HairShadingFastApp::load_models_generate_buffer(const char* filename)
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
      getchar();
    }


	struct _TRIANGLE
	{
		tinyobj::index_t idxs[3];
		RBVector3 tangent;
	};

	std::vector<_TRIANGLE> _triangles;

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {

		for (int l = 0; l < shapes[s].mesh.indices.size() / 3; ++l)
		{
			tinyobj::index_t idex0 = shapes[s].mesh.indices[l * 3];
			tinyobj::index_t idex1 = shapes[s].mesh.indices[l * 3+1];
			tinyobj::index_t idex2 = shapes[s].mesh.indices[l * 3+2];

			_TRIANGLE tr;
			tr.idxs[0] = idex0;
			tr.idxs[1] = idex1;
			tr.idxs[2] = idex2;

			{
				tinyobj::index_t v0i = idex0;
				RBVector3 v0p = RBVector3(attrib.vertices[3 * v0i.vertex_index + 0], attrib.vertices[3 * v0i.vertex_index + 1], attrib.vertices[3 * v0i.vertex_index + 2]);
				RBVector2 v0t = RBVector2(attrib.texcoords[2 * v0i.texcoord_index], attrib.texcoords[2 * v0i.texcoord_index + 1]);
				v0i = idex1;
				RBVector3 v1p = RBVector3(attrib.vertices[3 * v0i.vertex_index + 0], attrib.vertices[3 * v0i.vertex_index + 1], attrib.vertices[3 * v0i.vertex_index + 2]);
				RBVector2 v1t = RBVector2(attrib.texcoords[2 * v0i.texcoord_index], attrib.texcoords[2 * v0i.texcoord_index + 1]);
				v0i = idex2;
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

				tr.tangent = tangent1;
				_triangles.push_back(tr);
			}
		}



		std::vector<RBVector3> avg_tangents;
		int numvertex = attrib.vertices.size() / 3;
		_vecs.resize(numvertex);
		//memset(_vecs.data(), 0, sizeof(HairShadingFastApp::Vertex_PNTT)*numvertex);

		for (int l = 0; l < numvertex; ++l)
		{
			RBVector3 avg_tangent = RBVector3::zero_vector;
			for (auto i : _triangles)
			{
				for (int j = 0; j < 3; ++j)
				{
					if (i.idxs[j].vertex_index == l)
						avg_tangent += i.tangent;
				}

			}
			avg_tangent.normalize();
			avg_tangents.push_back(avg_tangent);
			_vecs[l].tangent = avg_tangent;
		}

#if 0
		//纹理坐标乱序，我日你地妈
		_idxs.resize(shapes[s].mesh.indices.size());
		memset(&_idxs[0], 0, _idxs.size()*sizeof(uint32));

		for (int l = 0; l < shapes[s].mesh.indices.size() / 3; ++l)
		{
			tinyobj::index_t idex0 = shapes[s].mesh.indices[l * 3];
			tinyobj::index_t idex1 = shapes[s].mesh.indices[l * 3 + 1];
			tinyobj::index_t idex2 = shapes[s].mesh.indices[l * 3 + 2];

			_idxs[l * 3] = idex0.vertex_index;
			_idxs[l * 3 + 1] = idex1.vertex_index;
			_idxs[l * 3 + 2] = idex2.vertex_index;

			_vecs[idex0.vertex_index].normal.x = attrib.normals[3*idex0.normal_index + 0];
			_vecs[idex0.vertex_index].normal.y = attrib.normals[3 * idex0.normal_index + 1];
			_vecs[idex0.vertex_index].normal.z = attrib.normals[3 * idex0.normal_index + 2];
			_vecs[idex0.vertex_index].pos.x = attrib.vertices[3 * idex0.vertex_index + 0];
			_vecs[idex0.vertex_index].pos.y = attrib.vertices[3 * idex0.vertex_index + 1];
			_vecs[idex0.vertex_index].pos.z = attrib.vertices[3 * idex0.vertex_index + 2];
			_vecs[idex0.vertex_index].texcoord.x = attrib.texcoords[2 * idex0.texcoord_index + 0];
			_vecs[idex0.vertex_index].texcoord.y = attrib.texcoords[2 * idex0.texcoord_index + 1];

			_vecs[idex1.vertex_index].normal.x = attrib.normals[3 * idex1.normal_index + 0];
			_vecs[idex1.vertex_index].normal.y = attrib.normals[3 * idex1.normal_index + 1];
			_vecs[idex1.vertex_index].normal.z = attrib.normals[3 * idex1.normal_index + 2];
			_vecs[idex1.vertex_index].pos.x = attrib.vertices[3 * idex1.vertex_index + 0];
			_vecs[idex1.vertex_index].pos.y = attrib.vertices[3 * idex1.vertex_index + 1];
			_vecs[idex1.vertex_index].pos.z = attrib.vertices[3 * idex1.vertex_index + 2];
			_vecs[idex1.vertex_index].texcoord.x = attrib.texcoords[2 * idex1.texcoord_index + 0];
			_vecs[idex1.vertex_index].texcoord.y = attrib.texcoords[2 * idex1.texcoord_index + 1];
			
			_vecs[idex2.vertex_index].normal.x = attrib.normals[3 * idex2.normal_index + 0];
			_vecs[idex2.vertex_index].normal.y = attrib.normals[3 * idex2.normal_index + 1];
			_vecs[idex2.vertex_index].normal.z = attrib.normals[3 * idex2.normal_index + 2];
			_vecs[idex2.vertex_index].pos.x = attrib.vertices[3 * idex2.vertex_index + 0];
			_vecs[idex2.vertex_index].pos.y = attrib.vertices[3 * idex2.vertex_index + 1];
			_vecs[idex2.vertex_index].pos.z = attrib.vertices[3 * idex2.vertex_index + 2];
			_vecs[idex2.vertex_index].texcoord.x = attrib.texcoords[2 * idex2.texcoord_index + 0];
			_vecs[idex2.vertex_index].texcoord.y = attrib.texcoords[2 * idex2.texcoord_index + 1];
		}
#endif
#if 1
		_vecs.clear();
		_idxs.clear();
		
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
		  vv.tangent = avg_tangents[idx.vertex_index];
          _vecs.push_back(vv);
          _idxs.push_back(v + index_offset);// idx.vertex_index);
        }

        index_offset += fv;

        // per-face material
        shapes[s].mesh.material_ids[f];
      }
#endif
    }
  }

  //平均切线

  {
    /** create vertex buffer with vertex data */
    D3D11_BUFFER_DESC des_def;
    ZeroMemory(&des_def, sizeof(des_def));
    des_def.Usage = D3D11_USAGE_IMMUTABLE;
    des_def.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    des_def.ByteWidth = PAD16(_vecs.size()*sizeof(Vertex_PNTT));
    des_def.MiscFlags = 0;
    des_def.StructureByteStride = 0;
    des_def.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA init_dat;
    init_dat.pSysMem = _vecs.data();
    init_dat.SysMemPitch = 0;
    init_dat.SysMemSlicePitch = 0;

    HR(_device->CreateBuffer(&des_def, &init_dat, &_vertex_buffer));
  }

  {
    /** create index buffer with index data */
    D3D11_BUFFER_DESC des_def;
    ZeroMemory(&des_def, sizeof(des_def));
    des_def.Usage = D3D11_USAGE_IMMUTABLE;
    des_def.BindFlags = D3D11_BIND_INDEX_BUFFER;
    des_def.ByteWidth = PAD16(_idxs.size()*sizeof(u32));
    des_def.MiscFlags = 0;
    des_def.StructureByteStride = 0;
    des_def.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA init_dat;
    init_dat.pSysMem = _idxs.data();
    init_dat.SysMemPitch = 0;
    init_dat.SysMemSlicePitch = 0;

    HR(_device->CreateBuffer(&des_def, &init_dat, &_index_buffer));
  }

  //delete return value 
  auto m2u = [&](char* src)->wchar_t*
  {
    int dest_len;
    dest_len = MultiByteToWideChar(CP_ACP, 0, src, -1, NULL, 0);
    wchar_t *dest = new wchar_t[dest_len];
    if (dest == NULL)
      return NULL;
    MultiByteToWideChar(CP_ACP, 0, src, -1, dest, dest_len);
    return dest;
  };

  //diffuse
  {
    const wchar_t* fileName = m2u("res/hair/diffuse.dds");
    auto ret = DirectX::CreateDDSTextureFromFile(_device, fileName, (ID3D11Resource**)&_texture1, &_texture1_sview);
    delete[] fileName;
    if (ret != S_OK)  printf("diffuse wrong!!\n");
  }
  //normal
  {
    auto ret = create_texture_from_file("res/hair/normal.jpg", &_normal_texture, &_normal_texture_shader_view);
    if (!ret)
      printf("normal wrong!!\n");
  }
  //power
  {
    const wchar_t* fileName = m2u("res/hair/power.dds");
    auto ret = DirectX::CreateDDSTextureFromFile(_device, fileName, (ID3D11Resource**)&_power_texture, &_power_texture_shader_view);
    delete[] fileName;
    if (ret != S_OK)  
      printf("power wrong!!\n");
  }
  //opacity
  {
    auto ret = create_texture_from_file("res/hair/opacity.jpg", &_opacity_texture, &_opacity_texture_shader_view);
    if (!ret)
      printf("opcity wrong!!\n");
  }

  //env texture
  do{
    const wchar_t* fileName = m2u("res/env/1DiffuseHDR.dds");
    auto ret = DirectX::CreateDDSTextureFromFile(_device, fileName, (ID3D11Resource**)&_env_texture_single, &_env_texture_single_shader_view);
    if (ret != S_OK)
    {
      printf("1DiffuseHDR wrong\n");
    }
  } while (0);
  {
    const wchar_t* fileName = m2u("res/env/1SpecularHDR.dds");
    auto ret = DirectX::CreateDDSTextureFromFile(_device, fileName, (ID3D11Resource**)&_env_texture_hdr, &_env_texture_hdr_shader_view);
    if (ret != S_OK)
      printf("1SpecularHDR wrong\n");
  }
  //noise
  {
	  auto ret = create_texture_from_file("res/hair/noise.jpg", &_noise_texture, &_noise_texture_shader_view);
	  if (!ret)
		  printf("noise wrong!!\n");
  }
}

void HairShadingFastApp::set_matrix(const RBMatrix& m, const RBMatrix& v, const RBMatrix& p)
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
  //auto res = RBVector4(1, 0, 0, 1)*mb->m.get_rotation()*mb->v.get_rotation();
  //printf("%f,%f,%f,%f\n", res.x, res.y, res.z, res.w);
  _context->Unmap(_matrix_buffer, 0);
  _context->VSSetConstantBuffers(1, 1, &_matrix_buffer);

}

void HairShadingFastApp::handle_input(f32 dt)
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

void HairShadingFastApp::set_screen_size()
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
  mb->hipow = _high_light;
  mb->factor_l = _factor_l;
  RBMatrix m;
  _cam->get_perspective_matrix(m);
  m = m.get_inverse_slow();
  mb->inv_projection = m;
  _cam->get_view_matrix(m);
  mb->mv = m;
  mb->metallic = _metallic;
  mb->shading_model = _shading_model;
  _context->Unmap(_screen_size_buffer, 0);
  _context->PSSetConstantBuffers(0, 1, &_screen_size_buffer);
}



void HairShadingFastApp::set_gpass_vbuffer()
{
  HRESULT hr;
  D3D11_MAPPED_SUBRESOURCE mpr;

  hr = _context->Map(_dv_cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mpr);
  if (FAILED(hr))
  {
    ::MessageBox(0, "Map screen size buffer failed!", "ERROR", MB_OK);
  }

  f32* mb;
  mb = (f32*)mpr.pData;
  *mb = _zplus;
  _context->Unmap(_dv_cbuffer, 0);
  _context->VSSetConstantBuffers(0, 1, &_dv_cbuffer);



}

void HairShadingFastApp::load_shaders()
{
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


    /** Gbuffer shader */
    {
      auto res_handle = g_res_manager->load_resource("shaders/deferred_gbuffer_v.hlsl", WIPResourceType::TEXT);
      TRefCountPtr<ID3DBlob> Shader;
      TRefCountPtr<ID3DBlob> Errors;

      HRESULT Result;

      if (compile_shader)
      {
        bool bException = false;

        Result = compile_shader(
          ((std::string*)res_handle->ptr)->data(),
          ((std::string*)res_handle->ptr)->length(),
          "shaders/deferred_gbuffer_v.hlsl",
          /*pDefines=*/ NULL,
          /*pInclude=*/ NULL,
          "VsMain",
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
            show_error_message(Errors, "shaders/deferred_gbuffer_v.hlsl");
          }
          assert(0);
        }
        else
        {
          _device->CreateVertexShader(Shader->GetBufferPointer(), Shader->GetBufferSize(), NULL, &_def_vertex_shader);
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
                    {
                      D3D11_BUFFER_DESC mbd;

                      mbd.Usage = D3D11_USAGE_DYNAMIC;
                      mbd.ByteWidth = PAD16(sizeof(f32));
                      mbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
                      mbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                      mbd.MiscFlags = 0;
                      mbd.StructureByteStride = 0;

                      HR(_device->CreateBuffer(&mbd, 0, &_dv_cbuffer));
                    }
                    _device->CreateInputLayout(RBD3D11InputElementDesc::vertexDescPNTT, 4, Shader->GetBufferPointer(), Shader->GetBufferSize(), &_layout);

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
      auto res_handle1 = g_res_manager->load_resource("shaders/deferred_gbuffer_g.hlsl", WIPResourceType::TEXT);
      TRefCountPtr<ID3DBlob> Shader;
      TRefCountPtr<ID3DBlob> Errors;

      HRESULT Result;

      if (compile_shader)
      {
        bool bException = false;

        Result = compile_shader(
          ((std::string*)res_handle1->ptr)->data(),
          ((std::string*)res_handle1->ptr)->length(),
          "shaders/deferred_gbuffer_g.hlsl",
          /*pDefines=*/ NULL,
          /*pInclude=*/ NULL,
          "GsMain",
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
            show_error_message(Errors, "shaders/deferred_gbuffer_g.hlsl");
          }
          assert(0);
        }
        else
        {
          _device->CreateGeometryShader(Shader->GetBufferPointer(), Shader->GetBufferSize(), NULL, &_def_geometry_shader);
        }
        g_res_manager->free(res_handle1, res_handle1->size);
      }
      else
      {
        printf("compile_shader failed!\n");
      }
    }
    {
      auto res_handle1 = g_res_manager->load_resource("shaders/deferred_gbuffer_p_hair.hlsl", WIPResourceType::TEXT);
      TRefCountPtr<ID3DBlob> Shader;
      TRefCountPtr<ID3DBlob> Errors;

      HRESULT Result;

      if (compile_shader)
      {
        bool bException = false;

        Result = compile_shader(
          ((std::string*)res_handle1->ptr)->data(),
          ((std::string*)res_handle1->ptr)->length(),
          "shaders/deferred_gbuffer_p_hair.hlsl",
          /*pDefines=*/ NULL,
          /*pInclude=*/ NULL,
          "PsMain",
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
            show_error_message(Errors, "shaders/deferred_gbuffer_p_hair.hlsl");
          }
          assert(0);
        }
        else
        {
          _device->CreatePixelShader(Shader->GetBufferPointer(), Shader->GetBufferSize(), NULL, &_def_pixel_shader);
        }
        g_res_manager->free(res_handle1, res_handle1->size);
      }
      else
      {
        printf("compile_shader failed!\n");
      }
    }

    /** composition pass shader */
    {
      auto res_handle1 = g_res_manager->load_resource("shaders/deferred_compass_v.hlsl", WIPResourceType::TEXT);
      TRefCountPtr<ID3DBlob> Shader;
      TRefCountPtr<ID3DBlob> Errors;

      HRESULT Result;

      if (compile_shader)
      {
        bool bException = false;

        Result = compile_shader(
          ((std::string*)res_handle1->ptr)->data(),
          ((std::string*)res_handle1->ptr)->length(),
          "shaders/deferred_compass_v.hlsl",
          /*pDefines=*/ NULL,
          /*pInclude=*/ NULL,
          "VsMain",
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
            show_error_message(Errors, "shaders/deferred_compass_v.hlsl");
          }
          assert(0);
        }
        else
        {
          _device->CreateVertexShader(Shader->GetBufferPointer(), Shader->GetBufferSize(), NULL, &_def_vertex_shader_light);
        }
        _device->CreateInputLayout(RBD3D11InputElementDesc::vertexDescPT, 2, Shader->GetBufferPointer(), Shader->GetBufferSize(), &_layout_lighting);
        g_res_manager->free(res_handle1, res_handle1->size);
      }
      else
      {
        printf("compile_shader failed!\n");
      }
    }
    {
      auto res_handle1 = g_res_manager->load_resource("shaders/deferrend_hair_shading_p.hlsl", WIPResourceType::TEXT);
      TRefCountPtr<ID3DBlob> Shader;
      TRefCountPtr<ID3DBlob> Errors;

      HRESULT Result;

      if (compile_shader)
      {
        bool bException = false;

        Result = compile_shader(
          ((std::string*)res_handle1->ptr)->data(),
          ((std::string*)res_handle1->ptr)->length(),
          "shaders/deferrend_hair_shading_p.hlsl",
          /*pDefines=*/ NULL,
          /*pInclude=*/ NULL,
          "PsMain",
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
            show_error_message(Errors, "shaders/deferrend_hair_shading_p.hlsl");
          }
          assert(0);
        }
        else
        {
          _device->CreatePixelShader(Shader->GetBufferPointer(), Shader->GetBufferSize(), NULL, &_def_pixel_shader_light);
          /** constant buffer */
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
        g_res_manager->free(res_handle1, res_handle1->size);
      }
      else
      {
        printf("compile_shader failed!\n");
      }
    }

  }
}

void HairShadingFastApp::load_deffered_pipe()
{
  /** create render target */
  {
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = ww;
    desc.Height = wh;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    HR(_device->CreateTexture2D(&desc, NULL, &_bufferA));
    D3D11_RENDER_TARGET_VIEW_DESC rtdes;
    rtdes.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtdes.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtdes.Texture2D.MipSlice = 0;
    HR(_device->CreateRenderTargetView(_bufferA, &rtdes, &_bufferA_target_view));
    D3D11_SHADER_RESOURCE_VIEW_DESC srdes;
    memset(&srdes, 0, sizeof(srdes));
    srdes.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srdes.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srdes.Texture2D.MostDetailedMip = 0;
    srdes.Texture2D.MipLevels = 1;
    HR(_device->CreateShaderResourceView(_bufferA, &srdes, &_bufferA_shader_view));
  }
  {
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = ww;
    desc.Height = wh;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    HR(_device->CreateTexture2D(&desc, NULL, &_bufferB));
    D3D11_RENDER_TARGET_VIEW_DESC rtdes;
    rtdes.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    rtdes.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtdes.Texture2D.MipSlice = 0;
    HR(_device->CreateRenderTargetView(_bufferB, &rtdes, &_bufferB_target_view));
    D3D11_SHADER_RESOURCE_VIEW_DESC srdes;
    memset(&srdes, 0, sizeof(srdes));
    srdes.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    srdes.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srdes.Texture2D.MostDetailedMip = 0;
    srdes.Texture2D.MipLevels = 1;
    HR(_device->CreateShaderResourceView(_bufferB, &srdes, &_bufferB_shader_view));
  }
  {
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = ww;
    desc.Height = wh;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    HR(_device->CreateTexture2D(&desc, NULL, &_bufferC));
    D3D11_RENDER_TARGET_VIEW_DESC rtdes;
    rtdes.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtdes.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtdes.Texture2D.MipSlice = 0;
    HR(_device->CreateRenderTargetView(_bufferC, &rtdes, &_bufferC_target_view));
    D3D11_SHADER_RESOURCE_VIEW_DESC srdes;
    memset(&srdes, 0, sizeof(srdes));
    srdes.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srdes.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srdes.Texture2D.MostDetailedMip = 0;
    srdes.Texture2D.MipLevels = 1;
    HR(_device->CreateShaderResourceView(_bufferC, &srdes, &_bufferC_shader_view));
  }
  {
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = ww;
    desc.Height = wh;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    HR(_device->CreateTexture2D(&desc, NULL, &_buffer_lighting));
    D3D11_RENDER_TARGET_VIEW_DESC rtdes;
    rtdes.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtdes.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtdes.Texture2D.MipSlice = 0;
    HR(_device->CreateRenderTargetView(_buffer_lighting, &rtdes, &_buffer_lighting_target_view));
    D3D11_SHADER_RESOURCE_VIEW_DESC srdes;
    memset(&srdes, 0, sizeof(srdes));
    srdes.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srdes.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srdes.Texture2D.MostDetailedMip = 0;
    srdes.Texture2D.MipLevels = 1;
    HR(_device->CreateShaderResourceView(_buffer_lighting, &srdes, &_buffer_lighting_shader_view));
  }
  {
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = ww;
    desc.Height = wh;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R32_FLOAT;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    HR(_device->CreateTexture2D(&desc, NULL, &_bufferDepth));
    D3D11_RENDER_TARGET_VIEW_DESC rtdes;
    rtdes.Format = DXGI_FORMAT_R32_FLOAT;
    rtdes.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtdes.Texture2D.MipSlice = 0;
    HR(_device->CreateRenderTargetView(_bufferDepth, &rtdes, &_bufferDepth_target_view));
    D3D11_SHADER_RESOURCE_VIEW_DESC srdes;
    memset(&srdes, 0, sizeof(srdes));
    srdes.Format = DXGI_FORMAT_R32_FLOAT;
    srdes.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srdes.Texture2D.MostDetailedMip = 0;
    srdes.Texture2D.MipLevels = 1;
    HR(_device->CreateShaderResourceView(_bufferDepth, &srdes, &_bufferDepth_shader_view));
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

  {
    /** create vertex buffer with vertex data */
    D3D11_BUFFER_DESC des_def;
    ZeroMemory(&des_def, sizeof(des_def));
    des_def.Usage = D3D11_USAGE_IMMUTABLE;
    des_def.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    des_def.ByteWidth = PAD16(4 * sizeof(Vertex_PT));
    des_def.MiscFlags = 0;
    des_def.StructureByteStride = 0;
    des_def.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA init_dat;
    init_dat.pSysMem = quad;
    init_dat.SysMemPitch = 0;
    init_dat.SysMemSlicePitch = 0;

    HR(_device->CreateBuffer(&des_def, &init_dat, &_vertex_buffer_lighting));
  }

  {
    /** create index buffer with index data */
    D3D11_BUFFER_DESC des_def;
    ZeroMemory(&des_def, sizeof(des_def));
    des_def.Usage = D3D11_USAGE_IMMUTABLE;
    des_def.BindFlags = D3D11_BIND_INDEX_BUFFER;
    des_def.ByteWidth = PAD16(6 * sizeof(u16));
    des_def.MiscFlags = 0;
    des_def.StructureByteStride = 0;
    des_def.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA init_dat;
    init_dat.pSysMem = quad_idx;
    init_dat.SysMemPitch = 0;
    init_dat.SysMemSlicePitch = 0;

    HR(_device->CreateBuffer(&des_def, &init_dat, &_index_buffer_lighting));
  }
}


void HairShadingFastApp::create_rhires()
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
    resd.CullMode = D3D11_CULL_BACK;
    resd.FrontCounterClockwise = FALSE;
    resd.DepthClipEnable = TRUE;

    HR(_device->CreateRasterizerState(&resd, &_raster_state));
  }
}

bool HairShadingFastApp::create_texture_from_file(const char* filename, ID3D11Texture2D** tex, ID3D11ShaderResourceView** tex_view)
{
  auto had = g_res_manager->load_resource(filename);

  TextureData* td = (TextureData*)had->extra;

  D3D11_TEXTURE2D_DESC desc;
  desc.Width = td->width;
  desc.Height = td->height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;
  D3D11_SUBRESOURCE_DATA subs;
  ZeroMemory(&subs, sizeof(D3D11_SUBRESOURCE_DATA));
  subs.pSysMem = had->ptr;
  subs.SysMemPitch = desc.Width * 4;
  subs.SysMemSlicePitch = 0;
  HR(_device->CreateTexture2D(&desc, &subs, tex));

  D3D11_SHADER_RESOURCE_VIEW_DESC srdes;
  memset(&srdes, 0, sizeof(srdes));
  srdes.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  srdes.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srdes.Texture2D.MostDetailedMip = 0;
  srdes.Texture2D.MipLevels = 1;
  HR(_device->CreateShaderResourceView(*tex, &srdes, tex_view));

  g_res_manager->free(had, -1);
  return true;
}
