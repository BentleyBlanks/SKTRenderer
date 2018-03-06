#pragma once


typedef int TextureID;
typedef int ShaderID;
typedef int VertexBufferID;
typedef int IndexBufferID;
typedef int VertexFormatID;
typedef int SamplerStateID;
typedef int BlendStateID;
typedef int DepthStateID;
typedef int RasterizerStateID;
typedef int FontID;
typedef int StructureBufferID;

/*
struct Texture;
struct Shader;
struct VertexBuffer;
struct IndexBuffer;
struct VertexFormat;
struct SamplerState;
struct BlendState;
struct DepthState;
struct RasterizerState;
*/





enum ConstantType {
  CONSTANT_FLOAT,
  CONSTANT_VEC2,
  CONSTANT_VEC3,
  CONSTANT_VEC4,
  CONSTANT_INT,
  CONSTANT_IVEC2,
  CONSTANT_IVEC3,
  CONSTANT_IVEC4,
  CONSTANT_BOOL,
  CONSTANT_BVEC2,
  CONSTANT_BVEC3,
  CONSTANT_BVEC4,
  CONSTANT_MAT2,
  CONSTANT_MAT3,
  CONSTANT_MAT4,

  CONSTANT_TYPE_COUNT
};

extern int constantTypeSizes[CONSTANT_TYPE_COUNT];

enum Filter {
  NEAREST,
  LINEAR,
  BILINEAR,
  TRILINEAR,
  BILINEAR_ANISO,
  TRILINEAR_ANISO,
};

enum AddressMode {
  WRAP,
  CLAMP,
  BORDER,
};

inline bool hasMipmaps(const Filter filter){ return (filter >= BILINEAR); }
inline bool hasAniso(const Filter filter){ return (filter >= BILINEAR_ANISO); }

struct Character {
  float x0, y0;
  float x1, y1;
  float ratio;
};

struct TexFont {
  Character chars[256];
  TextureID texture;
};

struct TexVertex {
  TexVertex(const RBVector2 p, const RBVector2 t){
    position = p;
    texCoord = t;
  }
  RBVector2 position;
  RBVector2 texCoord;
};

#define MAKEQUAD(x0, y0, x1, y1, o)\
	RBVector2(x0 + o, y0 + o),\
	RBVector2(x0 + o, y1 - o),\
	RBVector2(x1 - o, y0 + o),\
	RBVector2(x1 - o, y1 - o),

#define MAKETEXQUAD(x0, y0, x1, y1, o)\
	TexVertex(RBVector2(x0 + o, y0 + o), RBVector2(0, 0)),\
	TexVertex(RBVector2(x0 + o, y1 - o), RBVector2(0, 1)),\
	TexVertex(RBVector2(x1 - o, y0 + o), RBVector2(1, 0)),\
	TexVertex(RBVector2(x1 - o, y1 - o), RBVector2(1, 1)),

#define MAKERECT(x0, y0, x1, y1, lw)\
	RBVector2(x0, y0),\
	RBVector2(x0 + lw, y0 + lw),\
	RBVector2(x1, y0),\
	RBVector2(x1 - lw, y0 + lw),\
	RBVector2(x1, y1),\
	RBVector2(x1 - lw, y1 - lw),\
	RBVector2(x0, y1),\
	RBVector2(x0 + lw, y1 - lw),\
	RBVector2(x0, y0),\
	RBVector2(x0 + lw, y0 + lw),


#define TEXTURE_NONE  (-1)
#define SHADER_NONE   (-1)
#define BLENDING_NONE (-1)
#define VF_NONE   (-1)
#define VB_NONE   (-1)
#define IB_NONE   (-1)
#define SS_NONE   (-1)
#define BS_NONE   (-1)
#define DS_NONE   (-1)
#define RS_NONE   (-1)
#define SB_NONE   (-1)
#define FONT_NONE (-1)

#define FB_COLOR (-2)
#define FB_DEPTH (-2)
#define NO_SLICE (-1)
#define NO_UAV (-1)

#define DONTCARE (-2)

// Texture flags
#define CUBEMAP       0x1
#define HALF_FLOAT    0x2
#define SRGB          0x4
#define SAMPLE_DEPTH  0x8
#define SAMPLE_SLICES 0x10
#define RENDER_SLICES 0x20
#define USE_MIPGEN    0x40
#define READ_BACK 0x80

// Shader flags
#define ASSEMBLY 0x1

// Mask constants
#define RED   0x1
#define GREEN 0x2
#define BLUE  0x4
#define ALPHA 0x8

#define ALL (RED | GREEN | BLUE | ALPHA)
#define NONE 0


// reset() flags
#define RESET_ALL    0xFFFF
#define RESET_SHADER 0x1
#define RESET_VF     0x2
#define RESET_VB     0x4
#define RESET_IB     0x8
#define RESET_DS     0x10
#define RESET_BS     0x20
#define RESET_RS     0x40
#define RESET_SS     0x80
#define RESET_TEX    0x100
#define RESET_SB      0x200

enum BufferAccess {
  STATIC,
  DEFAULT,
  DYNAMIC,
};

