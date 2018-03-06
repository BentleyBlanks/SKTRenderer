#pragma once
#include "FastConfig.h"
#include <vector>
#include "RBMath/Inc/Vector2.h"
#include "RBMath/Inc/Vector3.h"
#include "RBMath/Inc/Vector4.h"
#include "RBMath/Inc/Matrix.h"
#include "Image.h"
#include "FastRenderResources.h"
#include "App.h"

#define PROFILE

class Renderer {
public:
  Renderer();
  virtual ~Renderer();

  virtual void resetToDefaults();
  virtual void reset(const uint flags = RESET_ALL);
  void apply(bool apply_shader_res = true);

  TextureID addTexture(const char *fileName, const bool useMipMaps, const SamplerStateID samplerState = SS_NONE, uint flags = 0);
  virtual TextureID addTexture(Image &img, const SamplerStateID samplerState = SS_NONE, uint flags = 0) = 0;
  TextureID addCubemap(const char **fileNames, const bool useMipMaps, const SamplerStateID samplerState = SS_NONE, const int nArraySlices = 1, uint flags = 0);
  TextureID addNormalMap(const char *fileName, const FORMAT destFormat, const bool useMipMaps, const SamplerStateID samplerState = SS_NONE, float sZ = 1.0f, float mipMapScaleZ = 2.0f, uint flags = 0);

  TextureID addRenderTarget(const int width, const int height, const FORMAT format, const SamplerStateID samplerState = SS_NONE, uint flags = 0){
    return addRenderTarget(width, height, 1, 1, 1, format, 1, samplerState, flags);
  }
  virtual TextureID addRenderTarget(const int width, const int height, const int depth, const int mipMapCount, const int arraySize, const FORMAT format, const int msaaSamples = 1, const SamplerStateID samplerState = SS_NONE, uint flags = 0) = 0;
  TextureID addRenderDepth(const int width, const int height, const int depthBits){
    return addRenderDepth(width, height, 1, depthBits <= 16 ? FORMAT_D16 : FORMAT_D24);
  }
  virtual TextureID addRenderDepth(const int width, const int height, const int arraySize, const FORMAT format, const int msaaSamples = 1, const SamplerStateID samplerState = SS_NONE, uint flags = 0) = 0;

  virtual bool resizeRenderTarget(const TextureID renderTarget, const int width, const int height, const int depth, const int mipMapCount, const int arraySize) = 0;
  virtual bool generateMipMaps(const TextureID renderTarget) = 0;

  virtual void removeTexture(const TextureID texture) = 0;


  ShaderID addShader(const char *fileName, const uint flags = 0);
  ShaderID addShader(const char *fileName, const char *extra, const uint flags = 0);
  ShaderID addShader(const char *fileName, const char **attributeNames, const int nAttributes, const char *extra = NULL, const uint flags = 0);
  virtual ShaderID addShader(const char *vsText, const char *gsText, const char *fsText, const int vsLine, const int gsLine, const int fsLine,
    const char *header = NULL, const char *extra = NULL, const char *fileName = NULL, const char **attributeNames = NULL, const int nAttributes = 0, const uint flags = 0) = 0;

  int getFormatSize(const AttributeFormat format) const;
  virtual VertexFormatID addVertexFormat(const FormatDesc *formatDesc, const uint nAttribs, const ShaderID shader = SHADER_NONE) = 0;
  virtual VertexBufferID addVertexBuffer(const long size, const BufferAccess bufferAccess, const void *data = NULL) = 0;
  virtual IndexBufferID addIndexBuffer(const uint nIndices, const uint indexSize, const BufferAccess bufferAccess, const void *data = NULL) = 0;
  

