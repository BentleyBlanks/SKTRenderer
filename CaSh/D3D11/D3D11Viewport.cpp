#include "D3D11Viewport.h"
#include "D3D11Util.h"
#include "RHID3D11.h"
#include "Logger.h"

extern void D3D11TextureAllocated2D(RBD3D11Texture2D& Texture);

/**
* Creates a FD3D11Surface to represent a swap chain's back buffer.
*/
RBD3D11Texture2D* GetSwapChainSurface(FD3D11DynamicRHI* D3DRHI, EPixelFormat PixelFormat, IDXGISwapChain* SwapChain)
{
	// Grab the back buffer
	TRefCountPtr<ID3D11Texture2D> BackBufferResource;
	VERIFYD3D11RESULT_EX(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)BackBufferResource.GetInitReference()), D3DRHI->GetDevice());

	// create the render target view
	TRefCountPtr<ID3D11RenderTargetView> BackBufferRenderTargetView;
	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
	RTVDesc.Format = DXGI_FORMAT_UNKNOWN;
	RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	RTVDesc.Texture2D.MipSlice = 0;
	VERIFYD3D11RESULT(D3DRHI->GetDevice()->CreateRenderTargetView(BackBufferResource, 0/*&RTVDesc*/, BackBufferRenderTargetView.GetInitReference()));

	D3D11_TEXTURE2D_DESC TextureDesc;
	BackBufferResource->GetDesc(&TextureDesc);

	std::vector<TRefCountPtr<ID3D11RenderTargetView> > RenderTargetViews;
	RenderTargetViews.push_back(BackBufferRenderTargetView);

	// create a shader resource view to allow using the backbuffer as a texture
	TRefCountPtr<ID3D11ShaderResourceView> BackBufferShaderResourceView;
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MostDetailedMip = 0;
	SRVDesc.Texture2D.MipLevels = 1;
	VERIFYD3D11RESULT(D3DRHI->GetDevice()->CreateShaderResourceView(BackBufferResource,0/*&SRVDesc*/, BackBufferShaderResourceView.GetInitReference()));

	RBD3D11Texture2D* NewTexture = new RBD3D11Texture2D(
		D3DRHI,
		BackBufferResource,
		BackBufferShaderResourceView,
		false,
		1,
		RenderTargetViews,
		NULL,
		TextureDesc.Width,
		TextureDesc.Height,
		1,
		1,
		1,
		PixelFormat,
		false,
		false,
		false,
		RBClearValueBinding::DepthFar
		);

	D3D11TextureAllocated2D(*NewTexture);

	NewTexture->DoNoDeferDelete();

	return NewTexture;
}