enum Primitives {
  PRIM_TRIANGLES = 0,
  PRIM_TRIANGLE_FAN = 1,
  PRIM_TRIANGLE_STRIP = 2,
  PRIM_QUADS = 3,
  PRIM_LINES = 4,
  PRIM_LINE_STRIP = 5,
  PRIM_LINE_LOOP = 6,
  PRIM_POINTS = 7,
};

enum AttributeType {
  TYPE_GENERIC = 0,
  TYPE_VERTEX = 1,
  TYPE_TEXCOORD = 2,
  TYPE_NORMAL = 3,
  TYPE_TANGENT = 4,
  TYPE_BINORMAL = 5,
};

enum AttributeFormat {
  FORMAT_FLOAT = 0,
  FORMAT_HALF = 1,
  FORMAT_UBYTE = 2,
};

struct FormatDesc {
  int stream;
  AttributeType type;
  AttributeFormat format;
  //component count
  int size;
};

#define MAX_MRTS 8
#define MAX_VERTEXSTREAM 8
#define MAX_TEXTUREUNIT  16
#define MAX_SAMPLERSTATE 16

// Blending constants
extern const int ZERO;
extern const int ONE;
extern const int SRC_COLOR;
extern const int ONE_MINUS_SRC_COLOR;
extern const int DST_COLOR;
extern const int ONE_MINUS_DST_COLOR;
extern const int SRC_ALPHA;
extern const int ONE_MINUS_SRC_ALPHA;
extern const int DST_ALPHA;
extern const int ONE_MINUS_DST_ALPHA;
extern const int SRC_ALPHA_SATURATE;

extern const int BM_ADD;
extern const int BM_SUBTRACT;
extern const int BM_REVERSE_SUBTRACT;
extern const int BM_MIN;
extern const int BM_MAX;

// Depth-test constants
extern const int NEVER;
extern const int LESS;
extern const int EQUAL;
extern const int LEQUAL;
extern const int GREATER;
extern const int NOTEQUAL;
extern const int GEQUAL;
extern const int ALWAYS;

// Stencil-test constants
extern const int KEEP;
extern const int SET_ZERO;
extern const int REPLACE;
extern const int INVERT;
extern const int INCR;
extern const int DECR;
extern const int INCR_SAT;
extern const int DECR_SAT;

// Culling constants
extern const int CULL_NONE;
extern const int CULL_BACK;
extern const int CULL_FRONT;

// Fillmode constants
extern const int SOLID;
extern const int WIREFRAME;


#ifdef FAST_RHI_D3D11
#include <d3d11.h>
#include "RBBasedata.h"

struct Texture
{
  ID3D11Resource *texture;
  ID3D11ShaderResourceView *srv;
  ID3D11RenderTargetView   *rtv;
  ID3D11DepthStencilView   *dsv;
  ID3D11UnorderedAccessView *uav;
  ID3D11ShaderResourceView **srvArray;
  ID3D11RenderTargetView   **rtvArray;
  ID3D11DepthStencilView   **dsvArray;
  DXGI_FORMAT texFormat;
  DXGI_FORMAT srvFormat;
  DXGI_FORMAT rtvFormat;
  DXGI_FORMAT dsvFormat;
  int width, height, depth;
  int arraySize;
  uint flags;
};

struct StructureBuffer
{
	int nElements;
	ID3D11UnorderedAccessView* uav;
	ID3D11ShaderResourceView* srv;
	ID3D11Buffer* buffer;
};

struct Constant
{
  char *name;
  u8 *vsData;
  u8 *gsData;
  u8 *psData;
  int vsBuffer;
  int gsBuffer;
  int psBuffer;
};

struct Sampler
{
  char *name;
  int vsIndex;
  int gsIndex;
  int psIndex;
};

struct Shader
{
  ID3D11VertexShader *vertexShader;
  ID3D11PixelShader *pixelShader;
  ID3D11GeometryShader *geometryShader;
  ID3DBlob *inputSignature;

  ID3D11Buffer **vsConstants;
  ID3D11Buffer **gsConstants;
  ID3D11Buffer **psConstants;
  uint nVSCBuffers;
  uint nGSCBuffers;
  uint nPSCBuffers;

  Constant *constants;
  Sampler *textures;
  Sampler *samplers;
  Sampler *uavs;

  uint nConstants;
  uint nTextures;
  uint nSamplers;
  uint nUAVs;

  u8 **vsConstMem;
  u8 **gsConstMem;
  u8 **psConstMem;

  bool *vsDirty;
  bool *gsDirty;
  bool *psDirty;
};

struct ShaderCompute
{
	ID3D11ComputeShader* computeShader;
};

struct VertexFormat
{
  ID3D11InputLayout *inputLayout;
  uint vertexSize[MAX_VERTEXSTREAM];
};

struct VertexBuffer
{
  ID3D11Buffer *vertexBuffer;
  long size;
};

struct IndexBuffer
{
  ID3D11Buffer *indexBuffer;
  uint nIndices;
  uint indexSize;
};

struct SamplerState
{
  ID3D11SamplerState *samplerState;
};

struct BlendState
{
  ID3D11BlendState *blendState;
};

struct DepthState
{
  ID3D11DepthStencilState *dsState;
};

struct RasterizerState
{
  ID3D11RasterizerState *rsState;
};



#else

#endif