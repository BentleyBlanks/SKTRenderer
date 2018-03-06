#include "RHID3D11.h"
#include "D3D11State.h"
#include "Logger.h"
#include "RHIUtil.h"

struct FRTVDesc
{
  uint32 Width;
  uint32 Height;
  DXGI_SAMPLE_DESC SampleDesc;
};

// Return an FRTVDesc structure whose
// Width and height dimensions are adjusted for the RTV's miplevel.
FRTVDesc GetRenderTargetViewDesc(ID3D11RenderTargetView* RenderTargetView)
{
  D3D11_RENDER_TARGET_VIEW_DESC TargetDesc;
  RenderTargetView->GetDesc(&TargetDesc);

  TRefCountPtr<ID3D11Resource> BaseResource;
  RenderTargetView->GetResource((ID3D11Resource**)BaseResource.GetInitReference());
  uint32 MipIndex = 0;
  FRTVDesc ret;
  memset(&ret, 0, sizeof(ret));

  switch (TargetDesc.ViewDimension)
  {
  case D3D11_RTV_DIMENSION_TEXTURE2D:
  case D3D11_RTV_DIMENSION_TEXTURE2DMS:
  case D3D11_RTV_DIMENSION_TEXTURE2DARRAY:
  case D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY:
  {
    D3D11_TEXTURE2D_DESC Desc;
    ((ID3D11Texture2D*)(BaseResource.GetReference()))->GetDesc(&Desc);
    ret.Width = Desc.Width;
    ret.Height = Desc.Height;
    ret.SampleDesc = Desc.SampleDesc;
    if (TargetDesc.ViewDimension == D3D11_RTV_DIMENSION_TEXTURE2D || TargetDesc.ViewDimension == D3D11_RTV_DIMENSION_TEXTURE2DARRAY)
    {
      // All the non-multisampled texture types have their mip-slice in the same position.
      MipIndex = TargetDesc.Texture2D.MipSlice;
    }
    break;
  }
  case D3D11_RTV_DIMENSION_TEXTURE3D:
  {
    D3D11_TEXTURE3D_DESC Desc;
    ((ID3D11Texture3D*)(BaseResource.GetReference()))->GetDesc(&Desc);
    ret.Width = Desc.Width;
    ret.Height = Desc.Height;
    ret.SampleDesc.Count = 1;
    ret.SampleDesc.Quality = 0;
    MipIndex = TargetDesc.Texture3D.MipSlice;
    break;
  }
  default:
  {
    CHECK(!"not expecting 1D targets.");
    //checkNoEntry();
  }
  }
  ret.Width >>= MipIndex;
  ret.Height >>= MipIndex;
  return ret;
}

// Primitive drawing.

static D3D11_PRIMITIVE_TOPOLOGY GetD3D11PrimitiveType(uint32 PrimitiveType, bool bUsingTessellation)
{
  if (bUsingTessellation)
  {
    switch (PrimitiveType)
    {
    case PT_1_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
    case PT_2_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;

      // This is the case for tessellation without AEN or other buffers, so just flip to 3 CPs
    case PT_TriangleList: return D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;

    case PT_LineList:
    case PT_TriangleStrip:
    case PT_QuadList:
    case PT_PointList:
      g_logger->debug(WIP_ERROR, TEXT("Invalid type specified for tessellated render, probably missing a case in FStaticMeshSceneProxy::GetMeshElement"));
      break;
    default:
      // Other cases are valid.
      break;
    };
  }

  switch (PrimitiveType)
  {
  case PT_TriangleList: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
  case PT_TriangleStrip: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
  case PT_LineList: return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
  case PT_PointList: return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;

    // ControlPointPatchList types will pretend to be TRIANGLELISTS with a stride of N 
    // (where N is the number of control points specified), so we can return them for
    // tessellation and non-tessellation. This functionality is only used when rendering a 
    // default material with something that claims to be tessellated, generally because the 
    // tessellation material failed to compile for some reason.
  case PT_3_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
  case PT_4_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;
  case PT_5_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST;
  case PT_6_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST;
  case PT_7_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST;
  case PT_8_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST;
  case PT_9_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST;
  case PT_10_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST;
  case PT_11_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST;
  case PT_12_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST;
  case PT_13_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST;
  case PT_14_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST;
  case PT_15_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST;
  case PT_16_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST;
  case PT_17_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST;
  case PT_18_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST;
  case PT_19_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST;
  case PT_20_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST;
  case PT_21_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST;
  case PT_22_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST;
  case PT_23_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST;
  case PT_24_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST;
  case PT_25_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST;
  case PT_26_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST;
  case PT_27_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST;
  case PT_28_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST;
  case PT_29_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST;
  case PT_30_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST;
  case PT_31_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST;
  case PT_32_ControlPointPatchList: return D3D11_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST;
  default: g_logger->debug(WIP_ERROR, TEXT("Unknown primitive type: %u"), PrimitiveType);
  };

  return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}


void FD3D11DynamicRHI::RHISetViewport(uint32 MinX, uint32 MinY, float MinZ, uint32 MaxX, uint32 MaxY, float MaxZ)
{
	// These are the maximum viewport extents for D3D11. Exceeding them leads to badness.
	CHECK(MinX <= (uint32)D3D11_VIEWPORT_BOUNDS_MAX);
	CHECK(MinY <= (uint32)D3D11_VIEWPORT_BOUNDS_MAX);
	CHECK(MaxX <= (uint32)D3D11_VIEWPORT_BOUNDS_MAX);
	CHECK(MaxY <= (uint32)D3D11_VIEWPORT_BOUNDS_MAX);

	D3D11_VIEWPORT Viewport = { (float)MinX, (float)MinY, (float)MaxX - MinX, (float)MaxY - MinY, MinZ, MaxZ };
	//avoid setting a 0 extent viewport, which the debug runtime doesn't like
	if (Viewport.Width > 0 && Viewport.Height > 0)
	{
		StateCache.SetViewport(Viewport);
    //D3D11 dont need
		//SetScissorRectIfRequiredWhenSettingViewport(MinX, MinY, MaxX, MaxY);
	}
}

void FD3D11DynamicRHI::RHISetScissorRect(bool bEnable, uint32 MinX, uint32 MinY, uint32 MaxX, uint32 MaxY)
{
  if (bEnable)
  {
    D3D11_RECT ScissorRect;
    ScissorRect.left = MinX;
    ScissorRect.right = MaxX;
    ScissorRect.top = MinY;
    ScissorRect.bottom = MaxY;
    Direct3DDeviceIMContext->RSSetScissorRects(1, &ScissorRect);
  }
  else
  {
    D3D11_RECT ScissorRect;
    ScissorRect.left = 0;
    ScissorRect.right = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
    ScissorRect.top = 0;
    ScissorRect.bottom = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
    Direct3DDeviceIMContext->RSSetScissorRects(1, &ScissorRect);
  }
}

void FD3D11DynamicRHI::RHISetDepthStencilState(RBDepthStencilStateRHIParamRef NewStateRHI, uint32 StencilRef)
{
  RBD3D11DepthStencilState* NewState = static_cast<RBD3D11DepthStencilState*>(NewStateRHI);

  //ValidateExclusiveDepthStencilAccess(NewState->AccessType);

  StateCache.SetDepthStencilState(NewState->Resource, StencilRef);
}


void FD3D11DynamicRHI::RHISetBlendState(RBBlendStateRHIParamRef NewStateRHI, const RBColorf& BlendFactor)
{
  RBD3D11BlendState* NewState = static_cast<RBD3D11BlendState*>(NewStateRHI);
  StateCache.SetBlendState(NewState->Resource, (const float*)&BlendFactor, 0xffffffff);
}

// Rasterizer state.
void FD3D11DynamicRHI::RHISetRasterizerState(RBRasterizerStateRHIParamRef NewStateRHI)
{
  RBD3D11RasterizerState* NewState = static_cast<RBD3D11RasterizerState*>(NewStateRHI);
  StateCache.SetRasterizerState(NewState->Resource);
}

// Raster operations.
void FD3D11DynamicRHI::RHIClear(bool bClearColor, const RBColorf& Color, bool bClearDepth, float Depth, bool bClearStencil, uint32 Stencil, RBAABBI ExcludeRect)
{
  FD3D11DynamicRHI::RHIClearMRTImpl(bClearColor, 1, &Color, bClearDepth, Depth, bClearStencil, Stencil, ExcludeRect, true, EForceFullScreenClear::EDoNotForce);
}

void FD3D11DynamicRHI::RHIClearMRT(bool bClearColor, int32 NumClearColors, const RBColorf* ClearColorArray, bool bClearDepth, float Depth, bool bClearStencil, uint32 Stencil, RBAABBI ExcludeRect)
{
  RHIClearMRTImpl(bClearColor, NumClearColors, ClearColorArray, bClearDepth, Depth, bClearStencil, Stencil, ExcludeRect, true, EForceFullScreenClear::EDoNotForce);
}

