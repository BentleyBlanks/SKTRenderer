#pragma once
#include "d3d11.h"
#include "RBMath/Inc/RBMath.h"
#include "AABB.h"
#include "Matrix.h"
#include "stdio.h"
#include "Assertion.h"
#include "Util.h"
#include <map>

/// Memory obtained by calling `bgfx::alloc`, `bgfx::copy`, or `bgfx::makeRef`.
///
/// @attention C99 equivalent is `bgfx_memory_t`.
///
struct Memory
{
  uint8_t* data;
  uint32_t size;
};

#define BGFX_CONFIG_MAX_FRAME_BUFFER_ATTACHMENTS 4
#define BGFX_CONFIG_MAX_OCCUSION_QUERIES 4

static const u16 invalidHandle = MAX_U16;
#define RES_HANDLE(_name) \
			struct _name { u16 idx; }; \
			inline bool is_valid(_name _handle) { return invalidHandle != _handle.idx; }
#define RES_INVALID_HANDLE { invalidHandle }

RES_HANDLE(DynamicIndexBufferHandle);
RES_HANDLE(DynamicVertexBufferHandle);
RES_HANDLE(FrameBufferHandle);
RES_HANDLE(IndexBufferHandle);
RES_HANDLE(IndirectBufferHandle);
RES_HANDLE(OcclusionQueryHandle);
RES_HANDLE(ProgramHandle);
RES_HANDLE(ShaderHandle);
RES_HANDLE(TextureHandle);
RES_HANDLE(UniformHandle);
RES_HANDLE(VertexBufferHandle);
RES_HANDLE(VertexDeclHandle);

struct PlatformData;

#define BX_MAKEFOURCC(_a, _b, _c, _d) ( ( (uint32)(_a) | ( (uint32)(_b) << 8) | ( (uint32)(_c) << 16) | ( (uint32)(_d) << 24) ) )
#define BGFX_CHUNK_MAGIC_CSH BX_MAKEFOURCC('C', 'S', 'H', 0x2)
#define BGFX_CHUNK_MAGIC_FSH BX_MAKEFOURCC('F', 'S', 'H', 0x4)
#define BGFX_CHUNK_MAGIC_TEX BX_MAKEFOURCC('T', 'E', 'X', 0x0)
#define BGFX_CHUNK_MAGIC_VSH BX_MAKEFOURCC('V', 'S', 'H', 0x4)

struct TextureCreate
{
  TextureFormat::Enum m_format;
  uint16 m_width;
  uint16 m_height;
  uint16 m_depth;
  uint16 m_numLayers;
  uint8 m_numMips;
  bool m_cubeMap;
  const Memory* m_mem;
};

/// Texture format enum.
///
/// Notation:
///
///       RGBA16S
///       ^   ^ ^
///       |   | +-- [ ]Unorm
///       |   |     [F]loat
///       |   |     [S]norm
///       |   |     [I]nt
///       |   |     [U]int
///       |   +---- Number of bits per component
///       +-------- Components
///
/// @attention Availability depends on Caps (see: formats).
///
/// @attention C99 equivalent is `bgfx_texture_format_t`.
///
struct TextureFormat
{
  /// Texture formats:
  enum Enum
  {
    BC1,          //!< DXT1
    BC2,          //!< DXT3
    BC3,          //!< DXT5
    BC4,          //!< LATC1/ATI1
    BC5,          //!< LATC2/ATI2
    BC6H,         //!< BC6H
    BC7,          //!< BC7
    ETC1,         //!< ETC1 RGB8
    ETC2,         //!< ETC2 RGB8
    ETC2A,        //!< ETC2 RGBA8
    ETC2A1,       //!< ETC2 RGB8A1
    PTC12,        //!< PVRTC1 RGB 2BPP
    PTC14,        //!< PVRTC1 RGB 4BPP
    PTC12A,       //!< PVRTC1 RGBA 2BPP
    PTC14A,       //!< PVRTC1 RGBA 4BPP
    PTC22,        //!< PVRTC2 RGBA 2BPP
    PTC24,        //!< PVRTC2 RGBA 4BPP

    Unknown,      // Compressed formats above.

    R1,
    A8,
    R8,
    R8I,
    R8U,
    R8S,
    R16,
    R16I,
    R16U,
    R16F,
    R16S,
    R32I,
    R32U,
    R32F,
    RG8,
    RG8I,
    RG8U,
    RG8S,
    RG16,
    RG16I,
    RG16U,
    RG16F,
    RG16S,
    RG32I,
    RG32U,
    RG32F,
    RGB8,
    RGB8I,
    RGB8U,
    RGB8S,
    RGB9E5F,
    BGRA8,
    RGBA8,
    RGBA8I,
    RGBA8U,
    RGBA8S,
    RGBA16,
    RGBA16I,
    RGBA16U,
    RGBA16F,
    RGBA16S,
    RGBA32I,
    RGBA32U,
    RGBA32F,
    R5G6B5,
    RGBA4,
    RGB5A1,
    RGB10A2,
    R11G11B10F,

    UnknownDepth, // Depth formats below.

    D16,
    D24,
    D24S8,
    D32,
    D16F,
    D24F,
    D32F,
    D0S8,

    Count
  };
};

/// Renderer backend type enum.
///
/// @attention C99 equivalent is `bgfx_renderer_type_t`.
///
struct RendererType
{
  /// Renderer types:
  enum Enum
  {
    Null,         //!< No rendering.
    Direct3D9,    //!< Direct3D 9.0
    Direct3D11,   //!< Direct3D 11.0
    Direct3D12,   //!< Direct3D 12.0
    Metal,        //!< Metal
    OpenGLES,     //!< OpenGL ES 2.0+
    OpenGL,       //!< OpenGL 2.1+
    Vulkan,       //!< Vulkan
    GNM,          //!< GNM

    Count
  };
};

/// Vertex attribute enum.
///
/// @attention C99 equivalent is `bgfx_attrib_t`.
///
struct Attrib
{
  /// Corresponds to vertex shader attribute. Attributes:
  enum Enum
  {
    Position,  //!< a_position
    Normal,    //!< a_normal
    Tangent,   //!< a_tangent
    Bitangent, //!< a_bitangent
    Color0,    //!< a_color0
    Color1,    //!< a_color1
    Indices,   //!< a_indices
    Weight,    //!< a_weight
    TexCoord0, //!< a_texcoord0
    TexCoord1, //!< a_texcoord1
    TexCoord2, //!< a_texcoord2
    TexCoord3, //!< a_texcoord3
    TexCoord4, //!< a_texcoord4
    TexCoord5, //!< a_texcoord5
    TexCoord6, //!< a_texcoord6
    TexCoord7, //!< a_texcoord7

