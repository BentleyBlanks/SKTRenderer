#pragma once
#include "FastApp.h"
#include "RHID3D11.h"
#include "D3D11FastRenderer.h"

class SVOFastApp : public FastApp
{
public:

  struct Vertex_PNT
  {
    RBVector3 pos;
    RBVector3 normal;
    RBVector2 texcoord;
  };

  struct Vertex_PT
  {
    RBVector3 pos;
    RBVector2 texcoord;
  };

  struct Vertex_PC
  {
	  RBColorf color;
	  RBVector4 pos;
  };

  struct MatrixBuffer
  {
    RBMatrix m;
    RBMatrix v;
    RBMatrix p;
  };
  struct ScreenSize
  {
    uint32 w; uint32 h;
    uint32 s; uint32 pad2;
    RBMatrix inv_projection;
  };
  struct ComputeCB
  {
	  int cur_levels;
	  int voxel_size;
	  int voxel_list_size;
	  int max_levels;
	  uint option;
  };

  struct CB
  {
	  RBMatrix inv_mv_mat;
	  float view_dist;
	  float tan_fov_x;
	  float tan_fov_y;
	  int voxel_size;
  };

  struct PointLight
  {
	  RBVector3 pos_view;
	  f32 start;
	  RBVector3 color;
	  f32 end;
	  RBVector4 attenuation_param;
  };

  typedef int MaterialID;
  struct Material
  {
	  std::string name;
	  f32 ns;
	  f32 ni;
	  f32 d;
	  f32 tr;
	  RBVector3 tf;
	  f32 illum;
	  RBVector3 ka;
	  RBVector3 kd;
	  RBVector3 ks;
	  RBVector3 ke;
	  TextureID map_metallic;
	  TextureID map_diffuse;
	  TextureID map_roughness;
	  TextureID map_normal;
	  TextureID map_mask;
	  Material()
	  {
		  name = "";
		  ns = ni = d = tr = illum = 0.f;
		  tf = ka = kd = ks = ke = RBVector3::zero_vector;
		  map_mask = map_diffuse = map_metallic = map_roughness = map_normal = -1;
	  }
  };

  std::vector<Material> material_lib;

  struct MeshModel
  {
	  std::vector<Vertex_PNT> vecs;
	  std::vector<u32> idxs;
	  MaterialID mat_id;
	  VertexBufferID vb;
	  IndexBufferID ib;
	  RBMatrix model;
	  MeshModel()
	  {
		  model = RBMatrix::identity;
		  mat_id = -1;
		  vb = ib = -1;
	  }
  };

  std::vector<MeshModel> models;
  //todo:change shader
  std::vector<MeshModel> models_alpha;

  virtual bool preinit() override;
  virtual bool init() override;
  virtual void update(f32 dt) override;
  virtual void draw() override;
  virtual void ter() override;
  virtual void postter() override;

  SVOFastApp(class AppBase* app);
  virtual ~SVOFastApp();
  SVOFastApp(const SVOFastApp& o) = delete;
  SVOFastApp& operator=(const SVOFastApp& o) = delete;


protected:
  void load_models_generate_buffer(const char* filename);
  void load_assets();
  void create_rhires();
  void handle_input(f32 dt);
  void set_matrix(const RBMatrix& m, const RBMatrix& v, const RBMatrix& p);
  void set_screen_size();
  void init_svo_build();
  void build_svo();
  void update_compute_cb();
  void voxelize_with_svo();
  void voxelize_without_svo();
  void read_back();
  void render_scene();
  void setup_lights(int n);

private:
  std::vector<Vertex_PNT> _vecs;
  std::vector<u32> _idxs;
  std::vector<PointLight> lights;
  VertexBufferID _vb;
  IndexBufferID _ib;

  ShaderID _bound_shader;
  ShaderID _null_shader;
  ShaderID _see_voxel;
  ShaderID _see_voxel1;
  ShaderID _see_voxel_svo;
  ShaderID _see_voxel_svo1;

  ShaderID _pbr_render_shader;

  ComputeCB _compute_cb_cpu;
  struct VoxelListEnt
  {
	  RBVector4 color;
	  RBVector3 pos;
  };

  struct GSBuffer
  {
	  RBMatrix v;
	  RBMatrix o;
	  int screen_size_x;
	  int screen_size_y;
	  float scale;
  };

  struct Param
  {
	  //µ±Ç°²ãµÄ½ÚµãÆ«ÒÆ
	  int cur_node_pool_offset;
	  //µ±Ç°²ãµÄ½ÚµãÊýÁ¿
	  uint cur_level_number;
	  //
	  uint child_number;
	  //
	  uint divided_number;
	  //
	  uint brick_offset;
	  //
	  uint brick_number;
	  //debug
	  uint total_node;
	  uint avaliable_node;
  };
  /*
  512x512x512 voxel volumn
use voxels format 4 byte color 3 byte pos
  if no svo should be 2G VRAM
  */
  ID3D11ComputeShader* _cs_flag;
  ID3D11ComputeShader* _cs_alloc;
  ID3D11ComputeShader* _cs_init_leaf;
  ID3D11ComputeShader* _cs_init_inner;
  ID3D11ComputeShader* _cs_update;
  ID3D11ComputeShader* _cs_generate_indirect;
  ID3D11ComputeShader* _cs_generate_vertex;

  //128M RW 
  StructureBufferID _voxel_list;
  //256M RW 
  StructureBufferID _node_pool;
  //only 1 element RW
  StructureBufferID _info;
  //256x256x256 64M RW
  TextureID _brick_pool;

  StructureBufferID _total_voxel_num;
  StructureBufferID _gen_counter;

  ID3D11Buffer* _compute_buffer;

  ID3D11Buffer* _CB_v;
  ID3D11Buffer* _CB_g;
  ID3D11Buffer* _CB_voxel_svo;
  ID3D11Buffer* _indirect_dispatch_buffer;
  ID3D11UnorderedAccessView* _indirect_dispatch_buffer_uav;
  ID3D11Buffer* _indirect_draw_buffer;
  ID3D11UnorderedAccessView* _indirect_draw_buffer_uav;

  DepthStateID _ds_enable;
  DepthStateID _ds_disable;

  BlendStateID _bs_enable;
  BlendStateID _bs_disable;

  SamplerStateID _ss;


  RasterizerStateID _rs;
  RasterizerStateID _rs_wireframe;
  RasterizerStateID _rs_cull;



  VertexFormatID _vf;
  VertexFormatID _vf1;

  TextureID _buffer_node_3d;
  TextureID _buffer_indirect_node_3d;
  TextureID _diffuse_tex;
  TextureID _texture_worley;

  TextureID read_back_3d;

  ID3D11Buffer* _indirect_primitives_buffer;
  ID3D11UnorderedAccessView* _indirect_primitives_buffer_uav;
  ID3D11ShaderResourceView* _indirect_primitives_buffer_srv;
  VertexFormatID _vf1_svo;
  //gpu ref
  ID3D11Buffer* _indirect_primitives_buffer1;
  ID3D11UnorderedAccessView* _indirect_primitives_buffer_uav1;
  ID3D11ShaderResourceView* _indirect_primitives_buffer_srv1;

  int voxel_resolution;
  int tdn;

  StructureBufferID _lights;
  int nlights;
  TextureID _buffer_depth_stencil;
  class RBCamera* _cam;
  FD3D11DynamicRHI* _rhi;
  Direct3D11Renderer* _d3d11_renderer;
};