void FD3D11DynamicRHI::RHIClearMRTImpl(bool bClearColor, int32 NumClearColors, const RBColorf* ClearColorArray, bool bClearDepth, float Depth, bool bClearStencil, uint32 Stencil, RBAABBI ExcludeRect, bool bForceShaderClear, EForceFullScreenClear ForceFullScreen)
{
  //don't force shaders clears for the moment.  There are bugs with the state cache/restore behavior.
  //will either fix this soon, or move clear out of the RHI entirely.
  bForceShaderClear = false;

#if 0
  // Helper struct to record and restore device states RHIClearMRT modifies.
  class FDeviceStateHelper
  {
    // New Monolithic Graphics drivers have optional "fast calls" replacing various D3d functions
    // Note that the FastXXX calls are in the new ID3D11DeviceContextX (derived from ID3D11DeviceContext1 which is derived from ID3D11DeviceContext)
    /** The global D3D device's immediate context */
    TRefCountPtr<ID3D11DeviceContext> Direct3DDeviceIMContext;

    enum { ResourceCount = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT };
    enum { ConstantBufferCount = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT };

    //////////////////////////////////////////////////////////////////////////
    // Relevant recorded states:
    ID3D11ShaderResourceView* VertResources[ResourceCount];
    ID3D11Buffer* VertexConstantBuffers[ConstantBufferCount];
    ID3D11Buffer* PixelConstantBuffers[ConstantBufferCount];
    ID3D11VertexShader* VSOld;
    ID3D11PixelShader* PSOld;
    ID3D11DepthStencilState* OldDepthStencilState;
    ID3D11RasterizerState* OldRasterizerState;
    ID3D11BlendState* OldBlendState;
    ID3D11InputLayout* OldInputLayout;
    uint32 StencilRef;
    float BlendFactor[4];
    uint32 SampleMask;
    RBBoundShaderStateRHIParamRef LastBoundShaderStateRHI;

    //////////////////////////////////////////////////////////////////////////
    void ReleaseResources()
    {
      SAFE_RELEASE(VSOld);
      SAFE_RELEASE(PSOld);

      ID3D11ShaderResourceView** Resources = VertResources;
      for (int32 i = 0; i < ResourceCount; i++, Resources++)
      {
        SAFE_RELEASE(*Resources);
      }
      for (int32 i = 0; i < ConstantBufferCount; ++i)
      {
        SAFE_RELEASE(VertexConstantBuffers[i]);
        SAFE_RELEASE(PixelConstantBuffers[i]);
      }

      SAFE_RELEASE(OldDepthStencilState);
      SAFE_RELEASE(OldBlendState);
      SAFE_RELEASE(OldRasterizerState);
      SAFE_RELEASE(OldInputLayout);
      LastBoundShaderStateRHI = nullptr;
    }
  public:
    /** The global D3D device's immediate context */
    FDeviceStateHelper(TRefCountPtr<ID3D11DeviceContext> InDirect3DDeviceIMContext) : Direct3DDeviceIMContext(InDirect3DDeviceIMContext) {}

    void CaptureDeviceState(RBD3D11StateCache& StateCacheRef, TGlobalResource< TBoundShaderStateHistory<10000> >& BSSHistory)
    {
      StateCacheRef.GetVertexShader(&VSOld);
      StateCacheRef.GetPixelShader(&PSOld);
      StateCacheRef.GetShaderResourceViews<SF_Vertex>(0, ResourceCount, &VertResources[0]);
      StateCacheRef.GetConstantBuffers<SF_Pixel>(0, ConstantBufferCount, &(PixelConstantBuffers[0]));
      StateCacheRef.GetConstantBuffers<SF_Vertex>(0, ConstantBufferCount, &(VertexConstantBuffers[0]));
      StateCacheRef.GetDepthStencilState(&OldDepthStencilState, &StencilRef);
      StateCacheRef.GetBlendState(&OldBlendState, BlendFactor, &SampleMask);
      StateCacheRef.GetRasterizerState(&OldRasterizerState);
      StateCacheRef.GetInputLayout(&OldInputLayout);
      LastBoundShaderStateRHI = BSSHistory.GetLast();
    }

    void ClearCurrentVertexResources(RBD3D11StateCache& StateCacheRef)
    {
      static ID3D11ShaderResourceView* NullResources[ResourceCount] = {};
      for (int ResourceLoop = 0; ResourceLoop < ResourceCount; ResourceLoop++)
      {
        StateCacheRef.SetShaderResourceView<SF_Vertex>(NullResources[0], 0);
      }
    }

    void RestoreDeviceState(RBD3D11StateCache& StateCacheRef, TGlobalResource< TBoundShaderStateHistory<10000> >& BSSHistory)
    {

      // Restore the old shaders
      StateCacheRef.SetVertexShader(VSOld);
      StateCacheRef.SetPixelShader(PSOld);
      for (int ResourceLoop = 0; ResourceLoop < ResourceCount; ResourceLoop++)
      {
        StateCacheRef.SetShaderResourceView<SF_Vertex>(VertResources[ResourceLoop], ResourceLoop);
      }
      for (int BufferIndex = 0; BufferIndex < ConstantBufferCount; ++BufferIndex)
      {
        StateCacheRef.SetConstantBuffer<SF_Pixel>(PixelConstantBuffers[BufferIndex], BufferIndex);
        StateCacheRef.SetConstantBuffer<SF_Vertex>(VertexConstantBuffers[BufferIndex], BufferIndex);
      }

      StateCacheRef.SetDepthStencilState(OldDepthStencilState, StencilRef);
      StateCacheRef.SetBlendState(OldBlendState, BlendFactor, SampleMask);
      StateCacheRef.SetRasterizerState(OldRasterizerState);
      StateCacheRef.SetInputLayout(OldInputLayout);

      BSSHistory.Add(LastBoundShaderStateRHI);
      ReleaseResources();
    }
  };
#endif

  {
    // <0: Auto
    int32 ClearWithExcludeRects = 2;

    if (ClearWithExcludeRects >= 2)
    {
      // by default use the exclude rect
      ClearWithExcludeRects = 1;

      /*
      if (IsRHIDeviceIntel())
      {
        // Disable exclude rect (Intel has fast clear so better we disable)
        ClearWithExcludeRects = 0;
      }
      */
    }

    if (!ClearWithExcludeRects)
    {
      // Disable exclude rect
      ExcludeRect = RBAABBI();
    }
  }

  FD3D11BoundRenderTargets BoundRenderTargets(Direct3DDeviceIMContext);

  // Must specify enough clear colors for all active RTs
  CHECK(!bClearColor || NumClearColors >= BoundRenderTargets.GetNumActiveTargets());

  // If we're clearing depth or stencil and we have a readonly depth/stencil view bound, we need to use a writable depth/stencil view
  if (CurrentDepthTexture)
  {
    RBExclusiveDepthStencil RequestedAccess;

    RequestedAccess.SetDepthStencilWrite(bClearDepth, bClearStencil);

    CHECK(RequestedAccess.IsValid(CurrentDSVAccessType));
  }

  ID3D11DepthStencilView* DepthStencilView = BoundRenderTargets.GetDepthStencilView();

  // Determine if we're trying to clear a subrect of the screen
  bool UseDrawClear = bForceShaderClear;
  uint32 NumViews = 1;
  D3D11_VIEWPORT Viewport;
  StateCache.GetViewports(&NumViews, &Viewport);
  if (Viewport.TopLeftX > 0 || Viewport.TopLeftY > 0)
  {
    UseDrawClear = true;
    CHECKF(ForceFullScreen == EForceFullScreenClear::EDoNotForce, TEXT("Forced Full Screen Clear ignoring Viewport Restriction"));
  }

  /*	// possible optimization
  if(ExcludeRect.Width() > 0 && ExcludeRect.Height() > 0 && HardwareHasLinearClearPerformance)
  {
  UseDrawClear = true;
  }
  */
  if (ExcludeRect.min.x == 0 && ExcludeRect.get_width() == Viewport.Width && ExcludeRect.min.y == 0 && ExcludeRect.get_height() == Viewport.Height)
  {
    // no need to do anything
    if (ForceFullScreen == EForceFullScreenClear::EDoNotForce)
    {
      return;
    }
    else
    {
      //ensureMsgf(false, TEXT("Forced Full Screen Clear ignoring Exclude Rect Restriction"));
    }
  }

  D3D11_RECT ScissorRect;
  uint32 NumRects = 1;
  Direct3DDeviceIMContext->RSGetScissorRects(&NumRects, &ScissorRect);
  if (ScissorRect.left > 0
    || ScissorRect.right < Viewport.TopLeftX + Viewport.Width
    || ScissorRect.top > 0
    || ScissorRect.bottom < Viewport.TopLeftY + Viewport.Height)
  {
    UseDrawClear = true;
    //ensureMsgf(ForceFullScreen == EForceFullScreenClear::EDoNotForce, TEXT("Forced Full Screen Clear ignoring Scissor Rect Restriction"));
  }

  if (!UseDrawClear)
  {
    uint32 Width = 0;
    uint32 Height = 0;
    if (BoundRenderTargets.GetRenderTargetView(0))
    {
      FRTVDesc RTVDesc = GetRenderTargetViewDesc(BoundRenderTargets.GetRenderTargetView(0));
      Width = RTVDesc.Width;
      Height = RTVDesc.Height;
    }
    else if (DepthStencilView)
    {
      ID3D11Texture2D* BaseTexture = NULL;
      DepthStencilView->GetResource((ID3D11Resource**)&BaseTexture);
      D3D11_TEXTURE2D_DESC Desc;
      BaseTexture->GetDesc(&Desc);
      Width = Desc.Width;
      Height = Desc.Height;
      BaseTexture->Release();

      // Adjust dimensions for the mip level we're clearing.
      D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc;
      DepthStencilView->GetDesc(&DSVDesc);
      if (DSVDesc.ViewDimension == D3D11_DSV_DIMENSION_TEXTURE1D ||
        DSVDesc.ViewDimension == D3D11_DSV_DIMENSION_TEXTURE1DARRAY ||
        DSVDesc.ViewDimension == D3D11_DSV_DIMENSION_TEXTURE2D ||
        DSVDesc.ViewDimension == D3D11_DSV_DIMENSION_TEXTURE2DARRAY)
      {
        // All the non-multisampled texture types have their mip-slice in the same position.
        uint32 MipIndex = DSVDesc.Texture2D.MipSlice;
        Width >>= MipIndex;
        Height >>= MipIndex;
      }
    }

    if ((Viewport.Width < Width || Viewport.Height < Height)
      && (Viewport.Width > 1 && Viewport.Height > 1))
    {
      UseDrawClear = true;
      //ensureMsgf(ForceFullScreen == EForceFullScreenClear::EDoNotForce, TEXT("Forced Full Screen Clear ignoring View Dimension Restriction"));
    }
  }

  if (ForceFullScreen == EForceFullScreenClear::EForce)
  {
    UseDrawClear = false;
  }

#if 0
  if (UseDrawClear)
  {
    // we don't support draw call clears before the RHI is initialized, reorder the code or make sure it's not a draw call clear
    CHECK(GIsRHIInitialized);

    if (CurrentDepthTexture)
    {
      // Clear all texture references to this depth buffer
      ConditionalClearShaderResource(CurrentDepthTexture);
    }

    // Build new states
    RBBlendStateRHIParamRef BlendStateRHI;

    if (BoundRenderTargets.GetNumActiveTargets() <= 1)
    {
      BlendStateRHI = (bClearColor && BoundRenderTargets.GetRenderTargetView(0))
        ? TStaticBlendState<>::GetRHI()
        : TStaticBlendState<CW_NONE>::GetRHI();
    }
    else
    {
      BlendStateRHI = (bClearColor && BoundRenderTargets.GetRenderTargetView(0))
        ? TStaticBlendState<>::GetRHI()
        : TStaticBlendStateWriteMask<CW_NONE, CW_NONE, CW_NONE, CW_NONE, CW_NONE, CW_NONE, CW_NONE, CW_NONE>::GetRHI();
    }

    RBRasterizerStateRHIParamRef RasterizerStateRHI = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI();
    float BF[4] = { 0, 0, 0, 0 };

    const RBDepthStencilStateRHIParamRef DepthStencilStateRHI =
      (bClearDepth && bClearStencil)
      ? TStaticDepthStencilState<
      true, CF_Always,
      true, CF_Always, SO_Replace, SO_Replace, SO_Replace,
      false, CF_Always, SO_Replace, SO_Replace, SO_Replace,
      0xff, 0xff
      >::GetRHI()
      : bClearDepth
      ? TStaticDepthStencilState<true, CF_Always>::GetRHI()
      : bClearStencil
      ? TStaticDepthStencilState<
      false, CF_Always,
      true, CF_Always, SO_Replace, SO_Replace, SO_Replace,
      false, CF_Always, SO_Replace, SO_Replace, SO_Replace,
      0xff, 0xff
      >::GetRHI()
      : TStaticDepthStencilState<false, CF_Always>::GetRHI();

    // Create an access type mask by setting the readonly bits according to the bClearDepth/bClearStencil bools.
    {
      RBExclusiveDepthStencil RequestedAccess;

      RequestedAccess.SetDepthStencilWrite(bClearDepth, bClearStencil);

      ValidateExclusiveDepthStencilAccess(RequestedAccess);
    }

    RBD3D11BlendState* BlendState = static_cast<RBD3D11BlendState*>(BlendStateRHI);
    RBD3D11RasterizerState* RasterizerState = static_cast<RBD3D11RasterizerState*>(RasterizerStateRHI);
    RBD3D11DepthStencilState* DepthStencilState = static_cast<RBD3D11DepthStencilState*>(DepthStencilStateRHI);

    // Store the current device state
    FDeviceStateHelper OriginalResourceState(Direct3DDeviceIMContext);
    OriginalResourceState.CaptureDeviceState(StateCache, BoundShaderStateHistory);

    // Set the cached state objects
    StateCache.SetBlendState(BlendState->Resource, BF, 0xffffffff);
    StateCache.SetDepthStencilState(DepthStencilState->Resource, Stencil);
    StateCache.SetRasterizerState(RasterizerState->Resource);
    OriginalResourceState.ClearCurrentVertexResources(StateCache);

    // Set the new shaders
    auto ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
    TShaderMapRef<TOneColorVS<true> > VertexShader(ShaderMap);

    FOneColorPS* PixelShader = NULL;

    // Set the shader to write to the appropriate number of render targets
    // On AMD PC hardware, outputting to a color index in the shader without a matching render target set has a significant performance hit
    if (BoundRenderTargets.GetNumActiveTargets() <= 1)
    {
      TShaderMapRef<TOneColorPixelShaderMRT<1> > MRTPixelShader(ShaderMap);
      PixelShader = *MRTPixelShader;
    }
    else if (BoundRenderTargets.GetNumActiveTargets() == 2)
    {
      TShaderMapRef<TOneColorPixelShaderMRT<2> > MRTPixelShader(ShaderMap);
      PixelShader = *MRTPixelShader;
    }
    else if (BoundRenderTargets.GetNumActiveTargets() == 3)
    {
      TShaderMapRef<TOneColorPixelShaderMRT<3> > MRTPixelShader(ShaderMap);
      PixelShader = *MRTPixelShader;
    }
    else if (BoundRenderTargets.GetNumActiveTargets() == 4)
    {
      TShaderMapRef<TOneColorPixelShaderMRT<4> > MRTPixelShader(ShaderMap);
      PixelShader = *MRTPixelShader;
    }
    else if (BoundRenderTargets.GetNumActiveTargets() == 5)
    {
      TShaderMapRef<TOneColorPixelShaderMRT<5> > MRTPixelShader(ShaderMap);
      PixelShader = *MRTPixelShader;
    }
    else if (BoundRenderTargets.GetNumActiveTargets() == 6)
    {
      TShaderMapRef<TOneColorPixelShaderMRT<6> > MRTPixelShader(ShaderMap);
      PixelShader = *MRTPixelShader;
    }
    else if (BoundRenderTargets.GetNumActiveTargets() == 7)
    {
      TShaderMapRef<TOneColorPixelShaderMRT<7> > MRTPixelShader(ShaderMap);
      PixelShader = *MRTPixelShader;
    }
    else if (BoundRenderTargets.GetNumActiveTargets() == 8)
    {
      TShaderMapRef<TOneColorPixelShaderMRT<8> > MRTPixelShader(ShaderMap);
      PixelShader = *MRTPixelShader;
    }

    {
      FRHICommandList_RecursiveHazardous RHICmdList(this);
      SetGlobalBoundShaderState(RHICmdList, GMaxRHIFeatureLevel, GD3D11ClearMRTBoundShaderState[FMath::Max(BoundRenderTargets.GetNumActiveTargets() - 1, 0)], GD3D11Vector4VertexDeclaration.VertexDeclarationRHI, *VertexShader, PixelShader);
      PixelShader->SetColors(RHICmdList, ClearColorArray, NumClearColors);

      {
        // Draw a fullscreen quad
        if (ExcludeRect.Width() > 0 && ExcludeRect.Height() > 0)
        {
          // with a hole in it (optimization in case the hardware has non constant clear performance)
          FVector4 OuterVertices[4];
          OuterVertices[0].Set(-1.0f, 1.0f, Depth, 1.0f);
          OuterVertices[1].Set(1.0f, 1.0f, Depth, 1.0f);
          OuterVertices[2].Set(1.0f, -1.0f, Depth, 1.0f);
          OuterVertices[3].Set(-1.0f, -1.0f, Depth, 1.0f);

          float InvViewWidth = 1.0f / Viewport.Width;
          float InvViewHeight = 1.0f / Viewport.Height;
          FVector4 FractionRect = FVector4(ExcludeRect.Min.X * InvViewWidth, ExcludeRect.Min.Y * InvViewHeight, (ExcludeRect.Max.X - 1) * InvViewWidth, (ExcludeRect.Max.Y - 1) * InvViewHeight);

          FVector4 InnerVertices[4];
          InnerVertices[0].Set(FMath::Lerp(-1.0f, 1.0f, FractionRect.X), FMath::Lerp(1.0f, -1.0f, FractionRect.Y), Depth, 1.0f);
          InnerVertices[1].Set(FMath::Lerp(-1.0f, 1.0f, FractionRect.Z), FMath::Lerp(1.0f, -1.0f, FractionRect.Y), Depth, 1.0f);
          InnerVertices[2].Set(FMath::Lerp(-1.0f, 1.0f, FractionRect.Z), FMath::Lerp(1.0f, -1.0f, FractionRect.W), Depth, 1.0f);
          InnerVertices[3].Set(FMath::Lerp(-1.0f, 1.0f, FractionRect.X), FMath::Lerp(1.0f, -1.0f, FractionRect.W), Depth, 1.0f);

          FVector4 Vertices[10];
          Vertices[0] = OuterVertices[0];
          Vertices[1] = InnerVertices[0];
          Vertices[2] = OuterVertices[1];
          Vertices[3] = InnerVertices[1];
          Vertices[4] = OuterVertices[2];
          Vertices[5] = InnerVertices[2];
          Vertices[6] = OuterVertices[3];
          Vertices[7] = InnerVertices[3];
          Vertices[8] = OuterVertices[0];
          Vertices[9] = InnerVertices[0];

          DrawPrimitiveUP(RHICmdList, PT_TriangleStrip, 8, Vertices, sizeof(Vertices[0]));
        }
        else
        {
          // without a hole
          FVector4 Vertices[4];
          Vertices[0].Set(-1.0f, 1.0f, Depth, 1.0f);
          Vertices[1].Set(1.0f, 1.0f, Depth, 1.0f);
          Vertices[2].Set(-1.0f, -1.0f, Depth, 1.0f);
          Vertices[3].Set(1.0f, -1.0f, Depth, 1.0f);
          DrawPrimitiveUP(RHICmdList, PT_TriangleStrip, 2, Vertices, sizeof(Vertices[0]));
        }
      }
      // Implicit flush. Always call flush when using a command list in RHI implementations before doing anything else. This is super hazardous.
    }

    // Restore the original device state
    OriginalResourceState.RestoreDeviceState(StateCache, BoundShaderStateHistory);
  }
  else
#endif
  {
    if (bClearColor && BoundRenderTargets.GetNumActiveTargets() > 0)
    {
      for (int32 TargetIndex = 0; TargetIndex < BoundRenderTargets.GetNumActiveTargets(); TargetIndex++)
      {
        Direct3DDeviceIMContext->ClearRenderTargetView(BoundRenderTargets.GetRenderTargetView(TargetIndex), (float*)&ClearColorArray[TargetIndex]);
      }
    }

    if ((bClearDepth || bClearStencil) && DepthStencilView)
    {
      uint32 ClearFlags = 0;
      if (bClearDepth)
      {
        ClearFlags |= D3D11_CLEAR_DEPTH;
      }
      if (bClearStencil)
      {
        ClearFlags |= D3D11_CLEAR_STENCIL;
      }
      Direct3DDeviceIMContext->ClearDepthStencilView(DepthStencilView, ClearFlags, Depth, Stencil);
    }
  }

}