    Count
  };
};

/// Vertex attribute type enum.
///
/// @attention C99 equivalent is `bgfx_attrib_type_t`.
///
struct AttribType
{
  /// Attribute types:
  enum Enum
  {
    Uint8,  //!< Uint8
    Uint10, //!< Uint10, availability depends on: `BGFX_CAPS_VERTEX_ATTRIB_UINT10`.
    Int16,  //!< Int16
    Half,   //!< Half, availability depends on: `BGFX_CAPS_VERTEX_ATTRIB_HALF`.
    Float,  //!< Float

    Count
  };
};

#pragma region UniformBuffer
/// Uniform type enum.
struct UniformType
{
  /// Uniform types:
  enum Enum
  {
    Int1,
    End,

    Vec4,
    Mat3,
    Mat4,

    Count
  };
};
#define CONSTANT_OPCODE_TYPE_SHIFT 27
#define CONSTANT_OPCODE_TYPE_MASK  UINT32_C(0xf8000000)
#define CONSTANT_OPCODE_LOC_SHIFT  11
#define CONSTANT_OPCODE_LOC_MASK   UINT32_C(0x07fff800)
#define CONSTANT_OPCODE_NUM_SHIFT  1
#define CONSTANT_OPCODE_NUM_MASK   UINT32_C(0x000007fe)
#define CONSTANT_OPCODE_COPY_SHIFT 0
#define CONSTANT_OPCODE_COPY_MASK  UINT32_C(0x00000001)

#define BGFX_UNIFORM_FRAGMENTBIT UINT8_C(0x10)
#define BGFX_UNIFORM_SAMPLERBIT  UINT8_C(0x20)
#define BGFX_UNIFORM_MASK (BGFX_UNIFORM_FRAGMENTBIT|BGFX_UNIFORM_SAMPLERBIT)

class UniformBuffer
{
public:
  static UniformBuffer* create(u32 _size = 1 << 20)
  {
    //uint32_t size = Align((RBMath::get_max(_size, (u32)sizeof(UniformBuffer))),16);
    void*    data = rb_align_malloc(RBMath::get_max(_size, (u32)sizeof(UniformBuffer)),16);
    return ::new(data)UniformBuffer(_size);
  }

  static void destroy(UniformBuffer* _uniformBuffer)
  {
    _uniformBuffer->~UniformBuffer();
    rb_align_free(_uniformBuffer);
  }

  static void update(UniformBuffer*& _uniformBuffer, u32 _treshold = 64 << 10, u32 _grow = 1 << 20)
  {
    if (_treshold >= _uniformBuffer->m_size - _uniformBuffer->m_pos)
    {
      void* data = rb_align_malloc(RBMath::get_max(_uniformBuffer->m_size + _grow, (u32)sizeof(UniformBuffer)),16);
      _uniformBuffer = reinterpret_cast<UniformBuffer*>(data);
      _uniformBuffer->m_size = RBMath::get_max(_uniformBuffer->m_size + _grow, (u32)sizeof(UniformBuffer));
    }
  }

  static u32 encodeOpcode(UniformType::Enum _type, u16 _loc, u16 _num, u16 _copy)
  {
    const u32 type = _type << CONSTANT_OPCODE_TYPE_SHIFT;
    const u32 loc = _loc << CONSTANT_OPCODE_LOC_SHIFT;
    const u32 num = _num << CONSTANT_OPCODE_NUM_SHIFT;
    const u32 copy = _copy << CONSTANT_OPCODE_COPY_SHIFT;
    return type | loc | num | copy;
  }

  static void decodeOpcode(u32 _opcode, UniformType::Enum& _type, u16& _loc, u16& _num, u16& _copy)
  {
    const u32 type = (_opcode&CONSTANT_OPCODE_TYPE_MASK) >> CONSTANT_OPCODE_TYPE_SHIFT;
    const u32 loc = (_opcode&CONSTANT_OPCODE_LOC_MASK) >> CONSTANT_OPCODE_LOC_SHIFT;
    const u32 num = (_opcode&CONSTANT_OPCODE_NUM_MASK) >> CONSTANT_OPCODE_NUM_SHIFT;
    const u32 copy = (_opcode&CONSTANT_OPCODE_COPY_MASK); // >> CONSTANT_OPCODE_COPY_SHIFT;

    _type = (UniformType::Enum)(type);
    _copy = (u16)copy;
    _num = (u16)num;
    _loc = (u16)loc;
  }

  void write(const void* _data, u32 _size)
  {
    CHECKF(m_pos + _size < m_size, "Write would go out of bounds. pos %d + size %d > max size: %d).", m_pos, _size, m_size);

    if (m_pos + _size < m_size)
    {
      memcpy(&m_buffer[m_pos], _data, _size);
      m_pos += _size;
    }
  }

  void write(u32 _value)
  {
    write(&_value, sizeof(u32));
  }

  const char* read(u32 _size)
  {
    CHECKF(m_pos < m_size, "Out of bounds %d (size: %d).", m_pos, m_size);
    const char* result = &m_buffer[m_pos];
    m_pos += _size;
    return result;
  }

  uint32_t read()
  {
    uint32_t result;
    memcpy(&result, read(sizeof(u32)), sizeof(u32));
    return result;
  }

  bool isEmpty() const
  {
    return 0 == m_pos;
  }

  u32 getPos() const
  {
    return m_pos;
  }

  void reset(u32 _pos = 0)
  {
    m_pos = _pos;
  }

  void finish()
  {
    write(UniformType::End);
    m_pos = 0;
  }

  void writeUniform(UniformType::Enum _type, u16 _loc, const void* _value, u16 _num = 1);
  void writeUniformHandle(UniformType::Enum _type, u16 _loc, UniformHandle _handle, u16 _num = 1);
  void writeMarker(const char* _marker);

private:
  UniformBuffer(u32 _size)
    : m_size(_size - sizeof(m_buffer))
    , m_pos(0)
  {
    finish();
  }

  ~UniformBuffer()
  {
  }