RBD3D11Viewport::RBD3D11Viewport(FD3D11DynamicRHI* InD3DRHI, HWND InWindowHandle, uint32 InSizeX, uint32 InSizeY, bool bInIsFullscreen, EPixelFormat InPreferredPixelFormat) :
D3DRHI(InD3DRHI),
LastFlipTime(0),
LastFrameComplete(0),
LastCompleteTime(0),
SyncCounter(0),
bSyncedLastFrame(false),
WindowHandle(InWindowHandle),
MaximumFrameLatency(3),
SizeX(InSizeX),
SizeY(InSizeY),
bIsFullscreen(bInIsFullscreen),
PixelFormat(InPreferredPixelFormat),
bIsValid(true)
{
  //CHECK(IsInGameThread());
  D3DRHI->Viewports.push_back(this);

  // Ensure that the D3D device has been created.
  // D3DRHI->InitD3DDevice();

  // Create a backbuffer/swapchain for each viewport
  TRefCountPtr<IDXGIDevice> DXGIDevice;
  VERIFYD3D11RESULT(D3DRHI->GetDevice()->QueryInterface(__uuidof(IDXGIDevice), (void**)DXGIDevice.GetInitReference()));

  // Create the swapchain.
  DXGI_SWAP_CHAIN_DESC SwapChainDesc;
  rb_memzero(&SwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

  SwapChainDesc.BufferDesc = SetupDXGI_MODE_DESC();
  // MSAA Sample count
  SwapChainDesc.SampleDesc.Count = 1;
  SwapChainDesc.SampleDesc.Quality = 0;
  SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
  // 1:single buffering, 2:double buffering, 3:triple buffering
  SwapChainDesc.BufferCount = 1;
  SwapChainDesc.OutputWindow = WindowHandle;
  SwapChainDesc.Windowed = !bIsFullscreen;
  // DXGI_SWAP_EFFECT_DISCARD / DXGI_SWAP_EFFECT_SEQUENTIAL
  SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
  SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  VERIFYD3D11RESULT(D3DRHI->GetFactory()->CreateSwapChain(DXGIDevice, &SwapChainDesc, SwapChain.GetInitReference()));

  // Set the DXGI message hook to not change the window behind our back.
  D3DRHI->GetFactory()->MakeWindowAssociation(WindowHandle, DXGI_MWA_NO_WINDOW_CHANGES);

  // Create a RHI surface to represent the viewport's back buffer.
  BackBuffer = GetSwapChainSurface(D3DRHI, PixelFormat, SwapChain);

  // Tell the window to redraw when they can.
  // @todo: For Slate viewports, it doesn't make sense to post WM_PAINT messages (we swallow those.)
  ::PostMessage(WindowHandle, WM_PAINT, 0, 0);

}

void RBD3D11Viewport::ConditionalResetSwapChain(bool bIgnoreFocus)
{
  if (!bIsValid)
  {
    // Check if the viewport's window is focused before resetting the swap chain's fullscreen state.
    HWND FocusWindow = ::GetFocus();
    const bool bIsFocused = FocusWindow == WindowHandle;
    const bool bIsIconic = !!::IsIconic(WindowHandle);
    if (bIgnoreFocus || (bIsFocused && !bIsIconic))
    {
      //wait to impl
      //FlushRenderingCommands();

      // Store the current cursor clip rectangle as it can be lost when fullscreen is reset.
      RECT OriginalCursorRect;
      GetClipCursor(&OriginalCursorRect);

      HRESULT Result = SwapChain->SetFullscreenState(bIsFullscreen, NULL);
      if (SUCCEEDED(Result))
      {
        ClipCursor(&OriginalCursorRect);
        bIsValid = true;
      }
      else
      {
        // Even though the docs say SetFullscreenState always returns S_OK, that doesn't always seem to be the case.
        g_logger->debug(WIP_INFO,TEXT("IDXGISwapChain::SetFullscreenState returned %08x; waiting for the next frame to try again."), Result);
      }
    }
  }
}


RBD3D11Viewport::~RBD3D11Viewport()
{
	//CHECK(IsInRenderingThread());

	// If the swap chain was in fullscreen mode, switch back to windowed before releasing the swap chain.
	// DXGI throws an error otherwise.
	VERIFYD3D11RESULT(SwapChain->SetFullscreenState(false, NULL));

	//FrameSyncEvent.ReleaseResource();

	D3DRHI->Viewports.erase(std::find(D3DRHI->Viewports.begin(), D3DRHI->Viewports.end(), (this)));
	//.remove(this);
}

DXGI_MODE_DESC RBD3D11Viewport::SetupDXGI_MODE_DESC() const
{
	DXGI_MODE_DESC Ret;

	Ret.Width = SizeX;
	Ret.Height = SizeY;
	Ret.RefreshRate.Numerator = 60;	// illamas: use 0 to avoid a potential mismatch with hw
	Ret.RefreshRate.Denominator = 1;	// illamas: ditto
	Ret.Format = GetRenderTargetFormat(PixelFormat);
	Ret.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	Ret.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	return Ret;
}

void RBD3D11Viewport::Resize(u32 InSizeX, u32 InSizeY, bool bInIsFullscreen)
{
	// Unbind any dangling references to resources
	//D3DRHI->RHISetRenderTargets(0, nullptr, nullptr, 0, nullptr);
	D3DRHI->ClearState();
	D3DRHI->GetDeviceContext()->Flush(); // Potential perf hit

	/*
	if (IsValidRef(CustomPresent))
	{
		CustomPresent->OnBackBufferResize();
	}
	*/

	// Release our backbuffer reference, as required by DXGI before calling ResizeBuffers.
	if (IsValidRef(BackBuffer))
	{
		CHECK(BackBuffer->GetRefCount() == 1);

		checkComRefCount(BackBuffer->GetResource(), 1);
		checkComRefCount(BackBuffer->GetRenderTargetView(0, -1), 1);
		checkComRefCount(BackBuffer->GetShaderResourceView(), 1);
	}
	BackBuffer.SafeRelease();

	if (SizeX != InSizeX || SizeY != InSizeY)
	{
		SizeX = InSizeX;
		SizeY = InSizeY;

		CHECK(SizeX > 0);
		CHECK(SizeY > 0);

		// Resize the swap chain.
		VERIFYD3D11RESULT_EX(SwapChain->ResizeBuffers(1, SizeX, SizeY, GetRenderTargetFormat(PixelFormat), DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH), D3DRHI->GetDevice());

		if (bInIsFullscreen)
		{
			DXGI_MODE_DESC BufferDesc = SetupDXGI_MODE_DESC();

			if (FAILED(SwapChain->ResizeTarget(&BufferDesc)))
			{
				ConditionalResetSwapChain(true);
			}
		}
	}

	if (bIsFullscreen != bInIsFullscreen)
	{
		bIsFullscreen = bInIsFullscreen;
		bIsValid = false;

		// Use ConditionalResetSwapChain to call SetFullscreenState, to handle the failure case.
		// Ignore the viewport's focus state; since Resize is called as the result of a user action we assume authority without waiting for Focus.
		ConditionalResetSwapChain(true);
	}

	// Create a RHI surface to represent the viewport's back buffer.
	BackBuffer = GetSwapChainSurface(D3DRHI, PixelFormat, SwapChain);
}

/** Returns true if desktop composition is enabled. */
static bool IsCompositionEnabled()
{
	BOOL bDwmEnabled = false;
#ifdef D3D11_WITH_DWMAPI
	DwmIsCompositionEnabled(&bDwmEnabled);
#endif	//D3D11_WITH_DWMAPI
	return !!bDwmEnabled;
}

/** Presents the swap chain checking the return result. */
bool RBD3D11Viewport::PresentChecked(int32 SyncInterval)
{
	HRESULT Result = S_OK;
	bool bNeedNativePresent = true;
	/*
	if (IsValidRef(CustomPresent))
	{
		bNeedNativePresent = CustomPresent->Present(SyncInterval);
	}
	*/
	if (bNeedNativePresent)
	{
		// Present the back buffer to the viewport window.
		Result = SwapChain->Present(SyncInterval, 0);
	}

	// Detect a lost device.
	if (Result == DXGI_ERROR_DEVICE_REMOVED || Result == DXGI_ERROR_DEVICE_RESET || Result == DXGI_ERROR_DRIVER_INTERNAL_ERROR)
	{
		// This variable is checked periodically by the main thread.
		D3DRHI->bDeviceRemoved = true;
	}
	else
	{
		VERIFYD3D11RESULT(Result);
	}
	return bNeedNativePresent;
}

/** Blocks the CPU to synchronize with vblank by communicating with DWM. */
void RBD3D11Viewport::PresentWithVsyncDWM()
{
#ifdef D3D11_WITH_DWMAPI
	LARGE_INTEGER Cycles;
	DWM_TIMING_INFO TimingInfo;

	// Find out how long since we last flipped and query DWM for timing information.
	QueryPerformanceCounter(&Cycles);
	FMemory::Memzero(TimingInfo);
	TimingInfo.cbSize = sizeof(DWM_TIMING_INFO);
	DwmGetCompositionTimingInfo(WindowHandle, &TimingInfo);

	uint64 QpcAtFlip = Cycles.QuadPart;
	uint64 CyclesSinceLastFlip = Cycles.QuadPart - LastFlipTime;
	float CPUTime = FPlatformTime::ToMilliseconds(CyclesSinceLastFlip);
	float GPUTime = FPlatformTime::ToMilliseconds(TimingInfo.qpcFrameComplete - LastCompleteTime);
	float DisplayRefreshPeriod = FPlatformTime::ToMilliseconds(TimingInfo.qpcRefreshPeriod);

	// Find the smallest multiple of the refresh rate that is >= 33ms, our target frame rate.
	float RefreshPeriod = DisplayRefreshPeriod;
	if (RHIConsoleVariables::bForceThirtyHz && RefreshPeriod > 1.0f)
	{
		while (RefreshPeriod - (1000.0f / 30.0f) < -1.0f)
		{
			RefreshPeriod *= 2.0f;
		}
	}

	// If the last frame hasn't completed yet, we don't know how long the GPU took.
	bool bValidGPUTime = (TimingInfo.cFrameComplete > LastFrameComplete);
	if (bValidGPUTime)
	{
		GPUTime /= (float)(TimingInfo.cFrameComplete - LastFrameComplete);
	}

	// Update the sync counter depending on how much time it took to complete the previous frame.
	float FrameTime = FMath::Max<float>(CPUTime, GPUTime);
	if (FrameTime >= RHIConsoleVariables::SyncRefreshThreshold * RefreshPeriod)
	{
		SyncCounter--;
	}
	else if (bValidGPUTime)
	{
		SyncCounter++;
	}
	SyncCounter = FMath::Clamp<int32>(SyncCounter, 0, RHIConsoleVariables::MaxSyncCounter);

	// If frames are being completed quickly enough, block for vsync.
	bool bSync = (SyncCounter >= RHIConsoleVariables::SyncThreshold);
	if (bSync)
	{
		// This flushes the previous present call and blocks until it is made available to DWM.
		D3DRHI->GetDeviceContext()->Flush();
		DwmFlush();

		// We sleep a percentage of the remaining time. The trick is to get the
		// present call in after the vblank we just synced for but with time to
		// spare for the next vblank.
		float MinFrameTime = RefreshPeriod * RHIConsoleVariables::RefreshPercentageBeforePresent;
		float TimeToSleep;
		do
		{
			QueryPerformanceCounter(&Cycles);
			float TimeSinceFlip = FPlatformTime::ToMilliseconds(Cycles.QuadPart - LastFlipTime);
			TimeToSleep = (MinFrameTime - TimeSinceFlip);
			if (TimeToSleep > 0.0f)
			{
				FPlatformProcess::Sleep(TimeToSleep * 0.001f);
			}
		} while (TimeToSleep > 0.0f);
	}

	// Present.
	PresentChecked(/*SyncInterval=*/ 0);

	// If we are forcing <= 30Hz, block the CPU an additional amount of time if needed.
	// This second block is only needed when RefreshPercentageBeforePresent < 1.0.
	if (bSync)
	{
		LARGE_INTEGER LocalCycles;
		float TimeToSleep;
		bool bSaveCycles = false;
		do
		{
			QueryPerformanceCounter(&LocalCycles);
			float TimeSinceFlip = FPlatformTime::ToMilliseconds(LocalCycles.QuadPart - LastFlipTime);
			TimeToSleep = (RefreshPeriod - TimeSinceFlip);
			if (TimeToSleep > 0.0f)
			{
				bSaveCycles = true;
				FPlatformProcess::Sleep(TimeToSleep * 0.001f);
			}
		} while (TimeToSleep > 0.0f);

		if (bSaveCycles)
		{
			Cycles = LocalCycles;
		}
	}

	// If we are dropping vsync reset the counter. This provides a debounce time
	// before which we try to vsync again.
	if (!bSync && bSyncedLastFrame)
	{
		SyncCounter = 0;
	}

	if (bSync != bSyncedLastFrame || UE_LOG_ACTIVE(LogRHI, VeryVerbose))
	{
		UE_LOG(LogRHI, Verbose, TEXT("BlockForVsync[%d]: CPUTime:%.2fms GPUTime[%d]:%.2fms Blocked:%.2fms Pending/Complete:%d/%d"),
			bSync,
			CPUTime,
			bValidGPUTime,
			GPUTime,
			FPlatformTime::ToMilliseconds(Cycles.QuadPart - QpcAtFlip),
			TimingInfo.cFramePending,
			TimingInfo.cFrameComplete);
	}

	// Remember if we synced, when the frame completed, etc.
	bSyncedLastFrame = bSync;
	LastFlipTime = Cycles.QuadPart;
	LastFrameComplete = TimingInfo.cFrameComplete;
	LastCompleteTime = TimingInfo.qpcFrameComplete;
#endif	//D3D11_WITH_DWMAPI
}

bool RBD3D11Viewport::Present(bool bLockToVsync)
{
	bool bNativelyPresented = true;
#ifdef	D3D11_WITH_DWMAPI
	// We can't call Present if !bIsValid, as it waits a window message to be processed, but the main thread may not be pumping the message handler.
	if (bIsValid)
	{
		// Check if the viewport's swap chain has been invalidated by DXGI.
		BOOL bSwapChainFullscreenState;
		TRefCountPtr<IDXGIOutput> SwapChainOutput;
		VERIFYD3D11RESULT(SwapChain->GetFullscreenState(&bSwapChainFullscreenState, SwapChainOutput.GetInitReference()));
		// Can't compare BOOL with bool...
		if ((!!bSwapChainFullscreenState) != bIsFullscreen)
		{
			bIsValid = false;

			// Minimize the window.
			// use SW_FORCEMINIMIZE if the messaging thread is likely to be blocked for a sizeable period.
			// SW_FORCEMINIMIZE also prevents the minimize animation from playing.
			::ShowWindow(WindowHandle, SW_MINIMIZE);
		}
	}

	if (MaximumFrameLatency != RHIConsoleVariables::MaximumFrameLatency)
	{
		MaximumFrameLatency = RHIConsoleVariables::MaximumFrameLatency;
		TRefCountPtr<IDXGIDevice1> DXGIDevice;
		VERIFYD3D11RESULT(D3DRHI->GetDevice()->QueryInterface(__uuidof(IDXGIDevice), (void**)DXGIDevice.GetInitReference()));
		DXGIDevice->SetMaximumFrameLatency(MaximumFrameLatency);
	}

	// When desktop composition is enabled, locking to vsync via the Present
	// call is unreliable. Instead, communicate with the desktop window manager
	// directly to enable vsync.
	const bool bSyncWithDWM = bLockToVsync && !bIsFullscreen && RHIConsoleVariables::bSyncWithDWM && IsCompositionEnabled();
	if (bSyncWithDWM)
	{
		PresentWithVsyncDWM();
	}
	else
#endif	//D3D11_WITH_DWMAPI
	{
		//1-4
		uint32 SyncInterval = 1;
		// Present the back buffer to the viewport window.
		bNativelyPresented = PresentChecked(bLockToVsync ? SyncInterval : 0);
	}
	return bNativelyPresented;
}

/*=============================================================================
*	The following RHI functions must be called from the main thread.
*=============================================================================*/
RBViewportRHIRef FD3D11DynamicRHI::RHICreateViewport(void* WindowHandle, uint32 SizeX, uint32 SizeY, bool bIsFullscreen, EPixelFormat PreferredPixelFormat)
{
  //CHECK(IsInGameThread());

  // Use a default pixel format if none was specified	
  if (PreferredPixelFormat == EPixelFormat::PF_Unknown)
  {
    PreferredPixelFormat = EPixelFormat::PF_A2B10G10R10;
  }

  return new RBD3D11Viewport(this, (HWND)WindowHandle, SizeX, SizeY, bIsFullscreen, PreferredPixelFormat);
}

void FD3D11DynamicRHI::RHIResizeViewport(RBViewportRHIParamRef ViewportRHI, uint32 SizeX, uint32 SizeY, bool bIsFullscreen)
{
  RBD3D11Viewport* Viewport = (RBD3D11Viewport*)(ViewportRHI);

  //CHECK(IsInGameThread());
  Viewport->Resize(SizeX, SizeY, bIsFullscreen);
}