  virtual SamplerStateID addSamplerState(const Filter filter, const AddressMode s, const AddressMode t, const AddressMode r, const float lod = 0, const uint maxAniso = 16, const int compareFunc = 0, const float *border_color = NULL) = 0;
  BlendStateID addBlendState(const int srcFactor, const int destFactor, const int blendMode = BM_ADD, const int mask = ALL, const bool alphaToCoverage = false){
    return addBlendState(srcFactor, destFactor, srcFactor, destFactor, blendMode, blendMode, mask, alphaToCoverage);
  }
  virtual BlendStateID addBlendState(const int srcFactorRGB, const int destFactorRGB, const int srcFactorAlpha, const int destFactorAlpha, const int blendModeRGB, const int blendModeAlpha, const int mask = ALL, const bool alphaToCoverage = false) = 0;
  virtual DepthStateID addDepthState(const bool depthTest, const bool depthWrite, const int depthFunc, const bool stencilTest, const uint8 stencilReadMask, const uint8 stencilWriteMask,
    const int stencilFuncFront, const int stencilFuncBack, const int stencilFailFront, const int stencilFailBack,
    const int depthFailFront, const int depthFailBack, const int stencilPassFront, const int stencilPassBack) = 0;
  DepthStateID addDepthState(const bool depthTest, const bool depthWrite, const int depthFunc = LEQUAL, const bool stencilTest = false,
    const uint8 stencilMask = 0xFF, const int stencilFunc = ALWAYS, const int stencilFail = KEEP, const int depthFail = KEEP, const int stencilPass = KEEP)
  {
    return addDepthState(depthTest, depthWrite, depthFunc, stencilTest, stencilMask, stencilMask, stencilFunc, stencilFunc, stencilFail, stencilFail, depthFail, depthFail, stencilPass, stencilPass);
  }
  virtual RasterizerStateID addRasterizerState(const int cullMode, const int fillMode = SOLID, const bool multiSample = true, const bool scissor = false, const float depthBias = 0.0f, const float slopeDepthBias = 0.0f) = 0;

  FontID addFont(const char *textureFile, const char *fontFile, const SamplerStateID samplerState);


  /*	void setTexture(const uint unit, const TextureID texture){
  selectedTextures[unit] = texture;
  }
  void setTexture(const char *textureName, const TextureID texture){
  int unit = getTextureUnit(selectedShader, textureName);
  if (unit >= 0) selectedTextures[unit] = texture;
  }
  void setTexture(const char *textureName, const TextureID texture, const SamplerStateID samplerState){
  int unit = getTextureUnit(selectedShader, textureName);
  if (unit >= 0){
  selectedTextures[unit] = texture;
  selectedSamplerStates[unit] = samplerState;
  }
  }
  */

  //	virtual int getTextureUnit(const ShaderID shader, const char *textureName) const = 0;
  //	virtual int getSamplerUnit(const ShaderID shader, const char *samplerName) const = 0;

  virtual void setTexture(const char *textureName, const TextureID texture) = 0;
  virtual void setTexture(const char *textureName, const TextureID texture, const SamplerStateID samplerState) = 0;
  virtual void setTextureSlice(const char *textureName, const TextureID texture, const int slice) = 0;

  /*
  virtual void changeTexture(const uint imageUnit, const TextureID texture) = 0;
  void changeTexture(const char *textureName, const TextureID texture){
  int unit = getTextureUnit(currentShader, textureName);
  if (unit >= 0) changeTexture(unit, texture);
  }
  */
  virtual void applyTextures() = 0;

  virtual void setSamplerState(const char *samplerName, const SamplerStateID samplerState) = 0;
  virtual void applySamplerStates() = 0;
  void applyVertexStates();



  void setShader(const ShaderID shader){
    selectedShader = shader;
  }
  void setVertexFormat(const VertexFormatID vertexFormat){
    selectedVertexFormat = vertexFormat;
  }
  void setVertexBuffer(const int stream, const VertexBufferID vertexBuffer, const ptrdiff_t offset = 0){
    selectedVertexBuffers[stream] = vertexBuffer;
    selectedOffsets[stream] = offset;
  }
  void setVertexBuffer(const int stream, const void *base){
    selectedVertexBuffers[stream] = VB_NONE;
    selectedOffsets[stream] = (ptrdiff_t)base;
  }
  void setIndexBuffer(const IndexBufferID indexBuffer){
    selectedIndexBuffer = indexBuffer;
  }