  u32 m_size;
  u32 m_pos;
  char m_buffer[8];
};

struct UniformInfo
{
  const void* m_data;
  UniformHandle m_handle;
};

#define BGFX_CONFIG_MAX_UNIFORMS 32
#define BGFX_CONFIG_MAX_INDEX_BUFFERS 32
#define BGFX_CONFIG_MAX_VERTEX_BUFFERS 32
#define BGFX_CONFIG_MAX_SHADERS 32
#define BGFX_CONFIG_MAX_PROGRAMS 32
#define BGFX_CONFIG_MAX_TEXTURES 32
#define BGFX_CONFIG_MAX_VERTEX_DECLS 32

class UniformRegistry
{
public:
  UniformRegistry()
  {
  }

  ~UniformRegistry()
  {
  }

  const UniformInfo* find(const char* _name) const
  {
    u16 handle = m_uniforms.find(hashMurmur2A(_name))->second;
    if (MAX_U16 != handle)
    {
      return &m_info[handle];
    }

    return NULL;
  }

  const UniformInfo& add(UniformHandle _handle, const char* _name, const void* _data)
  {
    CHECKF(is_valid(_handle), "Uniform handle is invalid (name: %s)!", _name);
    const u32 key = hashMurmur2A(_name);
    m_uniforms.erase(key);
    m_uniforms[key] = _handle.idx;

    UniformInfo& info = m_info[_handle.idx];
    info.m_data = _data;
    info.m_handle = _handle;

    return info;
  }

  void remove(UniformHandle _handle)
  {
    //m_uniforms.find(_handle.idx);
    for (auto i : m_uniforms)
    {
      if (i.second == _handle.idx)
        m_uniforms.erase(i.first);
    }
  }

private:
  typedef std::map<u32,u16> UniformHashMap;
  UniformHashMap m_uniforms;
  UniformInfo m_info[BGFX_CONFIG_MAX_UNIFORMS];
};
#pragma endregion

#pragma region VertDecal

static const uint8_t s_attribTypeSizeDx9[AttribType::Count][4] =
{
  { 4, 4, 4, 4 }, // Uint8
  { 4, 4, 4, 4 }, // Uint10
  { 4, 4, 8, 8 }, // Int16
  { 4, 4, 8, 8 }, // Half
  { 4, 8, 12, 16 }, // Float
};

static const uint8_t s_attribTypeSizeDx1x[AttribType::Count][4] =
{
  { 1, 2, 4, 4 }, // Uint8
  { 4, 4, 4, 4 }, // Uint10
  { 2, 4, 8, 8 }, // Int16
  { 2, 4, 8, 8 }, // Half
  { 4, 8, 12, 16 }, // Float
};

static const uint8_t s_attribTypeSizeGl[AttribType::Count][4] =
{
  { 1, 2, 4, 4 }, // Uint8
  { 4, 4, 4, 4 }, // Uint10
  { 2, 4, 6, 8 }, // Int16
  { 2, 4, 6, 8 }, // Half
  { 4, 8, 12, 16 }, // Float
};

static const uint8_t(*s_attribTypeSize[])[AttribType::Count][4] =
{
  &s_attribTypeSizeDx9,  // Null
  &s_attribTypeSizeDx9,  // Direct3D9
  &s_attribTypeSizeDx1x, // Direct3D11
  &s_attribTypeSizeDx1x, // Direct3D12
  &s_attribTypeSizeGl,   // Metal
  &s_attribTypeSizeGl,   // OpenGLES
  &s_attribTypeSizeGl,   // OpenGL
  &s_attribTypeSizeGl,   // Vulkan
  &s_attribTypeSizeGl,   // GNM
  &s_attribTypeSizeDx9,  // Count
};


/// Vertex declaration.
///
/// @attention C99 equivalent is `bgfx_vertex_decl_t`.
///
struct VertexDecl
{
  VertexDecl();

  /// Start VertexDecl.
  ///
  /// @attention C99 equivalent is `bgfx_vertex_decl_begin`.
  ///
  VertexDecl& begin(RendererType::Enum _renderer = RendererType::Null);

  /// End VertexDecl.
  ///
  /// @attention C99 equivalent is `bgfx_vertex_decl_begin`.
  ///
  void end();

  /// Add attribute to VertexDecl.
  ///
  /// @param[in] _attrib Attribute semantics. See: `bgfx::Attrib`
  /// @param[in] _num Number of elements 1, 2, 3 or 4.
  /// @param[in] _type Element type.
  /// @param[in] _normalized When using fixed point AttribType (f.e. Uint8)
  ///   value will be normalized for vertex shader usage. When normalized
  ///   is set to true, AttribType::Uint8 value in range 0-255 will be
  ///   in range 0.0-1.0 in vertex shader.
  /// @param[in] _asInt Packaging rule for vertexPack, vertexUnpack, and
  ///   vertexConvert for AttribType::Uint8 and AttribType::Int16.
  ///   Unpacking code must be implemented inside vertex shader.
  ///
  /// @remarks
  ///   Must be called between begin/end.
  ///
  /// @attention C99 equivalent is `bgfx_vertex_decl_add`.
  ///
  VertexDecl& add(
    Attrib::Enum _attrib
    , u8 _num
    , AttribType::Enum _type
    , bool _normalized = false
    , bool _asInt = false
    );

  /// Skip _num bytes in vertex stream.
  ///
  /// @attention C99 equivalent is `bgfx_vertex_decl_skip`.
  ///
  VertexDecl& skip(u8 _num);

  /// Decode attribute.
  ///
  /// @attention C99 equivalent is ``.
  ///
  void decode(
    Attrib::Enum _attrib
    , u8& _num
    , AttribType::Enum& _type
    , bool& _normalized
    , bool& _asInt
    ) const;

  /// Returns true if VertexDecl contains attribute.
  bool has(Attrib::Enum _attrib) const { return MAX_U16 != m_attributes[_attrib]; }

  /// Returns relative attribute offset from the vertex.
  u16 getOffset(Attrib::Enum _attrib) const { return m_offset[_attrib]; }

  /// Returns vertex stride.
  u16 getStride() const { return m_stride; }

  /// Returns size of vertex buffer for number of vertices.
  u32 getSize(u32 _num) const { return _num*m_stride; }

  u32 m_hash;
  u16 m_stride;
  u16 m_offset[Attrib::Count];
  u16 m_attributes[Attrib::Count];
};
#pragma endregion VertDecal


