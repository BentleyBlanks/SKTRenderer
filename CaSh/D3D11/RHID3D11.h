#pragma once
#include "RHIDynamic.h"
#include "d3d11.h"
#include "RefCount.h"
#include <vector>
#include "D3D11Resources.h"
#include <map>
#include "D3D11Util.h"
#include "D3D11StateCache.h"
#include "D3D11ConstantBuffer.h"
#include "Logger.h"
#include "App.h"

extern bool GIsRHIInitialized;

struct FD3D11Adapter
{
  /** -1 if not supported or FindAdpater() wasn't called. Ideally we would store a pointer to IDXGIAdapter but it's unlikely the adpaters change during engine init. */
  int32 AdapterIndex;
  /** The maximum D3D11 feature level supported. 0 if not supported or FindAdpater() wasn't called */
  D3D_FEATURE_LEVEL MaxSupportedFeatureLevel;

  // constructor
  FD3D11Adapter(int32 InAdapterIndex = -1, D3D_FEATURE_LEVEL InMaxSupportedFeatureLevel = (D3D_FEATURE_LEVEL)0)
    : AdapterIndex(InAdapterIndex)
    , MaxSupportedFeatureLevel(InMaxSupportedFeatureLevel)
  {
  }

  bool IsValid() const
  {
    return MaxSupportedFeatureLevel != (D3D_FEATURE_LEVEL)0 && AdapterIndex >= 0;
  }
};
/** The interface which is implemented by the dynamically bound RHI. */
class FD3D11DynamicRHI : public RBDynamicRHI, public IRHICommandContext
{
public:

  friend class RBD3D11Viewport;

  /** Global D3D11 lock list  */
  std::map<FD3D11LockedKey, FD3D11LockedData> OutstandingLocks;


  bool InitRHI();
  // RBDynamicRHI interface.
  virtual void Init() override;
  virtual void Shutdown() override;

  virtual RBDepthStencilStateRHIRef RHICreateDepthStencilState(const RBDepthStencilStateInitializerRHI& Initializer) final override;
  virtual RBSamplerStateRHIRef RHICreateSamplerState(const RBSamplerStateInitializerRHI& Initializer) final override;
  virtual RBRasterizerStateRHIRef RHICreateRasterizerState(const RBRasterizerStateInitializerRHI& Initializer) final override;
  virtual RBBlendStateRHIRef RHICreateBlendState(const RBBlendStateInitializerRHI& Initializer) final override;
  virtual RBUniformBufferRHIRef RHICreateUniformBuffer(const void* Contents, const RBRHIUniformBufferLayout& Layout, EUniformBufferUsage Usage) final override;
  virtual RBIndexBufferRHIRef RHICreateIndexBuffer(uint32 Stride, uint32 Size, uint32 InUsage, RBRHIResourceCreateInfo& CreateInfo) final override;
  virtual void* RHILockIndexBuffer(RBIndexBufferRHIParamRef IndexBuffer, uint32 Offset, uint32 Size, EResourceLockMode LockMode) final override;
  virtual void RHIUnlockIndexBuffer(RBIndexBufferRHIParamRef IndexBuffer) final override;
  virtual RBVertexBufferRHIRef RHICreateVertexBuffer(uint32 Size, uint32 InUsage, RBRHIResourceCreateInfo& CreateInfo) final override;
  virtual void* RHILockVertexBuffer(RBVertexBufferRHIParamRef VertexBuffer, uint32 Offset, uint32 SizeRHI, EResourceLockMode LockMode) final override;
  virtual void RHIUnlockVertexBuffer(RBVertexBufferRHIParamRef VertexBuffer) final override;
  virtual void RHICopyVertexBuffer(RBVertexBufferRHIParamRef SourceBuffer, RBVertexBufferRHIParamRef DestBuffer) final override;
  virtual RBStructuredBufferRHIRef RHICreateStructuredBuffer(uint32 Stride, uint32 Size, uint32 InUsage, RBRHIResourceCreateInfo& CreateInfo) final override;
  virtual void* RHILockStructuredBuffer(RBStructuredBufferRHIParamRef StructuredBuffer, uint32 Offset, uint32 SizeRHI, EResourceLockMode LockMode) final override;
  virtual void RHIUnlockStructuredBuffer(RBStructuredBufferRHIParamRef StructuredBuffer) final override;
  virtual RBPixelShaderRHIRef RHICreatePixelShader(const void* Code, uint32 CodeSize) final override;
  virtual RBVertexShaderRHIRef RHICreateVertexShader(const void* Code, uint32 CodeSize) final override;
  virtual RBHullShaderRHIRef RHICreateHullShader(const void* Code, uint32 CodeSize) final override;
  virtual RBDomainShaderRHIRef RHICreateDomainShader(const void* Code, uint32 CodeSize) final override;
  virtual RBGeometryShaderRHIRef RHICreateGeometryShader(const void* Code, uint32 CodeSize) final override;
  virtual RBBoundShaderStateRHIRef RHICreateBoundShaderState(RBVertexDeclarationRHIParamRef VertexDeclaration, RBVertexShaderRHIParamRef VertexShader, RBHullShaderRHIParamRef HullShader, RBDomainShaderRHIParamRef DomainShader, RBPixelShaderRHIParamRef PixelShader, RBGeometryShaderRHIParamRef GeometryShader);
  virtual RBTexture2DRHIRef RHICreateTexture2D(uint32 SizeX, uint32 SizeY, uint8 Format, uint32 NumMips, uint32 NumSamples, uint32 Flags, RBRHIResourceCreateInfo& CreateInfo) final override;
  virtual RBVertexDeclarationRHIRef RHICreateVertexDeclaration(const RBVertexDeclarationElementList& Elements) final override;