void FD3D11DynamicRHI::RHISetShaderResourceViewParameter(RBPixelShaderRHIParamRef PixelShaderRHI, uint32 TextureIndex, RBShaderResourceViewRHIParamRef SRVRHI)
{
  //VALIDATE_BOUND_SHADER(PixelShaderRHI);

  RBD3D11ShaderResourceView* SRV = static_cast<RBD3D11ShaderResourceView*>(SRVRHI);

  RBD3D11BaseShaderResource* Resource = nullptr;
  ID3D11ShaderResourceView* D3D11SRV = nullptr;

  if (SRV)
  {
    Resource = SRV->Resource;
    D3D11SRV = SRV->View;
  }

  SetShaderResourceView<SF_Pixel>(Resource, D3D11SRV, TextureIndex, "");
}

void FD3D11DynamicRHI::RHISetShaderResourceViewParameter(RBVertexShaderRHIParamRef VertexShaderRHI, uint32 TextureIndex, RBShaderResourceViewRHIParamRef SRVRHI)
{
  //VALIDATE_BOUND_SHADER(VertexShaderRHI);

  RBD3D11ShaderResourceView* SRV = static_cast<RBD3D11ShaderResourceView*>(SRVRHI);

  RBD3D11BaseShaderResource* Resource = nullptr;
  ID3D11ShaderResourceView* D3D11SRV = nullptr;

  if (SRV)
  {
    Resource = SRV->Resource;
    D3D11SRV = SRV->View;
  }

  SetShaderResourceView<SF_Vertex>(Resource, D3D11SRV, TextureIndex, "");
}