#pragma region Resources

struct Clear
{
  u8  m_index[8];
  f32 m_depth;
  u8  m_stencil;
  u16 m_flags;
};

#define BGFX_API_VERSION UINT32_C(22)

///
#define BGFX_STATE_RGB_WRITE               UINT64_C(0x0000000000000001) //!< Enable RGB write.
#define BGFX_STATE_ALPHA_WRITE             UINT64_C(0x0000000000000002) //!< Enable alpha write.
#define BGFX_STATE_DEPTH_WRITE             UINT64_C(0x0000000000000004) //!< Enable depth write.

#define BGFX_STATE_DEPTH_TEST_LESS         UINT64_C(0x0000000000000010) //!< Enable depth test, less.
#define BGFX_STATE_DEPTH_TEST_LEQUAL       UINT64_C(0x0000000000000020) //!< Enable depth test, less equal.
#define BGFX_STATE_DEPTH_TEST_EQUAL        UINT64_C(0x0000000000000030) //!< Enable depth test, equal.
#define BGFX_STATE_DEPTH_TEST_GEQUAL       UINT64_C(0x0000000000000040) //!< Enable depth test, greater equal.
#define BGFX_STATE_DEPTH_TEST_GREATER      UINT64_C(0x0000000000000050) //!< Enable depth test, greater.
#define BGFX_STATE_DEPTH_TEST_NOTEQUAL     UINT64_C(0x0000000000000060) //!< Enable depth test, not equal.
#define BGFX_STATE_DEPTH_TEST_NEVER        UINT64_C(0x0000000000000070) //!< Enable depth test, never.
#define BGFX_STATE_DEPTH_TEST_ALWAYS       UINT64_C(0x0000000000000080) //!< Enable depth test, always.
#define BGFX_STATE_DEPTH_TEST_SHIFT        4                            //!< Depth test state bit shift.
#define BGFX_STATE_DEPTH_TEST_MASK         UINT64_C(0x00000000000000f0) //!< Depth test state bit mask.

#define BGFX_STATE_BLEND_ZERO              UINT64_C(0x0000000000001000) //!<
#define BGFX_STATE_BLEND_ONE               UINT64_C(0x0000000000002000) //!<
#define BGFX_STATE_BLEND_SRC_COLOR         UINT64_C(0x0000000000003000) //!<
#define BGFX_STATE_BLEND_INV_SRC_COLOR     UINT64_C(0x0000000000004000) //!<
#define BGFX_STATE_BLEND_SRC_ALPHA         UINT64_C(0x0000000000005000) //!<
#define BGFX_STATE_BLEND_INV_SRC_ALPHA     UINT64_C(0x0000000000006000) //!<
#define BGFX_STATE_BLEND_DST_ALPHA         UINT64_C(0x0000000000007000) //!<
#define BGFX_STATE_BLEND_INV_DST_ALPHA     UINT64_C(0x0000000000008000) //!<
#define BGFX_STATE_BLEND_DST_COLOR         UINT64_C(0x0000000000009000) //!<
#define BGFX_STATE_BLEND_INV_DST_COLOR     UINT64_C(0x000000000000a000) //!<
#define BGFX_STATE_BLEND_SRC_ALPHA_SAT     UINT64_C(0x000000000000b000) //!<
#define BGFX_STATE_BLEND_FACTOR            UINT64_C(0x000000000000c000) //!<
#define BGFX_STATE_BLEND_INV_FACTOR        UINT64_C(0x000000000000d000) //!<
#define BGFX_STATE_BLEND_SHIFT             12                           //!< Blend state bit shift.
#define BGFX_STATE_BLEND_MASK              UINT64_C(0x000000000ffff000) //!< Blend state bit mask.

#define BGFX_STATE_BLEND_EQUATION_ADD      UINT64_C(0x0000000000000000) //!<
#define BGFX_STATE_BLEND_EQUATION_SUB      UINT64_C(0x0000000010000000) //!<
#define BGFX_STATE_BLEND_EQUATION_REVSUB   UINT64_C(0x0000000020000000) //!<
#define BGFX_STATE_BLEND_EQUATION_MIN      UINT64_C(0x0000000030000000) //!<
#define BGFX_STATE_BLEND_EQUATION_MAX      UINT64_C(0x0000000040000000) //!<
#define BGFX_STATE_BLEND_EQUATION_SHIFT    28                           //!< Blend equation bit shift.
#define BGFX_STATE_BLEND_EQUATION_MASK     UINT64_C(0x00000003f0000000) //!< Blend equation bit mask.

#define BGFX_STATE_BLEND_INDEPENDENT       UINT64_C(0x0000000400000000) //!< Enable blend independent.
#define BGFX_STATE_BLEND_ALPHA_TO_COVERAGE UINT64_C(0x0000000800000000) //!< Enable alpha to coverage.

#define BGFX_STATE_CULL_CW                 UINT64_C(0x0000001000000000) //!< Cull clockwise triangles.
#define BGFX_STATE_CULL_CCW                UINT64_C(0x0000002000000000) //!< Cull counter-clockwise triangles.
#define BGFX_STATE_CULL_SHIFT              36                           //!< Culling mode bit shift.
#define BGFX_STATE_CULL_MASK               UINT64_C(0x0000003000000000) //!< Culling mode bit mask.

/// See BGFX_STATE_ALPHA_REF(_ref) helper macro.
#define BGFX_STATE_ALPHA_REF_SHIFT         40                           //!< Alpha reference bit shift.
#define BGFX_STATE_ALPHA_REF_MASK          UINT64_C(0x0000ff0000000000) //!< Alpha reference bit mask.

#define BGFX_STATE_PT_TRISTRIP             UINT64_C(0x0001000000000000) //!< Tristrip.
#define BGFX_STATE_PT_LINES                UINT64_C(0x0002000000000000) //!< Lines.
#define BGFX_STATE_PT_LINESTRIP            UINT64_C(0x0003000000000000) //!< Line strip.
#define BGFX_STATE_PT_POINTS               UINT64_C(0x0004000000000000) //!< Points.
#define BGFX_STATE_PT_SHIFT                48                           //!< Primitive type bit shift.
#define BGFX_STATE_PT_MASK                 UINT64_C(0x0007000000000000) //!< Primitive type bit mask.