  //Commands
  virtual void RHISetMultipleViewports(uint32 Count, const RBViewportBounds* Data) final override{}
  virtual void RHIClearUAV(RBUnorderedAccessViewRHIParamRef UnorderedAccessViewRHI, const uint32* Values) final override{}
  //virtual void RHICopyToResolveTarget(RBTextureRHIParamRef SourceTexture, RBTextureRHIParamRef DestTexture, bool bKeepOriginalSurface, const FResolveParams& ResolveParams) final override;
  //virtual void RHITransitionResources(EResourceTransitionAccess TransitionType, RBTextureRHIParamRef* InTextures, int32 NumTextures) final override;
  virtual void RHITransitionResources(EResourceTransitionAccess TransitionType, EResourceTransitionPipeline TransitionPipeline, RBUnorderedAccessViewRHIParamRef* InUAVs, int32 NumUAVs, RBComputeFenceRHIParamRef WriteFence) final override{}
  virtual void RHIBeginRenderQuery(RBRenderQueryRHIParamRef RenderQuery) final override{}
  virtual void RHIEndRenderQuery(RBRenderQueryRHIParamRef RenderQuery) final override{}
  virtual void RHIBeginOcclusionQueryBatch() final override {}
  virtual void RHIEndOcclusionQueryBatch() final override{}
  virtual void RHISubmitCommandsHint() final override{}
  virtual void RHIBeginDrawingViewport(RBViewportRHIParamRef Viewport, RBTextureRHIParamRef RenderTargetRHI) final override{}
  virtual void RHIEndDrawingViewport(RBViewportRHIParamRef Viewport, bool bPresent, bool bLockToVsync) final override{}
  virtual void RHIBeginFrame() final override{}
  virtual void RHIEndFrame() final override{}
  virtual void RHIBeginScene() final override{}
  virtual void RHIEndScene() final override{}
  virtual void RHISetStreamSource(uint32 StreamIndex, RBVertexBufferRHIParamRef VertexBuffer, uint32 Stride, uint32 Offset) final override{}
  virtual void RHISetRasterizerState(RBRasterizerStateRHIParamRef NewState) final override;
  virtual void RHISetViewport(uint32 MinX, uint32 MinY, float MinZ, uint32 MaxX, uint32 MaxY, float MaxZ) final override;
  virtual void RHISetScissorRect(bool bEnable, uint32 MinX, uint32 MinY, uint32 MaxX, uint32 MaxY) final override;
  virtual void RHISetBoundShaderState(RBBoundShaderStateRHIParamRef BoundShaderState) final override;
  virtual void RHISetShaderTexture(RBVertexShaderRHIParamRef VertexShader, uint32 TextureIndex, RBTextureRHIParamRef NewTexture) final override;
  virtual void RHISetShaderTexture(RBHullShaderRHIParamRef HullShader, uint32 TextureIndex, RBTextureRHIParamRef NewTexture) final override;
  virtual void RHISetShaderTexture(RBDomainShaderRHIParamRef DomainShader, uint32 TextureIndex, RBTextureRHIParamRef NewTexture) final override;
  virtual void RHISetShaderTexture(RBGeometryShaderRHIParamRef GeometryShader, uint32 TextureIndex, RBTextureRHIParamRef NewTexture) final override;
  virtual void RHISetShaderTexture(RBPixelShaderRHIParamRef PixelShader, uint32 TextureIndex, RBTextureRHIParamRef NewTexture) final override;
  virtual void RHISetShaderTexture(RBComputeShaderRHIParamRef PixelShader, uint32 TextureIndex, RBTextureRHIParamRef NewTexture) final override;
  virtual void RHISetShaderSampler(RBComputeShaderRHIParamRef ComputeShader, uint32 SamplerIndex, RBSamplerStateRHIParamRef NewState) final override;
  virtual void RHISetShaderSampler(RBVertexShaderRHIParamRef VertexShader, uint32 SamplerIndex, RBSamplerStateRHIParamRef NewState) final override;
  virtual void RHISetShaderSampler(RBGeometryShaderRHIParamRef GeometryShader, uint32 SamplerIndex, RBSamplerStateRHIParamRef NewState) final override;
  virtual void RHISetShaderSampler(RBDomainShaderRHIParamRef DomainShader, uint32 SamplerIndex, RBSamplerStateRHIParamRef NewState) final override;
  virtual void RHISetShaderSampler(RBHullShaderRHIParamRef HullShader, uint32 SamplerIndex, RBSamplerStateRHIParamRef NewState) final override;
  virtual void RHISetShaderSampler(RBPixelShaderRHIParamRef PixelShader, uint32 SamplerIndex, RBSamplerStateRHIParamRef NewState) final override;
  virtual void RHISetUAVParameter(RBComputeShaderRHIParamRef ComputeShader, uint32 UAVIndex, RBUnorderedAccessViewRHIParamRef UAV) final override{}
  virtual void RHISetUAVParameter(RBComputeShaderRHIParamRef ComputeShader, uint32 UAVIndex, RBUnorderedAccessViewRHIParamRef UAV, uint32 InitialCount) final override{}
  virtual void RHISetShaderResourceViewParameter(RBPixelShaderRHIParamRef PixelShader, uint32 SamplerIndex, RBShaderResourceViewRHIParamRef SRV) final override;
  virtual void RHISetShaderResourceViewParameter(RBVertexShaderRHIParamRef VertexShader, uint32 SamplerIndex, RBShaderResourceViewRHIParamRef SRV) final override;
  virtual void RHISetShaderResourceViewParameter(RBComputeShaderRHIParamRef ComputeShader, uint32 SamplerIndex, RBShaderResourceViewRHIParamRef SRV) final override;
  virtual void RHISetShaderResourceViewParameter(RBHullShaderRHIParamRef HullShader, uint32 SamplerIndex, RBShaderResourceViewRHIParamRef SRV) final override;
  virtual void RHISetShaderResourceViewParameter(RBDomainShaderRHIParamRef DomainShader, uint32 SamplerIndex, RBShaderResourceViewRHIParamRef SRV) final override;
  virtual void RHISetShaderResourceViewParameter(RBGeometryShaderRHIParamRef GeometryShader, uint32 SamplerIndex, RBShaderResourceViewRHIParamRef SRV) final override;
  virtual void RHISetShaderUniformBuffer(RBVertexShaderRHIParamRef VertexShader, uint32 BufferIndex, RBUniformBufferRHIParamRef Buffer) final override;
  virtual void RHISetShaderUniformBuffer(RBHullShaderRHIParamRef HullShader, uint32 BufferIndex, RBUniformBufferRHIParamRef Buffer) final override;
  virtual void RHISetShaderUniformBuffer(RBDomainShaderRHIParamRef DomainShader, uint32 BufferIndex, RBUniformBufferRHIParamRef Buffer) final override;
  virtual void RHISetShaderUniformBuffer(RBGeometryShaderRHIParamRef GeometryShader, uint32 BufferIndex, RBUniformBufferRHIParamRef Buffer) final override;
  virtual void RHISetShaderUniformBuffer(RBPixelShaderRHIParamRef PixelShader, uint32 BufferIndex, RBUniformBufferRHIParamRef Buffer) final override;
  virtual void RHISetShaderUniformBuffer(RBComputeShaderRHIParamRef ComputeShader, uint32 BufferIndex, RBUniformBufferRHIParamRef Buffer) final override;
  virtual void RHISetShaderParameter(RBVertexShaderRHIParamRef VertexShader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue) final override;
  virtual void RHISetShaderParameter(RBPixelShaderRHIParamRef PixelShader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue) final override;
  virtual void RHISetShaderParameter(RBHullShaderRHIParamRef HullShader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue) final override;
  virtual void RHISetShaderParameter(RBDomainShaderRHIParamRef DomainShader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue) final override;
  virtual void RHISetShaderParameter(RBGeometryShaderRHIParamRef GeometryShader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue) final override;
  virtual void RHISetShaderParameter(RBComputeShaderRHIParamRef ComputeShader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue) final override;
  virtual void RHISetDepthStencilState(RBDepthStencilStateRHIParamRef NewState, uint32 StencilRef) final override;
  virtual void RHISetBlendState(RBBlendStateRHIParamRef NewState, const RBColorf& BlendFactor) final override;
  virtual void RHISetRenderTargets(uint32 NumSimultaneousRenderTargets, const RBRHIRenderTargetView* NewRenderTargets, const RBRHIDepthRenderTargetView* NewDepthStencilTarget, uint32 NumUAVs, const RBUnorderedAccessViewRHIParamRef* UAVs) final override;
  virtual void RHISetRenderTargetsAndClear(const RBRHISetRenderTargetsInfo& RenderTargetsInfo) final override;
  virtual void RHIBindClearMRTValues(bool bClearColor, bool bClearDepth, bool bClearStencil) final override{}
  virtual void RHIDrawPrimitive(uint32 PrimitiveType, uint32 BaseVertexIndex, uint32 NumPrimitives, uint32 NumInstances) final override;
  virtual void RHIDrawPrimitiveIndirect(uint32 PrimitiveType, RBVertexBufferRHIParamRef ArgumentBuffer, uint32 ArgumentOffset) final override;
  virtual void RHIDrawIndexedIndirect(RBIndexBufferRHIParamRef IndexBufferRHI, uint32 PrimitiveType, RBStructuredBufferRHIParamRef ArgumentsBufferRHI, int32 DrawArgumentsIndex, uint32 NumInstances) final override;
  virtual void RHIDrawIndexedPrimitive(RBIndexBufferRHIParamRef IndexBuffer, uint32 PrimitiveType, int32 BaseVertexIndex, uint32 RBirstInstance, uint32 NumVertices, uint32 StartIndex, uint32 NumPrimitives, uint32 NumInstances) final override;
  virtual void RHIDrawIndexedPrimitiveIndirect(uint32 PrimitiveType, RBIndexBufferRHIParamRef IndexBuffer, RBVertexBufferRHIParamRef ArgumentBuffer, uint32 ArgumentOffset) final override;
  virtual void RHIBeginDrawPrimitiveUP(uint32 PrimitiveType, uint32 NumPrimitives, uint32 NumVertices, uint32 VertexDataStride, void*& OutVertexData) final override{}
  virtual void RHIEndDrawPrimitiveUP() final override{}
  virtual void RHIBeginDrawIndexedPrimitiveUP(uint32 PrimitiveType, uint32 NumPrimitives, uint32 NumVertices, uint32 VertexDataStride, void*& OutVertexData, uint32 MinVertexIndex, uint32 NumIndices, uint32 IndexDataStride, void*& OutIndexData) final override{}
  virtual void RHIEndDrawIndexedPrimitiveUP() final override{}
  virtual void RHIClear(bool bClearColor, const RBColorf& Color, bool bClearDepth, float Depth, bool bClearStencil, uint32 Stencil, RBAABBI ExcludeRect) final override;
  virtual void RHIClearMRT(bool bClearColor, int32 NumClearColors, const RBColorf* ColorArray, bool bClearDepth, float Depth, bool bClearStencil, uint32 Stencil, RBAABBI ExcludeRect) final override;
  virtual void RHIEnableDepthBoundsTest(bool bEnable, float MinDepth, float MaxDepth) final override{}