  /*	void setSamplerState(const uint samplerUnit, const SamplerStateID samplerState){
  selectedSamplerStates[samplerUnit] = samplerState;
  }*/
  void setBlendState(const BlendStateID blendState, const uint sampleMask = ~0){
    selectedBlendState = blendState;
    selectedSampleMask = sampleMask;
  }
  void setDepthState(const DepthStateID depthState, const uint stencilRef = 0){
    selectedDepthState = depthState;
    selectedStencilRef = stencilRef;
  }
  void setRasterizerState(const RasterizerStateID rasterizerState){
    selectedRasterizerState = rasterizerState;
  }


  void setShaderConstant1i(const char *name, const int constant);
  void setShaderConstant1f(const char *name, const float constant);
  void setShaderConstant2f(const char *name, const RBVector2&constant);
  void setShaderConstant3f(const char *name, const RBVector3 &constant);
  void setShaderConstant4f(const char *name, const RBVector4 &constant);
  void setShaderConstant4x4f(const char *name, const RBMatrix &constant);
  void setShaderConstantArray1f(const char *name, const float *constant, const uint count);
  void setShaderConstantArray2f(const char *name, const RBVector2 *constant, const uint count);
  void setShaderConstantArray3f(const char *name, const RBVector3  *constant, const uint count);
  void setShaderConstantArray4f(const char *name, const RBVector4  *constant, const uint count);
  void setShaderConstantArray4x4f(const char *name, const RBMatrix *constant, const uint count);

  virtual void setShaderConstantRaw(const char *name, const void *data, const int size) = 0;
  virtual void applyConstants() = 0;


  void changeRenderTarget(const TextureID colorRT, const TextureID depthRT = TEXTURE_NONE, const int slice = NO_SLICE){
    changeRenderTargets(&colorRT, 1, depthRT, &slice);
  }
  void changeRenderTargets(const TextureID *colorRTs, const uint nRenderTargets, const TextureID depthRT, const int *faces){
    changeRenderTargets(colorRTs, nRenderTargets, depthRT, NO_SLICE, faces);
  }
  virtual void changeRenderTargets(const TextureID *colorRTs, const uint nRenderTargets, const TextureID depthRT = TEXTURE_NONE, const int depthSlice = NO_SLICE, const int *slices = NULL) = 0;
  virtual void changeToMainFramebuffer() = 0;
  virtual void changeShader(const ShaderID shader) = 0;
  virtual void changeVertexFormat(const VertexFormatID vertexFormat) = 0;
  virtual void changeVertexBuffer(const int stream, const VertexBufferID vertexBuffer, const ptrdiff_t offset = 0) = 0;
  void changeVertexBuffer(const void *base){
    changeVertexBuffer(0, VB_NONE, (ptrdiff_t)base);
  }
  virtual void changeIndexBuffer(const IndexBufferID indexBuffer) = 0;

  //	virtual void changeSamplerState(const uint samplerUnit, const SamplerStateID samplerState) = 0;
  virtual void changeBlendState(const BlendStateID blendState, const uint sampleMask = ~0) = 0;
  virtual void changeDepthState(const DepthStateID depthState, const uint stencilRef = 0) = 0;
  virtual void changeRasterizerState(const RasterizerStateID rasterizerState) = 0;
  /*
  virtual void changeShaderConstant1i(const char *name, const int constant) = 0;
  virtual void changeShaderConstant1f(const char *name, const float constant) = 0;
  virtual void changeShaderConstant2f(const char *name, constRBVector2&constant) = 0;
  virtual void changeShaderConstant3f(const char *name, const vec3 &constant) = 0;
  virtual void changeShaderConstant4f(const char *name, const vec4 &constant) = 0;
  virtual void changeShaderConstant3x3f(const char *name, const mat3 &constant) = 0;
  virtual void changeShaderConstant4x4f(const char *name, const mat4 &constant) = 0;
  virtual void changeShaderConstantArray1f(const char *name, const float *constant, const uint count) = 0;
  virtual void changeShaderConstantArray2f(const char *name, constRBVector2*constant, const uint count) = 0;
  virtual void changeShaderConstantArray3f(const char *name, const vec3 *constant, const uint count) = 0;
  virtual void changeShaderConstantArray4f(const char *name, const vec4 *constant, const uint count) = 0;
  */

