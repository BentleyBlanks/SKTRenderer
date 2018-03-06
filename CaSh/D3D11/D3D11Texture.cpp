#include "D3D11Resources.h"
#include "RenderUtils.h"
#include "RHID3D11.h"

/**
* Returns a texture to its pool.
*/
void ReturnPooledTexture2D(int32 MipCount, EPixelFormat PixelFormat, ID3D11Texture2D* InResource)
{
#ifdef USE_TEXTURE_POOLING
	FTexturePool* Pool = GetTexturePool(MipCount, PixelFormat);
	if (Pool)
	{
		FPooledTexture2D* PooledTexture = new(Pool->Textures) FPooledTexture2D;
		PooledTexture->Resource = InResource;
		{
			D3D11_TEXTURE2D_DESC Desc;
			PooledTexture->Resource->GetDesc(&Desc);
			CHECK(Desc.Format == GPixelFormats[PixelFormat].PlatformFormat);
			CHECK(MipCount == Desc.MipLevels);
			CHECK(Desc.Width == Desc.Height);
			CHECK(Desc.Width == (1 << (MipCount - 1)));
			int32 TextureSize = CalcTextureSize(Desc.Width, Desc.Height, PixelFormat, Desc.MipLevels);
			INC_MEMORY_STAT_BY(STAT_D3D11TexturePoolMemory, TextureSize);
		}
	}
#endif // #if USE_TEXTURE_POOLING
}


template<typename BaseResourceType>
void D3D11TextureAllocated(TD3D11Texture2D<BaseResourceType>& Texture)
{
  ID3D11Texture2D* D3D11Texture2D = Texture.GetResource();

  if (D3D11Texture2D)
  {
      D3D11_TEXTURE2D_DESC Desc;

      D3D11Texture2D->GetDesc(&Desc);
      CHECK(Texture.IsCubemap() == ((Desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) != 0));

      int64 TextureSize = CalcTextureSize(Desc.Width, Desc.Height, Texture.GetFormat(), Desc.MipLevels) * Desc.ArraySize;

      Texture.SetMemorySize(TextureSize);

  }
  
}

/*
template<typename BaseResourceType>
void D3D11TextureDeleted(TD3D11Texture2D<BaseResourceType>& Texture)
{
	ID3D11Texture2D* D3D11Texture2D = Texture.GetResource();

	if (D3D11Texture2D)
	{
		D3D11_TEXTURE2D_DESC Desc;

		D3D11Texture2D->GetDesc(&Desc);
		CHECK(Texture.IsCubemap() == ((Desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) != 0));

		// When using virtual textures use the current memory size, which is the number of physical pages allocated, not virtual
		int64 TextureSize = 0;
		{
			TextureSize = CalcTextureSize(Desc.Width, Desc.Height, Texture.GetFormat(), Desc.MipLevels) * Desc.ArraySize;
		}

	}
}
*/

void D3D11TextureAllocated2D(RBD3D11Texture2D& Texture)
{
  D3D11TextureAllocated(Texture);
}

/*

*/

