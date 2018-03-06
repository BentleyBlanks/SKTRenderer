#include "D3D11WRAP.h"
#include "Tools.h"
#include "Util.h"
#include "D3D11Util.h"

VertexDecl::VertexDecl()
{

}

VertexDecl& VertexDecl::begin(RendererType::Enum _renderer /*= RendererType::Null*/)
{
  m_hash = _renderer; // use hash to store renderer type while building VertexDecl.
  m_stride = 0;
  memset(m_attributes, 0xff, sizeof(m_attributes));
  memset(m_offset, 0, sizeof(m_offset));

  return *this;
}

void VertexDecl::end()
{
  HashMurmur2A murmur;
  murmur.begin();
  murmur.add(m_attributes, sizeof(m_attributes));
  murmur.add(m_offset, sizeof(m_offset));
  murmur.add(m_stride);
  m_hash = murmur.end();
}

VertexDecl& VertexDecl::add(Attrib::Enum _attrib, u8 _num, AttribType::Enum _type, bool _normalized, bool _asInt)
{
  const u16 encodedNorm = (_normalized & 1) << 7;
  const u16 encodedType = (_type & 7) << 3;
  const u16 encodedNum = (_num - 1) & 3;
  const u16 encodeAsInt = (_asInt&(!!"\x1\x1\x1\x0\x0"[_type])) << 8;
  m_attributes[_attrib] = encodedNorm | encodedType | encodedNum | encodeAsInt;

  m_offset[_attrib] = m_stride;
  m_stride += (*s_attribTypeSize[m_hash])[_type][_num - 1];

  return *this;
}

VertexDecl& VertexDecl::skip(u8 _num)
{
  m_stride += _num;

  return *this;
}

void VertexDecl::decode(Attrib::Enum _attrib, u8& _num, AttribType::Enum& _type, bool& _normalized, bool& _asInt) const
{
  u16 val = m_attributes[_attrib];
  _num = (val & 3) + 1;
  _type = AttribType::Enum((val >> 3) & 7);
  _normalized = !!(val&(1 << 7));
  _asInt = !!(val&(1 << 8));
}


#pragma region UnifromBuffer

const u32 g_uniformTypeSize[UniformType::Count + 1] =
{
  sizeof(i32),
  0,
  4 * sizeof(float),
  3 * 3 * sizeof(float),
  4 * 4 * sizeof(float),
  1,
};

void UniformBuffer::writeUniform(UniformType::Enum _type, u16 _loc, const void* _value, u16 _num /*= 1*/)
{
  u32 opcode = encodeOpcode(_type, _loc, _num, true);
  write(opcode);
  write(_value, g_uniformTypeSize[_type] * _num);
}

void UniformBuffer::writeUniformHandle(UniformType::Enum _type, u16 _loc, UniformHandle _handle, u16 _num /*= 1*/)
{
  u32 opcode = encodeOpcode(_type, _loc, _num, false);
  write(opcode);
  write(&_handle, sizeof(UniformHandle));
}

void UniformBuffer::writeMarker(const char* _marker)
{
  u16 num = (u16)strlen(_marker) + 1;
  u32 opcode = encodeOpcode(UniformType::Count, 0, num, true);
  write(opcode);
  write(_marker, num);
}

#pragma endregion



#pragma region Resources

/// Transient vertex buffer.
///
/// @attention C99 equivalent is `bgfx_transient_vertex_buffer_t`.
///
struct TransientVertexBuffer
{
  uint8_t* data;             //!< Pointer to data.
  u32 size;             //!< Data size.
  u32 startVertex;      //!< First vertex.
  u16 stride;           //!< Vertex stride.
  VertexBufferHandle handle; //!< Vertex buffer handle.
  VertexDeclHandle decl;     //!< Vertex declaration handle.
};

/// Transient index buffer.
///
/// @attention C99 equivalent is `bgfx_transient_index_buffer_t`.
///
struct TransientIndexBuffer
{
  uint8_t* data;            //!< Pointer to data.
  u32 size;            //!< Data size.
  u32 startIndex;      //!< First index.
  IndexBufferHandle handle; //!< Index buffer handle.
};

struct ClearQuad
{
  ClearQuad()
  {
    for (u32 ii = 0; ii < ARRAY_COUNT(m_program); ++ii)
    {
      m_program[ii].idx = invalidHandle;
    }
  }

  void init();
  void shutdown();

  TransientVertexBuffer* m_vb;
  VertexDecl m_decl;
  ProgramHandle m_program[BGFX_CONFIG_MAX_FRAME_BUFFER_ATTACHMENTS];
};

struct PredefinedUniform
{
  enum Enum
  {
    ViewRect,
    ViewTexel,
    View,
    InvView,
    Proj,
    InvProj,
    ViewProj,
    InvViewProj,
    Model,
    ModelView,
    ModelViewProj,
    AlphaRef,
    Count
  };

  u32 m_loc;
  u16 m_count;
  u8 m_type;
};

struct TextVideoMemBlitter
{
  void init();
  void shutdown();

  TextureHandle m_texture;
  TransientVertexBuffer* m_vb;
  TransientIndexBuffer* m_ib;
  VertexDecl m_decl;
  ProgramHandle m_program;
  bool m_init;
};

#define BGFX_CONFIG_MAX_TEXTURE_SAMPLERS 8

struct TextureStage
{
  TextureStage()
  {
    clear();
  }

  void clear()
  {
    memset(m_srv, 0, sizeof(m_srv));
    memset(m_sampler, 0, sizeof(m_sampler));
  }

  ID3D11ShaderResourceView* m_srv[BGFX_CONFIG_MAX_TEXTURE_SAMPLERS];
  ID3D11SamplerState* m_sampler[BGFX_CONFIG_MAX_TEXTURE_SAMPLERS];
};

struct Resolution
{
  Resolution()
    : m_width(1280)
    , m_height(720)
    , m_flags(0)
  {
  }

  u32 m_width;
  u32 m_height;
  u32 m_flags;
};

struct __declspec(novtable) RendererContextI
{
  virtual ~RendererContextI() = 0;
  virtual RendererType::Enum getRendererType() const = 0;
  virtual const char* getRendererName() const = 0;
  virtual void flip() = 0;
  virtual void createIndexBuffer(IndexBufferHandle _handle, Memory* _mem, uint16 _flags) = 0;
  virtual void destroyIndexBuffer(IndexBufferHandle _handle) = 0;
  virtual void createVertexDecl(VertexDeclHandle _handle, const VertexDecl& _decl) = 0;
  virtual void destroyVertexDecl(VertexDeclHandle _handle) = 0;
  virtual void createVertexBuffer(VertexBufferHandle _handle, Memory* _mem, VertexDeclHandle _declHandle, uint16 _flags) = 0;
  virtual void destroyVertexBuffer(VertexBufferHandle _handle) = 0;
  virtual void createDynamicIndexBuffer(IndexBufferHandle _handle, uint32 _size, uint16 _flags) = 0;
  virtual void updateDynamicIndexBuffer(IndexBufferHandle _handle, uint32 _offset, uint32 _size, Memory* _mem) = 0;
  virtual void destroyDynamicIndexBuffer(IndexBufferHandle _handle) = 0;
  virtual void createDynamicVertexBuffer(VertexBufferHandle _handle, uint32 _size, uint16 _flags) = 0;
  virtual void updateDynamicVertexBuffer(VertexBufferHandle _handle, uint32 _offset, uint32 _size, Memory* _mem) = 0;
  virtual void destroyDynamicVertexBuffer(VertexBufferHandle _handle) = 0;
  virtual void createShader(ShaderHandle _handle, Memory* _mem) = 0;
  virtual void destroyShader(ShaderHandle _handle) = 0;
  virtual void createProgram(ProgramHandle _handle, ShaderHandle _vsh, ShaderHandle _fsh) = 0;
  virtual void destroyProgram(ProgramHandle _handle) = 0;
  virtual void createTexture(TextureHandle _handle, Memory* _mem, uint32 _flags, uint8 _skip) = 0;
  virtual void updateTextureBegin(TextureHandle _handle, uint8 _side, uint8 _mip) = 0;
  virtual void updateTexture(TextureHandle _handle, uint8 _side, uint8 _mip, const Rect& _rect, uint16 _z, uint16 _depth, uint16 _pitch, const Memory* _mem) = 0;
  virtual void updateTextureEnd() = 0;
  virtual void readTexture(TextureHandle _handle, void* _data) = 0;
  virtual void resizeTexture(TextureHandle _handle, uint16 _width, uint16 _height, uint8 _numMips) = 0;
  virtual void overrideInternal(TextureHandle _handle, uintptr_t _ptr) = 0;
  virtual uintptr_t getInternal(TextureHandle _handle) = 0;
  virtual void destroyTexture(TextureHandle _handle) = 0;
  virtual void createFrameBuffer(FrameBufferHandle _handle, uint8 _num, const Attachment* _attachment) = 0;
  virtual void createFrameBuffer(FrameBufferHandle _handle, void* _nwh, uint32 _width, uint32 _height, TextureFormat::Enum _depthFormat) = 0;
  virtual void destroyFrameBuffer(FrameBufferHandle _handle) = 0;
  virtual void createUniform(UniformHandle _handle, UniformType::Enum _type, uint16 _num, const char* _name) = 0;
  virtual void destroyUniform(UniformHandle _handle) = 0;
  virtual void saveScreenShot(const char* _filePath) = 0;
  virtual void updateViewName(uint8 _id, const char* _name) = 0;
  virtual void updateUniform(uint16 _loc, const void* _data, uint32 _size) = 0;
  virtual void setMarker(const char* _marker, uint32 _size) = 0;
  virtual void submit(Frame* _render, ClearQuad& _clearQuad, TextVideoMemBlitter& _textVideoMemBlitter) = 0;
  virtual void blitSetup(TextVideoMemBlitter& _blitter) = 0;
  virtual void blitRender(TextVideoMemBlitter& _blitter, uint32 _numIndices) = 0;
};

inline RendererContextI::~RendererContextI()
{
}

#define BGFX_CONFIG_MAX_FRAME_BUFFERS 32

/// Platform data.
///
/// @attention C99 equivalent is `bgfx_platform_data_t`.
///
struct PlatformData
{
  void* ndt;          //!< Native display type.
  void* nwh;          //!< Native window handle.
  void* context;      //!< GL context, or D3D device.
  void* backBuffer;   //!< GL backbuffer, or D3D render target view.
  void* backBufferDS; //!< Backbuffer depth/stencil.
} g_platformData;

struct RendererContextD3D11 : public RendererContextI
{
  RendererContextD3D11()
    : m_d3d11dll(NULL)
    , m_dxgidll(NULL)
    , m_dxgidebugdll(NULL)
    , m_driverType(D3D_DRIVER_TYPE_NULL)
    , m_featureLevel(D3D_FEATURE_LEVEL(0))
    , m_adapter(NULL)
    , m_factory(NULL)
    , m_swapChain(NULL)
    , m_lost(0)
    , m_numWindows(0)
    , m_device(NULL)
    , m_deviceCtx(NULL)
    , m_infoQueue(NULL)
    , m_backBufferColor(NULL)
    , m_backBufferDepthStencil(NULL)
    , m_currentColor(NULL)
    , m_currentDepthStencil(NULL)
    , m_captureTexture(NULL)
    , m_captureResolve(NULL)
    , m_maxAnisotropy(1)
    , m_depthClamp(false)
    , m_wireframe(false)
    , m_currentProgram(NULL)
    , m_vsChanges(0)
    , m_fsChanges(0)
    , m_rtMsaa(false)
    , m_timerQuerySupport(false)
  {
    m_fbh.idx = invalidHandle;
    memset(&m_adapterDesc, 0, sizeof(m_adapterDesc));
    memset(&m_scd, 0, sizeof(m_scd));
    memset(&m_windows, 0xff, sizeof(m_windows));
  }

