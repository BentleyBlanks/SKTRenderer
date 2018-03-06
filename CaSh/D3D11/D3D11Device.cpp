#include "RHID3D11.h"
#include "D3D11Setting.h"
#include "D3D11Util.h"
#include "Logger.h"
#include <vector>
#include "Util.h"
#include "RBMath.h"
#include "delayimp.h"
#include "PixelFormat.h"
bool GIsRHIInitialized = false;
RBDynamicRHI* RBDynamicRHI::GetRHI(ERHITYPES rhi_tp)
{
	static RBDynamicRHI* _instance = nullptr;
	if (_instance) return _instance;
  {
    switch (rhi_tp)
    {
	case RHI_D3D11:
	{
		_instance = new FD3D11DynamicRHI();
		_instance->Init();
		return _instance;
		break;
	}
    case RHI_D3D12:
      return nullptr;
      break;
    case RHI_TOTAL:

      break;
    default:
      break;
    }
  }
  return nullptr;
}

int64 FD3D11GlobalStats::GTotalGraphicsMemory = 0;
int64 FD3D11GlobalStats::GSharedSystemMemory = 0;
int64 FD3D11GlobalStats::GDedicatedSystemMemory = 0;
int64 FD3D11GlobalStats::GDedicatedVideoMemory = 0;

/** This function is used as a SEH filter to catch only delay load exceptions. */
static bool IsDelayLoadException(PEXCEPTION_POINTERS ExceptionPointers)
{
#if WINVER > 0x502	// Windows SDK 7.1 doesn't define VcppException
  switch (ExceptionPointers->ExceptionRecord->ExceptionCode)
  {
  case VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND):
  case VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND):
    return EXCEPTION_EXECUTE_HANDLER;
  default:
    return EXCEPTION_CONTINUE_SEARCH;
  }
#else
  return EXCEPTION_EXECUTE_HANDLER;
#endif
}

/**
* Attempts to create a D3D11 device for the adapter using at most MaxFeatureLevel.
* If creation is successful, true is returned and the supported feature level is set in OutFeatureLevel.
*/
static bool SafeTestD3D11CreateDevice(IDXGIAdapter* Adapter, D3D_FEATURE_LEVEL MaxFeatureLevel, D3D_FEATURE_LEVEL* OutFeatureLevel)
{
  ID3D11Device* D3DDevice = NULL;
  ID3D11DeviceContext* D3DDeviceContext = NULL;
  uint32 DeviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;

  // Use a debug device if specified on the command line.
  if (true)
  {
    DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
  }

  D3D_FEATURE_LEVEL RequestedFeatureLevels[] =
  {
    D3D_FEATURE_LEVEL_11_1,
    D3D_FEATURE_LEVEL_11_0,
    D3D_FEATURE_LEVEL_10_0
  };

  int32 FirstAllowedFeatureLevel = 0;
  int32 NumAllowedFeatureLevels = ARRAY_COUNT(RequestedFeatureLevels);
  while (FirstAllowedFeatureLevel < NumAllowedFeatureLevels)
  {
    if (RequestedFeatureLevels[FirstAllowedFeatureLevel] == MaxFeatureLevel)
    {
      break;
    }
    FirstAllowedFeatureLevel++;
  }
  NumAllowedFeatureLevels -= FirstAllowedFeatureLevel;

  if (NumAllowedFeatureLevels == 0)
  {
    return false;
  }

  __try
  {
    // We don't want software renderer. Ideally we specify D3D_DRIVER_TYPE_HARDWARE on creation but
    // when we specify an adapter we need to specify D3D_DRIVER_TYPE_UNKNOWN (otherwise the call fails).
    // We cannot check the device type later (seems this is missing functionality in D3D).

	 HRESULT hr = D3D11CreateDevice(
		  Adapter,
		  D3D_DRIVER_TYPE_UNKNOWN,
		  NULL,
		  DeviceFlags,
		  &RequestedFeatureLevels[FirstAllowedFeatureLevel],
		  NumAllowedFeatureLevels,
		  D3D11_SDK_VERSION,
		  &D3DDevice,
		  OutFeatureLevel,
		  &D3DDeviceContext
		  );
	 if (hr==S_OK)
    {
      D3DDevice->Release();
      D3DDeviceContext->Release();
      return true;
    }
  }
  __except (IsDelayLoadException(GetExceptionInformation()))
  {
  }

  return false;
}