#define BGFX_STATE_POINT_SIZE_SHIFT        52                           //!< Point size bit shift.
#define BGFX_STATE_POINT_SIZE_MASK         UINT64_C(0x00f0000000000000) //!< Point size bit mask.

/// Enable MSAA write when writing into MSAA frame buffer. This flag is ignored when not writing into
/// MSAA frame buffer.
#define BGFX_STATE_MSAA                    UINT64_C(0x0100000000000000) //!< Enable MSAA rasterization.
#define BGFX_STATE_LINEAA                  UINT64_C(0x0200000000000000) //!< Enable line AA rasterization.
#define BGFX_STATE_CONSERVATIVE_RASTER     UINT64_C(0x0400000000000000) //!< Enable conservative rasterization.

/// Do not use!
#define BGFX_STATE_RESERVED_SHIFT          61                           //!< Internal bits shift.
#define BGFX_STATE_RESERVED_MASK           UINT64_C(0xe000000000000000) //!< Internal bits mask.

/// See BGFX_STATE_POINT_SIZE(_size) helper macro.
#define BGFX_STATE_NONE                    UINT64_C(0x0000000000000000) //!< No state.
#define BGFX_STATE_MASK                    UINT64_C(0xffffffffffffffff) //!< State mask.

/// Default state is write to RGB, alpha, and depth with depth test less enabled, with clockwise
/// culling and MSAA (when writing into MSAA frame buffer, otherwise this flag is ignored).
#define BGFX_STATE_DEFAULT (0 \
					| BGFX_STATE_RGB_WRITE \
					| BGFX_STATE_ALPHA_WRITE \
					| BGFX_STATE_DEPTH_TEST_LESS \
					| BGFX_STATE_DEPTH_WRITE \
					| BGFX_STATE_CULL_CW \
					| BGFX_STATE_MSAA \
					)

#define BGFX_STATE_ALPHA_REF(_ref)   ( ( (uint64_t)(_ref )<<BGFX_STATE_ALPHA_REF_SHIFT )&BGFX_STATE_ALPHA_REF_MASK)
#define BGFX_STATE_POINT_SIZE(_size) ( ( (uint64_t)(_size)<<BGFX_STATE_POINT_SIZE_SHIFT)&BGFX_STATE_POINT_SIZE_MASK)

///
#define BGFX_STATE_BLEND_FUNC_SEPARATE(_srcRGB, _dstRGB, _srcA, _dstA) (UINT64_C(0) \
					| ( ( (uint64_t)(_srcRGB)|( (uint64_t)(_dstRGB)<<4) )   ) \
					| ( ( (uint64_t)(_srcA  )|( (uint64_t)(_dstA  )<<4) )<<8) \
					)

#define BGFX_STATE_BLEND_EQUATION_SEPARATE(_rgb, _a) ( (uint64_t)(_rgb)|( (uint64_t)(_a)<<3) )

///
#define BGFX_STATE_BLEND_FUNC(_src, _dst)    BGFX_STATE_BLEND_FUNC_SEPARATE(_src, _dst, _src, _dst)
#define BGFX_STATE_BLEND_EQUATION(_equation) BGFX_STATE_BLEND_EQUATION_SEPARATE(_equation, _equation)

#define BGFX_STATE_BLEND_ADD         (BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE,       BGFX_STATE_BLEND_ONE          ) )
#define BGFX_STATE_BLEND_ALPHA       (BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) )
#define BGFX_STATE_BLEND_DARKEN      (BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE,       BGFX_STATE_BLEND_ONE          ) | BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_MIN) )
#define BGFX_STATE_BLEND_LIGHTEN     (BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE,       BGFX_STATE_BLEND_ONE          ) | BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_MAX) )
#define BGFX_STATE_BLEND_MULTIPLY    (BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_DST_COLOR, BGFX_STATE_BLEND_ZERO         ) )
#define BGFX_STATE_BLEND_NORMAL      (BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE,       BGFX_STATE_BLEND_INV_SRC_ALPHA) )
#define BGFX_STATE_BLEND_SCREEN      (BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE,       BGFX_STATE_BLEND_INV_SRC_COLOR) )
#define BGFX_STATE_BLEND_LINEAR_BURN (BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_DST_COLOR, BGFX_STATE_BLEND_INV_DST_COLOR) | BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_SUB) )

///
#define BGFX_STATE_BLEND_FUNC_RT_x(_src, _dst) (0 \
					| ( uint32_t( (_src)>>BGFX_STATE_BLEND_SHIFT) \
					| ( uint32_t( (_dst)>>BGFX_STATE_BLEND_SHIFT)<<4) ) \
					)

#define BGFX_STATE_BLEND_FUNC_RT_xE(_src, _dst, _equation) (0 \
					| BGFX_STATE_BLEND_FUNC_RT_x(_src, _dst) \
					| ( uint32_t( (_equation)>>BGFX_STATE_BLEND_EQUATION_SHIFT)<<8) \
					)

#define BGFX_STATE_BLEND_FUNC_RT_1(_src, _dst)  (BGFX_STATE_BLEND_FUNC_RT_x(_src, _dst)<< 0)
#define BGFX_STATE_BLEND_FUNC_RT_2(_src, _dst)  (BGFX_STATE_BLEND_FUNC_RT_x(_src, _dst)<<11)
#define BGFX_STATE_BLEND_FUNC_RT_3(_src, _dst)  (BGFX_STATE_BLEND_FUNC_RT_x(_src, _dst)<<22)

#define BGFX_STATE_BLEND_FUNC_RT_1E(_src, _dst, _equation) (BGFX_STATE_BLEND_FUNC_RT_xE(_src, _dst, _equation)<< 0)
#define BGFX_STATE_BLEND_FUNC_RT_2E(_src, _dst, _equation) (BGFX_STATE_BLEND_FUNC_RT_xE(_src, _dst, _equation)<<11)
#define BGFX_STATE_BLEND_FUNC_RT_3E(_src, _dst, _equation) (BGFX_STATE_BLEND_FUNC_RT_xE(_src, _dst, _equation)<<22)

///
#define BGFX_STENCIL_FUNC_REF_SHIFT      0                    //!<
#define BGFX_STENCIL_FUNC_REF_MASK       UINT32_C(0x000000ff) //!<
#define BGFX_STENCIL_FUNC_RMASK_SHIFT    8                    //!<
#define BGFX_STENCIL_FUNC_RMASK_MASK     UINT32_C(0x0000ff00) //!<