  ~RendererContextD3D11()
  {
  }

  bool init(ID3D11Device* indevice,ID3D11DeviceContext* incontext,IDXGIFactory* infactry)
  {

    m_fbh.idx = invalidHandle;
    memset(m_uniforms, 0, sizeof(m_uniforms));
    memset(&m_resolution, 0, sizeof(m_resolution));

    g_platformData.context = indevice;
    m_device = (ID3D11Device*)g_platformData.context;
    m_deviceCtx = incontext;

      m_featureLevel = m_device->GetFeatureLevel();
      return false;
  }

  void shutdown()
  {
    preReset();
    m_deviceCtx->ClearState();

    for (uint32 ii = 0; ii < ARRAY_COUNT(m_frameBuffers); ++ii)
    {
      m_frameBuffers[ii].destroy();
    }

    for (uint32 ii = 0; ii < ARRAY_COUNT(m_indexBuffers); ++ii)
    {
      m_indexBuffers[ii].destroy();
    }

    for (uint32 ii = 0; ii < ARRAY_COUNT(m_vertexBuffers); ++ii)
    {
      m_vertexBuffers[ii].destroy();
    }

    for (uint32 ii = 0; ii < ARRAY_COUNT(m_shaders); ++ii)
    {
      m_shaders[ii].destroy();
    }

    for (uint32 ii = 0; ii < ARRAY_COUNT(m_textures); ++ii)
    {
      m_textures[ii].destroy();
    }

  }

  RendererType::Enum getRendererType() const
  {
    return RendererType::Direct3D11;
  }

  const char* getRendererName() const
  {
    return "d3d11_render";
  }

    void createIndexBuffer(IndexBufferHandle _handle, Memory* _mem, uint16 _flags)
  {
    m_indexBuffers[_handle.idx].create(_mem->size, _mem->data, _flags);
  }

    void destroyIndexBuffer(IndexBufferHandle _handle)
  {
    m_indexBuffers[_handle.idx].destroy();
  }

    void createVertexDecl(VertexDeclHandle _handle, const VertexDecl& _decl)
  {
    VertexDecl& decl = m_vertexDecls[_handle.idx];
    memcpy(&decl, &_decl, sizeof(VertexDecl));
    //dump(decl);
  }

    void destroyVertexDecl(VertexDeclHandle /*_handle*/) 
  {
  }

  void createVertexBuffer(VertexBufferHandle _handle, Memory* _mem, VertexDeclHandle _declHandle, uint16 _flags) 
  {
    m_vertexBuffers[_handle.idx].create(_mem->size, _mem->data, _declHandle, _flags);
  }

    void destroyVertexBuffer(VertexBufferHandle _handle) 
  {
    m_vertexBuffers[_handle.idx].destroy();
  }

    void createDynamicIndexBuffer(IndexBufferHandle _handle, uint32 _size, uint16 _flags) 
  {
    m_indexBuffers[_handle.idx].create(_size, NULL, _flags);
  }

    void updateDynamicIndexBuffer(IndexBufferHandle _handle, uint32 _offset, uint32 _size, Memory* _mem) 
  {
    m_indexBuffers[_handle.idx].update(_offset, RBMath::get_min<u32>(_size, _mem->size), _mem->data);
  }

    void destroyDynamicIndexBuffer(IndexBufferHandle _handle) 
  {
    m_indexBuffers[_handle.idx].destroy();
  }

    void createDynamicVertexBuffer(VertexBufferHandle _handle, uint32 _size, uint16 _flags) 
  {
    VertexDeclHandle decl = RES_INVALID_HANDLE;
    m_vertexBuffers[_handle.idx].create(_size, NULL, decl, _flags);
  }

    void updateDynamicVertexBuffer(VertexBufferHandle _handle, uint32 _offset, uint32 _size, Memory* _mem) 
  {
    m_vertexBuffers[_handle.idx].update(_offset, RBMath::get_min<u32>(_size, _mem->size), _mem->data);
  }

    void destroyDynamicVertexBuffer(VertexBufferHandle _handle) 
  {
    m_vertexBuffers[_handle.idx].destroy();
  }

    void createShader(ShaderHandle _handle, Memory* _mem) 
  {
    m_shaders[_handle.idx].create(_mem);
  }

    void destroyShader(ShaderHandle _handle) 
  {
    m_shaders[_handle.idx].destroy();
  }

    void createProgram(ProgramHandle _handle, ShaderHandle _vsh, ShaderHandle _fsh) 
  {
    m_program[_handle.idx].create(&m_shaders[_vsh.idx], is_valid(_fsh) ? &m_shaders[_fsh.idx] : NULL);
  }

    void destroyProgram(ProgramHandle _handle) 
  {
    m_program[_handle.idx].destroy();
  }

    void createTexture(TextureHandle _handle, Memory* _mem, uint32 _flags, uint8 _skip) 
  {
    m_textures[_handle.idx].create(_mem, _flags, _skip);
  }

    void updateTextureBegin(TextureHandle /*_handle*/, uint8 /*_side*/, uint8 /*_mip*/) 
  {
  }

  void updateTexture(TextureHandle _handle, uint8 _side, uint8 _mip, const RBAABBI& _rect, uint16 _z, uint16 _depth, uint16 _pitch, const Memory* _mem) 
  {
    m_textures[_handle.idx].update(_side, _mip, _rect, _z, _depth, _pitch, _mem);
  }

    void updateTextureEnd() 
  {
  }

  void readTexture(TextureHandle _handle, void* _data,uint8 _bpp = 0) 
  {
    const TextureD3D11& texture = m_textures[_handle.idx];
    D3D11_MAPPED_SUBRESOURCE mapped;
    VERIFYD3D11RESULT(m_deviceCtx->Map(texture.m_ptr, 0, D3D11_MAP_READ, 0, &mapped));

    uint8* src = (uint8_t*)mapped.pData;
    uint32 srcPitch = mapped.RowPitch;

    const uint8 bpp = _bpp;// getBitsPerPixel(TextureFormat::Enum(texture.m_textureFormat));
    uint8* dst = (uint8*)_data;
    uint32 dstPitch = texture.m_width*bpp / 8;

    uint32 pitch = RBMath::get_min<u32>(srcPitch, dstPitch);

    for (uint32 yy = 0, height = texture.m_height; yy < height; ++yy)
    {
      memcpy(dst, src, pitch);

      src += srcPitch;
      dst += dstPitch;
    }

    m_deviceCtx->Unmap(texture.m_ptr, 0);
  }

    void resizeTexture(TextureHandle _handle, uint16 _width, uint16 _height, uint8 _numMips) BX_OVERRIDE
  {
    TextureD3D11& texture = m_textures[_handle.idx];

    uint32 size = sizeof(uint32) + sizeof(TextureCreate);
    const Memory* mem = Memory();

    mem->data = BX_ALLOC(g_crt_allocator,size);
     mem->size = size;

    StaticMemoryBlockWriter writer(mem->data, mem->size);
    uint32 magic = BGFX_CHUNK_MAGIC_TEX;
    write(&writer, magic);

    TextureCreate tc;
    tc.m_width = _width;
    tc.m_height = _height;
    tc.m_depth = 0;
    tc.m_numLayers = 1;
    tc.m_numMips = _numMips;
    tc.m_format = TextureFormat::Enum(texture.m_requestedFormat);
    tc.m_cubeMap = false;
    tc.m_mem = nullptr;
    write(&writer, tc);

    texture.destroy();
    texture.create(mem, texture.m_flags, 0);

    BX_FREE(g_crt_allocator,mem->data);
  }

    void overrideInternal(TextureHandle _handle, uintptr_t _ptr)
  {
    // Resource ref. counts might be messed up outside of bgfx.
    // Disabling ref. count check once texture is overridden.
    //setGraphicsDebuggerPresent(true);
    m_textures[_handle.idx].overrideInternal(_ptr);
  }

    uintptr_t getInternal(TextureHandle _handle)
  {
    // Resource ref. counts might be messed up outside of bgfx.
    // Disabling ref. count check once texture is overridden.
    //setGraphicsDebuggerPresent(true);
    return uintptr_t(m_textures[_handle.idx].m_ptr);
  }

    void destroyTexture(TextureHandle _handle) 
  {
    m_textures[_handle.idx].destroy();
  }

    void createFrameBuffer(FrameBufferHandle _handle, uint8 _num, const Attachment* _attachment) 
  {
    m_frameBuffers[_handle.idx].create(_num, _attachment);
  }

    void createFrameBuffer(FrameBufferHandle _handle, void* _nwh, uint32 _width, uint32 _height, TextureFormat::Enum _depthFormat)
  {
    uint16 denseIdx = m_numWindows++;
    m_windows[denseIdx] = _handle;
    m_frameBuffers[_handle.idx].create(denseIdx, _nwh, _width, _height, _depthFormat);
  }

    void destroyFrameBuffer(FrameBufferHandle _handle)
  {
    uint16 denseIdx = m_frameBuffers[_handle.idx].destroy();
    if (MAX_U16 != denseIdx)
    {
      --m_numWindows;
      if (m_numWindows > 1)
      {
        FrameBufferHandle handle = m_windows[m_numWindows];
        m_windows[denseIdx] = handle;
        m_frameBuffers[handle.idx].m_denseIdx = denseIdx;
      }
    }
  }

    void createUniform(UniformHandle _handle, UniformType::Enum _type, uint16 _num, const char* _name)
  {
    if (NULL != m_uniforms[_handle.idx])
    {
      BX_FREE(g_crt_allocator, m_uniforms[_handle.idx]);
    }

    uint32 size =Align(g_uniformTypeSize[_type] * _num,16);
    void* data = BX_ALLOC(g_crt_allocator, size);
    memset(data, 0, size);
    m_uniforms[_handle.idx] = data;
    m_uniformReg.add(_handle, _name, data);
  }

    void destroyUniform(UniformHandle _handle) 
  {
    BX_FREE(g_crt_allocator, m_uniforms[_handle.idx]);
    m_uniforms[_handle.idx] = NULL;
    m_uniformReg.remove(_handle);
  }

    void saveScreenShot(const char* _filePath) 
  {
    if (NULL == m_swapChain)
    {
      CHECKF("Unable to capture screenshot %s.", _filePath);
      return;
    }

    ID3D11Texture2D* backBuffer;
    VERIFYD3D11RESULT(m_swapChain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&backBuffer));

    D3D11_TEXTURE2D_DESC backBufferDesc;
    backBuffer->GetDesc(&backBufferDesc);

    D3D11_TEXTURE2D_DESC desc;
    memcpy(&desc, &backBufferDesc, sizeof(desc));
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    ID3D11Texture2D* texture;
    HRESULT hr = m_device->CreateTexture2D(&desc, NULL, &texture);
    if (SUCCEEDED(hr))
    {
      if (backBufferDesc.SampleDesc.Count == 1)
      {
        m_deviceCtx->CopyResource(texture, backBuffer);
      }
      else
      {
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.CPUAccessFlags = 0;
        ID3D11Texture2D* resolve;
        hr = m_device->CreateTexture2D(&desc, NULL, &resolve);
        if (SUCCEEDED(hr))
        {
          m_deviceCtx->ResolveSubresource(resolve, 0, backBuffer, 0, desc.Format);
          m_deviceCtx->CopyResource(texture, resolve);
          ReleaseCOM(resolve, 0);
        }
      }

      D3D11_MAPPED_SUBRESOURCE mapped;
      VERIFYD3D11RESULT(m_deviceCtx->Map(texture, 0, D3D11_MAP_READ, 0, &mapped));
      /*
      imageSwizzleBgra8(backBufferDesc.Width
        , backBufferDesc.Height
        , mapped.RowPitch
        , mapped.pData
        , mapped.pData
        );
      g_callback->screenShot(_filePath
        , backBufferDesc.Width
        , backBufferDesc.Height
        , mapped.RowPitch
        , mapped.pData
        , backBufferDesc.Height*mapped.RowPitch
        , false
        );
        */
      m_deviceCtx->Unmap(texture, 0);

      ReleaseCOM(texture, 0);
    }