static uint32 CountAdapterOutputs(TRefCountPtr<IDXGIAdapter1>& Adapter)
{
  uint32 OutputCount = 0;
  for (;;)
  {
    TRefCountPtr<IDXGIOutput> Output;
    HRESULT hr = Adapter->EnumOutputs(OutputCount, Output.GetInitReference());
    if (FAILED(hr))
    {
      break;
    }
    ++OutputCount;
  }

  return OutputCount;
}

const char* GetFeatureLevelString(D3D_FEATURE_LEVEL FeatureLevel)
{
  switch (FeatureLevel)
  {
  case D3D_FEATURE_LEVEL_9_1:		return TEXT("9_1");
  case D3D_FEATURE_LEVEL_9_2:		return TEXT("9_2");
  case D3D_FEATURE_LEVEL_9_3:		return TEXT("9_3");
  case D3D_FEATURE_LEVEL_10_0:	return TEXT("10_0");
  case D3D_FEATURE_LEVEL_10_1:	return TEXT("10_1");
  case D3D_FEATURE_LEVEL_11_0:	return TEXT("11_0");
  }
  return TEXT("X_X");
}



void FD3D11DynamicRHI::Init()
{

	// Initialize the platform pixel format map.
	GPixelFormats[PF_Unknown].PlatformFormat = DXGI_FORMAT_UNKNOWN;
	GPixelFormats[PF_A32B32G32R32F].PlatformFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
	GPixelFormats[PF_B8G8R8A8].PlatformFormat = DXGI_FORMAT_B8G8R8A8_TYPELESS;
	GPixelFormats[PF_G8].PlatformFormat = DXGI_FORMAT_R8_UNORM;
	GPixelFormats[PF_G16].PlatformFormat = DXGI_FORMAT_R16_UNORM;
	GPixelFormats[PF_DXT1].PlatformFormat = DXGI_FORMAT_BC1_TYPELESS;
	GPixelFormats[PF_DXT3].PlatformFormat = DXGI_FORMAT_BC2_TYPELESS;
	GPixelFormats[PF_DXT5].PlatformFormat = DXGI_FORMAT_BC3_TYPELESS;
	GPixelFormats[PF_BC4].PlatformFormat = DXGI_FORMAT_BC4_UNORM;
	GPixelFormats[PF_UYVY].PlatformFormat = DXGI_FORMAT_UNKNOWN;		// TODO: Not supported in D3D11
#ifdef DEPTH_32_BIT_CONVERSION
	GPixelFormats[PF_DepthStencil].PlatformFormat = DXGI_FORMAT_R32G8X24_TYPELESS;
	GPixelFormats[PF_DepthStencil].BlockBytes = 5;
	GPixelFormats[PF_X24_G8].PlatformFormat = DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
	GPixelFormats[PF_X24_G8].BlockBytes = 5;
#else
	GPixelFormats[PF_DepthStencil].PlatformFormat = DXGI_FORMAT_R24G8_TYPELESS;
	GPixelFormats[PF_DepthStencil].BlockBytes = 4;
	GPixelFormats[PF_X24_G8].PlatformFormat = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
	GPixelFormats[PF_X24_G8].BlockBytes = 4;
#endif
	GPixelFormats[PF_ShadowDepth].PlatformFormat = DXGI_FORMAT_R16_TYPELESS;
	GPixelFormats[PF_ShadowDepth].BlockBytes = 2;
	GPixelFormats[PF_R32_FLOAT].PlatformFormat = DXGI_FORMAT_R32_FLOAT;
	GPixelFormats[PF_G16R16].PlatformFormat = DXGI_FORMAT_R16G16_UNORM;
	GPixelFormats[PF_G16R16F].PlatformFormat = DXGI_FORMAT_R16G16_FLOAT;
	GPixelFormats[PF_G16R16F_FILTER].PlatformFormat = DXGI_FORMAT_R16G16_FLOAT;
	GPixelFormats[PF_G32R32F].PlatformFormat = DXGI_FORMAT_R32G32_FLOAT;
	GPixelFormats[PF_A2B10G10R10].PlatformFormat = DXGI_FORMAT_R10G10B10A2_UNORM;
	GPixelFormats[PF_A16B16G16R16].PlatformFormat = DXGI_FORMAT_R16G16B16A16_UNORM;
	GPixelFormats[PF_D24].PlatformFormat = DXGI_FORMAT_R24G8_TYPELESS;
	GPixelFormats[PF_R16F].PlatformFormat = DXGI_FORMAT_R16_FLOAT;
	GPixelFormats[PF_R16F_FILTER].PlatformFormat = DXGI_FORMAT_R16_FLOAT;

	GPixelFormats[PF_FloatRGB].PlatformFormat = DXGI_FORMAT_R11G11B10_FLOAT;
	GPixelFormats[PF_FloatRGB].BlockBytes = 4;
	GPixelFormats[PF_FloatRGBA].PlatformFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	GPixelFormats[PF_FloatRGBA].BlockBytes = 8;

	GPixelFormats[PF_FloatR11G11B10].PlatformFormat = DXGI_FORMAT_R11G11B10_FLOAT;
	GPixelFormats[PF_FloatR11G11B10].BlockBytes = 4;

	GPixelFormats[PF_V8U8].PlatformFormat = DXGI_FORMAT_R8G8_SNORM;
	GPixelFormats[PF_BC5].PlatformFormat = DXGI_FORMAT_BC5_UNORM;
	GPixelFormats[PF_A1].PlatformFormat = DXGI_FORMAT_R1_UNORM; // Not supported for rendering.
	GPixelFormats[PF_A8].PlatformFormat = DXGI_FORMAT_A8_UNORM;
	GPixelFormats[PF_R32_UINT].PlatformFormat = DXGI_FORMAT_R32_UINT;
	GPixelFormats[PF_R32_SINT].PlatformFormat = DXGI_FORMAT_R32_SINT;

	GPixelFormats[PF_R16_UINT].PlatformFormat = DXGI_FORMAT_R16_UINT;
	GPixelFormats[PF_R16_SINT].PlatformFormat = DXGI_FORMAT_R16_SINT;
	GPixelFormats[PF_R16G16B16A16_UINT].PlatformFormat = DXGI_FORMAT_R16G16B16A16_UINT;
	GPixelFormats[PF_R16G16B16A16_SINT].PlatformFormat = DXGI_FORMAT_R16G16B16A16_SINT;

	GPixelFormats[PF_R5G6B5_UNORM].PlatformFormat = DXGI_FORMAT_B5G6R5_UNORM;
	GPixelFormats[PF_R8G8B8A8].PlatformFormat = DXGI_FORMAT_R8G8B8A8_TYPELESS;
	GPixelFormats[PF_R8G8].PlatformFormat = DXGI_FORMAT_R8G8_UNORM;
	GPixelFormats[PF_R32G32B32A32_UINT].PlatformFormat = DXGI_FORMAT_R32G32B32A32_UINT;
	GPixelFormats[PF_R16G16_UINT].PlatformFormat = DXGI_FORMAT_R16G16_UINT;

	GPixelFormats[PF_BC6H].PlatformFormat = DXGI_FORMAT_BC6H_UF16;
	GPixelFormats[PF_BC7].PlatformFormat = DXGI_FORMAT_BC7_TYPELESS;

  VERIFYD3D11RESULT(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)DXGIFactory1.GetInitReference()));

  CHECK(DXGIFactory1.GetReference());
  TRefCountPtr<IDXGIAdapter1> TempAdapter;
  D3D_FEATURE_LEVEL MaxAllowedFeatureLevel = D3D_FEATURE_LEVEL_11_1;
  FD3D11Adapter FirstWithoutIntegratedAdapter;
  FD3D11Adapter FirstAdapter;
  // indexed by AdapterIndex, we store it instead of query it later from the created device to prevent some Optimus bug reporting the data/name of the wrong adapter
  std::vector<DXGI_ADAPTER_DESC> AdapterDescription;
  bool bIsAnyAMD = false;
  bool bIsAnyIntel = false;
  bool bIsAnyNVIDIA = false;
  g_logger->debug(WIP_INFO, TEXT("D3D11 adapters:"));
  // Enumerate the DXGIFactory's adapters.
  for (uint32 AdapterIndex = 0; DXGIFactory1->EnumAdapters1(AdapterIndex, TempAdapter.GetInitReference()) != DXGI_ERROR_NOT_FOUND; ++AdapterIndex)
  
  {
	 
    // to make sure the array elements can be indexed with AdapterIndex
    DXGI_ADAPTER_DESC AdapterDesc;
	//TempAdapter->GetDesc(&AdapterDesc);
    ZeroMemory(&AdapterDesc, sizeof(DXGI_ADAPTER_DESC));

    // Check that if adapter supports D3D11.
    if (TempAdapter)
    {
      D3D_FEATURE_LEVEL ActualFeatureLevel = (D3D_FEATURE_LEVEL)0;
      if (SafeTestD3D11CreateDevice(TempAdapter, MaxAllowedFeatureLevel, &ActualFeatureLevel))
      {
        // Log some information about the available D3D11 adapters.
        VERIFYD3D11RESULT(TempAdapter->GetDesc(&AdapterDesc));
        uint32 OutputCount = CountAdapterOutputs(TempAdapter);

        wchar_t *WStr = AdapterDesc.Description;
        size_t len = wcslen(WStr) + 1;
        size_t converted = 0;
        char *CStr;
        CStr = (char*)malloc(len*sizeof(char));
        wcstombs_s(&converted, CStr, len, WStr, _TRUNCATE);

        g_logger->debug(WIP_INFO,
          TEXT("  %2u. '%s' (Feature Level %s)"),
          AdapterIndex,
          CStr,
          GetFeatureLevelString(ActualFeatureLevel)
          );
        delete CStr;
        g_logger->debug(WIP_INFO,
          TEXT("      %u/%u/%u MB DedicatedVideo/DedicatedSystem/SharedSystem, Outputs:%d, VendorId:0x%x"),
          (uint32)(AdapterDesc.DedicatedVideoMemory / (1024 * 1024)),
          (uint32)(AdapterDesc.DedicatedSystemMemory / (1024 * 1024)),
          (uint32)(AdapterDesc.SharedSystemMemory / (1024 * 1024)),
          OutputCount,
          AdapterDesc.VendorId
          );

        bool bIsAMD = AdapterDesc.VendorId == 0x1002;
        bool bIsIntel = AdapterDesc.VendorId == 0x8086;
        bool bIsNVIDIA = AdapterDesc.VendorId == 0x10DE;
        bool bIsMicrosoft = AdapterDesc.VendorId == 0x1414;

        if (bIsAMD) bIsAnyAMD = true;
        if (bIsIntel) bIsAnyIntel = true;
        if (bIsNVIDIA) bIsAnyNVIDIA = true;

        // Simple heuristic but without profiling it's hard to do better
        const bool bIsIntegrated = bIsIntel;

        FD3D11Adapter CurrentAdapter(AdapterIndex, ActualFeatureLevel);

        if (bIsMicrosoft)
        {
          // Add special check to support HMDs, which do not have associated outputs.

          // To reject the software emulation, unless the cvar wants it.
          // https://msdn.microsoft.com/en-us/library/windows/desktop/bb205075(v=vs.85).aspx#WARP_new_for_Win8
          // Before we tested for no output devices but that failed where a laptop had a Intel (with output) and NVidia (with no output)
          continue;
        }

        if (!bIsIntegrated && !FirstWithoutIntegratedAdapter.IsValid())
        {
          FirstWithoutIntegratedAdapter = CurrentAdapter;
        }

        if (!FirstAdapter.IsValid())
        {
          FirstAdapter = CurrentAdapter;
        }
      }
    }

    AdapterDescription.push_back(AdapterDesc);
  }
  if ((bIsAnyAMD || bIsAnyNVIDIA))
  {
    _ChosenAdapter = FirstWithoutIntegratedAdapter;

    // We assume Intel is integrated graphics (slower than discrete) than NVIDIA or AMD cards and rather take a different one
    if (!_ChosenAdapter.IsValid())
    {
      _ChosenAdapter = FirstAdapter;
    }
  }
  else
  {
    _ChosenAdapter = FirstAdapter;
  }

  if (_ChosenAdapter.IsValid())
  {
    _ChosenDescription = AdapterDescription[_ChosenAdapter.AdapterIndex];
    g_logger->debug(WIP_INFO, TEXT("Chosen D3D11 Adapter: %u"), _ChosenAdapter.AdapterIndex);
  }
  else
  {
    g_logger->debug(WIP_ERROR, TEXT("Failed to choose a D3D11 Adapter."));
  }

  D3D_FEATURE_LEVEL FeatureLevel = D3D11Setting::feature_11 ? D3D_FEATURE_LEVEL_11_1 : D3D_FEATURE_LEVEL_11_0;
  TRefCountPtr<IDXGIAdapter1> Adapter;
  // In Direct3D 11, if you are trying to create a hardware or a software device, set pAdapter != NULL which constrains the other inputs to be:
  //		DriverType must be D3D_DRIVER_TYPE_UNKNOWN 
  //		Software must be NULL. 
  D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_UNKNOWN;

  uint32 DeviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;

  // Use a debug device if specified on the command line.
  const bool bWithD3DDebug = true;
  if (bWithD3DDebug)
  {
    DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    g_logger->debug(WIP_INFO, TEXT("InitD3DDevice: -D3DDebug = %s"), bWithD3DDebug ? ("on") : ("off"));
  }
  TRefCountPtr<IDXGIAdapter1> EnumAdapter;
  if (DXGIFactory1->EnumAdapters1(_ChosenAdapter.AdapterIndex, EnumAdapter.GetInitReference()) != DXGI_ERROR_NOT_FOUND)
  {
    if (EnumAdapter)// && EnumAdapter->CheckInterfaceSupport(__uuidof(ID3D11Device),NULL) == S_OK)
    {
      // we don't use AdapterDesc.Description as there is a bug with Optimus where it can report the wrong name
      DXGI_ADAPTER_DESC AdapterDesc = _ChosenDescription;
      Adapter = EnumAdapter;

      /*
      GRHIAdapterName = AdapterDesc.Description;
      GRHIVendorId = AdapterDesc.VendorId;
      GRHIDeviceId = AdapterDesc.DeviceId;
      */
      g_logger->debug(WIP_INFO, TEXT("    GPU DeviceId: 0x%x (for the marketing name, search the web for \"GPU Device Id\")"),
        AdapterDesc.DeviceId);

      /*
      // get driver version (todo: share with other RHIs)
      {
      FGPUDriverInfo GPUDriverInfo = FPlatformMisc::GetGPUDriverInfo(GRHIAdapterName);

      GRHIAdapterUserDriverVersion = GPUDriverInfo.UserDriverVersion;
      GRHIAdapterInternalDriverVersion = GPUDriverInfo.InternalDriverVersion;
      GRHIAdapterDriverDate = GPUDriverInfo.DriverDate;

      UE_LOG(LogD3D11RHI, Log, TEXT("    Adapter Name: %s"), *GRHIAdapterName);
      UE_LOG(LogD3D11RHI, Log, TEXT("  Driver Version: %s (internal:%s, unified:%s)"), *GRHIAdapterUserDriverVersion, *GRHIAdapterInternalDriverVersion, *GPUDriverInfo.GetUnifiedDriverVersion());
      UE_LOG(LogD3D11RHI, Log, TEXT("     Driver Date: %s"), *GRHIAdapterDriverDate);
      }
      */

      // Issue: 32bit windows doesn't report 64bit value, we take what we get.
      FD3D11GlobalStats::GDedicatedVideoMemory = int64(AdapterDesc.DedicatedVideoMemory);
      FD3D11GlobalStats::GDedicatedSystemMemory = int64(AdapterDesc.DedicatedSystemMemory);
      FD3D11GlobalStats::GSharedSystemMemory = int64(AdapterDesc.SharedSystemMemory);

      // Total amount of system memory, clamped to 8 GB
      //int64 TotalPhysicalMemory = RBMath::get_min(int64(FPlatformMemory::GetConstants().TotalPhysicalGB), 8ll) * (1024ll * 1024ll * 1024ll);

      // Consider 50% of the shared memory but max 25% of total system memory.
      //int64 ConsideredSharedSystemMemory = RBMath::get_min(FD3D11GlobalStats::GSharedSystemMemory / 2ll, TotalPhysicalMemory / 4ll);

      FD3D11GlobalStats::GTotalGraphicsMemory = 0;
      if (bIsAnyIntel)
      {
        // It's all system memory.
        FD3D11GlobalStats::GTotalGraphicsMemory = FD3D11GlobalStats::GDedicatedVideoMemory;
        FD3D11GlobalStats::GTotalGraphicsMemory += FD3D11GlobalStats::GDedicatedSystemMemory;
        //FD3D11GlobalStats::GTotalGraphicsMemory += ConsideredSharedSystemMemory;
      }
      else if (FD3D11GlobalStats::GDedicatedVideoMemory >= 200 * 1024 * 1024)
      {
        // Use dedicated video memory, if it's more than 200 MB
        FD3D11GlobalStats::GTotalGraphicsMemory = FD3D11GlobalStats::GDedicatedVideoMemory;
      }
      else if (FD3D11GlobalStats::GDedicatedSystemMemory >= 200 * 1024 * 1024)
      {
        // Use dedicated system memory, if it's more than 200 MB
        FD3D11GlobalStats::GTotalGraphicsMemory = FD3D11GlobalStats::GDedicatedSystemMemory;
      }
      else if (FD3D11GlobalStats::GSharedSystemMemory >= 400 * 1024 * 1024)
      {
        // Use some shared system memory, if it's more than 400 MB
        //FD3D11GlobalStats::GTotalGraphicsMemory = ConsideredSharedSystemMemory;
      }
      else
      {
        // Otherwise consider 25% of total system memory for graphics.
        //FD3D11GlobalStats::GTotalGraphicsMemory = TotalPhysicalMemory / 4ll;
      }

      if (sizeof(SIZE_T) < 8)
      {
        // Clamp to 1 GB if we're less than 64-bit
        FD3D11GlobalStats::GTotalGraphicsMemory = RBMath::get_min(FD3D11GlobalStats::GTotalGraphicsMemory, 1024ll * 1024ll * 1024ll);
      }
      else
      {
        // Clamp to 1.9 GB if we're 64-bit
        FD3D11GlobalStats::GTotalGraphicsMemory = RBMath::get_min(FD3D11GlobalStats::GTotalGraphicsMemory, 1945ll * 1024ll * 1024ll);
      }
    }
  }
  else
  {
    //CHECK(!"Internal error, EnumAdapters() failed but before it worked")
	  //在某些双显卡平台上直接列举设备失败，所以暂时直接创建设备

	  CurrentBoundedShaderState = nullptr;
	  
	  D3D_FEATURE_LEVEL RequestedFeatureLevels[] =
	  {
		  //D3D_FEATURE_LEVEL_11_1,
		  D3D_FEATURE_LEVEL_11_0,
		  D3D_FEATURE_LEVEL_10_0
	  };
	  int32 NumAllowedFeatureLevels = ARRAY_COUNT(RequestedFeatureLevels);
	  D3D_FEATURE_LEVEL featureLevel;
	  HRESULT hr = D3D11CreateDevice(
		  0,                 // default adapter
		  D3D_DRIVER_TYPE_HARDWARE,
		  0,                 // no software device
		  DeviceFlags,
		  RequestedFeatureLevels, NumAllowedFeatureLevels,              // default feature level array
		  D3D11_SDK_VERSION,
		  Direct3DDevice.GetInitReference(),
		  &featureLevel,
		  Direct3DDeviceIMContext.GetInitReference());
	  CHECKF(hr == S_OK, "Create defualt failed!Maybe this platform dont support D3D11!\n");
	  g_logger->debug(WIP_INFO, "feature level %s created!", GetFeatureLevelString(featureLevel));
	  GIsRHIInitialized = true;
	  return;
  }
  D3D_FEATURE_LEVEL ActualFeatureLevel = (D3D_FEATURE_LEVEL)D3D_FEATURE_LEVEL_9_1;
  if (bIsAnyAMD)
  {
    DeviceFlags &= ~D3D11_CREATE_DEVICE_SINGLETHREADED;
  }
  // Creating the Direct3D device.
  VERIFYD3D11RESULT(D3D11CreateDevice(
    Adapter,
    DriverType,
    NULL,
    DeviceFlags,
    &FeatureLevel,
    1,
    D3D11_SDK_VERSION,
    Direct3DDevice.GetInitReference(),
    &ActualFeatureLevel,
    Direct3DDeviceIMContext.GetInitReference()
    ));
  // We should get the feature level we asked for as earlier we checked to ensure it is supported.
  CHECK(ActualFeatureLevel == FeatureLevel);

  CurrentBoundedShaderState = nullptr;
  GIsRHIInitialized = true;

}