template<typename RHIResourceType>
void* TD3D11Texture2D<RHIResourceType>::Lock(uint32 MipIndex, uint32 ArrayIndex, EResourceLockMode LockMode, uint32& DestStride)
{
	//SCOPE_CYCLE_COUNTER(STAT_D3D11LockTextureTime);

	// Calculate the subresource index corresponding to the specified mip-map.
	const uint32 Subresource = D3D11CalcSubresource(MipIndex, ArrayIndex, GetNumMips());

	// Calculate the dimensions of the mip-map.
	const uint32 BlockSizeX = GPixelFormats[GetFormat()].BlockSizeX;
	const uint32 BlockSizeY = GPixelFormats[GetFormat()].BlockSizeY;
	const uint32 BlockBytes = GPixelFormats[GetFormat()].BlockBytes;
	const uint32 MipSizeX = RBMath::get_max(GetSizeX() >> MipIndex, BlockSizeX);
	const uint32 MipSizeY = RBMath::get_max(GetSizeY() >> MipIndex, BlockSizeY);
	const uint32 NumBlocksX = (MipSizeX + BlockSizeX - 1) / BlockSizeX;
	const uint32 NumBlocksY = (MipSizeY + BlockSizeY - 1) / BlockSizeY;
	const uint32 MipBytes = NumBlocksX * NumBlocksY * BlockBytes;

	FD3D11LockedData LockedData;
		if (LockMode == RLM_WriteOnly)
		{
			// If we're writing to the texture, allocate a system memory buffer to receive the new contents.
			LockedData.AllocData(MipBytes);
			LockedData.Pitch = DestStride = NumBlocksX * BlockBytes;
		}
		else
		{
			// If we're reading from the texture, we create a staging resource, copy the texture contents to it, and map it.

			// Create the staging texture.
			D3D11_TEXTURE2D_DESC StagingTextureDesc;
			GetResource()->GetDesc(&StagingTextureDesc);
			StagingTextureDesc.Width = MipSizeX;
			StagingTextureDesc.Height = MipSizeY;
			StagingTextureDesc.MipLevels = 1;
			StagingTextureDesc.ArraySize = 1;
			StagingTextureDesc.Usage = D3D11_USAGE_STAGING;
			StagingTextureDesc.BindFlags = 0;
			StagingTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			StagingTextureDesc.MiscFlags = 0;
			TRefCountPtr<ID3D11Texture2D> StagingTexture;
			VERIFYD3D11CREATETEXTURERESULT(
				D3DRHI->GetDevice()->CreateTexture2D(&StagingTextureDesc, NULL, StagingTexture.GetInitReference()),
				GetSizeX(),
				GetSizeY(),
				GetSizeZ(),
				StagingTextureDesc.Format,
				1,
				0,
				D3DRHI->GetDevice()
				);
			LockedData.StagingResource = StagingTexture;

			// Copy the mip-map data from the real resource into the staging resource
			D3DRHI->GetDeviceContext()->CopySubresourceRegion(StagingTexture, 0, 0, 0, 0, GetResource(), Subresource, NULL);

			// Map the staging resource, and return the mapped address.
			D3D11_MAPPED_SUBRESOURCE MappedTexture;
			VERIFYD3D11RESULT(D3DRHI->GetDeviceContext()->Map(StagingTexture, 0, D3D11_MAP_READ, 0, &MappedTexture));
			LockedData.SetData(MappedTexture.pData);
			LockedData.Pitch = DestStride = MappedTexture.RowPitch;
		}

	// Add the lock to the outstanding lock list.
	D3DRHI->OutstandingLocks.Add(FD3D11LockedKey(GetResource(), Subresource), LockedData);

	return (void*)LockedData.GetData();
}

template<typename RHIResourceType>
void TD3D11Texture2D<RHIResourceType>::Unlock(uint32 MipIndex, uint32 ArrayIndex)
{
	//SCOPE_CYCLE_COUNTER(STAT_D3D11UnlockTextureTime);

	// Calculate the subresource index corresponding to the specified mip-map.
	const uint32 Subresource = D3D11CalcSubresource(MipIndex, ArrayIndex, GetNumMips());

	// Find the object that is tracking this lock
	const FD3D11LockedKey LockedKey(GetResource(), Subresource);
	FD3D11LockedData* LockedData = D3DRHI->OutstandingLocks.Find(LockedKey);
	CHECK(LockedData);

#ifdef PLATFORM_SUPPORTS_VIRTUAL_TEXTURES
	if (D3DRHI->HandleSpecialUnlock(MipIndex, GetFlags(), GetResource(), RawTextureMemory))
	{
		// nothing left to do...
	}
	else
#endif
		if (!LockedData->StagingResource)
		{
			// If we're writing, we need to update the subresource
			D3DRHI->GetDeviceContext()->UpdateSubresource(GetResource(), Subresource, NULL, LockedData->GetData(), LockedData->Pitch, 0);
			LockedData->FreeData();
		}

	// Remove the lock from the outstanding lock list.
	D3DRHI->OutstandingLocks.Remove(LockedKey);
}