    ReleaseCOM(backBuffer, 0);
  }

    void updateViewName(uint8 _id, const char* _name)
  {
    /*
    strlcpy(&s_viewName[_id][BGFX_CONFIG_MAX_VIEW_NAME_RESERVED]
      , _name
      , ARRAY_COUNT(s_viewName[0]) - BGFX_CONFIG_MAX_VIEW_NAME_RESERVED
      );
      */
  }

    void updateUniform(uint16 _loc, const void* _data, uint32 _size)
  {
    memcpy(m_uniforms[_loc], _data, _size);
  }

    void setMarker(const char* _marker, uint32 _size)
  {

  }

  void submit(Frame* _render, ClearQuad& _clearQuad, TextVideoMemBlitter& _textVideoMemBlitter);

  void blitSetup(TextVideoMemBlitter& _blitter)
  {
    ID3D11DeviceContext* deviceCtx = m_deviceCtx;

    uint32 width = getBufferWidth();
    uint32 height = getBufferHeight();

    FrameBufferHandle fbh = RES_INVALID_HANDLE;
    setFrameBuffer(fbh, false);

    D3D11_VIEWPORT vp;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = (float)width;
    vp.Height = (float)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    deviceCtx->RSSetViewports(1, &vp);

    uint64 state = BGFX_STATE_RGB_WRITE
      | BGFX_STATE_ALPHA_WRITE
      | BGFX_STATE_DEPTH_TEST_ALWAYS
      ;

    setBlendState(state);
    setDepthStencilState(state);
    setRasterizerState(state);

    ProgramD3D11& program = m_program[_blitter.m_program.idx];
    m_currentProgram = &program;
    deviceCtx->VSSetShader(program.m_vsh->vertexShader, NULL, 0);
    deviceCtx->VSSetConstantBuffers(0, 1, &program.m_vsh->buffer);
    deviceCtx->PSSetShader(program.m_fsh->pixelShader, NULL, 0);
    deviceCtx->PSSetConstantBuffers(0, 1, &program.m_fsh->buffer);

    VertexBufferD3D11& vb = m_vertexBuffers[_blitter.m_vb->handle.idx];
    VertexDecl& vertexDecl = m_vertexDecls[_blitter.m_vb->decl.idx];
    uint32 stride = vertexDecl.m_stride;
    uint32 offset = 0;
    deviceCtx->IASetVertexBuffers(0, 1, &vb.ptr, &stride, &offset);
    setInputLayout(vertexDecl, program, 0);

    IndexBufferD3D11& ib = m_indexBuffers[_blitter.m_ib->handle.idx];
    deviceCtx->IASetIndexBuffer(ib.ptr, DXGI_FORMAT_R16_UINT, 0);

    RBMatrix proj = RBMatrix::get_ortho(0.0f, (float)width, (float)height, 0.0f, 0.0f, 1000.0f);

    PredefinedUniform& predefined = program.m_predefined[0];
    uint8 flags = predefined.m_type;
    setShaderUniform(flags, predefined.m_loc, proj, 4);

    commitShaderConstants();
    m_textures[_blitter.m_texture.idx].commit(0, BGFX_TEXTURE_INTERNAL_DEFAULT_SAMPLER, NULL);
    commitTextureStage();
  }

    void blitRender(TextVideoMemBlitter& _blitter, uint32 _numIndices)
  {
    const uint32 numVertices = _numIndices * 4 / 6;
    if (0 < numVertices)
    {
      ID3D11DeviceContext* deviceCtx = m_deviceCtx;

      m_indexBuffers[_blitter.m_ib->handle.idx].update(0, _numIndices * 2, _blitter.m_ib->data, true);
      m_vertexBuffers[_blitter.m_vb->handle.idx].update(0, numVertices*_blitter.m_decl.m_stride, _blitter.m_vb->data, true);

      deviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
      deviceCtx->DrawIndexed(_numIndices, 0, 0);
    }
  }

    void preReset()
  {
    ovrPreReset();

    if (m_timerQuerySupport)
    {
      m_gpuTimer.preReset();
    }
    m_occlusionQuery.preReset();

    if (NULL == g_platformData.backBufferDS)
    {
      ReleaseCOM(m_backBufferDepthStencil, 0);
    }

    if (NULL != m_swapChain)
    {
      ReleaseCOM(m_backBufferColor, 0);
    }

    for (uint32 ii = 0; ii < ARRAY_COUNT(m_frameBuffers); ++ii)
    {
      m_frameBuffers[ii].preReset();
    }

    //			invalidateCache();

    capturePreReset();
  }

  void postReset()
  {
    if (NULL != m_swapChain)
    {
      ID3D11Texture2D* color;
      VERIFYD3D11RESULT(m_swapChain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&color));

      D3D11_RENDER_TARGET_VIEW_DESC desc;
      desc.ViewDimension = (m_resolution.m_flags & BGFX_RESET_MSAA_MASK)
        ? D3D11_RTV_DIMENSION_TEXTURE2DMS
        : D3D11_RTV_DIMENSION_TEXTURE2D
        ;
      desc.Texture2D.MipSlice = 0;
      desc.Format = (m_resolution.m_flags & BGFX_RESET_SRGB_BACKBUFFER)
        ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
        : DXGI_FORMAT_R8G8B8A8_UNORM
        ;

      VERIFYD3D11RESULT(m_device->CreateRenderTargetView(color, &desc, &m_backBufferColor));
      ReleaseCOM(color, 0);
    }

    if (m_timerQuerySupport)
    {
      m_gpuTimer.postReset();
    }
    m_occlusionQuery.postReset();

    ovrPostReset();

    // If OVR doesn't create separate depth stencil view, create default one.
    if (NULL == m_backBufferDepthStencil)
    {
      D3D11_TEXTURE2D_DESC dsd;
      dsd.Width = getBufferWidth();
      dsd.Height = getBufferHeight();
      dsd.MipLevels = 1;
      dsd.ArraySize = 1;
      dsd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
      dsd.SampleDesc = m_scd.SampleDesc;
      dsd.Usage = D3D11_USAGE_DEFAULT;
      dsd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
      dsd.CPUAccessFlags = 0;
      dsd.MiscFlags = 0;

      ID3D11Texture2D* depthStencil;
      VERIFYD3D11RESULT(m_device->CreateTexture2D(&dsd, NULL, &depthStencil));
      VERIFYD3D11RESULT(m_device->CreateDepthStencilView(depthStencil, NULL, &m_backBufferDepthStencil));
      DX_RELEASE(depthStencil, 0);
    }

    m_deviceCtx->OMSetRenderTargets(1, &m_backBufferColor, m_backBufferDepthStencil);

    m_currentColor = m_backBufferColor;
    m_currentDepthStencil = m_backBufferDepthStencil;

    for (uint32 ii = 0; ii < ARRAY_COUNT(m_frameBuffers); ++ii)
    {
      m_frameBuffers[ii].postReset();
    }

    capturePostReset();
  }

  static bool isLost(HRESULT _hr)
  {
    return DXGI_ERROR_DEVICE_REMOVED == _hr
      || DXGI_ERROR_DEVICE_HUNG == _hr
      || DXGI_ERROR_DEVICE_RESET == _hr
      || DXGI_ERROR_DRIVER_INTERNAL_ERROR == _hr
      || DXGI_ERROR_NOT_CURRENTLY_AVAILABLE == _hr
      ;
  }

  void flip()
  {
    if (NULL != m_swapChain)
    {
      HRESULT hr = S_OK;
      uint32 syncInterval = BX_ENABLED(!BX_PLATFORM_WINDOWS)
        ? 1 // sync interval of 0 is not supported on WinRT
        : !!(m_resolution.m_flags & BGFX_RESET_VSYNC)
        ;

      for (uint32 ii = 1, num = m_numWindows; ii < num && SUCCEEDED(hr); ++ii)
      {
        hr = m_frameBuffers[m_windows[ii].idx].m_swapChain->Present(syncInterval, 0);
      }

     

      if (FAILED(hr)
        && isLost(hr))
      {
        ++m_lost;
        BGFX_FATAL(10 > m_lost, bgfx::Fatal::DeviceLost, "Device is lost. FAILED 0x%08x", hr);
      }
      else
      {
        m_lost = 0;
      }
    }
  }

    void invalidateCache()
  {
  }

  void invalidateCompute()
  { 
    m_deviceCtx->CSSetShader(NULL, NULL, 0);

    ID3D11UnorderedAccessView* uav[BGFX_MAX_COMPUTE_BINDINGS] = {};
    m_deviceCtx->CSSetUnorderedAccessViews(0, ARRAY_COUNT(uav), uav, NULL);

    ID3D11ShaderResourceView* srv[BGFX_MAX_COMPUTE_BINDINGS] = {};
    m_deviceCtx->CSSetShaderResources(0, ARRAY_COUNT(srv), srv);

    ID3D11SamplerState* samplers[BGFX_MAX_COMPUTE_BINDINGS] = {};
    m_deviceCtx->CSSetSamplers(0, ARRAY_COUNT(samplers), samplers);
  }

  void updateMsaa()
  {
    for (uint32 ii = 1, last = 0; ii < ARRAY_COUNT(s_msaa); ++ii)
    {
      uint32 msaa = s_checkMsaa[ii];
      uint32 quality = 0;
      HRESULT hr = m_device->CheckMultisampleQualityLevels(getBufferFormat(), msaa, &quality);

      if (SUCCEEDED(hr)
        && 0 < quality)
      {
        s_msaa[ii].Count = msaa;
        s_msaa[ii].Quality = quality - 1;
        last = ii;
      }
      else
      {
        s_msaa[ii] = s_msaa[last];
      }
    }
  }

  bool updateResolution(const Resolution& _resolution)
  {
    const bool suspended = !!(_resolution.m_flags & BGFX_RESET_SUSPEND);
    const bool wasSuspended = !!(m_resolution.m_flags & BGFX_RESET_SUSPEND);
    if (suspended && wasSuspended)
    {
      return true;
    }
    else if (suspended)
    {
      m_deviceCtx->Flush();
      m_deviceCtx->ClearState();
      trim(m_device);
      suspend(m_device);
      m_resolution.m_flags |= BGFX_RESET_SUSPEND;
      return true;
    }
    else if (wasSuspended)
    {
      resume(m_device);
      m_resolution.m_flags &= ~BGFX_RESET_SUSPEND;
    }

    bool recenter = !!(_resolution.m_flags & BGFX_RESET_HMD_RECENTER);

    uint32 maxAnisotropy = 1;
    if (!!(_resolution.m_flags & BGFX_RESET_MAXANISOTROPY))
    {
      maxAnisotropy = (m_featureLevel == D3D_FEATURE_LEVEL_9_1)
        ? D3D_FL9_1_DEFAULT_MAX_ANISOTROPY
        : D3D11_REQ_MAXANISOTROPY
        ;
    }

    if (m_maxAnisotropy != maxAnisotropy)
    {
      m_maxAnisotropy = maxAnisotropy;
      m_samplerStateCache.invalidate();
    }

    bool depthClamp = true
      && !!(_resolution.m_flags & BGFX_RESET_DEPTH_CLAMP)
      && m_featureLevel > D3D_FEATURE_LEVEL_9_3 // disabling depth clamp is only supported on 10_0+
      ;

    if (m_depthClamp != depthClamp)
    {
      m_depthClamp = depthClamp;
      m_rasterizerStateCache.invalidate();
    }

    const uint32 maskFlags = ~(0
      | BGFX_RESET_HMD_RECENTER
      | BGFX_RESET_MAXANISOTROPY
      | BGFX_RESET_DEPTH_CLAMP
      | BGFX_RESET_SUSPEND
      );

    if (m_resolution.m_width != _resolution.m_width
      || m_resolution.m_height != _resolution.m_height
      || (m_resolution.m_flags&maskFlags) != (_resolution.m_flags&maskFlags))
    {
      uint32 flags = _resolution.m_flags & (~BGFX_RESET_INTERNAL_FORCE);

      bool resize = true
        && !BX_ENABLED(BX_PLATFORM_XBOXONE || BX_PLATFORM_WINRT) // can't use ResizeBuffers on Windows Phone
        && (m_resolution.m_flags&BGFX_RESET_MSAA_MASK) == (flags&BGFX_RESET_MSAA_MASK)
        ;

      m_resolution = _resolution;
      m_resolution.m_flags = flags;

      m_textVideoMem.resize(false, _resolution.m_width, _resolution.m_height);
      m_textVideoMem.clear();

      setBufferSize(_resolution.m_width, _resolution.m_height);

      preReset();

      m_deviceCtx->Flush();
      m_deviceCtx->ClearState();

      if (NULL == m_swapChain)
      {
        // Updated backbuffer if it changed in PlatformData.
        m_backBufferColor = (ID3D11RenderTargetView*)g_platformData.backBuffer;
        m_backBufferDepthStencil = (ID3D11DepthStencilView*)g_platformData.backBufferDS;
      }
      else
      {
        if (resize)
        {
          m_deviceCtx->OMSetRenderTargets(1, s_zero.m_rtv, NULL);
          DX_CHECK(m_swapChain->ResizeBuffers(2
            , getBufferWidth()
            , getBufferHeight()
            , getBufferFormat()
            , DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
            ));
        }
        else
        {
          updateMsaa();
          m_scd.SampleDesc = s_msaa[(m_resolution.m_flags&BGFX_RESET_MSAA_MASK) >> BGFX_RESET_MSAA_SHIFT];

          DX_RELEASE(m_swapChain, 0);

          SwapChainDesc* scd = &m_scd;
          SwapChainDesc swapChainScd;
          if (0 != (m_resolution.m_flags & BGFX_RESET_HMD)
            && m_ovr.isInitialized())
          {
            swapChainScd = m_scd;
            swapChainScd.SampleDesc = s_msaa[0];
            scd = &swapChainScd;
          }


          HRESULT hr;
          hr = m_factory->CreateSwapChain(m_device
            , scd
            , &m_swapChain
            );
          BGFX_FATAL(SUCCEEDED(hr), bgfx::Fatal::UnableToInitialize, "Failed to create swap chain.");
        }
      }

      postReset();
    }

    return false;
  }

  void setShaderUniform(uint8 _flags, uint32 _regIndex, const void* _val, uint32 _numRegs)
  {
    if (_flags&BGFX_UNIFORM_FRAGMENTBIT)
    {
      memcpy(&m_fsScratch[_regIndex], _val, _numRegs * 16);
      m_fsChanges += _numRegs;
    }
    else
    {
      memcpy(&m_vsScratch[_regIndex], _val, _numRegs * 16);
      m_vsChanges += _numRegs;
    }
  }

  void setShaderUniform4f(uint8 _flags, uint32 _regIndex, const void* _val, uint32 _numRegs)
  {
    setShaderUniform(_flags, _regIndex, _val, _numRegs);
  }

  void setShaderUniform4x4f(uint8 _flags, uint32 _regIndex, const void* _val, uint32 _numRegs)
  {
    setShaderUniform(_flags, _regIndex, _val, _numRegs);
  }

  void commitShaderConstants()
  {
    if (0 < m_vsChanges)
    {
      if (NULL != m_currentProgram->m_vsh->m_buffer)
      {
        m_deviceCtx->UpdateSubresource(m_currentProgram->m_vsh->m_buffer, 0, 0, m_vsScratch, 0, 0);
      }

      m_vsChanges = 0;
    }

    if (0 < m_fsChanges)
    {
      if (NULL != m_currentProgram->m_fsh->m_buffer)
      {
        m_deviceCtx->UpdateSubresource(m_currentProgram->m_fsh->m_buffer, 0, 0, m_fsScratch, 0, 0);
      }

      m_fsChanges = 0;
    }
  }

  void setFrameBuffer(FrameBufferHandle _fbh, bool _msaa = true)
  {
    if (isValid(m_fbh)
      && m_fbh.idx != _fbh.idx
      &&  m_rtMsaa)
    {
      FrameBufferD3D11& frameBuffer = m_frameBuffers[m_fbh.idx];
      frameBuffer.resolve();
    }

    if (!isValid(_fbh))
    {
      m_deviceCtx->OMSetRenderTargets(1, &m_backBufferColor, m_backBufferDepthStencil);

      m_currentColor = m_backBufferColor;
      m_currentDepthStencil = m_backBufferDepthStencil;
    }
    else
    {
      invalidateTextureStage();

      FrameBufferD3D11& frameBuffer = m_frameBuffers[_fbh.idx];
      m_deviceCtx->OMSetRenderTargets(frameBuffer.m_num, frameBuffer.m_rtv, frameBuffer.m_dsv);

      m_currentColor = frameBuffer.m_rtv[0];
      m_currentDepthStencil = frameBuffer.m_dsv;
    }

    m_fbh = _fbh;
    m_rtMsaa = _msaa;
  }

  void clear(const Clear& _clear, const float _palette[][4])
  {
    if (isValid(m_fbh))
    {
      FrameBufferD3D11& frameBuffer = m_frameBuffers[m_fbh.idx];
      frameBuffer.clear(_clear, _palette);
    }
    else
    {
      if (NULL != m_currentColor
        &&  BGFX_CLEAR_COLOR & _clear.m_flags)
      {
        if (BGFX_CLEAR_COLOR_USE_PALETTE & _clear.m_flags)
        {
          uint8 index = _clear.m_index[0];
          if (UINT8_MAX != index)
          {
            m_deviceCtx->ClearRenderTargetView(m_currentColor, _palette[index]);
          }
        }
        else
        {
          float frgba[4] =
          {
            _clear.m_index[0] * 1.0f / 255.0f,
            _clear.m_index[1] * 1.0f / 255.0f,
            _clear.m_index[2] * 1.0f / 255.0f,
            _clear.m_index[3] * 1.0f / 255.0f,
          };
          m_deviceCtx->ClearRenderTargetView(m_currentColor, frgba);
        }
      }

      if (NULL != m_currentDepthStencil
        && (BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL) & _clear.m_flags)
      {
        DWORD flags = 0;
        flags |= (_clear.m_flags & BGFX_CLEAR_DEPTH) ? D3D11_CLEAR_DEPTH : 0;
        flags |= (_clear.m_flags & BGFX_CLEAR_STENCIL) ? D3D11_CLEAR_STENCIL : 0;
        m_deviceCtx->ClearDepthStencilView(m_currentDepthStencil, flags, _clear.m_depth, _clear.m_stencil);
      }
    }
  }

  void setInputLayout(const VertexDecl& _vertexDecl, const ProgramD3D11& _program, uint16 _numInstanceData)
  {
    uint64 layoutHash = (uint64_t(_vertexDecl.m_hash) << 32) | _program.m_vsh->m_hash;
    layoutHash ^= _numInstanceData;
    ID3D11InputLayout* layout = m_inputLayoutCache.find(layoutHash);
    if (NULL == layout)
    {
      D3D11_INPUT_ELEMENT_DESC vertexElements[Attrib::Count + 1 + BGFX_CONFIG_MAX_INSTANCE_DATA_COUNT];

      VertexDecl decl;
      memcpy(&decl, &_vertexDecl, sizeof(VertexDecl));
      const uint16_t* attrMask = _program.m_vsh->m_attrMask;

      for (uint32 ii = 0; ii < Attrib::Count; ++ii)
      {
        uint16 mask = attrMask[ii];
        uint16 attr = (decl.m_attributes[ii] & mask);
        decl.m_attributes[ii] = attr == 0 ? UINT16_MAX : attr == UINT16_MAX ? 0 : attr;
      }

      D3D11_INPUT_ELEMENT_DESC* elem = fillVertexDecl(vertexElements, decl);
      uint32 num = uint32_t(elem - vertexElements);

      const D3D11_INPUT_ELEMENT_DESC inst = { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 };

      for (uint32 ii = 0; ii < _numInstanceData; ++ii)
      {
        uint32 index = 7 - ii; // TEXCOORD7 = i_data0, TEXCOORD6 = i_data1, etc.

        uint32 jj;
        D3D11_INPUT_ELEMENT_DESC* curr = vertexElements;
        for (jj = 0; jj < num; ++jj)
        {
          curr = &vertexElements[jj];
          if (0 == strcmp(curr->SemanticName, "TEXCOORD")
            && curr->SemanticIndex == index)
          {
            break;
          }
        }

        if (jj == num)
        {
          curr = elem;
          ++elem;
        }

        memcpy(curr, &inst, sizeof(D3D11_INPUT_ELEMENT_DESC));
        curr->InputSlot = 1;
        curr->SemanticIndex = index;
        curr->AlignedByteOffset = ii * 16;
      }

      num = uint32_t(elem - vertexElements);
      DX_CHECK(m_device->CreateInputLayout(vertexElements
        , num
        , _program.m_vsh->m_code->data
        , _program.m_vsh->m_code->size
        , &layout
        ));
      m_inputLayoutCache.add(layoutHash, layout);
    }

    m_deviceCtx->IASetInputLayout(layout);
  }

  void setBlendState(uint64 _state, uint32 _rgba = 0)
  {
    _state &= BGFX_D3D11_BLEND_STATE_MASK;

    bx::HashMurmur2A murmur;
    murmur.begin();
    murmur.add(_state);
    murmur.add(!!(BGFX_STATE_BLEND_INDEPENDENT & _state)
      ? _rgba
      : -1
      );
    const uint32 hash = murmur.end();

    ID3D11BlendState* bs = m_blendStateCache.find(hash);
    if (NULL == bs)
    {
      D3D11_BLEND_DESC desc;
      desc.AlphaToCoverageEnable = !!(BGFX_STATE_BLEND_ALPHA_TO_COVERAGE & _state);
      desc.IndependentBlendEnable = !!(BGFX_STATE_BLEND_INDEPENDENT       & _state);

      D3D11_RENDER_TARGET_BLEND_DESC* drt = &desc.RenderTarget[0];
      drt->BlendEnable = !!(BGFX_STATE_BLEND_MASK & _state);

      const uint32 blend = uint32_t((_state&BGFX_STATE_BLEND_MASK) >> BGFX_STATE_BLEND_SHIFT);
      const uint32 equation = uint32_t((_state&BGFX_STATE_BLEND_EQUATION_MASK) >> BGFX_STATE_BLEND_EQUATION_SHIFT);

      const uint32 srcRGB = (blend)& 0xf;
      const uint32 dstRGB = (blend >> 4) & 0xf;
      const uint32 srcA = (blend >> 8) & 0xf;
      const uint32 dstA = (blend >> 12) & 0xf;

      const uint32 equRGB = (equation)& 0x7;
      const uint32 equA = (equation >> 3) & 0x7;

      drt->SrcBlend = s_blendFactor[srcRGB][0];
      drt->DestBlend = s_blendFactor[dstRGB][0];
      drt->BlendOp = s_blendEquation[equRGB];

      drt->SrcBlendAlpha = s_blendFactor[srcA][1];
      drt->DestBlendAlpha = s_blendFactor[dstA][1];
      drt->BlendOpAlpha = s_blendEquation[equA];

      uint8 writeMask = (_state&BGFX_STATE_ALPHA_WRITE)
        ? D3D11_COLOR_WRITE_ENABLE_ALPHA
        : 0
        ;
      writeMask |= (_state&BGFX_STATE_RGB_WRITE)
        ? D3D11_COLOR_WRITE_ENABLE_RED
        | D3D11_COLOR_WRITE_ENABLE_GREEN
        | D3D11_COLOR_WRITE_ENABLE_BLUE
        : 0
        ;

      drt->RenderTargetWriteMask = writeMask;

      if (desc.IndependentBlendEnable)
      {
        for (uint32 ii = 1, rgba = _rgba; ii < BGFX_CONFIG_MAX_FRAME_BUFFER_ATTACHMENTS; ++ii, rgba >>= 11)
        {
          drt = &desc.RenderTarget[ii];
          drt->BlendEnable = 0 != (rgba & 0x7ff);

          const uint32 src = (rgba)& 0xf;
          const uint32 dst = (rgba >> 4) & 0xf;
          const uint32 equ = (rgba >> 8) & 0x7;

          drt->SrcBlend = s_blendFactor[src][0];
          drt->DestBlend = s_blendFactor[dst][0];
          drt->BlendOp = s_blendEquation[equ];

          drt->SrcBlendAlpha = s_blendFactor[src][1];
          drt->DestBlendAlpha = s_blendFactor[dst][1];
          drt->BlendOpAlpha = s_blendEquation[equ];

          drt->RenderTargetWriteMask = writeMask;
        }
      }
      else
      {
        for (uint32 ii = 1; ii < BGFX_CONFIG_MAX_FRAME_BUFFER_ATTACHMENTS; ++ii)
        {
          memcpy(&desc.RenderTarget[ii], drt, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
        }
      }

      DX_CHECK(m_device->CreateBlendState(&desc, &bs));

      m_blendStateCache.add(hash, bs);
    }

    const uint64 f0 = BGFX_STATE_BLEND_FACTOR;
    const uint64 f1 = BGFX_STATE_BLEND_INV_FACTOR;
    const uint64 f2 = BGFX_STATE_BLEND_FACTOR << 4;
    const uint64 f3 = BGFX_STATE_BLEND_INV_FACTOR << 4;
    bool hasFactor = 0
      || f0 == (_state & f0)
      || f1 == (_state & f1)
      || f2 == (_state & f2)
      || f3 == (_state & f3)
      ;

    float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    if (hasFactor)
    {
      blendFactor[0] = ((_rgba >> 24)) / 255.0f;
      blendFactor[1] = ((_rgba >> 16) & 0xff) / 255.0f;
      blendFactor[2] = ((_rgba >> 8) & 0xff) / 255.0f;
      blendFactor[3] = ((_rgba)& 0xff) / 255.0f;
    }

    m_deviceCtx->OMSetBlendState(bs, blendFactor, 0xffffffff);
  }

  void setDepthStencilState(uint64 _state, uint64 _stencil = 0)
  {
    uint32 func = (_state&BGFX_STATE_DEPTH_TEST_MASK) >> BGFX_STATE_DEPTH_TEST_SHIFT;
    _state &= 0 == func ? 0 : BGFX_D3D11_DEPTH_STENCIL_MASK;

    uint32 fstencil = unpackStencil(0, _stencil);
    uint32 ref = (fstencil&BGFX_STENCIL_FUNC_REF_MASK) >> BGFX_STENCIL_FUNC_REF_SHIFT;
    _stencil &= packStencil(~BGFX_STENCIL_FUNC_REF_MASK, BGFX_STENCIL_MASK);

    bx::HashMurmur2A murmur;
    murmur.begin();
    murmur.add(_state);
    murmur.add(_stencil);
    uint32 hash = murmur.end();

    ID3D11DepthStencilState* dss = m_depthStencilStateCache.find(hash);
    if (NULL == dss)
    {
      D3D11_DEPTH_STENCIL_DESC desc;
      memset(&desc, 0, sizeof(desc));
      desc.DepthEnable = 0 != func;
      desc.DepthWriteMask = !!(BGFX_STATE_DEPTH_WRITE & _state) ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
      desc.DepthFunc = s_cmpFunc[func];

      uint32 bstencil = unpackStencil(1, _stencil);
      uint32 frontAndBack = bstencil != BGFX_STENCIL_NONE && bstencil != fstencil;
      bstencil = frontAndBack ? bstencil : fstencil;

      desc.StencilEnable = 0 != _stencil;
      desc.StencilReadMask = (fstencil&BGFX_STENCIL_FUNC_RMASK_MASK) >> BGFX_STENCIL_FUNC_RMASK_SHIFT;
      desc.StencilWriteMask = 0xff;
      desc.FrontFace.StencilFailOp = s_stencilOp[(fstencil&BGFX_STENCIL_OP_FAIL_S_MASK) >> BGFX_STENCIL_OP_FAIL_S_SHIFT];
      desc.FrontFace.StencilDepthFailOp = s_stencilOp[(fstencil&BGFX_STENCIL_OP_FAIL_Z_MASK) >> BGFX_STENCIL_OP_FAIL_Z_SHIFT];
      desc.FrontFace.StencilPassOp = s_stencilOp[(fstencil&BGFX_STENCIL_OP_PASS_Z_MASK) >> BGFX_STENCIL_OP_PASS_Z_SHIFT];
      desc.FrontFace.StencilFunc = s_cmpFunc[(fstencil&BGFX_STENCIL_TEST_MASK) >> BGFX_STENCIL_TEST_SHIFT];
      desc.BackFace.StencilFailOp = s_stencilOp[(bstencil&BGFX_STENCIL_OP_FAIL_S_MASK) >> BGFX_STENCIL_OP_FAIL_S_SHIFT];
      desc.BackFace.StencilDepthFailOp = s_stencilOp[(bstencil&BGFX_STENCIL_OP_FAIL_Z_MASK) >> BGFX_STENCIL_OP_FAIL_Z_SHIFT];
      desc.BackFace.StencilPassOp = s_stencilOp[(bstencil&BGFX_STENCIL_OP_PASS_Z_MASK) >> BGFX_STENCIL_OP_PASS_Z_SHIFT];
      desc.BackFace.StencilFunc = s_cmpFunc[(bstencil&BGFX_STENCIL_TEST_MASK) >> BGFX_STENCIL_TEST_SHIFT];

      DX_CHECK(m_device->CreateDepthStencilState(&desc, &dss));

      m_depthStencilStateCache.add(hash, dss);
    }

    m_deviceCtx->OMSetDepthStencilState(dss, ref);
  }

  void setDebugWireframe(bool _wireframe)
  {
    if (m_wireframe != _wireframe)
    {
      m_wireframe = _wireframe;
      m_rasterizerStateCache.invalidate();
    }
  }

  void setRasterizerState(uint64 _state, bool _wireframe = false, bool _scissor = false)
  {
    _state &= 0
      | BGFX_STATE_CULL_MASK
      | BGFX_STATE_MSAA
      | BGFX_STATE_LINEAA
      | BGFX_STATE_CONSERVATIVE_RASTER
      ;
    _state |= _wireframe ? BGFX_STATE_PT_LINES : BGFX_STATE_NONE;
    _state |= _scissor ? BGFX_STATE_RESERVED_MASK : 0;
    _state &= ~(m_deviceInterfaceVersion >= 3 ? 0 : BGFX_STATE_CONSERVATIVE_RASTER);

    ID3D11RasterizerState* rs = m_rasterizerStateCache.find(_state);
    if (NULL == rs)
    {
      uint32 cull = (_state&BGFX_STATE_CULL_MASK) >> BGFX_STATE_CULL_SHIFT;

#if BX_PLATFORM_WINDOWS
      if (m_deviceInterfaceVersion >= 3)
      {
        D3D11_RASTERIZER_DESC2 desc;
        desc.FillMode = _wireframe ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
        desc.CullMode = s_cullMode[cull];
        desc.FrontCounterClockwise = false;
        desc.DepthBias = 0;
        desc.DepthBiasClamp = 0.0f;
        desc.SlopeScaledDepthBias = 0.0f;
        desc.DepthClipEnable = !m_depthClamp;
        desc.ScissorEnable = _scissor;
        desc.MultisampleEnable = !!(_state&BGFX_STATE_MSAA);
        desc.AntialiasedLineEnable = !!(_state&BGFX_STATE_LINEAA);
        desc.ForcedSampleCount = 0;
        desc.ConservativeRaster = !!(_state&BGFX_STATE_CONSERVATIVE_RASTER)
          ? D3D11_CONSERVATIVE_RASTERIZATION_MODE_ON
          : D3D11_CONSERVATIVE_RASTERIZATION_MODE_OFF
          ;

        ID3D11Device3* device3 = reinterpret_cast<ID3D11Device3*>(m_device);
        DX_CHECK(device3->CreateRasterizerState2(&desc, reinterpret_cast<ID3D11RasterizerState2**>(&rs)));
      }
      else
#endif // BX_PLATFORM_WINDOWS
      {
        D3D11_RASTERIZER_DESC desc;
        desc.FillMode = _wireframe ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
        desc.CullMode = s_cullMode[cull];
        desc.FrontCounterClockwise = false;
        desc.DepthBias = 0;
        desc.DepthBiasClamp = 0.0f;
        desc.SlopeScaledDepthBias = 0.0f;
        desc.DepthClipEnable = !m_depthClamp;
        desc.ScissorEnable = _scissor;
        desc.MultisampleEnable = !!(_state&BGFX_STATE_MSAA);
        desc.AntialiasedLineEnable = !!(_state&BGFX_STATE_LINEAA);

        DX_CHECK(m_device->CreateRasterizerState(&desc, &rs));
      }

      m_rasterizerStateCache.add(_state, rs);
    }

    m_deviceCtx->RSSetState(rs);
  }

  ID3D11SamplerState* getSamplerState(uint32 _flags, const float _rgba[4])
  {
    const uint32 index = (_flags & BGFX_TEXTURE_BORDER_COLOR_MASK) >> BGFX_TEXTURE_BORDER_COLOR_SHIFT;
    _flags &= BGFX_TEXTURE_SAMPLER_BITS_MASK;

    // Force both min+max anisotropic, can't be set individually.
    _flags |= 0 != (_flags & (BGFX_TEXTURE_MIN_ANISOTROPIC | BGFX_TEXTURE_MAG_ANISOTROPIC))
      ? BGFX_TEXTURE_MIN_ANISOTROPIC | BGFX_TEXTURE_MAG_ANISOTROPIC
      : 0
      ;

    uint32 hash;
    ID3D11SamplerState* sampler;
    if (!needBorderColor(_flags))
    {
      bx::HashMurmur2A murmur;
      murmur.begin();
      murmur.add(_flags);
      murmur.add(-1);
      hash = murmur.end();
      _rgba = s_zero.m_zerof;

      sampler = m_samplerStateCache.find(hash);
    }
    else
    {
      bx::HashMurmur2A murmur;
      murmur.begin();
      murmur.add(_flags);
      murmur.add(index);
      hash = murmur.end();
      _rgba = NULL == _rgba ? s_zero.m_zerof : _rgba;

      sampler = m_samplerStateCache.find(hash);
      if (NULL != sampler)
      {
        D3D11_SAMPLER_DESC sd;
        sampler->GetDesc(&sd);
        if (0 != memcmp(_rgba, sd.BorderColor, 16))
        {
          // Sampler will be released when updated sampler
          // is added to cache.
          sampler = NULL;
        }
      }
    }

    if (NULL == sampler)
    {
      const uint32 cmpFunc = (_flags&BGFX_TEXTURE_COMPARE_MASK) >> BGFX_TEXTURE_COMPARE_SHIFT;
      const uint8  minFilter = s_textureFilter[0][(_flags&BGFX_TEXTURE_MIN_MASK) >> BGFX_TEXTURE_MIN_SHIFT];
      const uint8  magFilter = s_textureFilter[1][(_flags&BGFX_TEXTURE_MAG_MASK) >> BGFX_TEXTURE_MAG_SHIFT];
      const uint8  mipFilter = s_textureFilter[2][(_flags&BGFX_TEXTURE_MIP_MASK) >> BGFX_TEXTURE_MIP_SHIFT];
      const uint8  filter = 0 == cmpFunc ? 0 : D3D11_COMPARISON_FILTERING_BIT;

      D3D11_SAMPLER_DESC sd;
      sd.Filter = (D3D11_FILTER)(filter | minFilter | magFilter | mipFilter);
      sd.AddressU = s_textureAddress[(_flags&BGFX_TEXTURE_U_MASK) >> BGFX_TEXTURE_U_SHIFT];
      sd.AddressV = s_textureAddress[(_flags&BGFX_TEXTURE_V_MASK) >> BGFX_TEXTURE_V_SHIFT];
      sd.AddressW = s_textureAddress[(_flags&BGFX_TEXTURE_W_MASK) >> BGFX_TEXTURE_W_SHIFT];
      sd.MipLODBias = 0.0f;
      sd.MaxAnisotropy = m_maxAnisotropy;
      sd.ComparisonFunc = 0 == cmpFunc ? D3D11_COMPARISON_NEVER : s_cmpFunc[cmpFunc];
      sd.BorderColor[0] = _rgba[0];
      sd.BorderColor[1] = _rgba[1];
      sd.BorderColor[2] = _rgba[2];
      sd.BorderColor[3] = _rgba[3];
      sd.MinLOD = 0;
      sd.MaxLOD = D3D11_FLOAT32_MAX;

      m_device->CreateSamplerState(&sd, &sampler);
      DX_CHECK_REFCOUNT(sampler, 1);

      m_samplerStateCache.add(hash, sampler);
    }

    return sampler;
  }

  bool isVisible(Frame* _render, OcclusionQueryHandle _handle, bool _visible)
  {
    m_occlusionQuery.resolve(_render);
    return _visible == (0 != _render->m_occlusion[_handle.idx]);
  }

  DXGI_FORMAT getBufferFormat()
  {
#if BX_PLATFORM_WINDOWS
    return m_scd.BufferDesc.Format;
#else
    return m_scd.Format;
#endif
  }

  uint32 getBufferWidth()
  {
#if BX_PLATFORM_WINDOWS
    return m_scd.BufferDesc.Width;
#else
    return m_scd.Width;
#endif
  }

  uint32 getBufferHeight()
  {
#if BX_PLATFORM_WINDOWS
    return m_scd.BufferDesc.Height;
#else
    return m_scd.Height;
#endif
  }

  void setBufferSize(uint32 _width, uint32 _height)
  {
#if BX_PLATFORM_WINDOWS
    m_scd.BufferDesc.Width = _width;
    m_scd.BufferDesc.Height = _height;
#else
    m_scd.Width = _width;
    m_scd.Height = _height;
#endif
  }

  void commitTextureStage()
  {
    // vertex texture fetch not supported on 9_1 through 9_3
    if (m_featureLevel > D3D_FEATURE_LEVEL_9_3)
    {
      m_deviceCtx->VSSetShaderResources(0, BGFX_CONFIG_MAX_TEXTURE_SAMPLERS, m_textureStage.m_srv);
      m_deviceCtx->VSSetSamplers(0, BGFX_CONFIG_MAX_TEXTURE_SAMPLERS, m_textureStage.m_sampler);
    }

    m_deviceCtx->PSSetShaderResources(0, BGFX_CONFIG_MAX_TEXTURE_SAMPLERS, m_textureStage.m_srv);
    m_deviceCtx->PSSetSamplers(0, BGFX_CONFIG_MAX_TEXTURE_SAMPLERS, m_textureStage.m_sampler);
  }

  void invalidateTextureStage()
  {
    m_textureStage.clear();
    commitTextureStage();
  }

  ID3D11UnorderedAccessView* getCachedUav(TextureHandle _handle, uint8 _mip)
  {
    bx::HashMurmur2A murmur;
    murmur.begin();
    murmur.add(_handle);
    murmur.add(_mip);
    murmur.add(0);
    uint32 hash = murmur.end();

    IUnknown** ptr = m_srvUavLru.find(hash);
    ID3D11UnorderedAccessView* uav;
    if (NULL == ptr)
    {
      TextureD3D11& texture = m_textures[_handle.idx];

      D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
      desc.Format = s_textureFormat[texture.m_textureFormat].m_fmtSrv;
      switch (texture.m_type)
      {
      case TextureD3D11::Texture2D:
      case TextureD3D11::TextureCube:
        desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice = _mip;
        break;

      case TextureD3D11::Texture3D:
        desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
        desc.Texture3D.MipSlice = _mip;
        desc.Texture3D.FirstWSlice = 0;
        desc.Texture3D.WSize = UINT32_MAX;
        break;
      }

      DX_CHECK(m_device->CreateUnorderedAccessView(texture.m_ptr, &desc, &uav));

      m_srvUavLru.add(hash, uav, _handle.idx);
    }
    else
    {
      uav = static_cast<ID3D11UnorderedAccessView*>(*ptr);
    }

    return uav;
  }

  ID3D11ShaderResourceView* getCachedSrv(TextureHandle _handle, uint8 _mip)
  {
    bx::HashMurmur2A murmur;
    murmur.begin();
    murmur.add(_handle);
    murmur.add(_mip);
    murmur.add(0);
    uint32 hash = murmur.end();

    IUnknown** ptr = m_srvUavLru.find(hash);
    ID3D11ShaderResourceView* srv;
    if (NULL == ptr)
    {
      const TextureD3D11& texture = m_textures[_handle.idx];
      const uint32 msaaQuality = bx::uint32_satsub((texture.m_flags&BGFX_TEXTURE_RT_MSAA_MASK) >> BGFX_TEXTURE_RT_MSAA_SHIFT, 1);
      const DXGI_SAMPLE_DESC& msaa = s_msaa[msaaQuality];
      const bool msaaSample = 1 < msaa.Count && 0 != (texture.m_flags&BGFX_TEXTURE_MSAA_SAMPLE);

      D3D11_SHADER_RESOURCE_VIEW_DESC desc;
      desc.Format = s_textureFormat[texture.m_textureFormat].m_fmtSrv;
      switch (texture.m_type)
      {
      case TextureD3D11::Texture2D:
        desc.ViewDimension = msaaSample
          ? D3D11_SRV_DIMENSION_TEXTURE2DMS
          : D3D11_SRV_DIMENSION_TEXTURE2D
          ;
        desc.Texture2D.MostDetailedMip = _mip;
        desc.Texture2D.MipLevels = 1;
        break;

      case TextureD3D11::TextureCube:
        desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        desc.TextureCube.MostDetailedMip = _mip;
        desc.TextureCube.MipLevels = 1;
        break;

      case TextureD3D11::Texture3D:
        desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
        desc.Texture3D.MostDetailedMip = _mip;
        desc.Texture3D.MipLevels = 1;
        break;
      }

      DX_CHECK(m_device->CreateShaderResourceView(texture.m_ptr, &desc, &srv));

      m_srvUavLru.add(hash, srv, _handle.idx);
    }
    else
    {
      srv = static_cast<ID3D11ShaderResourceView*>(*ptr);
    }

    return srv;
  }

  void ovrPostReset()
  {
#if BGFX_CONFIG_USE_OVR
    if (m_resolution.m_flags & (BGFX_RESET_HMD | BGFX_RESET_HMD_DEBUG))
    {
      const uint32 msaaSamples = 1 << ((m_resolution.m_flags&BGFX_RESET_MSAA_MASK) >> BGFX_RESET_MSAA_SHIFT);
      m_ovr.postReset(msaaSamples, m_resolution.m_width, m_resolution.m_height);
    }
#endif // BGFX_CONFIG_USE_OVR
  }

  void ovrPreReset()
  {
#if BGFX_CONFIG_USE_OVR
    m_ovr.preReset();
#endif // BGFX_CONFIG_USE_OVR
  }

  void capturePostReset()
  {
    if (m_resolution.m_flags&BGFX_RESET_CAPTURE)
    {
      ID3D11Texture2D* backBuffer;
      DX_CHECK(m_swapChain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&backBuffer));

      D3D11_TEXTURE2D_DESC backBufferDesc;
      backBuffer->GetDesc(&backBufferDesc);

      D3D11_TEXTURE2D_DESC desc;
      memcpy(&desc, &backBufferDesc, sizeof(desc));
      desc.SampleDesc.Count = 1;
      desc.SampleDesc.Quality = 0;
      desc.Usage = D3D11_USAGE_STAGING;
      desc.BindFlags = 0;
      desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

      HRESULT hr = m_device->CreateTexture2D(&desc, NULL, &m_captureTexture);
      if (SUCCEEDED(hr))
      {
        if (backBufferDesc.SampleDesc.Count != 1)
        {
          desc.Usage = D3D11_USAGE_DEFAULT;
          desc.CPUAccessFlags = 0;
          m_device->CreateTexture2D(&desc, NULL, &m_captureResolve);
        }

        g_callback->captureBegin(backBufferDesc.Width, backBufferDesc.Height, backBufferDesc.Width * 4, TextureFormat::BGRA8, false);
      }

      DX_RELEASE(backBuffer, 0);
    }
  }

  void capturePreReset()
  {
    if (NULL != m_captureTexture)
    {
      g_callback->captureEnd();
    }

    DX_RELEASE(m_captureResolve, 0);
    DX_RELEASE(m_captureTexture, 0);
  }

  void capture()
  {
    if (NULL != m_captureTexture)
    {
      ID3D11Texture2D* backBuffer;
      DX_CHECK(m_swapChain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&backBuffer));

      if (NULL == m_captureResolve)
      {
        m_deviceCtx->CopyResource(m_captureTexture, backBuffer);
      }
      else
      {
        m_deviceCtx->ResolveSubresource(m_captureResolve, 0, backBuffer, 0, getBufferFormat());
        m_deviceCtx->CopyResource(m_captureTexture, m_captureResolve);
      }

      D3D11_MAPPED_SUBRESOURCE mapped;
      DX_CHECK(m_deviceCtx->Map(m_captureTexture, 0, D3D11_MAP_READ, 0, &mapped));

      imageSwizzleBgra8(getBufferWidth()
        , getBufferHeight()
        , mapped.RowPitch
        , mapped.pData
        , mapped.pData
        );

      g_callback->captureFrame(mapped.pData, getBufferHeight()*mapped.RowPitch);

      m_deviceCtx->Unmap(m_captureTexture, 0);

      DX_RELEASE(backBuffer, 0);
    }
  }

  void commit(UniformBuffer& _uniformBuffer)
  {
    _uniformBuffer.reset();

    for (;;)
    {
      uint32 opcode = _uniformBuffer.read();

      if (UniformType::End == opcode)
      {
        break;
      }

      UniformType::Enum type;
      uint16 loc;
      uint16 num;
      uint16 copy;
      UniformBuffer::decodeOpcode(opcode, type, loc, num, copy);

      const char* data;
      if (copy)
      {
        data = _uniformBuffer.read(g_uniformTypeSize[type] * num);
      }
      else
      {
        UniformHandle handle;
        memcpy(&handle, _uniformBuffer.read(sizeof(UniformHandle)), sizeof(UniformHandle));
        data = (const char*)m_uniforms[handle.idx];
      }

#define CASE_IMPLEMENT_UNIFORM(_uniform, _dxsuffix, _type) \
		case UniformType::_uniform: \
		case UniformType::_uniform|BGFX_UNIFORM_FRAGMENTBIT: \
            				{ \
					setShaderUniform(uint8_t(type), loc, data, num); \
            				} \
				break;

      switch ((uint32_t)type)
      {
      case UniformType::Mat3:
      case UniformType::Mat3 | BGFX_UNIFORM_FRAGMENTBIT: \
      {
        float* value = (float*)data;
        for (uint32 ii = 0, count = num / 3; ii < count; ++ii, loc += 3 * 16, value += 9)
        {
          Matrix4 mtx;
          mtx.un.val[0] = value[0];
          mtx.un.val[1] = value[1];
          mtx.un.val[2] = value[2];
          mtx.un.val[3] = 0.0f;
          mtx.un.val[4] = value[3];
          mtx.un.val[5] = value[4];
          mtx.un.val[6] = value[5];
          mtx.un.val[7] = 0.0f;
          mtx.un.val[8] = value[6];
          mtx.un.val[9] = value[7];
          mtx.un.val[10] = value[8];
          mtx.un.val[11] = 0.0f;
          setShaderUniform(uint8_t(type), loc, &mtx.un.val[0], 3);
        }
      }
                                                         break;

                                                         CASE_IMPLEMENT_UNIFORM(Int1, I, int);
                                                         CASE_IMPLEMENT_UNIFORM(Vec4, F, float);
                                                         CASE_IMPLEMENT_UNIFORM(Mat4, F, float);

      case UniformType::End:
        break;

      default:
        BX_TRACE("%4d: INVALID 0x%08x, t %d, l %d, n %d, c %d", _uniformBuffer.getPos(), opcode, type, loc, num, copy);
        break;
      }
#undef CASE_IMPLEMENT_UNIFORM
    }
  }

  void clearQuad(ClearQuad& _clearQuad, const Rect& _rect, const Clear& _clear, const float _palette[][4])
  {
    u32 width;
    u32 height;

    if (isValid(m_fbh))
    {
      const FrameBufferD3D11& fb = m_frameBuffers[m_fbh.idx];
      width = fb.m_width;
      height = fb.m_height;
    }
    else
    {
      width = getBufferWidth();
      height = getBufferHeight();
    }

    if (0 == _rect.m_x
      && 0 == _rect.m_y
      &&  width == _rect.m_width
      &&  height == _rect.m_height)
    {
      clear(_clear, _palette);
    }
    else
    {
      ID3D11DeviceContext* deviceCtx = m_deviceCtx;

      uint64 state = 0;
      state |= _clear.m_flags & BGFX_CLEAR_COLOR ? BGFX_STATE_RGB_WRITE | BGFX_STATE_ALPHA_WRITE : 0;
      state |= _clear.m_flags & BGFX_CLEAR_DEPTH ? BGFX_STATE_DEPTH_TEST_ALWAYS | BGFX_STATE_DEPTH_WRITE : 0;

      uint64 stencil = 0;
      stencil |= _clear.m_flags & BGFX_CLEAR_STENCIL ? 0
        | BGFX_STENCIL_TEST_ALWAYS
        | BGFX_STENCIL_FUNC_REF(_clear.m_stencil)
        | BGFX_STENCIL_FUNC_RMASK(0xff)
        | BGFX_STENCIL_OP_FAIL_S_REPLACE
        | BGFX_STENCIL_OP_FAIL_Z_REPLACE
        | BGFX_STENCIL_OP_PASS_Z_REPLACE
        : 0
        ;

      setBlendState(state);
      setDepthStencilState(state, stencil);
      setRasterizerState(state);

      uint32 numMrt = 1;
      FrameBufferHandle fbh = m_fbh;
      if (isValid(fbh))
      {
        const FrameBufferD3D11& fb = m_frameBuffers[fbh.idx];
        numMrt = bx::uint32_max(1, fb.m_num);
      }

      ProgramD3D11& program = m_program[_clearQuad.m_program[numMrt - 1].idx];
      m_currentProgram = &program;
      deviceCtx->VSSetShader(program.m_vsh->m_vertexShader, NULL, 0);
      deviceCtx->VSSetConstantBuffers(0, 1, s_zero.m_buffer);
      if (NULL != m_currentColor)
      {
        const ShaderD3D11* fsh = program.m_fsh;
        deviceCtx->PSSetShader(fsh->m_pixelShader, NULL, 0);
        deviceCtx->PSSetConstantBuffers(0, 1, &fsh->m_buffer);

        if (BGFX_CLEAR_COLOR_USE_PALETTE & _clear.m_flags)
        {
          float mrtClear[BGFX_CONFIG_MAX_FRAME_BUFFER_ATTACHMENTS][4];
          for (uint32 ii = 0; ii < numMrt; ++ii)
          {
            uint8 index = (uint8_t)bx::uint32_min(BGFX_CONFIG_MAX_COLOR_PALETTE - 1, _clear.m_index[ii]);
            memcpy(mrtClear[ii], _palette[index], 16);
          }

          deviceCtx->UpdateSubresource(fsh->m_buffer, 0, 0, mrtClear, 0, 0);
        }
        else
        {
          float rgba[4] =
          {
            _clear.m_index[0] * 1.0f / 255.0f,
            _clear.m_index[1] * 1.0f / 255.0f,
            _clear.m_index[2] * 1.0f / 255.0f,
            _clear.m_index[3] * 1.0f / 255.0f,
          };

          deviceCtx->UpdateSubresource(fsh->m_buffer, 0, 0, rgba, 0, 0);
        }
      }
      else
      {
        deviceCtx->PSSetShader(NULL, NULL, 0);
      }

      VertexBufferD3D11& vb = m_vertexBuffers[_clearQuad.m_vb->handle.idx];
      const VertexDecl& vertexDecl = m_vertexDecls[_clearQuad.m_vb->decl.idx];
      const uint32 stride = vertexDecl.m_stride;
      const uint32 offset = 0;

      {
        struct Vertex
        {
          float m_x;
          float m_y;
          float m_z;
        };

        Vertex* vertex = (Vertex*)_clearQuad.m_vb->data;
        BX_CHECK(stride == sizeof(Vertex), "Stride/Vertex mismatch (stride %d, sizeof(Vertex) %d)", stride, sizeof(Vertex));

        const float depth = _clear.m_depth;

        vertex->m_x = -1.0f;
        vertex->m_y = -1.0f;
        vertex->m_z = depth;
        vertex++;
        vertex->m_x = 1.0f;
        vertex->m_y = -1.0f;
        vertex->m_z = depth;
        vertex++;
        vertex->m_x = -1.0f;
        vertex->m_y = 1.0f;
        vertex->m_z = depth;
        vertex++;
        vertex->m_x = 1.0f;
        vertex->m_y = 1.0f;
        vertex->m_z = depth;
      }

      m_vertexBuffers[_clearQuad.m_vb->handle.idx].update(0, 4 * _clearQuad.m_decl.m_stride, _clearQuad.m_vb->data);
      deviceCtx->IASetVertexBuffers(0, 1, &vb.m_ptr, &stride, &offset);
      setInputLayout(vertexDecl, program, 0);

      deviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
      deviceCtx->Draw(4, 0);
    }
  }

  void* m_d3d11dll;
  void* m_dxgidll;
  void* m_dxgidebugdll;

  D3D_DRIVER_TYPE   m_driverType;
  D3D_FEATURE_LEVEL m_featureLevel;
  IDXGIAdapter*     m_adapter;
  DXGI_ADAPTER_DESC m_adapterDesc;
  IDXGIFactory*     m_factory;
  IDXGISwapChain*   m_swapChain;


  u16 m_lost;
  u16 m_numWindows;
  FrameBufferHandle m_windows[BGFX_CONFIG_MAX_FRAME_BUFFERS];

  ID3D11Device*        m_device;
  ID3D11DeviceContext* m_deviceCtx;
  ID3D11InfoQueue*     m_infoQueue;
  TimerQueryD3D11      m_gpuTimer;
  OcclusionQueryD3D11  m_occlusionQuery;

  u32 m_deviceInterfaceVersion;

  ID3D11RenderTargetView* m_backBufferColor;
  ID3D11DepthStencilView* m_backBufferDepthStencil;
  ID3D11RenderTargetView* m_currentColor;
  ID3D11DepthStencilView* m_currentDepthStencil;

  ID3D11Texture2D* m_captureTexture;
  ID3D11Texture2D* m_captureResolve;

  Resolution m_resolution;

  typedef DXGI_SWAP_CHAIN_DESC  SwapChainDesc;


  SwapChainDesc m_scd;
  u32 m_maxAnisotropy;
  bool m_depthClamp;
  bool m_wireframe;

  IndexBufferD3D11 m_indexBuffers[BGFX_CONFIG_MAX_INDEX_BUFFERS];
  VertexBufferD3D11 m_vertexBuffers[BGFX_CONFIG_MAX_VERTEX_BUFFERS];
  ShaderD3D11 m_shaders[BGFX_CONFIG_MAX_SHADERS];
  ProgramD3D11 m_program[BGFX_CONFIG_MAX_PROGRAMS];
  TextureD3D11 m_textures[BGFX_CONFIG_MAX_TEXTURES];
  VertexDecl m_vertexDecls[BGFX_CONFIG_MAX_VERTEX_DECLS];
  FrameBufferD3D11 m_frameBuffers[BGFX_CONFIG_MAX_FRAME_BUFFERS];
  void* m_uniforms[BGFX_CONFIG_MAX_UNIFORMS];
  RBMatrix m_predefinedUniforms[PredefinedUniform::Count];
  UniformRegistry m_uniformReg;

  /*
  StateCacheT<ID3D11BlendState> m_blendStateCache;
  StateCacheT<ID3D11DepthStencilState> m_depthStencilStateCache;
  StateCacheT<ID3D11InputLayout> m_inputLayoutCache;
  StateCacheT<ID3D11RasterizerState> m_rasterizerStateCache;
  StateCacheT<ID3D11SamplerState> m_samplerStateCache;
  StateCacheLru<IUnknown*, 1024> m_srvUavLru;
  */
  //TextVideoMem m_textVideoMem;

  TextureStage m_textureStage;

  ProgramD3D11* m_currentProgram;

  u8 m_vsScratch[64 << 10];
  u8 m_fsScratch[64 << 10];
  u32 m_vsChanges;
  u32 m_fsChanges;

  FrameBufferHandle m_fbh;
  bool m_rtMsaa;
  bool m_timerQuerySupport;

};