  virtual void RHISetComputeShader(RBComputeShaderRHIParamRef ComputeShader){}
  virtual void RHIDispatchComputeShader(uint32 ThreadGroupCountX, uint32 ThreadGroupCountY, uint32 ThreadGroupCountZ){}
  virtual void RHIDispatchIndirectComputeShader(RBVertexBufferRHIParamRef ArgumentBuffer, uint32 ArgumentOffset){}
  virtual void RHIAutomaticCacheFlushAfterComputeShader(bool bEnable){}
  virtual void RHIFlushComputeShaderCache(){}
  virtual void RHIPushEvent(const char* Name, RBColorf Color) {}
  virtual void RHIPopEvent(){}
  virtual void RHIUpdateTextureReference(RBTextureReferenceRHIParamRef TextureRef, RBTextureRHIParamRef NewTexture){}



  /**
  * Cleanup the D3D device.
  * This function must be called from the main game thread.
  */
  virtual void CleanupD3DDevice();
  void ReleasePooledUniformBuffers();
  /** The global D3D interface. */
  TRefCountPtr<IDXGIFactory1> DXGIFactory1;

  /** The global D3D device's immediate context */
  TRefCountPtr<ID3D11DeviceContext> Direct3DDeviceIMContext;

  /** The global D3D device's immediate context */
  TRefCountPtr<ID3D11Device> Direct3DDevice;