  // Backward compatibility
  void clear(const bool clearColor, const bool clearDepth, const float *color = NULL, const float depth = 1.0f){
    clear(clearColor, clearDepth, false, color, 1.0f, 0);
  }
  virtual void clear(const bool clearColor, const bool clearDepth, const bool clearStencil, const float *color = NULL, const float depth = 1.0f, const uint stencil = 0) = 0;

  virtual void drawArrays(const Primitives primitives, const int firstVertex, const int nVertices) = 0;
  virtual void drawElements(const Primitives primitives, const int firstIndex, const int nIndices, const int firstVertex, const int nVertices) = 0;

  virtual void setup2DMode(const float left, const float right, const float top, const float bottom) = 0;
  virtual void drawPlain(const Primitives primitives,RBVector2*vertices, const uint nVertices, const BlendStateID blendState, const DepthStateID depthState, const RBVector4 *color = NULL) = 0;
  virtual void drawTextured(const Primitives primitives, TexVertex *vertices, const uint nVertices, const TextureID texture, const SamplerStateID samplerState, const BlendStateID blendState, const DepthStateID depthState, const RBVector4 *color = NULL) = 0;

  float getTextWidth(const FontID font, const char *str, int length = -1) const;
  uint getTextQuads(const char *str) const;
  void fillTextBuffer(TexVertex *dest, const char *str, float x, float y, const float charWidth, const float charHeight, const FontID font) const;
  bool drawText(const char *str, float x, float y, const float charWidth, const float charHeight, const FontID font, const SamplerStateID samplerState, const BlendStateID blendState, const DepthStateID depthState);

  void setViewport(const int width, const int height);

  // Statistics
  void resetStatistics();
  void addDrawCalls(const uint nCalls){ nDrawCalls += nCalls; }
  uint getDrawCallCount(){ return nDrawCalls; }

#ifdef PROFILE
  // Profiling
  void profileFrameStart(const float frameTime);
  void profileFrameEnd();

  void profileBegin(const char *name);
  void profileNext(const char *name);
  void profileEnd();

  const char *getProfileString() const { return profileString; }
#endif

  // Synchronization
  virtual void flush() = 0;
  virtual void finish() = 0;

protected:
  public:
  std::vector<Texture> textures;
  std::vector<Shader> shaders;
  std::vector<VertexBuffer> vertexBuffers;
  std::vector<IndexBuffer> indexBuffers;
  std::vector<TexFont> fonts;
  std::vector<VertexFormat> vertexFormats;
  std::vector<SamplerState> samplerStates;
  std::vector<BlendState> blendStates;
  std::vector<DepthState> depthStates;
  std::vector<RasterizerState> rasterizerStates;
  std::vector<StructureBuffer> structureBuffers;

  uint nImageUnits, nMRTs;
  int maxAnisotropic;

  //	float *textureLod;

  ShaderID currentShader, selectedShader;

  VertexFormatID currentVertexFormat, selectedVertexFormat;
  VertexBufferID currentVertexBuffers[MAX_VERTEXSTREAM], selectedVertexBuffers[MAX_VERTEXSTREAM];
  ptrdiff_t selectedOffsets[MAX_VERTEXSTREAM], currentOffsets[MAX_VERTEXSTREAM];
  IndexBufferID currentIndexBuffer, selectedIndexBuffer;

  DepthStateID currentDepthState, selectedDepthState;
  uint currentStencilRef, selectedStencilRef;
  BlendStateID currentBlendState, selectedBlendState;
  uint currentSampleMask, selectedSampleMask;
  RasterizerStateID currentRasterizerState, selectedRasterizerState;
  StructureBufferID currentStructureBuffer, selectedStructureBuffer;

  TextureID currentColorRT[MAX_MRTS], currentDepthRT;
  int currentColorRTSlice[MAX_MRTS], currentDepthSlice;
  uint nCurrentRenderTargets;

  int viewportWidth, viewportHeight;

  // Statistics counters
  uint nDrawCalls;

#ifdef PROFILE
  // Profiling
  float profileStartTime;
  bool profileFrame;
  bool profileReset;

  char profileString[512];
  int profileStringIndex;
  float profileTimes[32];
  int profileTimeIndex;
  int profileFrameCount;
#endif


private:
  public:
  TexVertex *fontBuffer;
  uint fontBufferCount;
};