void FD3D11DynamicRHI::RHISetShaderResourceViewParameter(RBComputeShaderRHIParamRef ComputeShaderRHI, uint32 TextureIndex, RBShaderResourceViewRHIParamRef SRVRHI)
{
  //VALIDATE_BOUND_SHADER(ComputeShaderRHI);

  RBD3D11ShaderResourceView* SRV = static_cast<RBD3D11ShaderResourceView*>(SRVRHI);

  RBD3D11BaseShaderResource* Resource = nullptr;
  ID3D11ShaderResourceView* D3D11SRV = nullptr;

  if (SRV)
  {
    Resource = SRV->Resource;
    D3D11SRV = SRV->View;
  }

  SetShaderResourceView<SF_Compute>(Resource, D3D11SRV, TextureIndex, "");
}

void FD3D11DynamicRHI::RHISetShaderResourceViewParameter(RBHullShaderRHIParamRef HullShaderRHI, uint32 TextureIndex,RBShaderResourceViewRHIParamRef SRVRHI)
{
  //VALIDATE_BOUND_SHADER(HullShaderRHI);

  RBD3D11ShaderResourceView* SRV = static_cast<RBD3D11ShaderResourceView*>(SRVRHI);

  RBD3D11BaseShaderResource* Resource = nullptr;
  ID3D11ShaderResourceView* D3D11SRV = nullptr;

  if (SRV)
  {
    Resource = SRV->Resource;
    D3D11SRV = SRV->View;
  }

  SetShaderResourceView<SF_Hull>(Resource, D3D11SRV, TextureIndex, "");
}

void FD3D11DynamicRHI::RHISetShaderResourceViewParameter(RBDomainShaderRHIParamRef DomainShaderRHI, uint32 TextureIndex, RBShaderResourceViewRHIParamRef SRVRHI)
{
  //VALIDATE_BOUND_SHADER(DomainShaderRHI);

  RBD3D11ShaderResourceView* SRV = static_cast<RBD3D11ShaderResourceView*>(SRVRHI);

  RBD3D11BaseShaderResource* Resource = nullptr;
  ID3D11ShaderResourceView* D3D11SRV = nullptr;

  if (SRV)
  {
    Resource = SRV->Resource;
    D3D11SRV = SRV->View;
  }

  SetShaderResourceView<SF_Domain>(Resource, D3D11SRV, TextureIndex, "");
}

void FD3D11DynamicRHI::RHISetShaderResourceViewParameter(RBGeometryShaderRHIParamRef GeometryShaderRHI, uint32 TextureIndex, RBShaderResourceViewRHIParamRef SRVRHI)
{
  //VALIDATE_BOUND_SHADER(GeometryShaderRHI);

  RBD3D11ShaderResourceView* SRV = static_cast<RBD3D11ShaderResourceView*>(SRVRHI);

  RBD3D11BaseShaderResource* Resource = nullptr;
  ID3D11ShaderResourceView* D3D11SRV = nullptr;

  if (SRV)
  {
    Resource = SRV->Resource;
    D3D11SRV = SRV->View;
  }

  SetShaderResourceView<SF_Geometry>(Resource, D3D11SRV, TextureIndex, "");
}

void FD3D11DynamicRHI::RHISetShaderSampler(RBVertexShaderRHIParamRef VertexShaderRHI, uint32 SamplerIndex, RBSamplerStateRHIParamRef NewStateRHI)
{
  //VALIDATE_BOUND_SHADER(VertexShaderRHI);

  FD3D11VertexShader* VertexShader = static_cast<FD3D11VertexShader*>(VertexShaderRHI);
  RBD3D11SamplerState* NewState = static_cast<RBD3D11SamplerState*>(NewStateRHI);

  ID3D11SamplerState* StateResource = NewState->Resource;
  StateCache.SetSamplerState<SF_Vertex>(StateResource, SamplerIndex);
}

void FD3D11DynamicRHI::RHISetShaderSampler(RBHullShaderRHIParamRef HullShaderRHI, uint32 SamplerIndex, RBSamplerStateRHIParamRef NewStateRHI)
{
  //VALIDATE_BOUND_SHADER(HullShaderRHI);

  FD3D11HullShader* HullShader = static_cast<FD3D11HullShader*>(HullShaderRHI);
  RBD3D11SamplerState* NewState = static_cast<RBD3D11SamplerState*>(NewStateRHI);

  ID3D11SamplerState* StateResource = NewState->Resource;
  StateCache.SetSamplerState<SF_Hull>(StateResource, SamplerIndex);
}

void FD3D11DynamicRHI::RHISetShaderSampler(RBDomainShaderRHIParamRef DomainShaderRHI, uint32 SamplerIndex, RBSamplerStateRHIParamRef NewStateRHI)
{
  //VALIDATE_BOUND_SHADER(DomainShaderRHI);

  FD3D11DomainShader* DomainShader = static_cast<FD3D11DomainShader*>(DomainShaderRHI);
  RBD3D11SamplerState* NewState = static_cast<RBD3D11SamplerState*>(NewStateRHI);

  ID3D11SamplerState* StateResource = NewState->Resource;
  StateCache.SetSamplerState<SF_Domain>(StateResource, SamplerIndex);
}

void FD3D11DynamicRHI::RHISetShaderSampler(RBGeometryShaderRHIParamRef GeometryShaderRHI, uint32 SamplerIndex, RBSamplerStateRHIParamRef NewStateRHI)
{
  //VALIDATE_BOUND_SHADER(GeometryShaderRHI);

  FD3D11GeometryShader* GeometryShader = static_cast<FD3D11GeometryShader*>(GeometryShaderRHI);
  RBD3D11SamplerState* NewState = static_cast<RBD3D11SamplerState*>(NewStateRHI);

  ID3D11SamplerState* StateResource = NewState->Resource;
  StateCache.SetSamplerState<SF_Geometry>(StateResource, SamplerIndex);
}

void FD3D11DynamicRHI::RHISetShaderSampler(RBPixelShaderRHIParamRef PixelShaderRHI, uint32 SamplerIndex, RBSamplerStateRHIParamRef NewStateRHI)
{
  //VALIDATE_BOUND_SHADER(PixelShaderRHI);

  FD3D11PixelShader* PixelShader = static_cast<FD3D11PixelShader*>(PixelShaderRHI);
  RBD3D11SamplerState* NewState = static_cast<RBD3D11SamplerState*>(NewStateRHI);

  ID3D11SamplerState* StateResource = NewState->Resource;
  StateCache.SetSamplerState<SF_Pixel>(StateResource, SamplerIndex);
}

void FD3D11DynamicRHI::RHISetShaderSampler(RBComputeShaderRHIParamRef ComputeShaderRHI, uint32 SamplerIndex, RBSamplerStateRHIParamRef NewStateRHI)
{
  //VALIDATE_BOUND_SHADER(ComputeShaderRHI);
  FD3D11ComputeShader* ComputeShader = static_cast<FD3D11ComputeShader*>(ComputeShaderRHI);
  RBD3D11SamplerState* NewState = static_cast<RBD3D11SamplerState*>(NewStateRHI);

  ID3D11SamplerState* StateResource = NewState->Resource;
  StateCache.SetSamplerState<SF_Compute>(StateResource, SamplerIndex);
}

void FD3D11DynamicRHI::RHISetShaderUniformBuffer(RBVertexShaderRHIParamRef VertexShader, uint32 BufferIndex, RBUniformBufferRHIParamRef BufferRHI)
{
  //VALIDATE_BOUND_SHADER(VertexShader);
  RBD3D11UniformBuffer* Buffer = static_cast<RBD3D11UniformBuffer*>(BufferRHI);
  {
    ID3D11Buffer* ConstantBuffer = Buffer ? Buffer->Resource : NULL;
    StateCache.SetConstantBuffer<SF_Vertex>(ConstantBuffer, BufferIndex);
  }

  BoundUniformBuffers[SF_Vertex][BufferIndex] = BufferRHI;
  DirtyUniformBuffers[SF_Vertex] |= (1 << BufferIndex);
}

void FD3D11DynamicRHI::RHISetShaderUniformBuffer(RBHullShaderRHIParamRef HullShader, uint32 BufferIndex, RBUniformBufferRHIParamRef BufferRHI)
{
  //VALIDATE_BOUND_SHADER(HullShader);
  RBD3D11UniformBuffer* Buffer = static_cast<RBD3D11UniformBuffer*>(BufferRHI);
  {
    ID3D11Buffer* ConstantBuffer = Buffer ? Buffer->Resource : NULL;
    StateCache.SetConstantBuffer<SF_Hull>(ConstantBuffer, BufferIndex);
  }

  BoundUniformBuffers[SF_Hull][BufferIndex] = BufferRHI;
  DirtyUniformBuffers[SF_Hull] |= (1 << BufferIndex);
}

void FD3D11DynamicRHI::RHISetShaderUniformBuffer(RBDomainShaderRHIParamRef DomainShader, uint32 BufferIndex, RBUniformBufferRHIParamRef BufferRHI)
{
  //VALIDATE_BOUND_SHADER(DomainShader);
  RBD3D11UniformBuffer* Buffer = static_cast<RBD3D11UniformBuffer*>(BufferRHI);
  {
    ID3D11Buffer* ConstantBuffer = Buffer ? Buffer->Resource : NULL;
    StateCache.SetConstantBuffer<SF_Domain>(ConstantBuffer, BufferIndex);
  }

  BoundUniformBuffers[SF_Domain][BufferIndex] = BufferRHI;
  DirtyUniformBuffers[SF_Domain] |= (1 << BufferIndex);
}

void FD3D11DynamicRHI::RHISetShaderUniformBuffer(RBGeometryShaderRHIParamRef GeometryShader, uint32 BufferIndex, RBUniformBufferRHIParamRef BufferRHI)
{
  //VALIDATE_BOUND_SHADER(GeometryShader);
  RBD3D11UniformBuffer* Buffer = static_cast<RBD3D11UniformBuffer*>(BufferRHI);
  {
    ID3D11Buffer* ConstantBuffer = Buffer ? Buffer->Resource : NULL;
    StateCache.SetConstantBuffer<SF_Geometry>(ConstantBuffer, BufferIndex);
  }

  BoundUniformBuffers[SF_Geometry][BufferIndex] = BufferRHI;
  DirtyUniformBuffers[SF_Geometry] |= (1 << BufferIndex);
}