static RendererContextD3D11* s_renderD3D11;

RendererContextI* rendererCreate()
{
  s_renderD3D11 = new RendererContextD3D11();
  if (!s_renderD3D11->init())
  {
    delete s_renderD3D11;
    s_renderD3D11 = NULL;
  }
  return s_renderD3D11;
}

void rendererDestroy()
{
  s_renderD3D11->shutdown();
  delete s_renderD3D11;
  s_renderD3D11 = nullptr;
}



struct UavFormat
{
  DXGI_FORMAT format[3];
  u32    stride;
};

static const UavFormat s_uavFormat[] =
{	//  BGFX_BUFFER_COMPUTE_TYPE_UINT, BGFX_BUFFER_COMPUTE_TYPE_INT,   BGFX_BUFFER_COMPUTE_TYPE_FLOAT
  { { DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN }, 0 }, // ignored
  { { DXGI_FORMAT_R8_SINT, DXGI_FORMAT_R8_UINT, DXGI_FORMAT_UNKNOWN }, 1 }, // BGFX_BUFFER_COMPUTE_FORMAT_8x1
  { { DXGI_FORMAT_R8G8_SINT, DXGI_FORMAT_R8G8_UINT, DXGI_FORMAT_UNKNOWN }, 2 }, // BGFX_BUFFER_COMPUTE_FORMAT_8x2
  { { DXGI_FORMAT_R8G8B8A8_SINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_UNKNOWN }, 4 }, // BGFX_BUFFER_COMPUTE_FORMAT_8x4
  { { DXGI_FORMAT_R16_SINT, DXGI_FORMAT_R16_UINT, DXGI_FORMAT_R16_FLOAT }, 2 }, // BGFX_BUFFER_COMPUTE_FORMAT_16x1
  { { DXGI_FORMAT_R16G16_SINT, DXGI_FORMAT_R16G16_UINT, DXGI_FORMAT_R16G16_FLOAT }, 4 }, // BGFX_BUFFER_COMPUTE_FORMAT_16x2
  { { DXGI_FORMAT_R16G16B16A16_SINT, DXGI_FORMAT_R16G16B16A16_UINT, DXGI_FORMAT_R16G16B16A16_FLOAT }, 8 }, // BGFX_BUFFER_COMPUTE_FORMAT_16x4
  { { DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_FLOAT }, 4 }, // BGFX_BUFFER_COMPUTE_FORMAT_32x1
  { { DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32_FLOAT }, 8 }, // BGFX_BUFFER_COMPUTE_FORMAT_32x2
  { { DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_FLOAT }, 16 }, // BGFX_BUFFER_COMPUTE_FORMAT_32x4
};