void FD3D11DynamicRHI::Shutdown()
{
  g_logger->debug(WIP_INFO, TEXT("Shutdown"));
  //CHECK(IsInGameThread() && IsInRenderingThread());  // require that the render thread has been shut down

  // Cleanup the D3D device.
  CleanupD3DDevice();

  // Release the buffer of zeroes.
  //we will need.
  /*
  ::free(ZeroBuffer);
  ZeroBuffer = NULL;
  ZeroBufferSize = 0;
  */
}

void FD3D11DynamicRHI::CleanupD3DDevice()
{
  g_logger->debug(WIP_INFO, TEXT("CleanupD3DDevice"));

  if (GIsRHIInitialized)
  {
    CHECK(Direct3DDevice);
    CHECK(Direct3DDeviceIMContext);

    // Reset the RHI initialized flag.
    GIsRHIInitialized = false;

    //check(!GIsCriticalError);

    // Ask all initialized FRenderResources to release their RHI resources.
    //we need but not now 
    /*
    for (TLinkedList<FRenderResource*>::TIterator ResourceIt(FRenderResource::GetResourceList()); ResourceIt; ResourceIt.Next())
    {
      FRenderResource* Resource = *ResourceIt;
      check(Resource->IsInitialized());
      Resource->ReleaseRHI();
    }
    

    for (TLinkedList<FRenderResource*>::TIterator ResourceIt(FRenderResource::GetResourceList()); ResourceIt; ResourceIt.Next())
    {
      ResourceIt->ReleaseDynamicRHI();
    }
    */

    //extern void EmptyD3DSamplerStateCache();
    //EmptyD3DSamplerStateCache();

    // release our dynamic VB and IB buffers
    //DynamicVB = NULL;
    //DynamicIB = NULL;

    // Release references to bound uniform buffers.
    /*
    for (int32 Frequency = 0; Frequency < SF_NumFrequencies; ++Frequency)
    {
      for (int32 BindIndex = 0; BindIndex < MAX_UNIFORM_BUFFERS_PER_SHADER_STAGE; ++BindIndex)
      {
        BoundUniformBuffers[Frequency][BindIndex].SafeRelease();
      }
    }
    */

    // Release the device and its IC
    //StateCache.SetContext(nullptr);

    // Flush all pending deletes before destroying the device.
    //FRHIResource::FlushPendingDeletes();

    //ReleasePooledUniformBuffers();
    //ReleasePooledTextures();

    // When running with D3D debug, clear state and flush the device to get rid of spurious live objects in D3D11's report.
    if (true)
    {
      Direct3DDeviceIMContext->ClearState();
      Direct3DDeviceIMContext->Flush();
    }

    Direct3DDeviceIMContext = NULL;

    Direct3DDevice = NULL;

    DXGIFactory1 = NULL;
  }
}