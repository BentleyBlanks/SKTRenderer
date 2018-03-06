#include "SVOFastApp.h"
#include "Camera.h"
#include "thirdpart/imgui/imgui.h"
#include "Util.h"
#include "tiny_obj_loader.h"
#include "Input.h"
#include <iostream>

#define SVO
//#define SIMPLEREAD
bool SVOFastApp::preinit()
{
	voxel_resolution = 512;
	nlights = 1;
	lights.resize(nlights);
	return true;
}

bool SVOFastApp::init()
{
	//128 no svo 
	//triangle primitive
	_rhi = static_cast<FD3D11DynamicRHI*>(base_app->get_rhi());

	_cam = new RBCamera();
	if (!_cam) return false;
	//move camera not object
	_cam->set_position(0, 0, -voxel_resolution / 2);
	RBVector4 position(0, 0, 1);
	_cam->set_target(position);
	_cam->set_fov_y(60);
	_cam->set_ratio(16.f / 9.f);
	_cam->set_near_panel(1.f);
	_cam->set_far_panel(voxel_resolution + 1.f);

	_cam->set_move_speed(RBVector2(150, 150));
	_cam->set_rotate_speed(RBVector2(5, 5));

	_d3d11_renderer = new Direct3D11Renderer(_rhi->GetDevice(), _rhi->GetDeviceContext());

	load_assets();
	create_rhires();

	_d3d11_renderer->setViewport(ww, wh);
	_d3d11_renderer->setFrameBuffer(back_buffer_view, _d3d11_renderer->getDSV(_buffer_depth_stencil));
	_d3d11_renderer->changeToMainFramebuffer();



	return true;
}

void SVOFastApp::update(f32 dt)
{
	handle_input(dt);
}