void BufferD3D11::create(u32 insize, void * indata, u16 inflags, u16 instride /*= 0*/, bool invertex /*= false*/)
{
  uav = NULL;
  size = insize;
  flag = inflags;

  const bool needUav = 0 != (inflags & (BGFX_BUFFER_COMPUTE_WRITE | BGFX_BUFFER_DRAW_INDIRECT));
  const bool needSrv = 0 != (inflags & BGFX_BUFFER_COMPUTE_READ);
  const bool drawIndirect = 0 != (inflags & BGFX_BUFFER_DRAW_INDIRECT);
  bdynamic = NULL == indata && !needUav;

  D3D11_BUFFER_DESC desc;
  desc.ByteWidth = insize;
  desc.BindFlags = 0
    | (invertex ? D3D11_BIND_VERTEX_BUFFER : D3D11_BIND_INDEX_BUFFER)
    | (needUav ? D3D11_BIND_UNORDERED_ACCESS : 0)
    | (needSrv ? D3D11_BIND_SHADER_RESOURCE : 0)
    ;
  desc.MiscFlags = 0
    | (drawIndirect ? D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS : 0)
    ;
  desc.StructureByteStride = 0;

  DXGI_FORMAT format;
  u32    stride;

  if (drawIndirect)
  {
    format = DXGI_FORMAT_R32G32B32A32_UINT;
    stride = 16;
  }
  else
  {
    u32 uavFormat = (inflags & BGFX_BUFFER_COMPUTE_FORMAT_MASK) >> BGFX_BUFFER_COMPUTE_FORMAT_SHIFT;
    if (0 == uavFormat)
    {
      if (invertex)
      {
        format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        stride = 16;
      }
      else
      {
        if (0 == (inflags & BGFX_BUFFER_INDEX32))
        {
          format = DXGI_FORMAT_R16_UINT;
          stride = 2;
        }
        else
        {
          format = DXGI_FORMAT_R32_UINT;
          stride = 4;
        }
      }
    }
    else
    {
      const u32 uavType = uint32_satsub((inflags & BGFX_BUFFER_COMPUTE_TYPE_MASK) >> BGFX_BUFFER_COMPUTE_TYPE_SHIFT, 1);
      format = s_uavFormat[uavFormat].format[uavType];
      stride = s_uavFormat[uavFormat].stride;
    }
  }

  ID3D11Device* device = s_renderD3D11->m_device;

  if (needUav)
  {
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.CPUAccessFlags = 0;
    desc.StructureByteStride = _stride;

    DX_CHECK(device->CreateBuffer(&desc
      , NULL
      , &m_ptr
      ));

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavd;
    uavd.Format = format;
    uavd.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavd.Buffer.FirstElement = 0;
    uavd.Buffer.NumElements = m_size / stride;
    uavd.Buffer.Flags = 0;
    DX_CHECK(device->CreateUnorderedAccessView(m_ptr
      , &uavd
      , &m_uav
      ));
  }
  else if (m_dynamic)
  {
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    DX_CHECK(device->CreateBuffer(&desc
      , NULL
      , &m_ptr
      ));
  }
  else
  {
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA srd;
    srd.pSysMem = _data;
    srd.SysMemPitch = 0;
    srd.SysMemSlicePitch = 0;

    DX_CHECK(device->CreateBuffer(&desc
      , &srd
      , &m_ptr
      ));
  }

  if (needSrv)
  {
    D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
    srvd.Format = format;
    srvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvd.Buffer.FirstElement = 0;
    srvd.Buffer.NumElements = m_size / stride;
    DX_CHECK(device->CreateShaderResourceView(m_ptr
      , &srvd
      , &m_srv
      ));
  }
}