#define BGFX_STENCIL_TEST_LESS           UINT32_C(0x00010000) //!< Enable stencil test, less.
#define BGFX_STENCIL_TEST_LEQUAL         UINT32_C(0x00020000) //!<
#define BGFX_STENCIL_TEST_EQUAL          UINT32_C(0x00030000) //!<
#define BGFX_STENCIL_TEST_GEQUAL         UINT32_C(0x00040000) //!<
#define BGFX_STENCIL_TEST_GREATER        UINT32_C(0x00050000) //!<
#define BGFX_STENCIL_TEST_NOTEQUAL       UINT32_C(0x00060000) //!<
#define BGFX_STENCIL_TEST_NEVER          UINT32_C(0x00070000) //!<
#define BGFX_STENCIL_TEST_ALWAYS         UINT32_C(0x00080000) //!<
#define BGFX_STENCIL_TEST_SHIFT          16                   //!< Stencil test bit shift.
#define BGFX_STENCIL_TEST_MASK           UINT32_C(0x000f0000) //!< Stencil test bit mask.

#define BGFX_STENCIL_OP_FAIL_S_ZERO      UINT32_C(0x00000000) //!< Zero.
#define BGFX_STENCIL_OP_FAIL_S_KEEP      UINT32_C(0x00100000) //!< Keep.
#define BGFX_STENCIL_OP_FAIL_S_REPLACE   UINT32_C(0x00200000) //!< Replace.
#define BGFX_STENCIL_OP_FAIL_S_INCR      UINT32_C(0x00300000) //!< Increment and wrap.
#define BGFX_STENCIL_OP_FAIL_S_INCRSAT   UINT32_C(0x00400000) //!< Increment and clamp.
#define BGFX_STENCIL_OP_FAIL_S_DECR      UINT32_C(0x00500000) //!< Decrement and wrap.
#define BGFX_STENCIL_OP_FAIL_S_DECRSAT   UINT32_C(0x00600000) //!< Decrement and clamp.
#define BGFX_STENCIL_OP_FAIL_S_INVERT    UINT32_C(0x00700000) //!< Invert.
#define BGFX_STENCIL_OP_FAIL_S_SHIFT     20                   //!< Stencil operation fail bit shift.
#define BGFX_STENCIL_OP_FAIL_S_MASK      UINT32_C(0x00f00000) //!< Stencil operation fail bit mask.

#define BGFX_STENCIL_OP_FAIL_Z_ZERO      UINT32_C(0x00000000) //!<
#define BGFX_STENCIL_OP_FAIL_Z_KEEP      UINT32_C(0x01000000) //!<
#define BGFX_STENCIL_OP_FAIL_Z_REPLACE   UINT32_C(0x02000000) //!<
#define BGFX_STENCIL_OP_FAIL_Z_INCR      UINT32_C(0x03000000) //!<
#define BGFX_STENCIL_OP_FAIL_Z_INCRSAT   UINT32_C(0x04000000) //!<
#define BGFX_STENCIL_OP_FAIL_Z_DECR      UINT32_C(0x05000000) //!<
#define BGFX_STENCIL_OP_FAIL_Z_DECRSAT   UINT32_C(0x06000000) //!<
#define BGFX_STENCIL_OP_FAIL_Z_INVERT    UINT32_C(0x07000000) //!<
#define BGFX_STENCIL_OP_FAIL_Z_SHIFT     24                   //!< Stencil operation fail depth bit shift.
#define BGFX_STENCIL_OP_FAIL_Z_MASK      UINT32_C(0x0f000000) //!< Stencil operation fail depth bit mask.

#define BGFX_STENCIL_OP_PASS_Z_ZERO      UINT32_C(0x00000000) //!<
#define BGFX_STENCIL_OP_PASS_Z_KEEP      UINT32_C(0x10000000) //!<
#define BGFX_STENCIL_OP_PASS_Z_REPLACE   UINT32_C(0x20000000) //!<
#define BGFX_STENCIL_OP_PASS_Z_INCR      UINT32_C(0x30000000) //!<
#define BGFX_STENCIL_OP_PASS_Z_INCRSAT   UINT32_C(0x40000000) //!<
#define BGFX_STENCIL_OP_PASS_Z_DECR      UINT32_C(0x50000000) //!<
#define BGFX_STENCIL_OP_PASS_Z_DECRSAT   UINT32_C(0x60000000) //!<
#define BGFX_STENCIL_OP_PASS_Z_INVERT    UINT32_C(0x70000000) //!<
#define BGFX_STENCIL_OP_PASS_Z_SHIFT     28                   //!< Stencil operation pass depth bit shift.
#define BGFX_STENCIL_OP_PASS_Z_MASK      UINT32_C(0xf0000000) //!< Stencil operation pass depth bit mask.

#define BGFX_STENCIL_NONE                UINT32_C(0x00000000) //!<
#define BGFX_STENCIL_MASK                UINT32_C(0xffffffff) //!<
#define BGFX_STENCIL_DEFAULT             UINT32_C(0x00000000) //!<

/// Set stencil ref value.
#define BGFX_STENCIL_FUNC_REF(_ref) ( (uint32_t(_ref)<<BGFX_STENCIL_FUNC_REF_SHIFT)&BGFX_STENCIL_FUNC_REF_MASK)

/// Set stencil rmask value.
#define BGFX_STENCIL_FUNC_RMASK(_mask) ( (uint32_t(_mask)<<BGFX_STENCIL_FUNC_RMASK_SHIFT)&BGFX_STENCIL_FUNC_RMASK_MASK)