void FD3D11DynamicRHI::RHISetShaderUniformBuffer(RBPixelShaderRHIParamRef PixelShader, uint32 BufferIndex, RBUniformBufferRHIParamRef BufferRHI)
{
  //VALIDATE_BOUND_SHADER(PixelShader);
  RBD3D11UniformBuffer* Buffer = static_cast<RBD3D11UniformBuffer*>(BufferRHI);

  {
    ID3D11Buffer* ConstantBuffer = Buffer ? Buffer->Resource : NULL;
    StateCache.SetConstantBuffer<SF_Pixel>(ConstantBuffer, BufferIndex);
  }

  BoundUniformBuffers[SF_Pixel][BufferIndex] = BufferRHI;
  DirtyUniformBuffers[SF_Pixel] |= (1 << BufferIndex);
}

void FD3D11DynamicRHI::RHISetShaderUniformBuffer(RBComputeShaderRHIParamRef ComputeShader, uint32 BufferIndex, RBUniformBufferRHIParamRef BufferRHI)
{
  //VALIDATE_BOUND_SHADER(ComputeShader);
  RBD3D11UniformBuffer* Buffer = static_cast<RBD3D11UniformBuffer*>(BufferRHI);
  {
    ID3D11Buffer* ConstantBuffer = Buffer ? Buffer->Resource : NULL;
    StateCache.SetConstantBuffer<SF_Compute>(ConstantBuffer, BufferIndex);
  }

  BoundUniformBuffers[SF_Compute][BufferIndex] = BufferRHI;
  DirtyUniformBuffers[SF_Compute] |= (1 << BufferIndex);
}

void FD3D11DynamicRHI::RHISetShaderParameter(RBHullShaderRHIParamRef HullShaderRHI, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue)
{
  //VALIDATE_BOUND_SHADER(HullShaderRHI);
  CHECK(HSConstantBuffers[BufferIndex]);
  HSConstantBuffers[BufferIndex]->UpdateConstant((const uint8*)NewValue, BaseIndex, NumBytes);
}

void FD3D11DynamicRHI::RHISetShaderParameter(RBDomainShaderRHIParamRef DomainShaderRHI, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue)
{
  //VALIDATE_BOUND_SHADER(DomainShaderRHI);
  CHECK(DSConstantBuffers[BufferIndex]);
  DSConstantBuffers[BufferIndex]->UpdateConstant((const uint8*)NewValue, BaseIndex, NumBytes);
}

void FD3D11DynamicRHI::RHISetShaderParameter(RBVertexShaderRHIParamRef VertexShaderRHI, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue)
{
  //VALIDATE_BOUND_SHADER(VertexShaderRHI);
  CHECK(VSConstantBuffers[BufferIndex]);
  VSConstantBuffers[BufferIndex]->UpdateConstant((const uint8*)NewValue, BaseIndex, NumBytes);
}

void FD3D11DynamicRHI::RHISetShaderParameter(RBPixelShaderRHIParamRef PixelShaderRHI, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue)
{
  //VALIDATE_BOUND_SHADER(PixelShaderRHI);
  CHECK(PSConstantBuffers[BufferIndex]);
  PSConstantBuffers[BufferIndex]->UpdateConstant((const uint8*)NewValue, BaseIndex, NumBytes);
}

void FD3D11DynamicRHI::RHISetShaderParameter(RBGeometryShaderRHIParamRef GeometryShaderRHI, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue)
{
  //VALIDATE_BOUND_SHADER(GeometryShaderRHI);
  CHECK(GSConstantBuffers[BufferIndex]);
  GSConstantBuffers[BufferIndex]->UpdateConstant((const uint8*)NewValue, BaseIndex, NumBytes);
}

void FD3D11DynamicRHI::RHISetShaderParameter(RBComputeShaderRHIParamRef ComputeShaderRHI, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue)
{
  //VALIDATE_BOUND_SHADER(ComputeShaderRHI);
  CHECK(CSConstantBuffers[BufferIndex]);
  CSConstantBuffers[BufferIndex]->UpdateConstant((const uint8*)NewValue, BaseIndex, NumBytes);
}


template <EShaderFrequency ShaderFrequencyT>
static FORCEINLINE void CommitConstants(FD3D11ConstantBuffer* InConstantBuffer, RBD3D11StateCache& StateCache, uint32 Index, bool bDiscardSharedConstants)
{
  auto* ConstantBuffer = ((FWinD3D11ConstantBuffer*)InConstantBuffer);
  // Array may contain NULL entries to pad out to proper 
  if (ConstantBuffer && ConstantBuffer->CommitConstantsToDevice(bDiscardSharedConstants))
  {
    ID3D11Buffer* DeviceBuffer = ConstantBuffer->GetConstantBuffer();
    StateCache.SetConstantBuffer<ShaderFrequencyT>(DeviceBuffer, Index);
  }
}

void FD3D11DynamicRHI::CommitNonComputeShaderConstants()
{
  
  FD3D11BoundShaderState* CurrentBoundShaderState = (FD3D11BoundShaderState*)CurrentBoundedShaderState;//(FD3D11BoundShaderState*)BoundShaderStateHistory.GetLast();
  CHECK(CurrentBoundShaderState);

  // Only set the constant buffer if this shader needs the global constant buffer bound
  // Otherwise we will overwrite a different constant buffer
  if (CurrentBoundShaderState->bShaderNeedsGlobalConstantBuffer[SF_Vertex])
  {
    // Commit and bind vertex shader constants
    for (uint32 i = 0; i < MAX_CONSTANT_BUFFER_SLOTS; i++)
    {
      FD3D11ConstantBuffer* ConstantBuffer = VSConstantBuffers[i];
      CommitConstants<SF_Vertex>(ConstantBuffer, StateCache, i, bDiscardSharedConstants);
    }
  }

  // Skip HS/DS CB updates in cases where tessellation isn't being used
  // Note that this is *potentially* unsafe because bDiscardSharedConstants is cleared at the
  // end of the function, however we're OK for now because bDiscardSharedConstants
  // is always reset whenever bUsingTessellation changes in SetBoundShaderState()
  if (bUsingTessellation)
  {
    if (CurrentBoundShaderState->bShaderNeedsGlobalConstantBuffer[SF_Hull])
    {
      // Commit and bind hull shader constants
      for (uint32 i = 0; i < MAX_CONSTANT_BUFFER_SLOTS; i++)
      {
        FD3D11ConstantBuffer* ConstantBuffer = HSConstantBuffers[i];
        CommitConstants<SF_Hull>(ConstantBuffer, StateCache, i, bDiscardSharedConstants);
      }
    }

    if (CurrentBoundShaderState->bShaderNeedsGlobalConstantBuffer[SF_Domain])
    {
      // Commit and bind domain shader constants
      for (uint32 i = 0; i < MAX_CONSTANT_BUFFER_SLOTS; i++)
      {
        FD3D11ConstantBuffer* ConstantBuffer = DSConstantBuffers[i];
        CommitConstants<SF_Domain>(ConstantBuffer, StateCache, i, bDiscardSharedConstants);
      }
    }
  }

  if (CurrentBoundShaderState->bShaderNeedsGlobalConstantBuffer[SF_Geometry])
  {
    // Commit and bind geometry shader constants
    for (uint32 i = 0; i < MAX_CONSTANT_BUFFER_SLOTS; i++)
    {
      FD3D11ConstantBuffer* ConstantBuffer = GSConstantBuffers[i];
      CommitConstants<SF_Geometry>(ConstantBuffer, StateCache, i, bDiscardSharedConstants);
    }
  }

  if (CurrentBoundShaderState->bShaderNeedsGlobalConstantBuffer[SF_Pixel])
  {
    // Commit and bind pixel shader constants
    for (uint32 i = 0; i < MAX_CONSTANT_BUFFER_SLOTS; i++)
    {
      FD3D11ConstantBuffer* ConstantBuffer = PSConstantBuffers[i];
      CommitConstants<SF_Pixel>(ConstantBuffer, StateCache, i, bDiscardSharedConstants);
    }
  }

  bDiscardSharedConstants = false;
}

void FD3D11DynamicRHI::CommitComputeShaderConstants()
{
  bool bLocalDiscardSharedConstants = true;

  // Commit and bind compute shader constants
  for (uint32 i = 0; i < MAX_CONSTANT_BUFFER_SLOTS; i++)
  {
    FD3D11ConstantBuffer* ConstantBuffer = CSConstantBuffers[i];
    CommitConstants<SF_Compute>(ConstantBuffer, StateCache, i, bDiscardSharedConstants);
  }
}

/**
* Set bound shader state. This will set the vertex decl/shader, and pixel shader
* @param BoundShaderState - state resource
*/
void FD3D11DynamicRHI::RHISetBoundShaderState(RBBoundShaderStateRHIParamRef BoundShaderStateRHI)
{
  FD3D11BoundShaderState* BoundShaderState = static_cast< FD3D11BoundShaderState*>(BoundShaderStateRHI);

  StateCache.SetInputLayout(BoundShaderState->InputLayout);
  StateCache.SetVertexShader(BoundShaderState->VertexShader);
  StateCache.SetPixelShader(BoundShaderState->PixelShader);

  StateCache.SetHullShader(BoundShaderState->HullShader);
  StateCache.SetDomainShader(BoundShaderState->DomainShader);
  StateCache.SetGeometryShader(BoundShaderState->GeometryShader);

  if (BoundShaderState->HullShader != NULL && BoundShaderState->DomainShader != NULL)
  {
    bUsingTessellation = true;
  }
  else
  {
    bUsingTessellation = false;
  }

  // @TODO : really should only discard the constants if the shader state has actually changed.
  bDiscardSharedConstants = true;

  // Prevent transient bound shader states from being recreated for each use by keeping a history of the most recently used bound shader states.
  // The history keeps them alive, and the bound shader state cache allows them to be reused if needed.
  //BoundShaderStateHistory.Add(BoundShaderState);
  CurrentBoundedShaderState = BoundShaderState;

  // Shader changed so all resource tables are dirty
  DirtyUniformBuffers[SF_Vertex] = 0xffff;
  DirtyUniformBuffers[SF_Pixel] = 0xffff;
  DirtyUniformBuffers[SF_Hull] = 0xffff;
  DirtyUniformBuffers[SF_Domain] = 0xffff;
  DirtyUniformBuffers[SF_Geometry] = 0xffff;

  // Shader changed.  All UB's must be reset by high level code to match other platforms anway.
  // Clear to catch those bugs, and bugs with stale UB's causing layout mismatches.
  // Release references to bound uniform buffers.
  for (int32 Frequency = 0; Frequency < SF_NumFrequencies; ++Frequency)
  {
    for (int32 BindIndex = 0; BindIndex < MAX_UNIFORM_BUFFERS_PER_SHADER_STAGE; ++BindIndex)
    {
      BoundUniformBuffers[Frequency][BindIndex].SafeRelease();
    }
  }
}