void BufferD3D11::update(u32 inoffset, u32 insize, void* indata, bool indiscard /*= false*/)
{

}

void VertexBufferD3D11::create(u32 insize, void* data, D3D11_INPUT_ELEMENT_DESC indecl)
{

}

void ShaderD3D11::create(const void* indata)
{
  bx::MemoryReader reader(_mem->data, _mem->size);

  uint32_t magic;
  bx::read(&reader, magic);

  switch (magic)
  {
  case BGFX_CHUNK_MAGIC_CSH:
  case BGFX_CHUNK_MAGIC_FSH:
  case BGFX_CHUNK_MAGIC_VSH:
    break;

  default:
    BGFX_FATAL(false, Fatal::InvalidShader, "Unknown shader format %x.", magic);
    break;
  }

  bool fragment = BGFX_CHUNK_MAGIC_FSH == magic;

  uint32_t iohash;
  bx::read(&reader, iohash);

  uint16_t count;
  bx::read(&reader, count);

  m_numPredefined = 0;
  m_numUniforms = count;

  BX_TRACE("%s Shader consts %d"
    , BGFX_CHUNK_MAGIC_FSH == magic ? "Fragment" : BGFX_CHUNK_MAGIC_VSH == magic ? "Vertex" : "Compute"
    , count
    );

  uint8_t fragmentBit = fragment ? BGFX_UNIFORM_FRAGMENTBIT : 0;

  if (0 < count)
  {
    for (uint32_t ii = 0; ii < count; ++ii)
    {
      uint8_t nameSize = 0;
      bx::read(&reader, nameSize);

      char name[256] = { '\0' };
      bx::read(&reader, &name, nameSize);
      name[nameSize] = '\0';

      uint8_t type = 0;
      bx::read(&reader, type);

      uint8_t num = 0;
      bx::read(&reader, num);

      uint16_t regIndex = 0;
      bx::read(&reader, regIndex);

      uint16_t regCount = 0;
      bx::read(&reader, regCount);

      const char* kind = "invalid";

      PredefinedUniform::Enum predefined = nameToPredefinedUniformEnum(name);
      if (PredefinedUniform::Count != predefined)
      {
        kind = "predefined";
        m_predefined[m_numPredefined].m_loc = regIndex;
        m_predefined[m_numPredefined].m_count = regCount;
        m_predefined[m_numPredefined].m_type = uint8_t(predefined | fragmentBit);
        m_numPredefined++;
      }
      else if (0 == (BGFX_UNIFORM_SAMPLERBIT & type))
      {
        const UniformInfo* info = s_renderD3D11->m_uniformReg.find(name);
        BX_WARN(NULL != info, "User defined uniform '%s' is not found, it won't be set.", name);

        if (NULL != info)
        {
          if (NULL == m_constantBuffer)
          {
            m_constantBuffer = UniformBuffer::create(1024);
          }

          kind = "user";
          m_constantBuffer->writeUniformHandle((UniformType::Enum)(type | fragmentBit), regIndex, info->m_handle, regCount);
        }
      }
      else
      {
        kind = "sampler";
      }

      BX_TRACE("\t%s: %s (%s), num %2d, r.index %3d, r.count %2d"
        , kind
        , name
        , getUniformTypeName(UniformType::Enum(type&~BGFX_UNIFORM_MASK))
        , num
        , regIndex
        , regCount
        );
      BX_UNUSED(kind);
    }

    if (NULL != m_constantBuffer)
    {
      m_constantBuffer->finish();
    }
  }

  uint16_t shaderSize;
  bx::read(&reader, shaderSize);

  const void* code = reader.getDataPtr();
  bx::skip(&reader, shaderSize + 1);

  if (BGFX_CHUNK_MAGIC_FSH == magic)
  {
    m_hasDepthOp = hasDepthOp(code, shaderSize);
    DX_CHECK(s_renderD3D11->m_device->CreatePixelShader(code, shaderSize, NULL, &m_pixelShader));
    BGFX_FATAL(NULL != m_ptr, bgfx::Fatal::InvalidShader, "Failed to create fragment shader.");
  }
  else if (BGFX_CHUNK_MAGIC_VSH == magic)
  {
    m_hash = bx::hashMurmur2A(code, shaderSize);
    m_code = copy(code, shaderSize);

    DX_CHECK(s_renderD3D11->m_device->CreateVertexShader(code, shaderSize, NULL, &m_vertexShader));
    BGFX_FATAL(NULL != m_ptr, bgfx::Fatal::InvalidShader, "Failed to create vertex shader.");
  }
  else
  {
    DX_CHECK(s_renderD3D11->m_device->CreateComputeShader(code, shaderSize, NULL, &m_computeShader));
    BGFX_FATAL(NULL != m_ptr, bgfx::Fatal::InvalidShader, "Failed to create compute shader.");
  }

  uint8_t numAttrs = 0;
  bx::read(&reader, numAttrs);

  memset(m_attrMask, 0, sizeof(m_attrMask));

  for (uint32_t ii = 0; ii < numAttrs; ++ii)
  {
    uint16_t id;
    bx::read(&reader, id);

    Attrib::Enum attr = idToAttrib(id);

    if (Attrib::Count != attr)
    {
      m_attrMask[attr] = UINT16_MAX;
    }
  }

  uint16_t size;
  bx::read(&reader, size);

  if (0 < size)
  {
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = (size + 0xf) & ~0xf;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.CPUAccessFlags = 0;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;
    VERIFYD3D11RESULT(s_renderD3D11->m_device->CreateBuffer(&desc, NULL, &m_buffer));
  }
}