  RBD3D11StateCache StateCache;
  /*
  template<typename TRHIType>
  static FORCEINLINE typename TD3D11ResourceTraits<TRHIType>::TConcreteType* ResourceCast(TRHIType* Resource)
  {
    return static_cast<typename TD3D11ResourceTraits<TRHIType>::TConcreteType*>(Resource);
  }
  */

  template <EShaderFrequency RBEShaderFrequency>
  void ClearShaderResourceViews(RBD3D11BaseShaderResource* Resource);

  void ClearState();
  void ConditionalClearShaderResource(RBD3D11BaseShaderResource* Resource);
  void ClearAllShaderResources();

  // Accessors.
  ID3D11Device* GetDevice() const
  {
	  return Direct3DDevice;
  }
  ID3D11DeviceContext* GetDeviceContext() const
  {
	  return Direct3DDeviceIMContext;
  }
  IDXGIFactory1* GetFactory() const
  {
    return DXGIFactory1;
  }
  virtual RBViewportRHIRef RHICreateViewport(void* WindowHandle, uint32 SizeX, uint32 SizeY, bool bIsFullscreen, EPixelFormat PreferredPixelFormat) final override;
  virtual void RHIResizeViewport(RBViewportRHIParamRef Viewport, uint32 SizeX, uint32 SizeY, bool bIsFullscreen) final override;

  /** True if the device being used has been removed. */
  bool bDeviceRemoved;
	/** A list of all viewport RHIs that have been created. */
	std::vector<class RBD3D11Viewport*> Viewports;


  template <EShaderFrequency ShaderFrequency>
  void SetShaderResourceView(RBD3D11BaseShaderResource* Resource, ID3D11ShaderResourceView* SRV, int32 ResourceIndex, const char* SRVName, RBD3D11StateCache::ESRV_Type SrvType = RBD3D11StateCache::SRV_Unknown)
  {
    InternalSetShaderResourceView<ShaderFrequency>(Resource, SRV, ResourceIndex, SRVName, SrvType);
  }
protected:
	template<typename BaseResourceType>
	TD3D11Texture2D<BaseResourceType>* CreateD3D11Texture2D(uint32 SizeX, uint32 SizeY, uint32 SizeZ, bool bTextureArray, bool CubeTexture, uint8 Format,
		uint32 NumMips, uint32 NumSamples, uint32 Flags, RBRHIResourceCreateInfo& CreateInfo);
  /** Initializes the constant buffers.  Called once at RHI initialization time. */
  void InitConstantBuffers();

  /** needs to be called before each draw call */
  virtual void CommitNonComputeShaderConstants();