void FD3D11DynamicRHI::RHISetShaderTexture(RBVertexShaderRHIParamRef VertexShaderRHI, uint32 TextureIndex, RBTextureRHIParamRef NewTextureRHI)
{
  //VALIDATE_BOUND_SHADER(VertexShaderRHI);

  RBD3D11TextureBase* NewTexture = GetD3D11TextureFromRHITexture(NewTextureRHI);
  ID3D11ShaderResourceView* ShaderResourceView = NewTexture ? NewTexture->GetShaderResourceView() : NULL;

  if ((NewTexture == NULL) || (NewTexture->GetRenderTargetView(0, 0) != NULL) || (NewTexture->HasDepthStencilView()))
  {
    SetShaderResourceView<SF_Vertex>(NewTexture, ShaderResourceView, TextureIndex, NewTextureRHI ? NewTextureRHI->GetName().c_str() : "", RBD3D11StateCache::SRV_Dynamic);
  }
  else
  {
    SetShaderResourceView<SF_Vertex>(NewTexture, ShaderResourceView, TextureIndex, NewTextureRHI->GetName().c_str(), RBD3D11StateCache::SRV_Static);
  }
}

void FD3D11DynamicRHI::RHISetShaderTexture(RBHullShaderRHIParamRef HullShaderRHI, uint32 TextureIndex, RBTextureRHIParamRef NewTextureRHI)
{
  //VALIDATE_BOUND_SHADER(HullShaderRHI);

  RBD3D11TextureBase* NewTexture = GetD3D11TextureFromRHITexture(NewTextureRHI);
  ID3D11ShaderResourceView* ShaderResourceView = NewTexture ? NewTexture->GetShaderResourceView() : NULL;

  if ((NewTexture == NULL) || (NewTexture->GetRenderTargetView(0, 0) != NULL) || (NewTexture->HasDepthStencilView()))
  {
    SetShaderResourceView<SF_Hull>(NewTexture, ShaderResourceView, TextureIndex, NewTextureRHI ? NewTextureRHI->GetName().c_str() : "", RBD3D11StateCache::SRV_Dynamic);
  }
  else
  {
    SetShaderResourceView<SF_Hull>(NewTexture, ShaderResourceView, TextureIndex, NewTextureRHI->GetName().c_str(), RBD3D11StateCache::SRV_Static);
  }
}

void FD3D11DynamicRHI::RHISetShaderTexture(RBDomainShaderRHIParamRef DomainShaderRHI, uint32 TextureIndex, RBTextureRHIParamRef NewTextureRHI)
{
  //VALIDATE_BOUND_SHADER(DomainShaderRHI);

  RBD3D11TextureBase* NewTexture = GetD3D11TextureFromRHITexture(NewTextureRHI);
  ID3D11ShaderResourceView* ShaderResourceView = NewTexture ? NewTexture->GetShaderResourceView() : NULL;

  if ((NewTexture == NULL) || (NewTexture->GetRenderTargetView(0, 0) != NULL) || (NewTexture->HasDepthStencilView()))
  {
    SetShaderResourceView<SF_Domain>(NewTexture, ShaderResourceView, TextureIndex, NewTextureRHI ? NewTextureRHI->GetName().c_str() : "", RBD3D11StateCache::SRV_Dynamic);
  }
  else
  {
    SetShaderResourceView<SF_Domain>(NewTexture, ShaderResourceView, TextureIndex, NewTextureRHI->GetName().c_str(), RBD3D11StateCache::SRV_Static);
  }
}

void FD3D11DynamicRHI::RHISetShaderTexture(RBGeometryShaderRHIParamRef GeometryShaderRHI, uint32 TextureIndex, RBTextureRHIParamRef NewTextureRHI)
{
  //VALIDATE_BOUND_SHADER(GeometryShaderRHI);

  RBD3D11TextureBase* NewTexture = GetD3D11TextureFromRHITexture(NewTextureRHI);
  ID3D11ShaderResourceView* ShaderResourceView = NewTexture ? NewTexture->GetShaderResourceView() : NULL;

  if ((NewTexture == NULL) || (NewTexture->GetRenderTargetView(0, 0) != NULL) || (NewTexture->HasDepthStencilView()))
  {
    SetShaderResourceView<SF_Geometry>(NewTexture, ShaderResourceView, TextureIndex, NewTextureRHI ? NewTextureRHI->GetName().c_str() : "", RBD3D11StateCache::SRV_Dynamic);
  }
  else
  {
    SetShaderResourceView<SF_Geometry>(NewTexture, ShaderResourceView, TextureIndex, NewTextureRHI->GetName().c_str(), RBD3D11StateCache::SRV_Static);
  }
}

void FD3D11DynamicRHI::RHISetShaderTexture(RBPixelShaderRHIParamRef PixelShaderRHI, uint32 TextureIndex, RBTextureRHIParamRef NewTextureRHI)
{

  //VALIDATE_BOUND_SHADER(PixelShaderRHI);

  RBD3D11TextureBase* NewTexture = GetD3D11TextureFromRHITexture(NewTextureRHI);
  ID3D11ShaderResourceView* ShaderResourceView = NewTexture ? NewTexture->GetShaderResourceView() : NULL;
  if ((NewTexture == NULL) || (NewTexture->GetRenderTargetView(0, 0) != NULL) || (NewTexture->HasDepthStencilView()))
  {
    SetShaderResourceView<SF_Pixel>(NewTexture, ShaderResourceView, TextureIndex, NewTextureRHI ? NewTextureRHI->GetName().c_str() : "", RBD3D11StateCache::SRV_Dynamic);
  }
  else
  {
    SetShaderResourceView<SF_Pixel>(NewTexture, ShaderResourceView, TextureIndex, NewTextureRHI->GetName().c_str(), RBD3D11StateCache::SRV_Static);
  }
}

void FD3D11DynamicRHI::RHISetShaderTexture(RBComputeShaderRHIParamRef ComputeShaderRHI, uint32 TextureIndex, RBTextureRHIParamRef NewTextureRHI)
{
  //VALIDATE_BOUND_SHADER(ComputeShaderRHI);

  RBD3D11TextureBase* NewTexture = GetD3D11TextureFromRHITexture(NewTextureRHI);
  ID3D11ShaderResourceView* ShaderResourceView = NewTexture ? NewTexture->GetShaderResourceView() : NULL;

  if ((NewTexture == NULL) || (NewTexture->GetRenderTargetView(0, 0) != NULL) || (NewTexture->HasDepthStencilView()))
  {
    SetShaderResourceView<SF_Compute>(NewTexture, ShaderResourceView, TextureIndex, NewTextureRHI ? NewTextureRHI->GetName().c_str() : "", RBD3D11StateCache::SRV_Dynamic);
  }
  else
  {
    SetShaderResourceView<SF_Compute>(NewTexture, ShaderResourceView, TextureIndex, NewTextureRHI->GetName().c_str(), RBD3D11StateCache::SRV_Static);
  }
}