ShaderD3D11::ShaderD3D11()
{

}

void TextureD3D11::create(const void* _mem, u32 _flags, u8 _skip)
{

}

void TextureD3D11::destroy()
{

}

void TextureD3D11::overrideInternal(uintptr_t _ptr)
{

}

void TextureD3D11::update(u8 _side, u8 _mip, const RBAABBI& _rect, u16 _z, u16 _depth, u16 _pitch, const void* _mem)
{

}

void TextureD3D11::commit(u8 _stage, u32 _flags, const float _palette[][4])
{

}

void TextureD3D11::resolve() const
{

}

TextureHandle TextureD3D11::getHandle() const
{

}

void FrameBufferD3D11::create(u8 _num, const Attachment* _attachment)
{

}

void FrameBufferD3D11::create(u16 _denseIdx, void* _nwh, u32 _width, u32 _height, TextureFormat::Enum _depthFormat)
{

}

u16 FrameBufferD3D11::destroy()
{

}

void FrameBufferD3D11::preReset(bool _force /*= false*/)
{

}

void FrameBufferD3D11::postReset()
{

}

void FrameBufferD3D11::resolve()
{

}

void FrameBufferD3D11::clear(const Clear& _clear, const float _palette[][4])
{

}

void TimerQueryD3D11::postReset()
{

}

void TimerQueryD3D11::preReset()
{

}

void TimerQueryD3D11::begin()
{

}

void TimerQueryD3D11::end()
{

}

bool TimerQueryD3D11::get()
{

}

void OcclusionQueryD3D11::postReset()
{

}

void OcclusionQueryD3D11::preReset()
{

}

void OcclusionQueryD3D11::begin(class Frame* _render, OcclusionQueryHandle _handle)
{

}

void OcclusionQueryD3D11::end()
{

}

void OcclusionQueryD3D11::resolve(class Frame* _render, bool _wait /*= false*/)
{

}


#pragma endregion