  /** needs to be called before each dispatch call */
  virtual void CommitComputeShaderConstants();
	//FD3D11Texture3D* CreateD3D11Texture3D(uint32 SizeX, uint32 SizeY, uint32 SizeZ, uint8 Format, uint32 NumMips, uint32 Flags, RBRHIResourceCreateInfo& CreateInfo);
  void CommitRenderTargetsAndUAVs();

  template <class ShaderType> void SetResourcesFromTables(const ShaderType* );
  void CommitGraphicsResourceTables();
  void CommitComputeResourceTables(FD3D11ComputeShader* ComputeShader);

	// @return 0xffffffff if not not supported
	uint32 GetMaxMSAAQuality(uint32 SampleCount);

  enum class EForceFullScreenClear
  {
    EDoNotForce,
    EForce
  };


  virtual void RHIClearMRTImpl(bool bClearColor, int32 NumClearColors, const RBColorf* ColorArray, bool bClearDepth, float Depth, bool bClearStencil, uint32 Stencil, RBAABBI ExcludeRect, bool bForceShaderClear, EForceFullScreenClear ForceFullScreen);
  template <EShaderFrequency ShaderFrequency>
  void InternalSetShaderResourceView(RBD3D11BaseShaderResource* Resource, ID3D11ShaderResourceView* SRV, int32 ResourceIndex, const char* SRVName, RBD3D11StateCache::ESRV_Type SrvType = RBD3D11StateCache::SRV_Unknown);



private:
  FD3D11Adapter _ChosenAdapter;
  // we don't use GetDesc().Description as there is a bug with Optimus where it can report the wrong name
  DXGI_ADAPTER_DESC _ChosenDescription;
  /*
  template <RBEShaderFrequency RBEShaderFrequency>
  void InternalSetShaderResourceView(RBD3D11BaseShaderResource* Resource, ID3D11ShaderResourceView* SRV, int32 ResourceIndex, std::string SRVName, RBD3D11StateCache::ESRV_Type SrvType = RBD3D11StateCache::SRV_Unknown);
  */

  // Tracks the currently set state blocks.
  bool bCurrentDepthStencilStateIsReadOnly;