void FD3D11DynamicRHI::RHISetRenderTargets(
  uint32 NewNumSimultaneousRenderTargets,
  const RBRHIRenderTargetView* NewRenderTargetsRHI,
  const RBRHIDepthRenderTargetView* NewDepthStencilTargetRHI,
  uint32 NewNumUAVs,
  const RBUnorderedAccessViewRHIParamRef* UAVs
  )
{
  RBD3D11TextureBase* NewDepthStencilTarget = GetD3D11TextureFromRHITexture(NewDepthStencilTargetRHI ? NewDepthStencilTargetRHI->Texture : nullptr);

#if CHECK_SRV_TRANSITIONS
  // if the depth buffer is writable then it counts as unresolved.
  if (NewDepthStencilTargetRHI && NewDepthStencilTargetRHI->GetDepthStencilAccess() == FExclusiveDepthStencil::DepthWrite_StencilWrite && NewDepthStencilTarget)
  {
    check(UnresolvedTargetsConcurrencyGuard.Increment() == 1);
    UnresolvedTargets.Add(NewDepthStencilTarget->GetResource(), FUnresolvedRTInfo(NewDepthStencilTargetRHI->Texture->GetName(), 0, 1, -1, 1));
    check(UnresolvedTargetsConcurrencyGuard.Decrement() == 0);
  }
#endif

  CHECK(NewNumSimultaneousRenderTargets + NewNumUAVs <= MaxSimultaneousRenderTargets);

  bool bTargetChanged = false;

  // Set the appropriate depth stencil view depending on whether depth writes are enabled or not
  ID3D11DepthStencilView* DepthStencilView = NULL;
  if (NewDepthStencilTarget)
  {
    CurrentDSVAccessType = NewDepthStencilTargetRHI->GetDepthStencilAccess();
    DepthStencilView = NewDepthStencilTarget->GetDepthStencilView(CurrentDSVAccessType);

    // Unbind any shader views of the depth stencil target that are bound.
    ConditionalClearShaderResource(NewDepthStencilTarget);
  }

  // Check if the depth stencil target is different from the old state.
  if (CurrentDepthStencilTarget != DepthStencilView)
  {
    CurrentDepthTexture = NewDepthStencilTarget;
    CurrentDepthStencilTarget = DepthStencilView;
    bTargetChanged = true;
  }

  if (NewDepthStencilTarget)
  {
    uint32 CurrentFrame = PresentCounter;
    const EResourceTransitionAccess CurrentAccess = NewDepthStencilTarget->GetCurrentGPUAccess();
    const uint32 LastFrameWritten = NewDepthStencilTarget->GetLastFrameWritten();
    const bool bReadable = CurrentAccess == EResourceTransitionAccess::EReadable;
    const bool bDepthWrite = NewDepthStencilTargetRHI->GetDepthStencilAccess().IsDepthWrite();
    const bool bAccessValid = !bReadable ||
      LastFrameWritten != CurrentFrame ||
      !bDepthWrite;

    //CHECKF((GEnableDX11TransitionChecks == 0) || bAccessValid, TEXT("DepthTarget '%s' is not GPU writable."), *NewDepthStencilTargetRHI->Texture->GetName().ToString());

    //switch to writable state if this is the first render of the frame.  Don't switch if it's a later render and this is a depth test only situation
    if (!bAccessValid || (bReadable && bDepthWrite))
    {
      //DUMP_TRANSITION(NewDepthStencilTargetRHI->Texture->GetName(), EResourceTransitionAccess::EWritable);
      NewDepthStencilTarget->SetCurrentGPUAccess(EResourceTransitionAccess::EWritable);
    }

    if (bDepthWrite)
    {
      NewDepthStencilTarget->SetDirty(true, CurrentFrame);
    }
  }

  // Gather the render target views for the new render targets.
  ID3D11RenderTargetView* NewRenderTargetViews[MaxSimultaneousRenderTargets];
  for (uint32 RenderTargetIndex = 0; RenderTargetIndex < MaxSimultaneousRenderTargets; ++RenderTargetIndex)
  {
    ID3D11RenderTargetView* RenderTargetView = NULL;
    if (RenderTargetIndex < NewNumSimultaneousRenderTargets && NewRenderTargetsRHI[RenderTargetIndex].Texture != nullptr)
    {
      int32 RTMipIndex = NewRenderTargetsRHI[RenderTargetIndex].MipIndex;
      int32 RTSliceIndex = NewRenderTargetsRHI[RenderTargetIndex].ArraySliceIndex;
      RBD3D11TextureBase* NewRenderTarget = GetD3D11TextureFromRHITexture(NewRenderTargetsRHI[RenderTargetIndex].Texture);
      RenderTargetView = NewRenderTarget->GetRenderTargetView(RTMipIndex, RTSliceIndex);

      if (NewRenderTarget)
      {
        uint32 CurrentFrame = PresentCounter;
        const EResourceTransitionAccess CurrentAccess = NewRenderTarget->GetCurrentGPUAccess();
        const uint32 LastFrameWritten = NewRenderTarget->GetLastFrameWritten();
        const bool bReadable = CurrentAccess == EResourceTransitionAccess::EReadable;
        const bool bAccessValid = !bReadable || LastFrameWritten != CurrentFrame;
        //ensureMsgf((GEnableDX11TransitionChecks == 0) || bAccessValid, TEXT("RenderTarget '%s' is not GPU writable."), *NewRenderTargetsRHI[RenderTargetIndex].Texture->GetName().ToString());

        if (!bAccessValid || bReadable)
        {
          //DUMP_TRANSITION(NewRenderTargetsRHI[RenderTargetIndex].Texture->GetName(), EResourceTransitionAccess::EWritable);
          NewRenderTarget->SetCurrentGPUAccess(EResourceTransitionAccess::EWritable);
        }
        NewRenderTarget->SetDirty(true, CurrentFrame);
      }

      CHECKF(RenderTargetView, TEXT("Texture being set as render target has no RTV"));
#if CHECK_SRV_TRANSITIONS			
      if (RenderTargetView)
      {
        // remember this target as having been bound for write.
        ID3D11Resource* RTVResource;
        RenderTargetView->GetResource(&RTVResource);
        check(UnresolvedTargetsConcurrencyGuard.Increment() == 1);
        UnresolvedTargets.Add(RTVResource, FUnresolvedRTInfo(NewRenderTargetsRHI[RenderTargetIndex].Texture->GetName(), RTMipIndex, 1, RTSliceIndex, 1));
        check(UnresolvedTargetsConcurrencyGuard.Decrement() == 0);
        RTVResource->Release();
      }
#endif

      // Unbind any shader views of the render target that are bound.
      ConditionalClearShaderResource(NewRenderTarget);

#if 1	
      // A check to allow you to pinpoint what is using mismatching targets
      // We filter our d3ddebug spew that checks for this as the d3d runtime's check is wrong.
      // For filter code, see D3D11Device.cpp look for "OMSETRENDERTARGETS_INVALIDVIEW"
      if (RenderTargetView && DepthStencilView)
      {
        FRTVDesc RTTDesc = GetRenderTargetViewDesc(RenderTargetView);

        TRefCountPtr<ID3D11Texture2D> DepthTargetTexture;
        DepthStencilView->GetResource((ID3D11Resource**)DepthTargetTexture.GetInitReference());

        D3D11_TEXTURE2D_DESC DTTDesc;
        DepthTargetTexture->GetDesc(&DTTDesc);

        // enforce color target is <= depth and MSAA settings match
        if (RTTDesc.Width > DTTDesc.Width || RTTDesc.Height > DTTDesc.Height ||
          RTTDesc.SampleDesc.Count != DTTDesc.SampleDesc.Count ||
          RTTDesc.SampleDesc.Quality != DTTDesc.SampleDesc.Quality)
        {
          g_logger->debug(WIP_ERROR, TEXT("RTV(%i,%i c=%i,q=%i) and DSV(%i,%i c=%i,q=%i) have mismatching dimensions and/or MSAA levels!"),
            RTTDesc.Width, RTTDesc.Height, RTTDesc.SampleDesc.Count, RTTDesc.SampleDesc.Quality,
            DTTDesc.Width, DTTDesc.Height, DTTDesc.SampleDesc.Count, DTTDesc.SampleDesc.Quality);
        }
      }
#endif
    }

    NewRenderTargetViews[RenderTargetIndex] = RenderTargetView;

    // Check if the render target is different from the old state.
    if (CurrentRenderTargets[RenderTargetIndex] != RenderTargetView)
    {
      CurrentRenderTargets[RenderTargetIndex] = RenderTargetView;
      bTargetChanged = true;
    }
  }
  if (NumSimultaneousRenderTargets != NewNumSimultaneousRenderTargets)
  {
    NumSimultaneousRenderTargets = NewNumSimultaneousRenderTargets;
    bTargetChanged = true;
  }

  // Gather the new UAVs.
  for (uint32 UAVIndex = 0; UAVIndex < MaxSimultaneousUAVs; ++UAVIndex)
  {
    ID3D11UnorderedAccessView* UAV = NULL;
    if (UAVIndex < NewNumUAVs && UAVs[UAVIndex] != NULL)
    {
      RBD3D11UnorderedAccessView* RHIUAV = (RBD3D11UnorderedAccessView*)UAVs[UAVIndex];
      UAV = RHIUAV->View;

      if (UAV)
      {
        //check it's safe for r/w for this UAV
        const EResourceTransitionAccess CurrentUAVAccess = RHIUAV->Resource->GetCurrentGPUAccess();
        const bool UAVDirty = RHIUAV->Resource->IsDirty();
        const bool bAccessPass = (CurrentUAVAccess == EResourceTransitionAccess::ERWBarrier && !UAVDirty) || (CurrentUAVAccess == EResourceTransitionAccess::ERWNoBarrier);
        //ensureMsgf((GEnableDX11TransitionChecks == 0) || bAccessPass, TEXT("UAV: %i is in unsafe state for GPU R/W: %s"), UAVIndex, *FResourceTransitionUtility::ResourceTransitionAccessStrings[(int32)CurrentUAVAccess]);

        //UAVs get set to dirty.  If the shader just wanted to read it should have used an SRV.
        RHIUAV->Resource->SetDirty(true, PresentCounter);
      }

      // Unbind any shader views of the UAV's resource.
      ConditionalClearShaderResource(RHIUAV->Resource);
    }

    if (CurrentUAVs[UAVIndex] != UAV)
    {
      CurrentUAVs[UAVIndex] = UAV;
      bTargetChanged = true;
    }
  }
  if (NumUAVs != NewNumUAVs)
  {
    NumUAVs = NewNumUAVs;
    bTargetChanged = true;
  }

  // Only make the D3D call to change render targets if something actually changed.
  if (bTargetChanged)
  {
    CommitRenderTargetsAndUAVs();
  }

  // Set the viewport to the full size of render target 0.
  if (NewRenderTargetViews[0])
  {
    // check target 0 is valid
    CHECK(0 < NewNumSimultaneousRenderTargets && NewRenderTargetsRHI[0].Texture != nullptr);
    FRTVDesc RTTDesc = GetRenderTargetViewDesc(NewRenderTargetViews[0]);
    RHISetViewport(0, 0, 0.0f, RTTDesc.Width, RTTDesc.Height, 1.0f);
  }
  else if (DepthStencilView)
  {
    TRefCountPtr<ID3D11Texture2D> DepthTargetTexture;
    DepthStencilView->GetResource((ID3D11Resource**)DepthTargetTexture.GetInitReference());

    D3D11_TEXTURE2D_DESC DTTDesc;
    DepthTargetTexture->GetDesc(&DTTDesc);
    RHISetViewport(0, 0, 0.0f, DTTDesc.Width, DTTDesc.Height, 1.0f);
  }
}

/*
void FD3D11DynamicRHI::RHIDiscardRenderTargets(bool Depth, bool Stencil, uint32 ColorBitMask)
{
  // Could support in DX11.1 via ID3D11DeviceContext1::Discard*() functions.
}
*/

void FD3D11DynamicRHI::RHISetRenderTargetsAndClear(const RBRHISetRenderTargetsInfo& RenderTargetsInfo)
{
  // Here convert to FUnorderedAccessViewRHIParamRef* in order to call RHISetRenderTargets
  RBUnorderedAccessViewRHIParamRef UAVs[MaxSimultaneousUAVs] = {};
  for (int32 UAVIndex = 0; UAVIndex < RenderTargetsInfo.NumUAVs; ++UAVIndex)
  {
    UAVs[UAVIndex] = RenderTargetsInfo.UnorderedAccessView[UAVIndex].GetReference();
  }

  this->RHISetRenderTargets(RenderTargetsInfo.NumColorRenderTargets,
    RenderTargetsInfo.ColorRenderTarget,
    &RenderTargetsInfo.DepthStencilRenderTarget,
    RenderTargetsInfo.NumUAVs,
    UAVs);

  if (RenderTargetsInfo.bClearColor || RenderTargetsInfo.bClearStencil || RenderTargetsInfo.bClearDepth)
  {
    RBColorf ClearColors[MaxSimultaneousRenderTargets];
    float DepthClear = 0.0;
    uint32 StencilClear = 0;

    if (RenderTargetsInfo.bClearColor)
    {
      for (int32 i = 0; i < RenderTargetsInfo.NumColorRenderTargets; ++i)
      {
        if (RenderTargetsInfo.ColorRenderTarget[i].Texture != nullptr)
        {
          const RBClearValueBinding& ClearValue = RenderTargetsInfo.ColorRenderTarget[i].Texture->GetClearBinding();
          CHECKF(ClearValue.ColorBinding == EClearBinding::EColorBound, TEXT("Texture: %s does not have a color bound for fast clears"), *RenderTargetsInfo.ColorRenderTarget[i].Texture->GetName().c_str());
          ClearColors[i] = ClearValue.GetClearColor();
        }
      }
    }
    if (RenderTargetsInfo.bClearDepth || RenderTargetsInfo.bClearStencil)
    {
      const RBClearValueBinding& ClearValue = RenderTargetsInfo.DepthStencilRenderTarget.Texture->GetClearBinding();
      CHECKF(ClearValue.ColorBinding == EClearBinding::EDepthStencilBound, TEXT("Texture: %s does not have a DS value bound for fast clears"), *RenderTargetsInfo.DepthStencilRenderTarget.Texture->GetName().c_str());
      ClearValue.GetDepthStencil(DepthClear, StencilClear);
    }

    this->RHIClearMRTImpl(RenderTargetsInfo.bClearColor, RenderTargetsInfo.NumColorRenderTargets, ClearColors, RenderTargetsInfo.bClearDepth, DepthClear, RenderTargetsInfo.bClearStencil, StencilClear, RBAABBI(), false, EForceFullScreenClear::EForce);
  }
}

