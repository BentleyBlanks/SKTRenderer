#pragma once
#include "d3d11.h"
#include "PixelFormat.h"
#include "Vector2.h"
#include "D3D11Resources.h"


static DXGI_FORMAT GetRenderTargetFormat(EPixelFormat PixelFormat)
{
	DXGI_FORMAT	DXFormat = (DXGI_FORMAT)GPixelFormats[PixelFormat].PlatformFormat;
	switch (DXFormat)
	{
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:		return DXGI_FORMAT_B8G8R8A8_UNORM;
	case DXGI_FORMAT_BC1_TYPELESS:			return DXGI_FORMAT_BC1_UNORM;
	case DXGI_FORMAT_BC2_TYPELESS:			return DXGI_FORMAT_BC2_UNORM;
	case DXGI_FORMAT_BC3_TYPELESS:			return DXGI_FORMAT_BC3_UNORM;
	case DXGI_FORMAT_R16_TYPELESS:			return DXGI_FORMAT_R16_UNORM;
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:		return DXGI_FORMAT_R8G8B8A8_UNORM;
	default: 								return DXFormat;
	}
}


class RBD3D11Viewport : public RBRHIViewport
{
public:
	RBD3D11Viewport(class FD3D11DynamicRHI* InD3DRHI, HWND InWindowHandle, uint32 InSizeX, uint32 InSizeY, bool bInIsFullscreen, EPixelFormat InPreferredPixelFormat);
	~RBD3D11Viewport();

	void Resize(uint32 InSizeX, uint32 InSizeY, bool bInIsFullscreen);

	/**
	* If the swap chain has been invalidated by DXGI, resets the swap chain to the expected state; otherwise, does nothing.
	* Called once/frame by the game thread on all viewports.
	* @param bIgnoreFocus - Whether the reset should happen regardless of whether the window is focused.
	*/
	void ConditionalResetSwapChain(bool bIgnoreFocus);

	/** Presents the swap chain.
	* Returns true if Present was done by Engine.
	*/
	bool Present(bool bLockToVsync);

	// Accessors.
	RBVector2I GetSizeXY() const { return RBVector2I(SizeX, SizeY); }
	RBD3D11Texture2D* GetBackBuffer() const { return BackBuffer; }

	IDXGISwapChain* GetSwapChain() const { return SwapChain; }

	virtual void* GetNativeSwapChain() const override { return GetSwapChain(); }
	virtual void* GetNativeBackBufferTexture() const override { return BackBuffer->GetResource(); }
	virtual void* GetNativeBackBufferRT() const override { return BackBuffer->GetRenderTargetView(0, 0); }

	virtual void SetCustomPresent(RBRHICustomPresent* InCustomPresent) override
	{
		CustomPresent = InCustomPresent;
	}
	virtual RBRHICustomPresent* GetCustomPresent() const { return CustomPresent; }

	virtual void* GetNativeWindow(void** AddParam = nullptr) const override { return (void*)WindowHandle; }

private:

	/** Presents the frame synchronizing with DWM. */
	void PresentWithVsyncDWM();

	/**
	* Presents the swap chain checking the return result.
	* Returns true if Present was done by Engine.
	*/
	bool PresentChecked(int32 SyncInterval);

	FD3D11DynamicRHI* D3DRHI;
	u64 LastFlipTime;
	u64 LastFrameComplete;
	u64 LastCompleteTime;
	int32 SyncCounter;
	bool bSyncedLastFrame;
	HWND WindowHandle;
	uint32 MaximumFrameLatency;
	uint32 SizeX;
	uint32 SizeY;
	bool bIsFullscreen;
	EPixelFormat PixelFormat;
	bool bIsValid;
	TRefCountPtr<IDXGISwapChain> SwapChain;
	TRefCountPtr<RBD3D11Texture2D> BackBuffer;

	FCustomPresentRHIRef CustomPresent;

	DXGI_MODE_DESC SetupDXGI_MODE_DESC() const;
};

/*
template<>
struct TD3D11ResourceTraits < RBRHIViewport >
{
	typedef RBD3D11Viewport TConcreteType;
};
*/