/**
* Creates a 2D texture optionally guarded by a structured exception handler.
*/
void SafeCreateTexture2D(ID3D11Device* Direct3DDevice, const D3D11_TEXTURE2D_DESC* TextureDesc, const D3D11_SUBRESOURCE_DATA* SubResourceData, ID3D11Texture2D** OutTexture2D)
{
#if GUARDED_TEXTURE_CREATES
	bool bDriverCrash = true;
	__try
	{
#endif // #if GUARDED_TEXTURE_CREATES
		VERIFYD3D11CREATETEXTURERESULT(
			Direct3DDevice->CreateTexture2D(TextureDesc, SubResourceData, OutTexture2D),
			TextureDesc->Width,
			TextureDesc->Height,
			TextureDesc->ArraySize,
			TextureDesc->Format,
			TextureDesc->MipLevels,
			TextureDesc->BindFlags,
			Direct3DDevice
			);
#if GUARDED_TEXTURE_CREATES
		bDriverCrash = false;
	}
	__finally
	{
		if (bDriverCrash)
		{
			UE_LOG(LogD3D11RHI, Error,
				TEXT("Driver crashed while creating texture: %ux%ux%u %s(0x%08x) with %u mips"),
				TextureDesc->Width,
				TextureDesc->Height,
				TextureDesc->ArraySize,
				GetD3D11TextureFormatString(TextureDesc->Format),
				(uint32)TextureDesc->Format,
				TextureDesc->MipLevels
				);
		}
	}
#endif // #if GUARDED_TEXTURE_CREATES
}