void SVOFastApp::setup_lights(int n)
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
	ImGui::SliderFloat("White Scale", &white_scale, 0, 1);

	if (use_white)
	{
		c[0] = c[1] = c[2] = 100 * white_scale;
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

void SVOFastApp::render_scene()
{
	RBMatrix mp;
	RBMatrix mv;

	_cam->get_view_matrix(mv);
	_cam->get_perspective_matrix(mp);
	//render object
	_d3d11_renderer->setDepthState(_ds_enable);
	_d3d11_renderer->clear(false, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);

	_d3d11_renderer->setShader(_pbr_render_shader);
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
	_d3d11_renderer->setRasterizerState(_rs_cull);
	setup_lights(nlights);
	static float pos[3] = {254.5,254.5,254.5};
	
	ImGui::SliderFloat3("Offset", pos, -500, 500);
	if (Input::get_key_up(WIP_Z)) pos[0] += 0.5;
	if (Input::get_key_up(WIP_X)) pos[1] += 0.5;
	if (Input::get_key_up(WIP_C)) pos[2] += 0.5;
	if (Input::get_key_up(WIP_V)) pos[0] -= 0.5;
	if (Input::get_key_up(WIP_B)) pos[1] -= 0.5;
	if (Input::get_key_up(WIP_N)) pos[2] -= 0.5;
	for (auto& model : models)
	{
		RBMatrix mw = model.model;
		mw.set_translation(pos[0], pos[1], pos[2]);
		_d3d11_renderer->setShaderConstant4x4f("m", mw);
		_d3d11_renderer->setShaderConstant4x4f("m1", mw);
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

	for (auto& model : models_alpha)
	{

		RBMatrix mw = model.model;
		mw.set_translation(pos[0], pos[1], pos[2]);
		_d3d11_renderer->setShaderConstant4x4f("m", mw);
		_d3d11_renderer->setShaderConstant4x4f("m1", mw);
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
}

void SVOFastApp::draw()
{

	static bool bvoxel = false;
	_d3d11_renderer->setRasterizerState(_rs);

	_d3d11_renderer->clear(true, true, true, reinterpret_cast<const float*>(&RBColorf::blank), 1, 1);

	RBMatrix mm;
	RBMatrix mp;
	RBMatrix mv;

	mm.set_identity();
	_cam->get_view_matrix(mv);
	_cam->get_perspective_matrix(mp);
	if (!bvoxel)
	{
		//voxelize_without_svo();
		build_svo();
		bvoxel = true;
	}
	//_d3d11_renderer->setDepthState(_ds_disable);


	render_scene();

	static bool raymarch = true;
	ImGui::Checkbox("Ray march", &raymarch);

	static bool slowcam = false;
	if (ImGui::Checkbox("Slow Camera", &slowcam))
	{
		if (slowcam)
			_cam->set_move_speed(RBVector2(15, 15));
	}

	static bool show_ref = false;
	ImGui::Checkbox("Show reference", &show_ref);

	if (raymarch)
	{

#ifdef SVO
		_d3d11_renderer->setShader(_see_voxel_svo);

		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE mpr;

		hr = _d3d11_renderer->context->Map(_CB_voxel_svo, 0, D3D11_MAP_WRITE_DISCARD, 0, &mpr);
		if ((hr) != S_OK)
		{
			g_logger->debug(WIP_ERROR, "map svo_see constant buffer failed!");
		}
		CB* cb = (CB*)mpr.pData;
		cb->inv_mv_mat = mv.get_inverse_slow();
		cb->tan_fov_x = _cam->get_tan_half_fovx();
		cb->tan_fov_y = _cam->get_tan_half_fovy();
		cb->view_dist = _cam->get_near_panel();
		cb->voxel_size = voxel_resolution;
		_d3d11_renderer->context->Unmap(_CB_voxel_svo, 0);




		_d3d11_renderer->apply();
		_d3d11_renderer->context->PSSetConstantBuffers(0, 1, &_CB_voxel_svo);
		_d3d11_renderer->context->PSSetShaderResources(0, 1, &_d3d11_renderer->textures[_brick_pool].srv);
		_d3d11_renderer->context->PSSetShaderResources(1,1,&_d3d11_renderer->structureBuffers[_node_pool].srv);
		_d3d11_renderer->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_d3d11_renderer->context->Draw(3, 0);
#else
		_d3d11_renderer->setShader(_see_voxel);
		_d3d11_renderer->setShaderConstant4x4f("inv_mv_mat", mv.get_inverse_slow());
		_d3d11_renderer->setShaderConstant1f("view_dist", _cam->get_near_panel());
		_d3d11_renderer->setShaderConstant1f("tan_fov_x", _cam->get_tan_half_fovx());
		_d3d11_renderer->setShaderConstant1f("tan_fov_y", _cam->get_tan_half_fovy());
		_d3d11_renderer->setTexture("voxel", _buffer_node_3d);


		_d3d11_renderer->apply();

		_d3d11_renderer->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_d3d11_renderer->context->Draw(3, 0);
#endif
	}
	else
	{
#ifdef SVO
		_d3d11_renderer->setRasterizerState(_rs_wireframe);
		_d3d11_renderer->apply();
		if (!show_ref)
		{
			_d3d11_renderer->setShader(_see_voxel_svo1);
			_d3d11_renderer->setShaderConstant4x4f("m", mm);
			_d3d11_renderer->setShaderConstant4x4f("v", mv);
			_d3d11_renderer->setShaderConstant4x4f("p", mp);
			_d3d11_renderer->setVertexFormat(_vf1_svo);
			_d3d11_renderer->apply();
			_d3d11_renderer->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
			uint stride = 32;
			uint offset = 0;
			_d3d11_renderer->context->IASetVertexBuffers(0, 1, &_indirect_primitives_buffer, &stride, &offset);
			//for (int i = 0; i < 128*128*128/2048;++i)
			//_d3d11_renderer->context->Draw(2048,i*2048);
			//_d3d11_renderer->context->Draw(tdn, 0);
			_d3d11_renderer->context->DrawInstanced(tdn, 1, 0, 0);

		}
		else
		{
			_d3d11_renderer->setShader(_see_voxel_svo1);
			_d3d11_renderer->setShaderConstant4x4f("m", mm);
			_d3d11_renderer->setShaderConstant4x4f("v", mv);
			_d3d11_renderer->setShaderConstant4x4f("p", mp);
			_d3d11_renderer->setVertexFormat(_vf1_svo);
			_d3d11_renderer->apply();
			_d3d11_renderer->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
			uint stride = 32;
			uint offset = 0;
			_d3d11_renderer->context->IASetVertexBuffers(0, 1, &_indirect_primitives_buffer1, &stride, &offset);
			//for (int i = 0; i < 128*128*128/2048;++i)
			//_d3d11_renderer->context->Draw(2048,i*2048);
			//_d3d11_renderer->context->Draw(tdn, 0);
			_d3d11_renderer->context->DrawInstanced(tdn, 1, 0, 0);
		}
		_d3d11_renderer->setRasterizerState(_rs);
		_d3d11_renderer->apply();
#else
		_d3d11_renderer->setShader(_see_voxel1);
		_d3d11_renderer->setShaderConstant4x4f("m", mm);
		_d3d11_renderer->setShaderConstant4x4f("v", mv);
		_d3d11_renderer->setShaderConstant4x4f("p", mp);
		_d3d11_renderer->setTexture("voxel_tex", _buffer_node_3d);
		_d3d11_renderer->apply();
		_d3d11_renderer->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		//for (int i = 0; i < 128*128*128/2048;++i)
		//_d3d11_renderer->context->Draw(2048,i*2048);
		_d3d11_renderer->context->Draw(64 * 64 * 64 * 16, 0);
		_d3d11_renderer->context->Draw(64 * 64 * 64, 64 * 64 * 64);
		//_d3d11_renderer->context->Draw(64 * 64 * 64, 0);
#endif
	}
	_d3d11_renderer->setShader(_null_shader);
	_d3d11_renderer->apply();

	//_d3d11_renderer->context->GSSetShader(0, 0, 0);

}




void SVOFastApp::load_models_generate_buffer(const char* filename)
{
#ifdef SIMPLEREAD
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
					//donnot translate so that protect to use the same camera view matrix
					vv.pos = RBVector3(vx, vy, -vz);
					vv.normal = RBVector3(nx, ny, -nz);
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
#else
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
				  vv.pos = RBVector3(vx, vy, vz)*0.15;
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
#endif

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

#ifdef SIMPLEREAD
	_vb = _d3d11_renderer->addVertexBuffer(_vecs.size()*sizeof(Vertex_PNT), BufferAccess::STATIC, _vecs.data());
	_ib = _d3d11_renderer->addIndexBuffer(_idxs.size(), sizeof(u32), BufferAccess::STATIC, _idxs.data());
#endif
}

void SVOFastApp::load_assets()
{
	{
		auto sh1 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/voxelize_v.hlsl", "main", "vs_5_0");
		auto sh2 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/voxelize_g.hlsl", "main", "gs_5_0");
		auto sh3 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/voxelize_p.hlsl", "main", "ps_5_0");
		_bound_shader = _d3d11_renderer->addShader(sh1.GetReference(), sh2.GetReference(), sh3.GetReference(), 0);
	}
  {
	  auto sh1 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/post_visulize_voxel.hlsl", "main_vs", "vs_5_0");
	  //auto sh2 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/post_visulize_voxel.hlsl", "main", "gs_5_0");
	  auto sh3 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/post_visulize_voxel.hlsl", "main_ps", "ps_5_0");
	  _see_voxel = _d3d11_renderer->addShader(sh1.GetReference(), 0, sh3.GetReference(), D3D11ShaderCompiler::reflect_shader);
  }
	{
		auto sh1 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/render_voxel_v.hlsl", "main", "vs_5_0");
		auto sh2 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/render_voxel_g.hlsl", "main", "gs_5_0");
		auto sh3 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/render_voxel_p.hlsl", "main", "ps_5_0");
		_see_voxel1 = _d3d11_renderer->addShader(sh1.GetReference(), sh2, sh3.GetReference(), D3D11ShaderCompiler::reflect_shader);
	}
	  {
		  auto sh1 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/post_visulize_voxel_svo.hlsl", "main_vs", "vs_5_0");
		  auto sh3 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/post_visulize_voxel_svo.hlsl", "main_ps", "ps_5_0");
		  _see_voxel_svo = _d3d11_renderer->addShader(sh1.GetReference(), 0, sh3.GetReference(), 0);
	  }
		  {
			  auto sh1 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/render_voxel_v_svo.hlsl", "main1", "vs_5_0");
			  auto sh2 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/render_voxel_g_svo.hlsl", "main", "gs_5_0");
			  auto sh3 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/render_voxel_p_svo.hlsl", "main", "ps_5_0");
			  _see_voxel_svo1 = _d3d11_renderer->addShader(sh1.GetReference(), sh2.GetReference(), sh3.GetReference(), D3D11ShaderCompiler::reflect_shader);
		  }
		  {
			  auto sh1 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/simple_v.hlsl", "main", "vs_5_0");
			  auto sh2 = D3D11ShaderCompiler::compile(_d3d11_renderer->device, "shaders/simple_p.hlsl", "main", "ps_5_0");
			  _pbr_render_shader = _d3d11_renderer->addShader(sh1, 0, sh2, D3D11ShaderCompiler::reflect_shader);
		  }
	_null_shader = _d3d11_renderer->addShader(0, 0, 0, D3D11ShaderCompiler::reflect_shader);
#ifdef SIMPLEREAD
	load_models_generate_buffer("res/pot.obj");
#else
	load_models_generate_buffer("res/sponza/pbr/sp.obj");
#endif
}

void SVOFastApp::create_rhires()
{

	init_svo_build();

	_buffer_depth_stencil = _d3d11_renderer->addRenderDepth(ww, wh, 1, FORMAT::FORMAT_D24S8);

	ID3D11Texture3D* tex3d1;
	ID3D11UnorderedAccessView* tex3d1_view;
	D3D11_TEXTURE3D_DESC desc;
	desc.Width = voxel_resolution;
	desc.Height = voxel_resolution;
	desc.Depth = voxel_resolution;
	desc.Format = DXGI_FORMAT_R32_UINT;
	desc.MipLevels = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	HRESULT hr = _d3d11_renderer->device->CreateTexture3D(&desc, 0, (ID3D11Texture3D **)&tex3d1);
	if (hr != S_OK)
		g_logger->debug(WIP_ERROR, "create %d texture3d failed!", voxel_resolution);
	_buffer_indirect_node_3d = _d3d11_renderer->addTexture(tex3d1, 0);
	_d3d11_renderer->textures[_buffer_indirect_node_3d].uav = 0;
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavd;
	memset(&uavd, 0, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	uavd.Format = DXGI_FORMAT_R32_UINT;
	uavd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
	uavd.Texture3D.MipSlice = 0;
	uavd.Texture3D.FirstWSlice = 0;
	uavd.Texture3D.WSize = -1;

	hr = _d3d11_renderer->device->CreateUnorderedAccessView(tex3d1, &uavd, &tex3d1_view);
	if (hr != S_OK)
		g_logger->debug(WIP_ERROR, "create %d texture3d view failed!", voxel_resolution);
	_d3d11_renderer->textures[_buffer_indirect_node_3d].uav = tex3d1_view;

	{
		ID3D11Texture3D* tex3d1;
		ID3D11UnorderedAccessView* tex3d1_view;
		D3D11_TEXTURE3D_DESC desc;
		desc.Width = voxel_resolution;
		desc.Height = voxel_resolution;
		desc.Depth = voxel_resolution;
		desc.Format = DXGI_FORMAT_R32_UINT;
		desc.MipLevels = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		HRESULT hr = _d3d11_renderer->device->CreateTexture3D(&desc, 0, (ID3D11Texture3D **)&tex3d1);
		if (hr != S_OK)
			g_logger->debug(WIP_ERROR, "create %d texture3d failed!", voxel_resolution);
		_buffer_node_3d = _d3d11_renderer->addTexture(tex3d1, 0);
		_d3d11_renderer->textures[_buffer_node_3d].uav = 0;
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavd;
		memset(&uavd, 0, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
		uavd.Format = DXGI_FORMAT_R32_UINT;
		uavd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
		uavd.Texture3D.MipSlice = 0;
		uavd.Texture3D.FirstWSlice = 0;
		uavd.Texture3D.WSize = -1;

		hr = _d3d11_renderer->device->CreateUnorderedAccessView(tex3d1, &uavd, &tex3d1_view);
		if (hr != S_OK)
			g_logger->debug(WIP_ERROR, "create %d texture3d view failed!", voxel_resolution);
		_d3d11_renderer->textures[_buffer_node_3d].uav = tex3d1_view;
	}

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
	float a[] = { 0, 0, 0, 0 };
	_ss = _d3d11_renderer->addSamplerState(Filter::BILINEAR_ANISO, AddressMode::WRAP, AddressMode::WRAP, AddressMode::WRAP, 0, 16, 0, a);
	_rs = _d3d11_renderer->addRasterizerState(CULL_NONE, SOLID, false, false, 0, 0);
	_rs_cull = _d3d11_renderer->addRasterizerState(CULL_BACK, SOLID, false, false, 0, 0);
	_rs_wireframe = _d3d11_renderer->addRasterizerState(CULL_NONE, WIREFRAME, false, false, 0, 0);

	_lights = _d3d11_renderer->addStructureBuffer<PointLight>(nlights, true);



	Image im1;
	im1.loadImage("res/qipan.png");
	_diffuse_tex = _d3d11_renderer->addTexture(im1);
	im1.free();

	Image img;
	img.loadImage("res/cloud/worley3d.dds");
	_texture_worley = _d3d11_renderer->addTexture(img);


}

void SVOFastApp::handle_input(f32 dt)
{
	_cam->control_update(dt);
}

void SVOFastApp::ter()
{
	_d3d11_renderer->finish();
	delete _d3d11_renderer;
}

void SVOFastApp::postter()
{

}


SVOFastApp::SVOFastApp(class AppBase* app) : FastApp(app)
{

}

SVOFastApp::~SVOFastApp()
{

}

void SVOFastApp::set_matrix(const RBMatrix& m, const RBMatrix& v, const RBMatrix& p)
{
	_d3d11_renderer->setShaderConstant4x4f("m", m);

	_d3d11_renderer->setShaderConstant4x4f("v", v);

	_d3d11_renderer->setShaderConstant4x4f("o", p);
}

void SVOFastApp::set_screen_size()
{
	_d3d11_renderer->setShaderConstant1i("screen_size_x", voxel_resolution);
	_d3d11_renderer->setShaderConstant1i("screen_size_y", voxel_resolution);
}

void SVOFastApp::init_svo_build()
{
	auto sh1 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/svo_flag_c.hlsl", "main", "cs_5_0");
	auto sh2 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/svo_alloc_c.hlsl", "main", "cs_5_0");
	auto sh3 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/svo_init_c.hlsl", "main_leaf", "cs_5_0");
	auto sh4 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/svo_init_c.hlsl", "main_inner", "cs_5_0");
	auto sh5 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/svo_update_param_c.hlsl", "main", "cs_5_0");
	auto sh6 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/svo_generate_indirect_buffer_c.hlsl", "main", "cs_5_0");
	auto sh7 = D3D11ShaderCompiler::compile(((FD3D11DynamicRHI*)_rhi)->GetDevice(), "shaders/voxel_list_to_vertex_buffer_c.hlsl", "main", "cs_5_0");


	HRESULT hr = _d3d11_renderer->device->CreateComputeShader(sh1->GetBufferPointer(), sh1->GetBufferSize(), NULL, &_cs_flag);
	if (hr != S_OK)
		g_logger->debug(WIP_ERROR, "create compute shader flag failed!");
	hr = _d3d11_renderer->device->CreateComputeShader(sh2->GetBufferPointer(), sh2->GetBufferSize(), NULL, &_cs_alloc);
	if (hr != S_OK)
		g_logger->debug(WIP_ERROR, "create compute shader alloc failed!");
	hr = _d3d11_renderer->device->CreateComputeShader(sh3->GetBufferPointer(), sh3->GetBufferSize(), NULL, &_cs_init_leaf);
	if (hr != S_OK)
		g_logger->debug(WIP_ERROR, "create compute shader init leaf failed!");
	hr = _d3d11_renderer->device->CreateComputeShader(sh4->GetBufferPointer(), sh4->GetBufferSize(), NULL, &_cs_init_inner);
	if (hr != S_OK)
		g_logger->debug(WIP_ERROR, "create compute shader init inner failed!");
	hr = _d3d11_renderer->device->CreateComputeShader(sh5->GetBufferPointer(), sh5->GetBufferSize(), NULL, &_cs_update);
	if (hr != S_OK)
		g_logger->debug(WIP_ERROR, "create compute shader update param failed!");
	hr = _d3d11_renderer->device->CreateComputeShader(sh6->GetBufferPointer(), sh6->GetBufferSize(), NULL, &_cs_generate_indirect);
	if (hr != S_OK)
		g_logger->debug(WIP_ERROR, "create compute shader _cs_generate_indirect failed!");
	hr = _d3d11_renderer->device->CreateComputeShader(sh7->GetBufferPointer(), sh7->GetBufferSize(), NULL, &_cs_generate_vertex);
	if (hr != S_OK)
		g_logger->debug(WIP_ERROR, "create compute shader _cs_generate_vertex failed!");

	_voxel_list = _d3d11_renderer->addStructureBuffer<VoxelListEnt>(128 * 1024 * 1024 / sizeof(VoxelListEnt));
	_node_pool = _d3d11_renderer->addStructureBuffer<uint>(256 * 1024 * 1024 / sizeof(uint));
	_info = _d3d11_renderer->addStructureBuffer<Param>(1);
	_total_voxel_num = _d3d11_renderer->addStructureBuffer<uint>(1);
	_gen_counter = _d3d11_renderer->addStructureBuffer<uint>(1);


	{
		ID3D11Texture3D* tex3d1;
		ID3D11UnorderedAccessView* tex3d1_view;
		D3D11_TEXTURE3D_DESC desc;
		desc.Width = 256;
		desc.Height = 256;
		desc.Depth = 256;
		desc.Format = DXGI_FORMAT_R32_UINT;
		desc.MipLevels = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		HRESULT hr = _d3d11_renderer->device->CreateTexture3D(&desc, 0, (ID3D11Texture3D **)&tex3d1);
		if (hr != S_OK)
			g_logger->debug(WIP_ERROR, "create %d texture3d failed!", voxel_resolution);
		_brick_pool = _d3d11_renderer->addTexture(tex3d1, 0);
		_d3d11_renderer->textures[_brick_pool].uav = 0;
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavd;
		memset(&uavd, 0, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
		uavd.Format = DXGI_FORMAT_R32_UINT;
		uavd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
		uavd.Texture3D.MipSlice = 0;
		uavd.Texture3D.FirstWSlice = 0;
		uavd.Texture3D.WSize = -1;

		hr = _d3d11_renderer->device->CreateUnorderedAccessView(tex3d1, &uavd, &tex3d1_view);
		if (hr != S_OK)
			g_logger->debug(WIP_ERROR, "create %d texture3d view failed!", voxel_resolution);
		_d3d11_renderer->textures[_brick_pool].uav = tex3d1_view;
	}
	{
		D3D11_BUFFER_DESC desc;
		rb_memzero(&desc, sizeof(desc));
		desc.ByteWidth = AlignArbitrary(sizeof(ComputeCB), 16);
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hr = _d3d11_renderer->device->CreateBuffer(&desc, 0, &_compute_buffer);
		if (hr != S_OK)
		{
			g_logger->debug(WIP_ERROR, "create compute constant buffer failed!");
		}
	}
		{
			D3D11_BUFFER_DESC desc;
			rb_memzero(&desc, sizeof(desc));
			desc.ByteWidth = AlignArbitrary(sizeof(RBMatrix), 16);
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			HRESULT hr = _d3d11_renderer->device->CreateBuffer(&desc, 0, &_CB_v);
			if (hr != S_OK)
			{
				g_logger->debug(WIP_ERROR, "create CB_v constant buffer failed!");
			}
		}
			{
				D3D11_BUFFER_DESC desc;
				rb_memzero(&desc, sizeof(desc));
				desc.ByteWidth = AlignArbitrary(sizeof(GSBuffer), 16);
				desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				desc.Usage = D3D11_USAGE_DYNAMIC;
				desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

				HRESULT hr = _d3d11_renderer->device->CreateBuffer(&desc, 0, &_CB_g);
				if (hr != S_OK)
				{
					g_logger->debug(WIP_ERROR, "create CB_g constant buffer failed!");
				}
			}

			{
				D3D11_BUFFER_DESC desc;
				rb_memzero(&desc, sizeof(desc));
				desc.ByteWidth = AlignArbitrary(sizeof(CB), 16);
				desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				desc.Usage = D3D11_USAGE_DYNAMIC;
				desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

				HRESULT hr = _d3d11_renderer->device->CreateBuffer(&desc, 0, &_CB_voxel_svo);
				if (hr != S_OK)
				{
					g_logger->debug(WIP_ERROR, "create CB_voxel_svo constant buffer failed!");
				}
			}




			  {
				  ID3D11Texture3D* tex;
				  D3D11_TEXTURE3D_DESC desc;
				  rb_memzero(&desc, sizeof(desc));
				  desc.Width = 256;
				  desc.Height = 256;
				  desc.Depth = 256;
				  desc.Format = DXGI_FORMAT_R32_UINT;
				  desc.MipLevels = 1;
				  desc.Usage = D3D11_USAGE_STAGING;
				  desc.BindFlags = 0;
				  desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
				  HRESULT res = _d3d11_renderer->device->CreateTexture3D(&desc, 0, &tex);
				  if (res != S_OK)
				  {
					  g_logger->debug(WIP_ERROR, "Couldn't create 3d staging texture!");
				  }

				  read_back_3d = _d3d11_renderer->addTexture(tex, 0);
			  }


}

void SVOFastApp::build_svo()
{
	RBMatrix mm;
	RBMatrix mp;
	RBMatrix mv;

	mm.set_identity();
	_cam->get_view_matrix(mv);
	_cam->get_perspective_matrix(mp);


	mp = RBMatrix::get_ortho(0, voxel_resolution, 0, voxel_resolution, _cam->get_near_panel(), _cam->get_far_panel());

	_d3d11_renderer->setViewport(voxel_resolution, voxel_resolution);
	_d3d11_renderer->changeToMainFramebuffer();

	_d3d11_renderer->setShader(_bound_shader);

	/*
	set_matrix(mm, mv, mp);
	set_screen_size();

	static float scale = 1;
	_d3d11_renderer->setShaderConstant1f("scale", scale);

	_d3d11_renderer->setTexture("diffuse_tex", _diffuse_tex);
	_d3d11_renderer->setSamplerState("tex_sampler", _ss);
	*/

#ifdef SIMPLEREAD
	_d3d11_renderer->setVertexFormat(_vf);
	_d3d11_renderer->setVertexBuffer(0, _vb);
	_d3d11_renderer->setIndexBuffer(_ib);

	_d3d11_renderer->apply(false);
	ID3D11UnorderedAccessView* uavs[] = { _d3d11_renderer->structureBuffers[_voxel_list].uav, _d3d11_renderer->structureBuffers[_total_voxel_num].uav };
	_d3d11_renderer->context->OMSetRenderTargetsAndUnorderedAccessViews(0, 0, 0, 1, 2, uavs, 0);

	do{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT res = _d3d11_renderer->context->Map(_CB_g, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (res != S_OK)
			g_logger->debug(WIP_ERROR, "Map resource _CB_g failed!");
		GSBuffer* m = (GSBuffer*)mappedResource.pData;
		m->v = mv;
		m->o = mp;
		m->scale = 1;
		m->screen_size_x = ww;
		m->screen_size_y = wh;
		_d3d11_renderer->context->Unmap(_CB_g, 0);
		_d3d11_renderer->context->GSSetConstantBuffers(0, 1, &_CB_g);
	} while (0);


	_d3d11_renderer->context->PSSetShaderResources(0, 1, &_d3d11_renderer->textures[_diffuse_tex].srv);
	_d3d11_renderer->context->PSSetSamplers(0, 1, &_d3d11_renderer->samplerStates[_ss].samplerState);
	do
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT res = _d3d11_renderer->context->Map(_CB_v, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (res != S_OK)
			g_logger->debug(WIP_ERROR, "Map resource _CB_v failed!");
		RBMatrix* m = (RBMatrix*)mappedResource.pData;
		*m = mm;
		_d3d11_renderer->context->Unmap(_CB_v, 0);
		_d3d11_renderer->context->VSSetConstantBuffers(0, 1, &_CB_v);
	} while (0);
	_d3d11_renderer->drawElements(Primitives::PRIM_TRIANGLES, 0, _idxs.size(), 0, 0);
#else


	for (auto& model : models)
	{
		_d3d11_renderer->setVertexFormat(_vf);
		_d3d11_renderer->setVertexBuffer(0, model.vb);
		_d3d11_renderer->setIndexBuffer(model.ib);

		_d3d11_renderer->apply(false);
		ID3D11UnorderedAccessView* uavs[] = { _d3d11_renderer->structureBuffers[_voxel_list].uav, _d3d11_renderer->structureBuffers[_total_voxel_num].uav };
		_d3d11_renderer->context->OMSetRenderTargetsAndUnorderedAccessViews(0, 0, 0, 1, 2, uavs, 0);
		do{
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			HRESULT res = _d3d11_renderer->context->Map(_CB_g, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			if (res != S_OK)
				g_logger->debug(WIP_ERROR, "Map resource _CB_g failed!");
			GSBuffer* m = (GSBuffer*)mappedResource.pData;
			m->v = mv;
			m->o = mp;
			m->scale = 1;
			m->screen_size_x = ww;
			m->screen_size_y = wh;
			_d3d11_renderer->context->Unmap(_CB_g, 0);
			_d3d11_renderer->context->GSSetConstantBuffers(0, 1, &_CB_g);
		} while (0);
		_d3d11_renderer->context->PSSetSamplers(0, 1, &_d3d11_renderer->samplerStates[_ss].samplerState);
		_d3d11_renderer->context->PSSetShaderResources(0, 1, &_d3d11_renderer->textures[material_lib[model.mat_id].map_diffuse].srv);
		
		do
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			HRESULT res = _d3d11_renderer->context->Map(_CB_v, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			if (res != S_OK)
				g_logger->debug(WIP_ERROR, "Map resource _CB_v failed!");
			RBMatrix* m = (RBMatrix*)mappedResource.pData;
			*m = model.model;
			_d3d11_renderer->context->Unmap(_CB_v, 0);
			_d3d11_renderer->context->VSSetConstantBuffers(0, 1, &_CB_v);
		} while (0);


		_d3d11_renderer->apply();
		_d3d11_renderer->drawElements(Primitives::PRIM_TRIANGLES, 0, model.idxs.size(), 0, 0);

	}
	for (auto& model : models_alpha)
	{

		_d3d11_renderer->setVertexFormat(_vf);


		_d3d11_renderer->apply(false);
		ID3D11UnorderedAccessView* uavs[] = { _d3d11_renderer->structureBuffers[_voxel_list].uav, _d3d11_renderer->structureBuffers[_total_voxel_num].uav };
		_d3d11_renderer->context->OMSetRenderTargetsAndUnorderedAccessViews(0, 0, 0, 1, 2, uavs, 0);
		_d3d11_renderer->context->PSSetSamplers(0, 1, &_d3d11_renderer->samplerStates[_ss].samplerState);
		do{
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			HRESULT res = _d3d11_renderer->context->Map(_CB_g, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			if (res != S_OK)
				g_logger->debug(WIP_ERROR, "Map resource _CB_g failed!");
			GSBuffer* m = (GSBuffer*)mappedResource.pData;
			m->v = mv;
			m->o = mp;
			m->scale = 1;
			m->screen_size_x = ww;
			m->screen_size_y = wh;
			_d3d11_renderer->context->Unmap(_CB_g, 0);
			_d3d11_renderer->context->GSSetConstantBuffers(0, 1, &_CB_g);
		} while (0);
		_d3d11_renderer->context->PSSetShaderResources(0, 1, &_d3d11_renderer->textures[material_lib[model.mat_id].map_diffuse].srv);

		do
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			HRESULT res = _d3d11_renderer->context->Map(_CB_v, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			if (res != S_OK)
				g_logger->debug(WIP_ERROR, "Map resource _CB_v failed!");
			RBMatrix* m = (RBMatrix*)mappedResource.pData;
			*m = model.model;
			_d3d11_renderer->context->Unmap(_CB_v, 0);
			_d3d11_renderer->context->VSSetConstantBuffers(0, 1, &_CB_v);
		} while (0);

		_d3d11_renderer->setVertexBuffer(0, model.vb);
		_d3d11_renderer->setIndexBuffer(model.ib);
		_d3d11_renderer->apply();
		_d3d11_renderer->drawElements(Primitives::PRIM_TRIANGLES, 0, model.idxs.size(), 0, 0);

	}
#endif

	//_d3d11_renderer->applyUAV(true);

	_d3d11_renderer->setViewport(ww, wh);
	_d3d11_renderer->changeToMainFramebuffer();
	_d3d11_renderer->setShader(_null_shader);
	_d3d11_renderer->apply();
//#define CHECK_UAV_RES


	uint total_voxels = 0;

	{
		uint * t = _d3d11_renderer->mapStructureBufferDiscard<uint>(_total_voxel_num, false);
		total_voxels = *t;
		tdn = total_voxels;
//#define CHECK_UAV_RES
#ifdef CHECK_UAV_RES
		g_logger->debug(WIP_INFO, "read:%d", *t);
#endif
		_d3d11_renderer->umapStructureBuffer(_total_voxel_num);


#ifdef CHECK_UAV_RES
		VoxelListEnt * vl = _d3d11_renderer->mapStructureBufferDiscard<VoxelListEnt>(_voxel_list, false);

		for (int j = 0; j < (*t); ++j)
		{
			g_logger->debug_log(WIP_INFO, "\npos:%f,%f,%f,%f,%f,%f",
				vl[j].pos.x, vl[j].pos.y, vl[j].pos.z,vl[j].color.x,vl[j].color.y,vl[j].color.z);
		}
		_d3d11_renderer->umapStructureBuffer(_voxel_list);


#endif
	}

	//indirect buffer
	{
		
			CD3D11_BUFFER_DESC desc(12, D3D11_BIND_UNORDERED_ACCESS,
			D3D11_USAGE_DEFAULT,
			0,
			D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS | D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS,
			4);
			_d3d11_renderer->device->CreateBuffer(&desc, 0, &_indirect_dispatch_buffer);
		//D3D11_BUFFER_UAV_FLAG_COUNTER
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc1;
		rb_memzero(&desc, sizeof(desc1));
		desc1.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		desc1.Buffer.FirstElement = 0;
		desc1.Format = DXGI_FORMAT_R32_TYPELESS;
		desc1.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
		desc1.Buffer.NumElements = 3;
		_d3d11_renderer->device->CreateUnorderedAccessView(_indirect_dispatch_buffer, &desc1, &_indirect_dispatch_buffer_uav);
	}
		{

			CD3D11_BUFFER_DESC desc(12, D3D11_BIND_UNORDERED_ACCESS,
				D3D11_USAGE_DEFAULT,
				0,
				D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS | D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS,
				4);
			_d3d11_renderer->device->CreateBuffer(&desc, 0, &_indirect_draw_buffer);
			//D3D11_BUFFER_UAV_FLAG_COUNTER
			D3D11_UNORDERED_ACCESS_VIEW_DESC desc1;
			rb_memzero(&desc, sizeof(desc1));
			desc1.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			desc1.Buffer.FirstElement = 0;
			desc1.Format = DXGI_FORMAT_R32_TYPELESS;
			desc1.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
			desc1.Buffer.NumElements = 3;
			_d3d11_renderer->device->CreateUnorderedAccessView(_indirect_draw_buffer, &desc1, &_indirect_draw_buffer_uav);
		}

			{
				CD3D11_BUFFER_DESC desc(sizeof(Vertex_PC) * total_voxels, D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_UNORDERED_ACCESS,
					D3D11_USAGE_DEFAULT,
					0,
					D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS,
					4);
				_d3d11_renderer->device->CreateBuffer(&desc, 0, &_indirect_primitives_buffer);
				//D3D11_BUFFER_UAV_FLAG_COUNTER
				D3D11_UNORDERED_ACCESS_VIEW_DESC desc1;
				rb_memzero(&desc, sizeof(desc1));
				desc1.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
				desc1.Buffer.FirstElement = 0;
				desc1.Format = DXGI_FORMAT_R32_TYPELESS;
				desc1.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
				desc1.Buffer.NumElements = sizeof(Vertex_PC) * total_voxels * 0.25;
				_d3d11_renderer->device->CreateUnorderedAccessView(_indirect_primitives_buffer, &desc1, &_indirect_primitives_buffer_uav);
			}
			{
				CD3D11_BUFFER_DESC desc(sizeof(Vertex_PC) * total_voxels, D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_UNORDERED_ACCESS,
					D3D11_USAGE_DEFAULT,
					0,
					D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS,
					4);
				_d3d11_renderer->device->CreateBuffer(&desc, 0, &_indirect_primitives_buffer1);
				//D3D11_BUFFER_UAV_FLAG_COUNTER
				D3D11_UNORDERED_ACCESS_VIEW_DESC desc1;
				rb_memzero(&desc, sizeof(desc1));
				desc1.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
				desc1.Buffer.FirstElement = 0;
				desc1.Format = DXGI_FORMAT_R32_TYPELESS;
				desc1.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
				desc1.Buffer.NumElements = sizeof(Vertex_PC) * total_voxels * 0.25;
				_d3d11_renderer->device->CreateUnorderedAccessView(_indirect_primitives_buffer1, &desc1, &_indirect_primitives_buffer_uav1);
			}
			{
					FormatDesc fd[2];
					fd[0].format = AttributeFormat::FORMAT_FLOAT;
					fd[0].size = 4;
					fd[0].stream = 0;
					fd[0].type = AttributeType::TYPE_GENERIC;
					fd[1].format = AttributeFormat::FORMAT_FLOAT;
					fd[1].size = 4;
					fd[1].stream = 0;
					fd[1].type = AttributeType::TYPE_VERTEX;
					_vf1_svo = _d3d11_renderer->addVertexFormat(fd, 2, _see_voxel_svo1);
					//_d3d11_renderer->device->CreateShaderResourceView(_indirect_primitives_buffer, 0, &_indirect_primitives_buffer_srv);
			}


	rb_memzero(&_compute_cb_cpu, sizeof(_compute_cb_cpu));
	_d3d11_renderer->context->CSSetShader(_cs_update, 0, 0);
	_d3d11_renderer->context->CSSetUnorderedAccessViews(1, 1, &_d3d11_renderer->structureBuffers[_info].uav, 0);
	_d3d11_renderer->context->CSSetUnorderedAccessViews(0, 1, &_d3d11_renderer->structureBuffers[_node_pool].uav, 0);

	_compute_cb_cpu.option = 0;
	update_compute_cb();

	_d3d11_renderer->context->CSSetConstantBuffers(0, 1, &_compute_buffer);
	_d3d11_renderer->context->Dispatch(1, 1, 1);



#ifdef CHECK_UAV_RES
	{
		Param * t = _d3d11_renderer->mapStructureBufferDiscard<Param>(_info, false);
		g_logger->debug_print(WIP_INFO, "\nParam:\n cur_node_pool_offset:%d\n cur_level_number:%d\n child_number:%d\n divided_number:%d\n brick_offset:%d \n brick_number:%d ",
			t->cur_node_pool_offset, t->cur_level_number, t->child_number, t->divided_number, t->brick_offset, t->brick_number
			);
		_d3d11_renderer->umapStructureBuffer(_info);

		uint * t1 = _d3d11_renderer->mapStructureBufferDiscard<uint>(_node_pool, false);
		g_logger->debug_print(WIP_INFO, "%d", *t1);
		_d3d11_renderer->umapStructureBuffer(_node_pool);
	}
#endif

	//1024/XXX
	static const int power_table2[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
	//512
	int vsize = voxel_resolution;
	int total_level = power_table2[1024 / vsize];
	_compute_cb_cpu.max_levels = total_level;
	_compute_cb_cpu.voxel_size = vsize;
	_compute_cb_cpu.voxel_list_size = total_voxels;// 128 * 1024 * 1024 / sizeof(VoxelListEnt);


	ID3D11UnorderedAccessView* null_uav[] = { nullptr, nullptr, nullptr, nullptr };
	ID3D11ShaderResourceView* null_srv[] = { nullptr, nullptr, nullptr };

	for (int lvl = 0; lvl <= total_level; ++lvl)
	{
		_compute_cb_cpu.cur_levels = lvl;
		update_compute_cb();
		//flag
		_d3d11_renderer->context->CSSetConstantBuffers(0, 1, &_compute_buffer);
		_d3d11_renderer->context->CSSetShader(_cs_flag, 0, 0);
		_d3d11_renderer->context->CSSetUnorderedAccessViews(1, 1, &_d3d11_renderer->structureBuffers[_info].uav, 0);
		_d3d11_renderer->context->CSSetUnorderedAccessViews(0, 1, &_d3d11_renderer->structureBuffers[_node_pool].uav, 0);
		_d3d11_renderer->context->CSSetUnorderedAccessViews(2, 1, &_d3d11_renderer->structureBuffers[_voxel_list].uav, 0);

		_d3d11_renderer->context->Dispatch(vsize, vsize, vsize);

		_d3d11_renderer->context->CSSetUnorderedAccessViews(0, 3, null_uav, 0);

		/*
		uint divided_number = 0;
		{
			Param * t = _d3d11_renderer->mapStructureBufferDiscard<Param>(_info, false);
			divided_number = t->divided_number;
			_d3d11_renderer->umapStructureBuffer(_info);
		}
		*/
		/*
				{
				Param * t = _d3d11_renderer->mapStructureBufferDiscard<Param>(_info, false);
				g_logger->debug(WIP_INFO, "\nParam:\n cur_node_pool_offset:%d\n cur_level_number:%d\n child_number:%d\n divided_number:%d\n brick_offset:%x \n brick_number:%d ",
				t->cur_node_pool_offset, t->cur_level_number, t->child_number, t->divided_number, t->brick_offset, t->brick_number
				);
				_d3d11_renderer->umapStructureBuffer(_info);


				uint * vl = _d3d11_renderer->mapStructureBufferDiscard<uint>(_node_pool, false);

				for (int j = 0; j < t->divided_number; ++j)
				{
				g_logger->debug(WIP_INFO, "\nnode_pool:%x",
				vl[j]);
				}
				_d3d11_renderer->umapStructureBuffer(_node_pool);
				}
				*/
		//alloc
		_d3d11_renderer->context->CSSetShader(_cs_alloc, 0, 0);
		_d3d11_renderer->context->CSSetUnorderedAccessViews(1, 1, &_d3d11_renderer->structureBuffers[_info].uav, 0);
		_d3d11_renderer->context->CSSetUnorderedAccessViews(0, 1, &_d3d11_renderer->structureBuffers[_node_pool].uav, 0);
		_d3d11_renderer->context->CSSetShaderResources(0, 1, &_d3d11_renderer->textures[_brick_pool].srv);

		_d3d11_renderer->context->Dispatch(vsize, vsize, vsize);

		_d3d11_renderer->context->CSSetUnorderedAccessViews(0, 2, null_uav, 0);
		_d3d11_renderer->context->CSSetShaderResources(0, 1, null_srv);


#ifdef CHECK_UAV_RES
		{
			Param * t = _d3d11_renderer->mapStructureBufferDiscard<Param>(_info, false);
			g_logger->debug(WIP_RAW, "\n total:%u , avaliable:%u \n",t->total_node,t->avaliable_node);
			_d3d11_renderer->umapStructureBuffer(_info);
		}
#endif


		//update offset
		_d3d11_renderer->context->CSSetShader(_cs_update, 0, 0);
		_d3d11_renderer->context->CSSetUnorderedAccessViews(1, 1, &_d3d11_renderer->structureBuffers[_info].uav, 0);
		_d3d11_renderer->context->CSSetUnorderedAccessViews(0, 1, &_d3d11_renderer->structureBuffers[_node_pool].uav, 0);

		_compute_cb_cpu.option = 2;
		update_compute_cb();

		_d3d11_renderer->context->CSSetConstantBuffers(0, 1, &_compute_buffer);
		_d3d11_renderer->context->Dispatch(1, 1, 1);

		_d3d11_renderer->context->CSSetUnorderedAccessViews(0, 2, null_uav, 0);

		
		uint child_number = 0;
		{
			Param * t = _d3d11_renderer->mapStructureBufferDiscard<Param>(_info, false);
			child_number = t->child_number;
			_d3d11_renderer->umapStructureBuffer(_info);
		}
		


		//init
		_d3d11_renderer->context->CSSetShader(_cs_init_inner, 0, 0);
		_d3d11_renderer->context->CSSetConstantBuffers(0, 1, &_compute_buffer);
		_d3d11_renderer->context->CSSetUnorderedAccessViews(0, 1, &_d3d11_renderer->textures[_brick_pool].uav, 0);
		_d3d11_renderer->context->CSSetUnorderedAccessViews(1, 1, &_d3d11_renderer->structureBuffers[_node_pool].uav, 0);
		_d3d11_renderer->context->CSSetUnorderedAccessViews(2, 1, &_d3d11_renderer->structureBuffers[_info].uav, 0);
		_d3d11_renderer->context->CSSetUnorderedAccessViews(3, 1, &_d3d11_renderer->structureBuffers[_voxel_list].uav, 0);
		uint Y = child_number / (vsize*vsize);
		_d3d11_renderer->context->Dispatch(512, Y+1, 512);
		_d3d11_renderer->context->CSSetUnorderedAccessViews(0, 4, null_uav, 0);

		/*
		#ifdef CHECK_UAV_RES
		{
		Param * t = _d3d11_renderer->mapStructureBufferDiscard<Param>(_info, false);
		g_logger->debug(WIP_INFO, "\nParam:\n cur_node_pool_offset:%d\n cur_level_number:%d\n child_number:%d\n divided_number:%d\n brick_offset:%d \n brick_number:%d ",
		t->cur_node_pool_offset, t->cur_level_number, t->child_number, t->divided_number, t->brick_offset, t->brick_number
		);
		_d3d11_renderer->umapStructureBuffer(_info);
		}
		#endif
		*/
#ifdef CHECK_UAV_RES
		{
			Param * t = _d3d11_renderer->mapStructureBufferDiscard<Param>(_info, false);
			g_logger->debug(WIP_INFO, "\nParam:\n cur_node_pool_offset:%d\n cur_level_number:%d\n child_number:%d\n divided_number:%d\n brick_offset:%x \n brick_number:%d ",
				t->cur_node_pool_offset, t->cur_level_number, t->child_number, t->divided_number, t->brick_offset, t->brick_number
				);
			_d3d11_renderer->umapStructureBuffer(_info);


			uint * vl = _d3d11_renderer->mapStructureBufferDiscard<uint>(_node_pool, false);

			for (int j = 0; j <= t->cur_node_pool_offset; ++j)
			{
				if (vl[j] != 0)
					g_logger->debug_log(WIP_RAW, "%d:%d|%d", j, vl[j] >> 31, vl[j] & 0x7fffffff);
			}
			_d3d11_renderer->umapStructureBuffer(_node_pool);
		}
#endif
		_d3d11_renderer->context->CSSetShader(_cs_update, 0, 0);
		_d3d11_renderer->context->CSSetUnorderedAccessViews(1, 1, &_d3d11_renderer->structureBuffers[_info].uav, 0);
		_d3d11_renderer->context->CSSetUnorderedAccessViews(0, 1, &_d3d11_renderer->structureBuffers[_node_pool].uav, 0);

		_compute_cb_cpu.option = 1;
		update_compute_cb();

		_d3d11_renderer->context->CSSetConstantBuffers(0, 1, &_compute_buffer);
		_d3d11_renderer->context->Dispatch(1, 1, 1);
		_d3d11_renderer->context->CSSetUnorderedAccessViews(0, 2, null_uav, 0);


	}

	//handle leaf node 
	do
	{
		/*
		uint dispatch_n = 0;
		{
			Param * t = _d3d11_renderer->mapStructureBufferDiscard<Param>(_info, false);
			g_logger->debug(WIP_INFO, "\nParam:\n cur_node_pool_offset:%d\n cur_level_number:%d\n child_number:%d\n divided_number:%d\n brick_offset:%x \n brick_number:%d ",
				t->cur_node_pool_offset, t->cur_level_number, t->child_number, t->divided_number, t->brick_offset, t->brick_number
				);
			dispatch_n =t->cur_level_number ;
			_d3d11_renderer->umapStructureBuffer(_info);
		}
		*/

		_compute_cb_cpu.cur_levels = 9;
		update_compute_cb();
		//init leaf
		_d3d11_renderer->context->CSSetConstantBuffers(0, 1, &_compute_buffer);
		_d3d11_renderer->context->CSSetShader(_cs_init_leaf, 0, 0);
		_d3d11_renderer->context->CSSetUnorderedAccessViews(0, 1, &_d3d11_renderer->textures[_brick_pool].uav, 0);
		_d3d11_renderer->context->CSSetUnorderedAccessViews(1, 1, &_d3d11_renderer->structureBuffers[_node_pool].uav, 0);
		_d3d11_renderer->context->CSSetUnorderedAccessViews(2, 1, &_d3d11_renderer->structureBuffers[_info].uav, 0);
		_d3d11_renderer->context->CSSetShaderResources(0, 1, &_d3d11_renderer->structureBuffers[_voxel_list].srv);

		_d3d11_renderer->context->Dispatch(voxel_resolution, voxel_resolution, voxel_resolution);

		_d3d11_renderer->context->CSSetUnorderedAccessViews(0, 3, null_uav, 0);
		_d3d11_renderer->context->CSSetShaderResources(0, 1, null_srv);


		
#ifdef CHECK_UAV_RES
		{
			Param * t = _d3d11_renderer->mapStructureBufferDiscard<Param>(_info, false);
			g_logger->debug(WIP_INFO, "\nParam:\n cur_node_pool_offset:%d\n cur_level_number:%d\n child_number:%d\n divided_number:%d\n brick_offset:%d \n brick_number:%d ",
				t->cur_node_pool_offset, t->cur_level_number, t->child_number, t->divided_number, t->brick_offset, t->brick_number
				);
			_d3d11_renderer->umapStructureBuffer(_info);


			uint * vl = _d3d11_renderer->mapStructureBufferDiscard<uint>(_node_pool, false);

			for (int j = 0; j <= t->cur_node_pool_offset; ++j)
			{
				g_logger->debug_log(WIP_RAW, "node_pool:%d:%d|%d", j, vl[j] >> 31, vl[j] & 0x7fffffff);
			}
			_d3d11_renderer->umapStructureBuffer(_node_pool);
		}
		read_back();
#endif

		
	} 
	while (0);

	//traverse svo to make a indirect vertex buffer for visulizing the svo

#ifndef VIS_SVO
	_d3d11_renderer->context->CSSetConstantBuffers(0, 1, &_compute_buffer);
	_d3d11_renderer->context->CSSetShader(_cs_generate_indirect, 0, 0);
	_d3d11_renderer->context->CSSetShaderResources(0, 1, &_d3d11_renderer->textures[_brick_pool].srv);
	_d3d11_renderer->context->CSSetShaderResources(1, 1, &_d3d11_renderer->structureBuffers[_node_pool].srv);
	_d3d11_renderer->context->CSSetUnorderedAccessViews(0, 1, &_indirect_primitives_buffer_uav, 0);
	_d3d11_renderer->context->CSSetUnorderedAccessViews(1, 1, &_d3d11_renderer->structureBuffers[_gen_counter].uav, 0);
	_d3d11_renderer->context->Dispatch(voxel_resolution,voxel_resolution,voxel_resolution);
	_d3d11_renderer->context->CSSetUnorderedAccessViews(0, 2, null_uav, 0);

	//gpu ref
	_d3d11_renderer->context->CSSetConstantBuffers(0, 1, &_compute_buffer);
	_d3d11_renderer->context->CSSetShader(_cs_generate_vertex, 0, 0);
	_d3d11_renderer->context->CSSetUnorderedAccessViews(0, 1, &_indirect_primitives_buffer_uav1, 0);
	_d3d11_renderer->context->CSSetUnorderedAccessViews(1, 1, &_d3d11_renderer->structureBuffers[_voxel_list].uav, 0);
	_d3d11_renderer->context->Dispatch(voxel_resolution, voxel_resolution, voxel_resolution);
	_d3d11_renderer->context->CSSetUnorderedAccessViews(0, 2, null_uav, 0);
#endif
	do
	{
		ID3D11Buffer* read_back_buffer;
		CD3D11_BUFFER_DESC desc(sizeof(Vertex_PC) * total_voxels, 0,
			D3D11_USAGE_STAGING,
			D3D11_CPU_ACCESS_READ,
			0,
			4);
		_d3d11_renderer->device->CreateBuffer(&desc, 0, &read_back_buffer);

		_d3d11_renderer->context->CopyResource(read_back_buffer,_indirect_primitives_buffer);

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT res = _d3d11_renderer->context->Map(read_back_buffer, 0, D3D11_MAP_READ, 0, &mappedResource);
		if (res != S_OK)
			g_logger->debug(WIP_ERROR, "Map resource _indirect_primitives_buffer failed!");
		Vertex_PC* m = (Vertex_PC*)mappedResource.pData;

		_d3d11_renderer->context->Unmap(read_back_buffer, 0);

	} while (0);
}


void SVOFastApp::update_compute_cb()
{
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mpr;

	hr = _d3d11_renderer->context->Map(_compute_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mpr);
	if ((hr) != S_OK)
	{
		g_logger->debug(WIP_ERROR, "map compute constant buffer failed!");
	}
	memcpy(mpr.pData, &_compute_cb_cpu, sizeof(_compute_cb_cpu));
	_d3d11_renderer->context->Unmap(_compute_buffer, 0);

}

void SVOFastApp::read_back()
{
	std::vector<uint> v;
	v.resize(256  * 256*256);
	// Copy the data to a staging resource.
	uint32 Subresource = 0;
	D3D11_BOX	Rect;
	Rect.left = 0;
	Rect.top = 0;
	Rect.right = 256;
	Rect.bottom = 256;
	Rect.back = 256;
	Rect.front = 0;
	_d3d11_renderer->context->CopySubresourceRegion(_d3d11_renderer->textures[read_back_3d].texture, 0, 0, 0, 0, _d3d11_renderer->textures[_brick_pool].texture, Subresource, &Rect);
	// Lock the staging resource.
	D3D11_MAPPED_SUBRESOURCE LockedRect;
	HRESULT hs = (_d3d11_renderer->context->Map(_d3d11_renderer->textures[read_back_3d].texture, 0, D3D11_MAP_READ, 0, &LockedRect));
	CHECKF(hs == S_OK, "read tex3d failed!");
	memcpy(&v[0], LockedRect.pData, v.size()*sizeof(uint));
	for (int i = 0; i < v.size();++i)
	{
		if (v[i]!=0)
		g_logger->debug_log(WIP_RAW, "brick:%d|%u", i,v[i]);
	}
	_d3d11_renderer->context->Unmap(_d3d11_renderer->textures[read_back_3d].texture, 0);
}

void SVOFastApp::voxelize_with_svo()
{

}

void SVOFastApp::voxelize_without_svo()
{

	RBMatrix mm;
	RBMatrix mv;

	mm.set_identity();
	_cam->get_view_matrix(mv);
	RBMatrix mp = RBMatrix::get_ortho(0, voxel_resolution, 0, voxel_resolution, _cam->get_near_panel(), _cam->get_far_panel());

	_d3d11_renderer->setViewport(voxel_resolution, voxel_resolution);
	_d3d11_renderer->changeToMainFramebuffer();

	_d3d11_renderer->setShader(_bound_shader);

	_d3d11_renderer->context->PSSetShaderResources(0, 1, &_d3d11_renderer->textures[_diffuse_tex].srv);
	_d3d11_renderer->context->PSSetSamplers(0, 1, &_d3d11_renderer->samplerStates[_ss].samplerState);


	//_d3d11_renderer->setUAV("voxel_volum", _buffer_node_3d);

	_d3d11_renderer->context->OMSetRenderTargetsAndUnorderedAccessViews(0, 0, 0, 1, 1, &_d3d11_renderer->textures[_buffer_node_3d].uav, 0);



	_d3d11_renderer->setVertexFormat(_vf);
	_d3d11_renderer->setVertexBuffer(0, _vb);
	_d3d11_renderer->setIndexBuffer(_ib);

	_d3d11_renderer->apply(false);

	do
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT res = _d3d11_renderer->context->Map(_CB_v, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (res != S_OK)
			g_logger->debug(WIP_ERROR, "Map resource _CB_v failed!");
		RBMatrix* m = (RBMatrix*)mappedResource.pData;
		*m = mm;
		_d3d11_renderer->context->Unmap(_CB_v, 0);
		_d3d11_renderer->context->VSSetConstantBuffers(0, 1, &_CB_v);
	} while (0);
	do{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT res = _d3d11_renderer->context->Map(_CB_g, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (res != S_OK)
			g_logger->debug(WIP_ERROR, "Map resource _CB_g failed!");
		GSBuffer* m = (GSBuffer*)mappedResource.pData;
		m->v = mv;
		m->o = mp;
		m->scale = 1;
		m->screen_size_x = ww;
		m->screen_size_y = wh;
		_d3d11_renderer->context->Unmap(_CB_g, 0);
		_d3d11_renderer->context->GSSetConstantBuffers(0, 1, &_CB_g);
	} while (0);
	//_d3d11_renderer->applyUAV();

	_d3d11_renderer->drawElements(Primitives::PRIM_TRIANGLES, 0, _idxs.size(), 0, 0);

	//_d3d11_renderer->applyUAV(true);

	_d3d11_renderer->setViewport(ww, wh);
	_d3d11_renderer->changeToMainFramebuffer();
	_d3d11_renderer->setShader(_null_shader);
	_d3d11_renderer->apply();
}