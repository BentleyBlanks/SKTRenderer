#pragma once

#include "FastApp.h"
#include "RHID3D11.h"
#include "D3DCompiler.h"
#include <vector>
#include "Vector3.h"
#include "Vector2.h"
#include "Matrix.h"
#include "D3D11FastRenderer.h"


using std::vector;
#define PAD16(x) (((x)+15)/16*16)

class CRFastApp : public FastApp
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
	virtual bool preinit() override;
	virtual bool init() override;
	virtual void update(f32 dt) override;
	virtual void draw() override;
	virtual void ter() override;
	virtual void postter() override;

	CRFastApp(class AppBase* app);
	virtual ~CRFastApp();
	CRFastApp(const CRFastApp& o) = delete;
	CRFastApp& operator=(const CRFastApp& o) = delete;


protected:
	void load_models_generate_buffer(const char* filename);
	void load_assets();
	void create_rhires();
	void set_matrix(const RBMatrix& m, const RBMatrix& v, const RBMatrix& p);
	void set_screen_size();
	void handle_input(f32 dt);

private:
	std::vector<Vertex_PNT> _vecs;
	std::vector<u32> _idxs;

	VertexBufferID _vb;
	IndexBufferID _ib;

  ShaderID _bound_shader;
	
	DepthStateID _ds_enable;
	DepthStateID _ds_disable;

	BlendStateID _bs_enable;
	BlendStateID _bs_disable;

	SamplerStateID _ss;

	RasterizerStateID _rs;

	TextureID _buffer_depth_stencil;
  VertexFormatID _vf;

	class RBCamera* _cam;

	int dx;

	FD3D11DynamicRHI* _rhi;
	Direct3D11Renderer* _d3d11_renderer;
};