void FD3D11DynamicRHI::CommitRenderTargetsAndUAVs()
{
  ID3D11RenderTargetView* RTArray[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
  for (uint32 RenderTargetIndex = 0; RenderTargetIndex < NumSimultaneousRenderTargets; ++RenderTargetIndex)
  {
    RTArray[RenderTargetIndex] = CurrentRenderTargets[RenderTargetIndex];
  }
  ID3D11UnorderedAccessView* UAVArray[D3D11_PS_CS_UAV_REGISTER_COUNT];
  uint32 UAVInitialCountArray[D3D11_PS_CS_UAV_REGISTER_COUNT];
  for (uint32 UAVIndex = 0; UAVIndex < NumUAVs; ++UAVIndex)
  {
    UAVArray[UAVIndex] = CurrentUAVs[UAVIndex];
    // Using the value that indicates to keep the current UAV counter
    UAVInitialCountArray[UAVIndex] = -1;
  }

  if (NumUAVs > 0)
  {
    Direct3DDeviceIMContext->OMSetRenderTargetsAndUnorderedAccessViews(
      NumSimultaneousRenderTargets,
      RTArray,
      CurrentDepthStencilTarget,
      NumSimultaneousRenderTargets,
      NumUAVs,
      UAVArray,
      UAVInitialCountArray
      );
  }
  else
  {
    // Use OMSetRenderTargets if there are no UAVs, works around a crash in PIX
    Direct3DDeviceIMContext->OMSetRenderTargets(
      NumSimultaneousRenderTargets,
      RTArray,
      CurrentDepthStencilTarget
      );
  }
}



static int32 PeriodicCheck = 0;

void FD3D11DynamicRHI::CommitGraphicsResourceTables()
{
  FD3D11BoundShaderState*  CurrentBoundShaderState = (FD3D11BoundShaderState*)CurrentBoundedShaderState;// (FD3D11BoundShaderState*)BoundShaderStateHistory.GetLast();
  CHECK(CurrentBoundShaderState);

  if (auto* Shader = CurrentBoundShaderState->GetVertexShader())
  {
    SetResourcesFromTables(Shader);
  }
  if (auto* Shader = CurrentBoundShaderState->GetPixelShader())
  {
    SetResourcesFromTables(Shader);
  }
  if (auto* Shader = CurrentBoundShaderState->GetHullShader())
  {
    SetResourcesFromTables(Shader);
  }
  if (auto* Shader = CurrentBoundShaderState->GetDomainShader())
  {
    SetResourcesFromTables(Shader);
  }
  if (auto* Shader = CurrentBoundShaderState->GetGeometryShader())
  {
    SetResourcesFromTables(Shader);
  }
}

void FD3D11DynamicRHI::CommitComputeResourceTables(FD3D11ComputeShader* InComputeShader)
{
  FD3D11ComputeShader*  ComputeShader = InComputeShader;
  CHECK(ComputeShader);
  SetResourcesFromTables(ComputeShader);
}


void FD3D11DynamicRHI::RHIDrawPrimitive(uint32 PrimitiveType, uint32 BaseVertexIndex, uint32 NumPrimitives, uint32 NumInstances)
{
  //RHI_DRAW_CALL_STATS(PrimitiveType, NumInstances*NumPrimitives);

  CommitGraphicsResourceTables();
  CommitNonComputeShaderConstants();

  uint32 VertexCount = GetVertexCountForPrimitiveCount(NumPrimitives, PrimitiveType);

  //GPUProfilingData.RegisterGPUWork(NumPrimitives * NumInstances, VertexCount * NumInstances);

  StateCache.SetPrimitiveTopology(GetD3D11PrimitiveType(PrimitiveType, bUsingTessellation));
  if (NumInstances > 1)
  {
    Direct3DDeviceIMContext->DrawInstanced(VertexCount, NumInstances, BaseVertexIndex, 0);
  }
  else
  {
    Direct3DDeviceIMContext->Draw(VertexCount, BaseVertexIndex);
  }
}

void FD3D11DynamicRHI::RHIDrawPrimitiveIndirect(uint32 PrimitiveType, RBVertexBufferRHIParamRef ArgumentBufferRHI, uint32 ArgumentOffset)
{
  RBD3D11VertexBuffer* ArgumentBuffer = static_cast<RBD3D11VertexBuffer*>(ArgumentBufferRHI);

  //RHI_DRAW_CALL_INC();

  //GPUProfilingData.RegisterGPUWork(0);

  CommitGraphicsResourceTables();
  CommitNonComputeShaderConstants();

  StateCache.SetPrimitiveTopology(GetD3D11PrimitiveType(PrimitiveType, bUsingTessellation));
  Direct3DDeviceIMContext->DrawInstancedIndirect(ArgumentBuffer->Resource, ArgumentOffset);
}

void FD3D11DynamicRHI::RHIDrawIndexedIndirect(RBIndexBufferRHIParamRef IndexBufferRHI, uint32 PrimitiveType, RBStructuredBufferRHIParamRef ArgumentsBufferRHI, int32 DrawArgumentsIndex, uint32 NumInstances)
{
  RBD3D11IndexBuffer* IndexBuffer = static_cast<RBD3D11IndexBuffer*>(IndexBufferRHI);
  RBD3D11StructuredBuffer* ArgumentsBuffer = static_cast<RBD3D11StructuredBuffer*>(ArgumentsBufferRHI);

  //RHI_DRAW_CALL_INC();

  //GPUProfilingData.RegisterGPUWork(1);

  CommitGraphicsResourceTables();
  CommitNonComputeShaderConstants();

  // determine 16bit vs 32bit indices
  uint32 SizeFormat = sizeof(DXGI_FORMAT);
  const DXGI_FORMAT Format = (IndexBuffer->GetStride() == sizeof(uint16) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT);

  StateCache.SetIndexBuffer(IndexBuffer->Resource, Format, 0);
  StateCache.SetPrimitiveTopology(GetD3D11PrimitiveType(PrimitiveType, bUsingTessellation));

  if (NumInstances > 1)
  {
    Direct3DDeviceIMContext->DrawIndexedInstancedIndirect(ArgumentsBuffer->Resource, DrawArgumentsIndex * 5 * sizeof(uint32));
  }
  else
  {
    CHECK(0);
  }
}

void FD3D11DynamicRHI::RHIDrawIndexedPrimitive(RBIndexBufferRHIParamRef IndexBufferRHI, uint32 PrimitiveType, int32 BaseVertexIndex, uint32 FirstInstance, uint32 NumVertices, uint32 StartIndex, uint32 NumPrimitives, uint32 NumInstances)
{
  RBD3D11IndexBuffer* IndexBuffer = static_cast<RBD3D11IndexBuffer*>(IndexBufferRHI);

  // called should make sure the input is valid, this avoid hidden bugs
  CHECK(NumPrimitives > 0);

  //RHI_DRAW_CALL_STATS(PrimitiveType, NumInstances*NumPrimitives);

  //GPUProfilingData.RegisterGPUWork(NumPrimitives * NumInstances, NumVertices * NumInstances);

  CommitGraphicsResourceTables();
  CommitNonComputeShaderConstants();

  // determine 16bit vs 32bit indices
  uint32 SizeFormat = sizeof(DXGI_FORMAT);
  const DXGI_FORMAT Format = (IndexBuffer->GetStride() == sizeof(uint16) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT);

  uint32 IndexCount = GetVertexCountForPrimitiveCount(NumPrimitives, PrimitiveType);

  // Verify that we are not trying to read outside the index buffer range
  // test is an optimized version of: StartIndex + IndexCount <= IndexBuffer->GetSize() / IndexBuffer->GetStride() 
  CHECKF((StartIndex + IndexCount) * IndexBuffer->GetStride() <= IndexBuffer->GetSize(),
    TEXT("Start %u, Count %u, Type %u, Buffer Size %u, Buffer stride %u"), StartIndex, IndexCount, PrimitiveType, IndexBuffer->GetSize(), IndexBuffer->GetStride());

  StateCache.SetIndexBuffer(IndexBuffer->Resource, Format, 0);
  StateCache.SetPrimitiveTopology(GetD3D11PrimitiveType(PrimitiveType, bUsingTessellation));

  if (NumInstances > 1 || FirstInstance != 0)
  {
    Direct3DDeviceIMContext->DrawIndexedInstanced(IndexCount, NumInstances, StartIndex, BaseVertexIndex, FirstInstance);
  }
  else
  {
    Direct3DDeviceIMContext->DrawIndexed(IndexCount, StartIndex, BaseVertexIndex);
  }
}

void FD3D11DynamicRHI::RHIDrawIndexedPrimitiveIndirect(uint32 PrimitiveType, RBIndexBufferRHIParamRef IndexBufferRHI, RBVertexBufferRHIParamRef ArgumentBufferRHI, uint32 ArgumentOffset)
{
  RBD3D11IndexBuffer* IndexBuffer = static_cast<RBD3D11IndexBuffer*>(IndexBufferRHI);
  RBD3D11VertexBuffer* ArgumentBuffer = static_cast<RBD3D11VertexBuffer*>(ArgumentBufferRHI);

  //RHI_DRAW_CALL_INC();

  //GPUProfilingData.RegisterGPUWork(0);

  CommitGraphicsResourceTables();
  CommitNonComputeShaderConstants();

  // Set the index buffer.
  const uint32 SizeFormat = sizeof(DXGI_FORMAT);
  const DXGI_FORMAT Format = (IndexBuffer->GetStride() == sizeof(uint16) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT);
  StateCache.SetIndexBuffer(IndexBuffer->Resource, Format, 0);
  StateCache.SetPrimitiveTopology(GetD3D11PrimitiveType(PrimitiveType, bUsingTessellation));
  Direct3DDeviceIMContext->DrawIndexedInstancedIndirect(ArgumentBuffer->Resource, ArgumentOffset);
}