  TRefCountPtr<ID3D11RenderTargetView> CurrentRenderTargets[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
  TRefCountPtr<ID3D11UnorderedAccessView> CurrentUAVs[D3D11_PS_CS_UAV_REGISTER_COUNT];
  TRefCountPtr<ID3D11DepthStencilView> CurrentDepthStencilTarget;
  TRefCountPtr<RBD3D11TextureBase> CurrentDepthTexture;
  RBD3D11BaseShaderResource* CurrentResourcesBoundAsSRVs[SF_NumFrequencies][D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
  int32 MaxBoundShaderResourcesIndex[SF_NumFrequencies];
  uint32 NumSimultaneousRenderTargets;
  uint32 NumUAVs;
  /** Tracks the current depth stencil access type. */
  RBExclusiveDepthStencil CurrentDSVAccessType;

  /** Internal frame counter, incremented on each call to RHIBeginScene. */
  uint32 SceneFrameCounter;

  /** Internal frame counter that just counts calls to Present */
  uint32 PresentCounter;

  /** D3D11 defines a maximum of 14 constant buffers per shader stage. */
  enum { MAX_UNIFORM_BUFFERS_PER_SHADER_STAGE = 14 };

  /** Track the currently bound uniform buffers. */
  RBUniformBufferRHIRef BoundUniformBuffers[SF_NumFrequencies][MAX_UNIFORM_BUFFERS_PER_SHADER_STAGE];

  /** Bit array to track which uniform buffers have changed since the last draw call. */
  uint16 DirtyUniformBuffers[SF_NumFrequencies];

  /** A list of all D3D constant buffers RHIs that have been created. */
  std::vector<TRefCountPtr<FD3D11ConstantBuffer> > VSConstantBuffers;
  std::vector<TRefCountPtr<FD3D11ConstantBuffer> > HSConstantBuffers;
  std::vector<TRefCountPtr<FD3D11ConstantBuffer> > DSConstantBuffers;
  std::vector<TRefCountPtr<FD3D11ConstantBuffer> > PSConstantBuffers;
  std::vector<TRefCountPtr<FD3D11ConstantBuffer> > GSConstantBuffers;
  std::vector<TRefCountPtr<FD3D11ConstantBuffer> > CSConstantBuffers;

  RBBoundShaderStateRHIParamRef CurrentBoundedShaderState;

  bool bDiscardSharedConstants;
  bool bUsingTessellation;


};

extern RBD3D11BaseShaderResource* CurrentResourcesBoundAsSRVs[SF_NumFrequencies][D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
extern int32 MaxBoundShaderResourcesIndex[SF_NumFrequencies];

template <EShaderFrequency ShaderFrequency>
void FD3D11DynamicRHI::ClearShaderResourceViews(RBD3D11BaseShaderResource* Resource)
{
  int32 MaxIndex = MaxBoundShaderResourcesIndex[ShaderFrequency];
  for (int32 ResourceIndex = MaxIndex; ResourceIndex >= 0; --ResourceIndex)
  {
    if (CurrentResourcesBoundAsSRVs[ShaderFrequency][ResourceIndex] == Resource)
    {
      // Unset the SRV from the device context
      InternalSetShaderResourceView<ShaderFrequency>(nullptr, nullptr, ResourceIndex, "");
    }
  }
}

template <EShaderFrequency Frequency>
FORCEINLINE void SetResource(FD3D11DynamicRHI*  D3D11RHI, RBD3D11StateCache*  StateCache, uint32 BindIndex, ID3D11SamplerState*  SamplerState)
{
  StateCache->SetSamplerState<Frequency>(SamplerState, BindIndex);
}

template <EShaderFrequency Frequency>
FORCEINLINE void SetResource(FD3D11DynamicRHI*  D3D11RHI, RBD3D11StateCache*  StateCache, uint32 BindIndex, RBD3D11BaseShaderResource*  ShaderResource, ID3D11ShaderResourceView*  SRV, const char* ResourceName = "")
{
  // We set the resource through the RHI to track state for the purposes of unbinding SRVs when a UAV or RTV is bound.
  // todo: need to support SRV_Static for faster calls when possible
  D3D11RHI->SetShaderResourceView<Frequency>(ShaderResource, SRV, BindIndex, ResourceName, RBD3D11StateCache::SRV_Unknown);
}

template <EShaderFrequency ShaderFrequency>
inline int32 SetShaderResourcesFromBuffer_Surface(FD3D11DynamicRHI*  D3D11RHI, RBD3D11StateCache* StateCache, RBD3D11UniformBuffer* Buffer, const uint32*  ResourceMap, int32 BufferIndex)
{
  const TRefCountPtr<RBRHIResource>* Resources = Buffer->ResourceTable.data();
  float CurrentTime = g_base_app->get_time_in_millisecond();
  int32 NumSetCalls = 0;
  uint32 BufferOffset = ResourceMap[BufferIndex];
  if (BufferOffset > 0)
  {
    const uint32*  ResourceInfos = &ResourceMap[BufferOffset];
    uint32 ResourceInfo = *ResourceInfos++;
    do
    {
      CHECK(FRHIResourceTableEntry::GetUniformBufferIndex(ResourceInfo) == BufferIndex);
      const uint16 ResourceIndex = FRHIResourceTableEntry::GetResourceIndex(ResourceInfo);
      const uint8 BindIndex = FRHIResourceTableEntry::GetBindIndex(ResourceInfo);

      RBD3D11BaseShaderResource* ShaderResource = nullptr;
      ID3D11ShaderResourceView* D3D11Resource = nullptr;

      RBRHITexture* TextureRHI = (RBRHITexture*)Resources[ResourceIndex].GetReference();
      TextureRHI->SetLastRenderTime(CurrentTime);
      RBD3D11TextureBase* TextureD3D11 = GetD3D11TextureFromRHITexture(TextureRHI);
      ShaderResource = TextureD3D11->GetBaseShaderResource();
      D3D11Resource = TextureD3D11->GetShaderResourceView();

      // todo: could coalesce adjacent bound resources.
      SetResource<ShaderFrequency>(D3D11RHI, StateCache, BindIndex, ShaderResource, D3D11Resource, TextureRHI->GetName().c_str());
      NumSetCalls++;
      ResourceInfo = *ResourceInfos++;
    } while (FRHIResourceTableEntry::GetUniformBufferIndex(ResourceInfo) == BufferIndex);
  }
  return NumSetCalls;
}


template <EShaderFrequency ShaderFrequency>
inline int32 SetShaderResourcesFromBuffer_SRV(FD3D11DynamicRHI* D3D11RHI, RBD3D11StateCache*  StateCache, RBD3D11UniformBuffer* Buffer, const uint32* ResourceMap, int32 BufferIndex)
{
  const TRefCountPtr<RBRHIResource>* Resources = Buffer->ResourceTable.data();
  float CurrentTime = g_base_app->get_time_in_millisecond();
  int32 NumSetCalls = 0;
  uint32 BufferOffset = ResourceMap[BufferIndex];
  if (BufferOffset > 0)
  {
    const uint32* ResourceInfos = &ResourceMap[BufferOffset];
    uint32 ResourceInfo = *ResourceInfos++;
    do
    {
      CHECK(FRHIResourceTableEntry::GetUniformBufferIndex(ResourceInfo) == BufferIndex);
      const uint16 ResourceIndex = FRHIResourceTableEntry::GetResourceIndex(ResourceInfo);
      const uint8 BindIndex = FRHIResourceTableEntry::GetBindIndex(ResourceInfo);

      RBD3D11BaseShaderResource* ShaderResource = nullptr;
      ID3D11ShaderResourceView* D3D11Resource = nullptr;

      RBD3D11ShaderResourceView* ShaderResourceViewRHI = (RBD3D11ShaderResourceView*)Resources[ResourceIndex].GetReference();
      ShaderResource = ShaderResourceViewRHI->Resource.GetReference();
      D3D11Resource = ShaderResourceViewRHI->View.GetReference();

      // todo: could coalesce adjacent bound resources.
      SetResource<ShaderFrequency>(D3D11RHI, StateCache, BindIndex, ShaderResource, D3D11Resource);
      NumSetCalls++;
      ResourceInfo = *ResourceInfos++;
    } while (FRHIResourceTableEntry::GetUniformBufferIndex(ResourceInfo) == BufferIndex);
  }
  return NumSetCalls;
}

template <EShaderFrequency ShaderFrequency>
inline int32 SetShaderResourcesFromBuffer_Sampler(FD3D11DynamicRHI*D3D11RHI, RBD3D11StateCache* StateCache, RBD3D11UniformBuffer* Buffer, const uint32* ResourceMap, int32 BufferIndex)
{
  const TRefCountPtr<RBRHIResource>* Resources = Buffer->ResourceTable.data();
  int32 NumSetCalls = 0;
  uint32 BufferOffset = ResourceMap[BufferIndex];
  if (BufferOffset > 0)
  {
    const uint32* ResourceInfos = &ResourceMap[BufferOffset];
    uint32 ResourceInfo = *ResourceInfos++;
    do
    {
      CHECK(FRHIResourceTableEntry::GetUniformBufferIndex(ResourceInfo) == BufferIndex);
      const uint16 ResourceIndex = FRHIResourceTableEntry::GetResourceIndex(ResourceInfo);
      const uint8 BindIndex = FRHIResourceTableEntry::GetBindIndex(ResourceInfo);

      ID3D11SamplerState* D3D11Resource = ((RBD3D11SamplerState*)Resources[ResourceIndex].GetReference())->Resource.GetReference();

      // todo: could coalesce adjacent bound resources.
      SetResource<ShaderFrequency>(D3D11RHI, StateCache, BindIndex, D3D11Resource);
      NumSetCalls++;
      ResourceInfo = *ResourceInfos++;
    } while (FRHIResourceTableEntry::GetUniformBufferIndex(ResourceInfo) == BufferIndex);
  }
  return NumSetCalls;
}

template <class ShaderType>
void FD3D11DynamicRHI::SetResourcesFromTables(const ShaderType*  Shader)
{
  CHECK(Shader);

  // Mask the dirty bits by those buffers from which the shader has bound resources.
  uint32 DirtyBits = Shader->ShaderResourceTable.ResourceTableBits & DirtyUniformBuffers[ShaderType::StaticFrequency];
  while (DirtyBits)
  {
    // Scan for the lowest set bit, compute its index, clear it in the set of dirty bits.
    //-(int32)DirtyBits 等价 ~DirtyBits+1（求补操作，所有整形取负都是求补操作），因为编译器禁止对uint取负，所以转成int
    const uint32 LowestBitMask = (DirtyBits)& (-(int32)DirtyBits);
    const int32 BufferIndex = RBMath::floor_log2(LowestBitMask); // todo: This has a branch on zero, we know it could never be zero...
    DirtyBits ^= LowestBitMask;
    RBD3D11UniformBuffer* Buffer = (RBD3D11UniformBuffer*)BoundUniformBuffers[ShaderType::StaticFrequency][BufferIndex].GetReference();
    CHECK(Buffer);
    CHECK(BufferIndex < Shader->ShaderResourceTable.ResourceTableLayoutHashes.size());

#if 1//!(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    // to track down OR-7159 CRASH: Client crashed at start of match in D3D11Commands.cpp
    {
      if (Buffer->GetLayout().GetHash() != Shader->ShaderResourceTable.ResourceTableLayoutHashes[BufferIndex])
      {
        auto& BufferLayout = Buffer->GetLayout();
        std::string DebugName = BufferLayout.GetDebugName();
        const std::string& ShaderName = Shader->ShaderName;
#if 1//UE_BUILD_DEBUG
        std::string ShaderUB;
        if (BufferIndex < Shader->UniformBuffers.size())
        {
          char s[1024];
          sprintf(s,("expecting UB '%s'"), *Shader->UniformBuffers[BufferIndex].data());
          ShaderUB = s;
        }
        g_logger->debug(WIP_ERROR, ("SetResourcesFromTables upcoming check(%08x != %08x); Bound Layout='%s' Shader='%s' %s"), BufferLayout.GetHash(), Shader->ShaderResourceTable.ResourceTableLayoutHashes[BufferIndex], DebugName.c_str(), ShaderName.c_str(), ShaderUB.c_str());
        std::string ResourcesString;
        for (int32 Index = 0; Index < BufferLayout.Resources.size(); ++Index)
        {
          char s[1024];
          sprintf(s,("%d "), BufferLayout.Resources[Index]);
          ResourcesString += s;
        }
        g_logger->debug(WIP_ERROR, ("Layout CB Size %d Res Offs %d; %d Resources: %s"), BufferLayout.ConstantBufferSize, BufferLayout.ResourceOffset, BufferLayout.Resources.size(), ResourcesString.c_str());
#else
        UE_LOG(LogD3D11RHI, Error, TEXT("Bound Layout='%s' Shader='%s', Layout CB Size %d Res Offs %d; %d"), *DebugName, *ShaderName, BufferLayout.ConstantBufferSize, BufferLayout.ResourceOffset, BufferLayout.Resources.Num());
#endif
        // this might mean you are accessing a data you haven't bound e.g. GBuffer
        CHECK(BufferLayout.GetHash() == Shader->ShaderResourceTable.ResourceTableLayoutHashes[BufferIndex]);
      }
    }
#endif

    // todo: could make this two pass: gather then set
    SetShaderResourcesFromBuffer_Surface<(EShaderFrequency)ShaderType::StaticFrequency>(this, &StateCache, Buffer, Shader->ShaderResourceTable.TextureMap.data(), BufferIndex);
    SetShaderResourcesFromBuffer_SRV<(EShaderFrequency)ShaderType::StaticFrequency>(this, &StateCache, Buffer, Shader->ShaderResourceTable.ShaderResourceViewMap.data(), BufferIndex);
    SetShaderResourcesFromBuffer_Sampler<(EShaderFrequency)ShaderType::StaticFrequency>(this, &StateCache, Buffer, Shader->ShaderResourceTable.SamplerMap.data(), BufferIndex);
  }
  DirtyUniformBuffers[ShaderType::StaticFrequency] = 0;
}



struct FD3D11GlobalStats
{
  // in bytes, never change after RHI, needed to scale game features
  static int64 GDedicatedVideoMemory;

  // in bytes, never change after RHI, needed to scale game features
  static int64 GDedicatedSystemMemory;

  // in bytes, never change after RHI, needed to scale game features
  static int64 GSharedSystemMemory;

  // In bytes. Never changed after RHI init. Our estimate of the amount of memory that we can use for graphics resources in total.
  static int64 GTotalGraphicsMemory;
};



/** Find an appropriate DXGI format for the input format and SRGB setting. */
inline DXGI_FORMAT FindShaderResourceDXGIFormat(DXGI_FORMAT InFormat, bool bSRGB)
{
	if (bSRGB)
	{
		switch (InFormat)
		{
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:    return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:    return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		case DXGI_FORMAT_BC1_TYPELESS:         return DXGI_FORMAT_BC1_UNORM_SRGB;
		case DXGI_FORMAT_BC2_TYPELESS:         return DXGI_FORMAT_BC2_UNORM_SRGB;
		case DXGI_FORMAT_BC3_TYPELESS:         return DXGI_FORMAT_BC3_UNORM_SRGB;
		case DXGI_FORMAT_BC7_TYPELESS:         return DXGI_FORMAT_BC7_UNORM_SRGB;
		};
	}
	else
	{
		switch (InFormat)
		{
		case DXGI_FORMAT_B8G8R8A8_TYPELESS: return DXGI_FORMAT_B8G8R8A8_UNORM;
		case DXGI_FORMAT_R8G8B8A8_TYPELESS: return DXGI_FORMAT_R8G8B8A8_UNORM;
		case DXGI_FORMAT_BC1_TYPELESS:      return DXGI_FORMAT_BC1_UNORM;
		case DXGI_FORMAT_BC2_TYPELESS:      return DXGI_FORMAT_BC2_UNORM;
		case DXGI_FORMAT_BC3_TYPELESS:      return DXGI_FORMAT_BC3_UNORM;
		case DXGI_FORMAT_BC7_TYPELESS:      return DXGI_FORMAT_BC7_UNORM;
		};
	}
	switch (InFormat)
	{
	case DXGI_FORMAT_R24G8_TYPELESS: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	case DXGI_FORMAT_R32_TYPELESS: return DXGI_FORMAT_R32_FLOAT;
	case DXGI_FORMAT_R16_TYPELESS: return DXGI_FORMAT_R16_UNORM;
#if DEPTH_32_BIT_CONVERSION
		// Changing Depth Buffers to 32 bit on Dingo as D24S8 is actually implemented as a 32 bit buffer in the hardware
	case DXGI_FORMAT_R32G8X24_TYPELESS: return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
#endif
	}
	return InFormat;
}

/** Find an appropriate DXGI format unordered access of the raw format. */
inline DXGI_FORMAT FindUnorderedAccessDXGIFormat(DXGI_FORMAT InFormat)
{
	switch (InFormat)
	{
	case DXGI_FORMAT_B8G8R8A8_TYPELESS: return DXGI_FORMAT_B8G8R8A8_UNORM;
	case DXGI_FORMAT_R8G8B8A8_TYPELESS: return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	return InFormat;
}

/** Find the appropriate depth-stencil targetable DXGI format for the given format. */
inline DXGI_FORMAT FindDepthStencilDXGIFormat(DXGI_FORMAT InFormat)
{
	switch (InFormat)
	{
	case DXGI_FORMAT_R24G8_TYPELESS:
		return DXGI_FORMAT_D24_UNORM_S8_UINT;
#if DEPTH_32_BIT_CONVERSION
		// Changing Depth Buffers to 32 bit on Dingo as D24S8 is actually implemented as a 32 bit buffer in the hardware
	case DXGI_FORMAT_R32G8X24_TYPELESS:
		return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
#endif
	case DXGI_FORMAT_R32_TYPELESS:
		return DXGI_FORMAT_D32_FLOAT;
	case DXGI_FORMAT_R16_TYPELESS:
		return DXGI_FORMAT_D16_UNORM;
	};
	return InFormat;
}

/**
* Returns whether the given format contains stencil information.
* Must be passed a format returned by FindDepthStencilDXGIFormat, so that typeless versions are converted to their corresponding depth stencil view format.
*/
inline bool HasStencilBits(DXGI_FORMAT InFormat)
{
  switch (InFormat)
  {
  case DXGI_FORMAT_D24_UNORM_S8_UINT:
    return true;
#if  DEPTH_32_BIT_CONVERSION
    // Changing Depth Buffers to 32 bit on Dingo as D24S8 is actually implemented as a 32 bit buffer in the hardware
  case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    return true;
#endif
  };
  return false;
}