template<typename BaseResourceType>
TD3D11Texture2D<BaseResourceType>* FD3D11DynamicRHI::CreateD3D11Texture2D(uint32 SizeX, uint32 SizeY, uint32 SizeZ, bool bTextureArray, bool bCubeTexture, uint8 Format,
	uint32 NumMips, uint32 NumSamples, uint32 Flags, RBRHIResourceCreateInfo& CreateInfo)
{
	CHECK(SizeX > 0 && SizeY > 0 && NumMips > 0);

	if (bCubeTexture)
	{
		CHECK(SizeX <= D3D11_REQ_TEXTURECUBE_DIMENSION);
		CHECK(SizeX == SizeY);
	}
	else
	{
		CHECK(SizeX <= D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION);
		CHECK(SizeY <= D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION);
	}

	if (bTextureArray)
	{
		CHECK(SizeZ <= D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION);
	}

	// Render target allocation with UAV flag will silently fail in feature level 10
	//CHECK(!(Flags & TexCreate_UAV));

	bool bPooledTexture = true;

	if (0)//GMaxRHIFeatureLevel <= ERHIFeatureLevel::ES3_1)
	{
		// Remove sRGB read flag when not supported
		Flags &= ~TexCreate_SRGB;
	}

	const bool bSRGB = (Flags & TexCreate_SRGB) != 0;

	const DXGI_FORMAT PlatformResourceFormat = (DXGI_FORMAT)GPixelFormats[Format].PlatformFormat;
	const DXGI_FORMAT PlatformShaderResourceFormat = FindShaderResourceDXGIFormat(PlatformResourceFormat, bSRGB);
	const DXGI_FORMAT PlatformRenderTargetFormat = FindShaderResourceDXGIFormat(PlatformResourceFormat, bSRGB);

	// Determine the MSAA settings to use for the texture.
	D3D11_DSV_DIMENSION DepthStencilViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	D3D11_RTV_DIMENSION RenderTargetViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	D3D11_SRV_DIMENSION ShaderResourceViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	uint32 CPUAccessFlags = 0;
	D3D11_USAGE TextureUsage = D3D11_USAGE_DEFAULT;
	uint32 BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bool bCreateShaderResource = true;

	uint32 ActualMSAACount = NumSamples;

	uint32 ActualMSAAQuality = GetMaxMSAAQuality(ActualMSAACount);

	// 0xffffffff means not supported
	if (ActualMSAAQuality == 0xffffffff || (Flags & TexCreate_Shared) != 0)
	{
		// no MSAA
		ActualMSAACount = 1;
		ActualMSAAQuality = 0;
	}

	if (ActualMSAACount > 1)
	{
		DepthStencilViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		RenderTargetViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
		ShaderResourceViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
		bPooledTexture = false;
	}

	if (NumMips < 1 || SizeX != SizeY || (1 << (NumMips - 1)) != SizeX || (Flags & TexCreate_Shared) != 0)
	{
		bPooledTexture = false;
	}

	if (Flags & TexCreate_CPUReadback)
	{
		CHECK(!(Flags & TexCreate_RenderTargetable));
		CHECK(!(Flags & TexCreate_DepthStencilTargetable));
		CHECK(!(Flags & TexCreate_ShaderResource));

		CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		TextureUsage = D3D11_USAGE_STAGING;
		BindFlags = 0;
		bCreateShaderResource = false;
	}

	// Describe the texture.
	D3D11_TEXTURE2D_DESC TextureDesc;
	ZeroMemory(&TextureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	TextureDesc.Width = SizeX;
	TextureDesc.Height = SizeY;
	TextureDesc.MipLevels = NumMips;
	TextureDesc.ArraySize = SizeZ;
	TextureDesc.Format = PlatformResourceFormat;
	TextureDesc.SampleDesc.Count = ActualMSAACount;
	TextureDesc.SampleDesc.Quality = ActualMSAAQuality;
	TextureDesc.Usage = TextureUsage;
	TextureDesc.BindFlags = BindFlags;
	TextureDesc.CPUAccessFlags = CPUAccessFlags;
	TextureDesc.MiscFlags = bCubeTexture ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;

	if (Flags & TexCreate_Shared)
	{
		//¶àdevice·ÖÏíÎÆÀí
		TextureDesc.MiscFlags |= D3D11_RESOURCE_MISC_SHARED;
	}

	if (Flags & TexCreate_GenerateMipCapable)
	{
		// Set the flag that allows us to call GenerateMips on this texture later
		TextureDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		bPooledTexture = false;
	}

	// Set up the texture bind flags.
	bool bCreateRTV = false;
	bool bCreateDSV = false;
	bool bCreatedRTVPerSlice = false;

	if (Flags & TexCreate_RenderTargetable)
	{
		CHECK(!(Flags & TexCreate_DepthStencilTargetable));
		CHECK(!(Flags & TexCreate_ResolveTargetable));
		TextureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		bCreateRTV = true;
	}
	else if (Flags & TexCreate_DepthStencilTargetable)
	{
		CHECK(!(Flags & TexCreate_RenderTargetable));
		CHECK(!(Flags & TexCreate_ResolveTargetable));
		TextureDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
		bCreateDSV = true;
	}
	else if (Flags & TexCreate_ResolveTargetable)
	{
		CHECK(!(Flags & TexCreate_RenderTargetable));
		CHECK(!(Flags & TexCreate_DepthStencilTargetable));
		if (Format == PF_DepthStencil || Format == PF_ShadowDepth || Format == PF_D24)
		{
			TextureDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
			bCreateDSV = true;
		}
		else
		{
			TextureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
			bCreateRTV = true;
		}
	}

	if (Flags & TexCreate_UAV)
	{
		TextureDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		bPooledTexture = false;
	}

	if (bCreateDSV || bCreateRTV || bCubeTexture || bTextureArray)
	{
		bPooledTexture = false;
	}

	RBVRamAllocation VRamAllocation;

	if (/*FPlatformProperties::SupportsFastVRAMMemory()*/false)
	{
		if (Flags & TexCreate_FastVRAM)
		{
			//VRamAllocation = FFastVRAMAllocator::GetFastVRAMAllocator()->AllocTexture2D(TextureDesc);
		}
	}

	TRefCountPtr<ID3D11Texture2D> TextureResource;
	TRefCountPtr<ID3D11ShaderResourceView> ShaderResourceView;
	std::vector<TRefCountPtr<ID3D11RenderTargetView> > RenderTargetViews;
	TRefCountPtr<ID3D11DepthStencilView> DepthStencilViews[RBExclusiveDepthStencil::MaxIndex];

#if PLATFORM_SUPPORTS_VIRTUAL_TEXTURES
	// Turn off pooling when we are using virtual textures or the texture is offline processed as we control when the memory is released
	if ((Flags & (TexCreate_Virtual | TexCreate_OfflineProcessed)) != 0)
	{
		bPooledTexture = false;
	}
	void* RawTextureMemory = nullptr;
#else
	Flags &= ~TexCreate_Virtual;
#endif

	//we dont need pool texture now
	if (bPooledTexture)
	{
		/*
		RBPooledTexture2D PooledTexture;
		if (GetPooledTexture2D(NumMips, (EPixelFormat)Format, &PooledTexture))
		{
			TextureResource = PooledTexture.Resource;
		}
		*/
	}

	if (!IsValidRef(TextureResource))
	{
		std::vector<D3D11_SUBRESOURCE_DATA> SubResourceData;

		if (CreateInfo.BulkData)
		{
			uint8* Data = (uint8*)CreateInfo.BulkData->GetResourceBulkData();

			// each mip of each array slice counts as a subresource
			SubResourceData.resize(NumMips * SizeZ);
			rb_memzero(&SubResourceData[0], SubResourceData.size()*sizeof(D3D11_SUBRESOURCE_DATA));

			uint32 SliceOffset = 0;
			for (uint32 ArraySliceIndex = 0; ArraySliceIndex < SizeZ; ++ArraySliceIndex)
			{
				uint32 MipOffset = 0;
				for (uint32 MipIndex = 0; MipIndex < NumMips; ++MipIndex)
				{
					uint32 DataOffset = SliceOffset + MipOffset;
					uint32 SubResourceIndex = ArraySliceIndex * NumMips + MipIndex;

					uint32 NumBlocksX = RBMath::get_max<uint32>(1, (SizeX >> MipIndex) / GPixelFormats[Format].BlockSizeX);
					uint32 NumBlocksY = RBMath::get_max<uint32>(1, (SizeY >> MipIndex) / GPixelFormats[Format].BlockSizeY);

					SubResourceData[SubResourceIndex].pSysMem = &Data[DataOffset];
					SubResourceData[SubResourceIndex].SysMemPitch = NumBlocksX * GPixelFormats[Format].BlockBytes;
					SubResourceData[SubResourceIndex].SysMemSlicePitch = NumBlocksX * NumBlocksY * SubResourceData[MipIndex].SysMemPitch;

					MipOffset += NumBlocksY * SubResourceData[MipIndex].SysMemPitch;
				}
				SliceOffset += MipOffset;
			}
		}

#if PLATFORM_SUPPORTS_VIRTUAL_TEXTURES
		if ((Flags & (TexCreate_Virtual | TexCreate_OfflineProcessed)) != 0)
		{
			RawTextureMemory = CreateVirtualTexture(SizeX, SizeY, SizeZ, NumMips, bCubeTexture, Flags, &TextureDesc, &TextureResource);
		}
		else
#endif
		{
			SafeCreateTexture2D(Direct3DDevice, &TextureDesc, CreateInfo.BulkData != NULL ? (const D3D11_SUBRESOURCE_DATA*)SubResourceData.data() : NULL, TextureResource.GetInitReference());
		}

		if (bCreateRTV)
		{
			// Create a render target view for each mip
			for (uint32 MipIndex = 0; MipIndex < NumMips; MipIndex++)
			{
				if ((Flags & TexCreate_TargetArraySlicesIndependently) && (bTextureArray || bCubeTexture))
				{
					bCreatedRTVPerSlice = true;

					for (uint32 SliceIndex = 0; SliceIndex < TextureDesc.ArraySize; SliceIndex++)
					{
						D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
						rb_memzero(&RTVDesc, sizeof(RTVDesc));
						RTVDesc.Format = PlatformRenderTargetFormat;
						RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
						RTVDesc.Texture2DArray.FirstArraySlice = SliceIndex;
						RTVDesc.Texture2DArray.ArraySize = 1;
						RTVDesc.Texture2DArray.MipSlice = MipIndex;

						TRefCountPtr<ID3D11RenderTargetView> RenderTargetView;
						VERIFYD3D11RESULT(Direct3DDevice->CreateRenderTargetView(TextureResource, &RTVDesc, RenderTargetView.GetInitReference()));
						RenderTargetViews.push_back(RenderTargetView);
					}
				}
				else
				{
					D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
					rb_memzero(&RTVDesc, sizeof(RTVDesc));
					RTVDesc.Format = PlatformRenderTargetFormat;
					if (bTextureArray || bCubeTexture)
					{
						RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
						RTVDesc.Texture2DArray.FirstArraySlice = 0;
						RTVDesc.Texture2DArray.ArraySize = TextureDesc.ArraySize;
						RTVDesc.Texture2DArray.MipSlice = MipIndex;
					}
					else
					{
						RTVDesc.ViewDimension = RenderTargetViewDimension;
						RTVDesc.Texture2D.MipSlice = MipIndex;
					}

					TRefCountPtr<ID3D11RenderTargetView> RenderTargetView;
					VERIFYD3D11RESULT(Direct3DDevice->CreateRenderTargetView(TextureResource, &RTVDesc, RenderTargetView.GetInitReference()));
					RenderTargetViews.push_back(RenderTargetView);
				}
			}
		}

		if (bCreateDSV)
		{
			// Create a depth-stencil-view for the texture.
			D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc;
			rb_memzero(&DSVDesc, sizeof(DSVDesc));
			DSVDesc.Format = FindDepthStencilDXGIFormat(PlatformResourceFormat);
			if (bTextureArray || bCubeTexture)
			{
				DSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
				DSVDesc.Texture2DArray.FirstArraySlice = 0;
				DSVDesc.Texture2DArray.ArraySize = TextureDesc.ArraySize;
				DSVDesc.Texture2DArray.MipSlice = 0;
			}
			else
			{
				DSVDesc.ViewDimension = DepthStencilViewDimension;
				DSVDesc.Texture2D.MipSlice = 0;
			}

			for (uint32 AccessType = 0; AccessType < RBExclusiveDepthStencil::MaxIndex; ++AccessType)
			{
				// Create a read-only access views for the texture.
				// Read-only DSVs are not supported in Feature Level 10 so 
				// a dummy DSV is created in order reduce logic complexity at a higher-level.
				if (Direct3DDevice->GetFeatureLevel() == D3D_FEATURE_LEVEL_11_0)
				{
					DSVDesc.Flags = (AccessType & RBExclusiveDepthStencil::DepthRead_StencilWrite) ? D3D11_DSV_READ_ONLY_DEPTH : 0;
					if (HasStencilBits(DSVDesc.Format))
					{
						DSVDesc.Flags |= (AccessType & RBExclusiveDepthStencil::DepthWrite_StencilRead) ? D3D11_DSV_READ_ONLY_STENCIL : 0;
					}
				}
				VERIFYD3D11RESULT(Direct3DDevice->CreateDepthStencilView(TextureResource, &DSVDesc, DepthStencilViews[AccessType].GetInitReference()));
			}
		}
	}
	CHECK(IsValidRef(TextureResource));

	// Create a shader resource view for the texture.
	if (bCreateShaderResource)
	{
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
			SRVDesc.Format = PlatformShaderResourceFormat;

			if (bCubeTexture && bTextureArray)
			{
				SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
				SRVDesc.TextureCubeArray.MostDetailedMip = 0;
				SRVDesc.TextureCubeArray.MipLevels = NumMips;
				SRVDesc.TextureCubeArray.First2DArrayFace = 0;
				SRVDesc.TextureCubeArray.NumCubes = SizeZ / 6;
			}
			else if (bCubeTexture)
			{
				SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
				SRVDesc.TextureCube.MostDetailedMip = 0;
				SRVDesc.TextureCube.MipLevels = NumMips;
			}
			else if (bTextureArray)
			{
				SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
				SRVDesc.Texture2DArray.MostDetailedMip = 0;
				SRVDesc.Texture2DArray.MipLevels = NumMips;
				SRVDesc.Texture2DArray.FirstArraySlice = 0;
				SRVDesc.Texture2DArray.ArraySize = TextureDesc.ArraySize;
			}
			else
			{
				SRVDesc.ViewDimension = ShaderResourceViewDimension;
				SRVDesc.Texture2D.MostDetailedMip = 0;
				SRVDesc.Texture2D.MipLevels = NumMips;
			}
			VERIFYD3D11RESULT(Direct3DDevice->CreateShaderResourceView(TextureResource, &SRVDesc, ShaderResourceView.GetInitReference()));
		}

		CHECK(IsValidRef(ShaderResourceView));
	}

	TD3D11Texture2D<BaseResourceType>* Texture2D = new TD3D11Texture2D<BaseResourceType>(
		this,
		TextureResource,
		ShaderResourceView,
		bCreatedRTVPerSlice,
		TextureDesc.ArraySize,
		RenderTargetViews,
		DepthStencilViews,
		SizeX,
		SizeY,
		SizeZ,
		NumMips,
		ActualMSAACount,
		(EPixelFormat)Format,
		bCubeTexture,
		Flags,
		bPooledTexture,
		CreateInfo.ClearValueBinding
#if PLATFORM_SUPPORTS_VIRTUAL_TEXTURES
		, RawTextureMemory
#endif
		);

	Texture2D->ResourceInfo.VRamAllocation = VRamAllocation;

	if (Flags & TexCreate_RenderTargetable)
	{
		Texture2D->SetCurrentGPUAccess(EResourceTransitionAccess::EWritable);
	}

	D3D11TextureAllocated(*Texture2D);

	return Texture2D;
}


//CMD
RBTexture2DRHIRef FD3D11DynamicRHI::RHICreateTexture2D(uint32 SizeX, uint32 SizeY, uint8 Format, uint32 NumMips, uint32 NumSamples, uint32 Flags, RBRHIResourceCreateInfo& CreateInfo)
{
	return CreateD3D11Texture2D<RBD3D11BaseTexture2D>(SizeX, SizeY, 1, false, false, Format, NumMips, NumSamples, Flags, CreateInfo);
}