///
#define BGFX_CLEAR_NONE                  UINT16_C(0x0000) //!< No clear flags.
#define BGFX_CLEAR_COLOR                 UINT16_C(0x0001) //!< Clear color.
#define BGFX_CLEAR_DEPTH                 UINT16_C(0x0002) //!< Clear depth.
#define BGFX_CLEAR_STENCIL               UINT16_C(0x0004) //!< Clear stencil.
#define BGFX_CLEAR_DISCARD_COLOR_0       UINT16_C(0x0008) //!< Discard frame buffer attachment 0.
#define BGFX_CLEAR_DISCARD_COLOR_1       UINT16_C(0x0010) //!< Discard frame buffer attachment 1.
#define BGFX_CLEAR_DISCARD_COLOR_2       UINT16_C(0x0020) //!< Discard frame buffer attachment 2.
#define BGFX_CLEAR_DISCARD_COLOR_3       UINT16_C(0x0040) //!< Discard frame buffer attachment 3.
#define BGFX_CLEAR_DISCARD_COLOR_4       UINT16_C(0x0080) //!< Discard frame buffer attachment 4.
#define BGFX_CLEAR_DISCARD_COLOR_5       UINT16_C(0x0100) //!< Discard frame buffer attachment 5.
#define BGFX_CLEAR_DISCARD_COLOR_6       UINT16_C(0x0200) //!< Discard frame buffer attachment 6.
#define BGFX_CLEAR_DISCARD_COLOR_7       UINT16_C(0x0400) //!< Discard frame buffer attachment 7.
#define BGFX_CLEAR_DISCARD_DEPTH         UINT16_C(0x0800) //!< Discard frame buffer depth attachment.
#define BGFX_CLEAR_DISCARD_STENCIL       UINT16_C(0x1000) //!< Discard frame buffer stencil attachment.

#define BGFX_CLEAR_DISCARD_COLOR_MASK (0 \
			| BGFX_CLEAR_DISCARD_COLOR_0 \
			| BGFX_CLEAR_DISCARD_COLOR_1 \
			| BGFX_CLEAR_DISCARD_COLOR_2 \
			| BGFX_CLEAR_DISCARD_COLOR_3 \
			| BGFX_CLEAR_DISCARD_COLOR_4 \
			| BGFX_CLEAR_DISCARD_COLOR_5 \
			| BGFX_CLEAR_DISCARD_COLOR_6 \
			| BGFX_CLEAR_DISCARD_COLOR_7 \
			)
#define BGFX_CLEAR_DISCARD_MASK (0 \
			| BGFX_CLEAR_DISCARD_COLOR_MASK \
			| BGFX_CLEAR_DISCARD_DEPTH \
			| BGFX_CLEAR_DISCARD_STENCIL \
			)

#define BGFX_DEBUG_NONE                  UINT32_C(0x00000000) //!< No debug.
#define BGFX_DEBUG_WIREFRAME             UINT32_C(0x00000001) //!< Enable wireframe for all primitives.
#define BGFX_DEBUG_IFH                   UINT32_C(0x00000002) //!< Enable infinitely fast hardware test. No draw calls will be submitted to driver. It¡¯s useful when profiling to quickly assess bottleneck between CPU and GPU.
#define BGFX_DEBUG_STATS                 UINT32_C(0x00000004) //!< Enable statistics display.
#define BGFX_DEBUG_TEXT                  UINT32_C(0x00000008) //!< Enable debug text display.

///
#define BGFX_BUFFER_NONE                 UINT16_C(0x0000) //!<

#define BGFX_BUFFER_COMPUTE_FORMAT_8x1   UINT16_C(0x0001) //!<
#define BGFX_BUFFER_COMPUTE_FORMAT_8x2   UINT16_C(0x0002) //!<
#define BGFX_BUFFER_COMPUTE_FORMAT_8x4   UINT16_C(0x0003) //!<
#define BGFX_BUFFER_COMPUTE_FORMAT_16x1  UINT16_C(0x0004) //!<
#define BGFX_BUFFER_COMPUTE_FORMAT_16x2  UINT16_C(0x0005) //!<
#define BGFX_BUFFER_COMPUTE_FORMAT_16x4  UINT16_C(0x0006) //!<
#define BGFX_BUFFER_COMPUTE_FORMAT_32x1  UINT16_C(0x0007) //!<
#define BGFX_BUFFER_COMPUTE_FORMAT_32x2  UINT16_C(0x0008) //!<
#define BGFX_BUFFER_COMPUTE_FORMAT_32x4  UINT16_C(0x0009) //!<
#define BGFX_BUFFER_COMPUTE_FORMAT_SHIFT 0                //!<
#define BGFX_BUFFER_COMPUTE_FORMAT_MASK  UINT16_C(0x000f) //!<

#define BGFX_BUFFER_COMPUTE_TYPE_UINT    UINT16_C(0x0010) //!<
#define BGFX_BUFFER_COMPUTE_TYPE_INT     UINT16_C(0x0020) //!<
#define BGFX_BUFFER_COMPUTE_TYPE_FLOAT   UINT16_C(0x0030) //!<
#define BGFX_BUFFER_COMPUTE_TYPE_SHIFT   4                //!<
#define BGFX_BUFFER_COMPUTE_TYPE_MASK    UINT16_C(0x0030) //!<


#define BGFX_BUFFER_COMPUTE_READ         UINT16_C(0x0100) //!< Buffer will be read by shader.
#define BGFX_BUFFER_COMPUTE_WRITE        UINT16_C(0x0200) //!< Buffer will be used for writing.
#define BGFX_BUFFER_DRAW_INDIRECT        UINT16_C(0x0400) //!< Buffer will be used for storing draw indirect commands.
#define BGFX_BUFFER_ALLOW_RESIZE         UINT16_C(0x0800) //!<
#define BGFX_BUFFER_INDEX32              UINT16_C(0x1000) //!<

#define BGFX_BUFFER_COMPUTE_READ_WRITE (0 \
			| BGFX_BUFFER_COMPUTE_READ \
			| BGFX_BUFFER_COMPUTE_WRITE \
			)

struct BufferD3D11 
{
  BufferD3D11():
    ptr(nullptr), srv(nullptr), uav(nullptr), flag(0), bdynamic(false), size(0)
  {}
  void create(u32 inzise, void * indata,u16 inflags, u16 instride = 0, bool invertex = false);
  void update(u32 inoffset, u32 insize, void* indata, bool indiscard = false);
  void destroy()
  {
    if (nullptr != ptr)
    {
      ptr->Release();
      ptr = nullptr;
    }
    srv->Release(); srv = nullptr;
    uav->Release(); uav = nullptr;
    size = 0;
    flag = 0;
    bdynamic = false;
    
  }
  ID3D11Buffer* ptr;
  ID3D11ShaderResourceView*  srv;
  ID3D11UnorderedAccessView* uav;
  u32 size;
  u16 flag;
  bool bdynamic;
};

typedef BufferD3D11 IndexBufferD3D11;

struct VertexBufferD3D11 : public BufferD3D11
{
  void create(u32 insize, void* data, VertexDeclHandle _declHandle, uint16 _flags);
  VertexDeclHandle m_decl;
};

struct ShaderD3D11
{
  ShaderD3D11();
  void create(const void* indata);
  void destroy()
  {
    if (NULL != constantBuffer)
    {
      UniformBuffer::destroy((UniformBuffer*)constantBuffer);
      constantBuffer = NULL;
    }

    //m_numPredefined = 0;

    if (NULL != buffer)
    {
      buffer->Release();
      buffer = nullptr;
    }

    ptr->Release();
    ptr = nullptr;

    if (NULL != code)
    {
      free(code);
      code = nullptr;
      hash = 0;
    }
  }
  union
  {
    ID3D11ComputeShader* computeShader;
    ID3D11PixelShader*   pixelShader;
    ID3D11VertexShader*  vertexShader;
    IUnknown*            ptr;
  };
  const void* code;
  ID3D11Buffer* buffer;
  void* constantBuffer;

  //PredefinedUniform m_predefined[PredefinedUniform::Count];
  u16 m_attrMask[2];

  u32 hash;

  u16 numUniforms;
  u8 numPredefined;
  bool bhasDepthOp;
};

struct ProgramD3D11
{
  ProgramD3D11()
    : m_vsh(NULL)
    , m_fsh(NULL)
  {
  }

  void create(const ShaderD3D11* _vsh, const ShaderD3D11* _fsh)
  {
    CHECKF(NULL != _vsh->ptr, "Vertex shader doesn't exist.");
    m_vsh = _vsh;
    //memcpy(&m_predefined[0], _vsh->m_predefined, _vsh->m_numPredefined*sizeof(PredefinedUniform));
    //m_numPredefined = _vsh->m_numPredefined;

    if (nullptr != _fsh)
    {
      CHECKF(NULL != _fsh->ptr, "Fragment shader doesn't exist.");
      m_fsh = _fsh;
      //memcpy(&m_predefined[m_numPredefined], _fsh->m_predefined, _fsh->m_numPredefined*sizeof(PredefinedUniform));
      //m_numPredefined += _fsh->m_numPredefined;
    }
  }

  void destroy()
  {
    m_numPredefined = 0;
    m_vsh = nullptr;
    m_fsh = nullptr;
  }

  const ShaderD3D11* m_vsh;
  const ShaderD3D11* m_fsh;

  PredefinedUniform m_predefined[PredefinedUniform::Count * 2];
  u8 m_numPredefined;
};

struct TextureD3D11
{
  enum Enum
  {
    Texture2D,
    Texture3D,
    TextureCube,
  };

  TextureD3D11()
    : m_ptr(NULL)
    , m_rt(NULL)
    , m_srv(NULL)
    , m_uav(NULL)
    , m_numMips(0)
  {
  }

  void create(const void* _mem, u32 _flags, u8 _skip);
  void destroy();
  void overrideInternal(uintptr_t _ptr);
  void update(u8 _side, u8 _mip, const RBAABBI& _rect, u16 _z, u16 _depth, u16 _pitch, const void* _mem);
  void commit(u8 _stage, u32 _flags, const float _palette[][4]);
  void resolve() const;
  TextureHandle getHandle() const;

  union
  {
    ID3D11Resource*  m_ptr;
    ID3D11Texture2D* m_texture2d;
    ID3D11Texture3D* m_texture3d;
  };

  union
  {
    ID3D11Resource* m_rt;
    ID3D11Texture2D* m_rt2d;
  };

  ID3D11ShaderResourceView*  m_srv;
  ID3D11UnorderedAccessView* m_uav;
  u32 m_flags;
  u32 m_width;
  u32 m_height;
  u32 m_depth;
  u8  m_type;
  u8  m_requestedFormat;
  u8  m_textureFormat;
  u8  m_numMips;
};


struct Attachment
{
  TextureHandle handle; //!< Texture handle.
  u16 mip;         //!< Mip level.
  u16 layer;       //!< Cubemap side or depth layer/slice.
};

struct FrameBufferD3D11
{
  FrameBufferD3D11()
    : m_dsv(NULL)
    , m_swapChain(NULL)
    , m_width(0)
    , m_height(0)
    , m_denseIdx(MAX_U16)
    , m_num(0)
    , m_numTh(0)
  {
  }

  void create(u8 _num, const Attachment* _attachment);
  void create(u16 _denseIdx, void* _nwh, u32 _width, u32 _height, TextureFormat::Enum _depthFormat);
  u16 destroy();
  void preReset(bool _force = false);
  void postReset();
  void resolve();
  void clear(const Clear& _clear, const float _palette[][4]);

  ID3D11RenderTargetView* m_rtv[BGFX_CONFIG_MAX_FRAME_BUFFER_ATTACHMENTS - 1];
  ID3D11ShaderResourceView* m_srv[BGFX_CONFIG_MAX_FRAME_BUFFER_ATTACHMENTS - 1];
  ID3D11DepthStencilView* m_dsv;
  IDXGISwapChain* m_swapChain;
  u32 m_width;
  u32 m_height;

  Attachment m_attachment[BGFX_CONFIG_MAX_FRAME_BUFFER_ATTACHMENTS];
  u16 m_denseIdx;
  u8 m_num;
  u8 m_numTh;
};

struct TimerQueryD3D11
{
  TimerQueryD3D11()
  /*: m_control(ARRAY_COUNT(m_frame))*/
  {
  }

  void postReset();
  void preReset();
  void begin();
  void end();
  bool get();

  struct Frame
  {
    ID3D11Query* m_disjoint;
    ID3D11Query* m_begin;
    ID3D11Query* m_end;
  };

  u64 m_begin;
  u64 m_end;
  u64 m_elapsed;
  u64 m_frequency;

  Frame m_frame[4];
  //bx::RingBufferControl m_control;
};

struct OcclusionQueryD3D11
{
  OcclusionQueryD3D11()
  /* : m_control(ARRAY_COUNT(m_query))*/
  {
  }

  void postReset();
  void preReset();
  void begin(class Frame* _render, OcclusionQueryHandle _handle);
  void end();
  void resolve(class Frame* _render, bool _wait = false);

  struct Query
  {
    ID3D11Query* m_ptr;
    OcclusionQueryHandle m_handle;
  };

  Query m_query[BGFX_CONFIG_MAX_OCCUSION_QUERIES];
  //bx::RingBufferControl m_control;
};

#pragma endregion