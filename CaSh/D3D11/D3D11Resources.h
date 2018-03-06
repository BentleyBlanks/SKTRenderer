#pragma once

#include "RHIResources.h"
#include <vector>
#include "d3d11.h"
#include "D3D11BoundShaderStateCache.h"


/** Convenience typedef: preallocated array of D3D11 input element descriptions. */
typedef std::vector<D3D11_INPUT_ELEMENT_DESC> RBD3D11VertexElements;

/** This represents a vertex declaration that hasn't been combined with a specific shader to create a bound shader. */
class RBD3D11VertexDeclaration : public RBRHIVertexDeclaration
{
public:
	/** Elements of the vertex declaration. */
	RBD3D11VertexElements VertexElements;

	/** Initialization constructor. */
	explicit RBD3D11VertexDeclaration(const RBD3D11VertexElements& InElements)
		: VertexElements(InElements)
	{
	}
};

struct RBBaseShaderResourceTable
{
	/** Bits indicating which resource tables contain resources bound to this shader. */
	uint32 ResourceTableBits;

	/** Mapping of bound SRVs to their location in resource tables. */
	std::vector<uint32> ShaderResourceViewMap;

	/** Mapping of bound sampler states to their location in resource tables. */
	std::vector<uint32> SamplerMap;

	/** Mapping of bound UAVs to their location in resource tables. */
	std::vector<uint32> UnorderedAccessViewMap;

	/** Hash of the layouts of resource tables at compile time, used for runtime validation. */
	std::vector<uint32> ResourceTableLayoutHashes;

	RBBaseShaderResourceTable() :
		ResourceTableBits(0)
	{
	}

	friend bool operator==(const RBBaseShaderResourceTable &A, const RBBaseShaderResourceTable& B)
	{
		bool bEqual = true;
		bEqual &= (A.ResourceTableBits == B.ResourceTableBits);
		bEqual &= (A.ShaderResourceViewMap.size() == B.ShaderResourceViewMap.size());
		bEqual &= (A.SamplerMap.size() == B.SamplerMap.size());
		bEqual &= (A.UnorderedAccessViewMap.size() == B.UnorderedAccessViewMap.size());
		bEqual &= (A.ResourceTableLayoutHashes.size() == B.ResourceTableLayoutHashes.size());
		if (!bEqual)
		{
			return false;
		}
		bEqual &= (::memcmp(A.ShaderResourceViewMap.data(), B.ShaderResourceViewMap.data(), sizeof(uint32)*A.ShaderResourceViewMap.size()) == 0);
		bEqual &= (::memcmp(A.SamplerMap.data(), B.SamplerMap.data(), sizeof(uint32)*A.SamplerMap.size()) == 0);
		bEqual &= (::memcmp(A.UnorderedAccessViewMap.data(), B.UnorderedAccessViewMap.data(), sizeof(uint32)*A.UnorderedAccessViewMap.size()) == 0);
		bEqual &= (::memcmp(A.ResourceTableLayoutHashes.data(), B.ResourceTableLayoutHashes.data(), sizeof(uint32)*A.ResourceTableLayoutHashes.size()) == 0);
		return bEqual;
	}
};

struct RBD3D11ShaderResourceTable
  : public RBBaseShaderResourceTable
{
  /** Mapping of bound Textures to their location in resource tables. */
  std::vector<uint32> TextureMap;

  friend bool operator==(const RBD3D11ShaderResourceTable& A, const RBD3D11ShaderResourceTable& B)
  {
    const RBBaseShaderResourceTable& BaseA = A;
    const RBBaseShaderResourceTable& BaseB = B;
    return A == B && (::memcmp(A.TextureMap.data(), B.TextureMap.data(), sizeof(
		uint32)*A.TextureMap.size()) == 0);
  }
};

struct RBShaderCompilerResourceTable
{
	/** Bits indicating which resource tables contain resources bound to this shader. */
	uint32 ResourceTableBits;

	/** The max index of a uniform buffer from which resources are bound. */
	uint32 MaxBoundResourceTable;

	/** Mapping of bound Textures to their location in resource tables. */
	std::vector<uint32> TextureMap;

	/** Mapping of bound SRVs to their location in resource tables. */
	std::vector<uint32> ShaderResourceViewMap;

	/** Mapping of bound sampler states to their location in resource tables. */
	std::vector<uint32> SamplerMap;

	/** Mapping of bound UAVs to their location in resource tables. */
	std::vector<uint32> UnorderedAccessViewMap;

	/** Hash of the layouts of resource tables at compile time, used for runtime validation. */
	std::vector<uint32> ResourceTableLayoutHashes;

	RBShaderCompilerResourceTable()
		: ResourceTableBits(0)
		, MaxBoundResourceTable(0)
	{
	}
};

struct RBD3D11ShaderData
{
	RBD3D11ShaderResourceTable	ShaderResourceTable;
	std::vector<std::string>				UniformBuffers;
	bool						bShaderNeedsGlobalConstantBuffer;
};

/** This represents a vertex shader that hasn't been combined with a specific declaration to create a bound shader. */
class FD3D11VertexShader : public RBRHIVertexShader, public RBD3D11ShaderData
{
public:
  enum { StaticFrequency = SF_Vertex };

  /** The vertex shader resource. */
  TRefCountPtr<ID3D11VertexShader> Resource;

  /** The vertex shader's bytecode, with custom data attached. */
  std::vector<uint8> Code;

  // TEMP remove with removal of bound shader state
  int32 Offset;
};

class FD3D11GeometryShader : public RBRHIGeometryShader, public RBD3D11ShaderData
{
public:
  enum { StaticFrequency = SF_Geometry };

  /** The shader resource. */
  TRefCountPtr<ID3D11GeometryShader> Resource;
};

class FD3D11HullShader : public RBRHIHullShader, public RBD3D11ShaderData
{
public:
  enum { StaticFrequency = SF_Hull };

  /** The shader resource. */
  TRefCountPtr<ID3D11HullShader> Resource;
};

class FD3D11DomainShader : public RBRHIDomainShader, public RBD3D11ShaderData
{
public:
  enum { StaticFrequency = SF_Domain };

  /** The shader resource. */
  TRefCountPtr<ID3D11DomainShader> Resource;
};

class FD3D11PixelShader : public RBRHIPixelShader, public RBD3D11ShaderData
{
public:
  enum { StaticFrequency = SF_Pixel };

  /** The shader resource. */
  TRefCountPtr<ID3D11PixelShader> Resource;
};

class FD3D11ComputeShader : public RBRHIComputeShader, public RBD3D11ShaderData
{
public:
  enum { StaticFrequency = SF_Compute };

  /** The shader resource. */
  TRefCountPtr<ID3D11ComputeShader> Resource;
};

/**
* Combined shader state and vertex definition for rendering geometry.
* Each unique instance consists of a vertex decl, vertex shader, and pixel shader.
*/
class FD3D11BoundShaderState : public RBRHIBoundShaderState
{
public:

  FCachedBoundShaderStateLink CacheLink;

  TRefCountPtr<ID3D11InputLayout> InputLayout;
  TRefCountPtr<ID3D11VertexShader> VertexShader;
  TRefCountPtr<ID3D11PixelShader> PixelShader;
  TRefCountPtr<ID3D11HullShader> HullShader;
  TRefCountPtr<ID3D11DomainShader> DomainShader;
  TRefCountPtr<ID3D11GeometryShader> GeometryShader;

  bool bShaderNeedsGlobalConstantBuffer[SF_NumFrequencies];


  /** Initialization constructor. */
  FD3D11BoundShaderState(
    RBVertexDeclarationRHIParamRef InVertexDeclarationRHI,
    RBVertexShaderRHIParamRef InVertexShaderRHI,
    RBPixelShaderRHIParamRef InPixelShaderRHI,
    RBHullShaderRHIParamRef InHullShaderRHI,
    RBDomainShaderRHIParamRef InDomainShaderRHI,
    RBGeometryShaderRHIParamRef InGeometryShaderRHI,
    ID3D11Device* Direct3DDevice
    );

  ~FD3D11BoundShaderState();

  /**
  * Get the shader for the given frequency.
  */
  
  FORCEINLINE FD3D11VertexShader*   GetVertexShader() const   { return (FD3D11VertexShader*)CacheLink.GetVertexShader(); }
  FORCEINLINE FD3D11PixelShader*    GetPixelShader() const    { return (FD3D11PixelShader*)CacheLink.GetPixelShader(); }
  FORCEINLINE FD3D11HullShader*     GetHullShader() const     { return (FD3D11HullShader*)CacheLink.GetHullShader(); }
  FORCEINLINE FD3D11DomainShader*   GetDomainShader() const   { return (FD3D11DomainShader*)CacheLink.GetDomainShader(); }
  FORCEINLINE FD3D11GeometryShader* GetGeometryShader() const { return (FD3D11GeometryShader*)CacheLink.GetGeometryShader(); }
  
};

/** The base class of resources that may be bound as shader resources. */
class RBD3D11BaseShaderResource : public IRefCountedObject
{
public:
	RBD3D11BaseShaderResource()
		: CurrentGPUAccess(EResourceTransitionAccess::EReadable)
		, LastFrameWritten(-1)
		, bDirty(false)
	{}

	void SetCurrentGPUAccess(EResourceTransitionAccess Access)
	{
		if (Access == EResourceTransitionAccess::EReadable)
		{
			bDirty = false;
		}
		CurrentGPUAccess = Access;
	}

	EResourceTransitionAccess GetCurrentGPUAccess() const
	{
		return CurrentGPUAccess;
	}

	u32 GetLastFrameWritten() const
	{
		return LastFrameWritten;
	}

	void SetDirty(bool bInDirty, u32 CurrentFrame)
	{
		bDirty = bInDirty;
		if (bDirty)
		{
			LastFrameWritten = CurrentFrame;
		}
		//CHECKF((GEnableDX11TransitionChecks == 0) || !(CurrentGPUAccess == EResourceTransitionAccess::EReadable && bDirty), TEXT("ShaderResource is dirty, but set to Readable."));
	}

	bool IsDirty() const
	{
		return bDirty;
	}

private:

	/** Whether the current resource is logically GPU readable or writable.  Mostly for validation for newer RHI's*/
	EResourceTransitionAccess CurrentGPUAccess;

	/** Most recent frame this resource was written to. */
	u32 LastFrameWritten;

	/** Resource has been written to without a subsequent read barrier.  Mostly for UAVs */
	bool bDirty;
};

/** Texture base class. */
class RBD3D11TextureBase : public RBD3D11BaseShaderResource
{
public:

	RBD3D11TextureBase(
	class FD3D11DynamicRHI* InD3DRHI,
		ID3D11Resource* InResource,
		ID3D11ShaderResourceView* InShaderResourceView,
		i32 InRTVArraySize,
		bool bInCreatedRTVsPerSlice,
		const std::vector<TRefCountPtr<ID3D11RenderTargetView> >& InRenderTargetViews,
		TRefCountPtr<ID3D11DepthStencilView>* InDepthStencilViews
		)
		: D3DRHI(InD3DRHI)
		, MemorySize(0)
		, BaseShaderResource(this)
		, Resource(InResource)
		, ShaderResourceView(InShaderResourceView)
		, RenderTargetViews(InRenderTargetViews)
		, bCreatedRTVsPerSlice(bInCreatedRTVsPerSlice)
		, RTVArraySize(InRTVArraySize)
		, NumDepthStencilViews(0)
	{
		// Set the DSVs for all the access type combinations
		if (InDepthStencilViews != nullptr)
		{
			for (u32 Index = 0; Index < RBExclusiveDepthStencil::MaxIndex; Index++)
			{
				DepthStencilViews[Index] = InDepthStencilViews[Index];
				// New Monolithic Graphics drivers have optional "fast calls" replacing various D3d functions
				// You can't use fast version of XXSetShaderResources (called XXSetFastShaderResource) on dynamic or d/s targets
				if (DepthStencilViews[Index] != NULL)
					NumDepthStencilViews++;
			}
		}
	}

	virtual ~RBD3D11TextureBase() {}

	i32 GetMemorySize() const
	{
		return MemorySize;
	}

	void SetMemorySize(i32 InMemorySize)
	{
		MemorySize = InMemorySize;
	}

	// Accessors.
	ID3D11Resource* GetResource() const { return Resource; }
	ID3D11ShaderResourceView* GetShaderResourceView() const { return ShaderResourceView; }
	RBD3D11BaseShaderResource* GetBaseShaderResource() const { return BaseShaderResource; }

	/**
	* Get the render target view for the specified mip and array slice.
	* An array slice of -1 is used to indicate that no array slice should be required.
	*/
	ID3D11RenderTargetView* GetRenderTargetView(i32 MipIndex, i32 ArraySliceIndex) const
	{
		i32 ArrayIndex = MipIndex;

		if (bCreatedRTVsPerSlice)
		{
			CHECK(ArraySliceIndex >= 0);
			ArrayIndex = MipIndex * RTVArraySize + ArraySliceIndex;
		}
		else
		{
			// Catch attempts to use a specific slice without having created the texture to support it
			CHECK(ArraySliceIndex == -1 || ArraySliceIndex == 0);
		}

		if ((uint32)ArrayIndex < (uint32)RenderTargetViews.size())
		{
			return RenderTargetViews[ArrayIndex];
		}
		return 0;
	}
	ID3D11DepthStencilView* GetDepthStencilView(RBExclusiveDepthStencil AccessType) const
	{
		return DepthStencilViews[AccessType.GetIndex()];
	}

	// New Monolithic Graphics drivers have optional "fast calls" replacing various D3d functions
	// You can't use fast version of XXSetShaderResources (called XXSetFastShaderResource) on dynamic or d/s targets
	bool HasDepthStencilView()
	{
		return (NumDepthStencilViews > 0);
	}

protected:

	/** The D3D11 RHI that created this texture. */
	FD3D11DynamicRHI* D3DRHI;

	/** Amount of memory allocated by this texture, in bytes. */
	i32 MemorySize;

	/** Pointer to the base shader resource. Usually the object itself, but not for texture references. */
	RBD3D11BaseShaderResource* BaseShaderResource;

	/** The texture resource. */
	TRefCountPtr<ID3D11Resource> Resource;

	/** A shader resource view of the texture. */
	TRefCountPtr<ID3D11ShaderResourceView> ShaderResourceView;

	/** A render targetable view of the texture. */
	std::vector<TRefCountPtr<ID3D11RenderTargetView> > RenderTargetViews;

	bool bCreatedRTVsPerSlice;

	i32 RTVArraySize;

	/** A depth-stencil targetable view of the texture. */
	TRefCountPtr<ID3D11DepthStencilView> DepthStencilViews[RBExclusiveDepthStencil::MaxIndex];

	/** Number of Depth Stencil Views - used for fast call tracking. */
	u32	NumDepthStencilViews;
};


/** 2D texture (vanilla, cubemap or 2D array) */
template<typename BaseResourceType>
class TD3D11Texture2D : public BaseResourceType, public RBD3D11TextureBase
{
public:

	/** Flags used when the texture was created */
	uint32 Flags;

	/** Initialization constructor. */
	TD3D11Texture2D(
	class FD3D11DynamicRHI* InD3DRHI,
		ID3D11Texture2D* InResource,
		ID3D11ShaderResourceView* InShaderResourceView,
		bool bInCreatedRTVsPerSlice,
		int32 InRTVArraySize,
		const std::vector<TRefCountPtr<ID3D11RenderTargetView> >& InRenderTargetViews,
		TRefCountPtr<ID3D11DepthStencilView>* InDepthStencilViews,
		uint32 InSizeX,
		uint32 InSizeY,
		uint32 InSizeZ,
		uint32 InNumMips,
		uint32 InNumSamples,
		EPixelFormat InFormat,
		bool bInCubemap,
		uint32 InFlags,
		bool bInPooled,
		const RBClearValueBinding& InClearValue
		)
		: BaseResourceType(
		InSizeX,
		InSizeY,
		InSizeZ,
		InNumMips,
		InNumSamples,
		InFormat,
		InFlags,
		InClearValue
		)
		, RBD3D11TextureBase(
		InD3DRHI,
		InResource,
		InShaderResourceView,
		InRTVArraySize,
		bInCreatedRTVsPerSlice,
		InRenderTargetViews,
		InDepthStencilViews
		)
		, Flags(InFlags)
		, bCubemap(bInCubemap)
		, bPooled(bInPooled)
	{
	}

  virtual ~TD3D11Texture2D();

	/**
	* Locks one of the texture's mip-maps.
	* @return A pointer to the specified texture data.
	*/
	void* Lock(uint32 MipIndex, uint32 ArrayIndex, EResourceLockMode LockMode, uint32& DestStride);

	/** Unlocks a previously locked mip-map. */
	void Unlock(uint32 MipIndex, uint32 ArrayIndex);

	// Accessors.
	ID3D11Texture2D* GetResource() const { return (ID3D11Texture2D*)RBD3D11TextureBase::GetResource(); }
	bool IsCubemap() const { return bCubemap; }

	/** FRHITexture override.  See FRHITexture::GetNativeResource() */
	virtual void* GetNativeResource() const override final
	{
		return GetResource();
	}
	virtual void* GetNativeShaderResourceView() const override final
	{
		return GetShaderResourceView();
	}
	virtual void* GetTextureBaseRHI() override final
	{
		return static_cast<RBD3D11TextureBase*>(this);
	}


	// IRefCountedObject interface.
	virtual uint32 AddRef() const
	{
		return RBRHIResource::AddRef();
	}
	virtual uint32 Release() const
	{
		return RBRHIResource::Release();
	}
	virtual uint32 GetRefCount() const
	{
		return RBRHIResource::GetRefCount();
	}

private:

	/** Whether the texture is a cube-map. */
	const uint32 bCubemap : 1;
	/** Whether the texture can be pooled. */
	const uint32 bPooled : 1;

};

/** 3D Texture */
class FD3D11Texture3D : public RBRHITexture3D, public RBD3D11TextureBase
{
public:

  /** Initialization constructor. */
  FD3D11Texture3D(
  class FD3D11DynamicRHI* InD3DRHI,
    ID3D11Texture3D* InResource,
    ID3D11ShaderResourceView* InShaderResourceView,
    const std::vector<TRefCountPtr<ID3D11RenderTargetView> >& InRenderTargetViews,
    uint32 InSizeX,
    uint32 InSizeY,
    uint32 InSizeZ,
    uint32 InNumMips,
    EPixelFormat InFormat,
    uint32 InFlags,
    const RBClearValueBinding& InClearValue
    )
    : RBRHITexture3D(InSizeX, InSizeY, InSizeZ, InNumMips, InFormat, InFlags, InClearValue)
    , RBD3D11TextureBase(
    InD3DRHI,
    InResource,
    InShaderResourceView,
    1,
    false,
    InRenderTargetViews,
    NULL
    )
  {
  }

  virtual ~FD3D11Texture3D();

  // Accessors.
  ID3D11Texture3D* GetResource() const { return (ID3D11Texture3D*)RBD3D11TextureBase::GetResource(); }

  virtual void* GetTextureBaseRHI() override final
  {
    return static_cast<RBD3D11TextureBase*>(this);
  }

  // IRefCountedObject interface.
  virtual uint32 AddRef() const
  {
    return RBRHIResource::AddRef();
  }
  virtual uint32 Release() const
  {
    return RBRHIResource::Release();
  }
  virtual uint32 GetRefCount() const
  {
    return RBRHIResource::GetRefCount();
  }
};

class RBD3D11BaseTexture2D : public RBRHITexture2D
{
public:
	RBD3D11BaseTexture2D(uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, uint32 InNumMips, uint32 InNumSamples, EPixelFormat InFormat, uint32 InFlags, const RBClearValueBinding& InClearValue)
		: RBRHITexture2D(InSizeX, InSizeY, InNumMips, InNumSamples, InFormat, InFlags, InClearValue)
	{}
	uint32 GetSizeZ() const { return 0; }
};

class FD3D11BaseTexture2DArray : public RBRHITexture2DArray
{
public:
  FD3D11BaseTexture2DArray(uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, uint32 InNumMips, uint32 InNumSamples, EPixelFormat InFormat, uint32 InFlags, const RBClearValueBinding& InClearValue)
    : RBRHITexture2DArray(InSizeX, InSizeY, InSizeZ, InNumMips, InFormat, InFlags, InClearValue)
  {
    CHECK(InNumSamples == 1);
  }
};

class FD3D11BaseTextureCube : public RBRHITextureCube
{
public:
  FD3D11BaseTextureCube(uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, uint32 InNumMips, uint32 InNumSamples, EPixelFormat InFormat, uint32 InFlags, const RBClearValueBinding& InClearValue)
    : RBRHITextureCube(InSizeX, InNumMips, InFormat, InFlags, InClearValue)
  {
    CHECK(InNumSamples == 1);
  }
  uint32 GetSizeX() const { return GetSize(); }
  uint32 GetSizeY() const { return GetSize(); } //-V524
  uint32 GetSizeZ() const { return 0; }
};

typedef TD3D11Texture2D<RBRHITexture>              RBD3D11Texture;
typedef TD3D11Texture2D<RBD3D11BaseTexture2D>      RBD3D11Texture2D;
typedef TD3D11Texture2D<FD3D11BaseTexture2DArray> FD3D11Texture2DArray;
typedef TD3D11Texture2D<FD3D11BaseTextureCube>    FD3D11TextureCube;

/** Texture reference class. */
class FD3D11TextureReference : public RBRHITextureReference, public RBD3D11TextureBase
{
public:
  FD3D11TextureReference(class FD3D11DynamicRHI* InD3DRHI, RBLastRenderTimeContainer* LastRenderTime)
    : RBRHITextureReference(LastRenderTime)
    , RBD3D11TextureBase(InD3DRHI, NULL, NULL, 0, false, std::vector<TRefCountPtr<ID3D11RenderTargetView> >(), NULL)
  {
    BaseShaderResource = NULL;
  }

  void SetReferencedTexture(RBRHITexture* InTexture, RBD3D11BaseShaderResource* InBaseShaderResource, ID3D11ShaderResourceView* InSRV)
  {
    ShaderResourceView = InSRV;
    BaseShaderResource = InBaseShaderResource;
    RBRHITextureReference::SetReferencedTexture(InTexture);
  }

  virtual void* GetTextureBaseRHI() override final
  {
    return static_cast<RBD3D11TextureBase*>(this);
  }
  // IRefCountedObject interface.
  virtual uint32 AddRef() const
  {
    return RBRHIResource::AddRef();
  }
  virtual uint32 Release() const
  {
    return RBRHIResource::Release();
  }
  virtual uint32 GetRefCount() const
  {
    return RBRHIResource::GetRefCount();
  }
};

/** Given a pointer to a RHI texture that was created by the D3D11 RHI, returns a pointer to the FD3D11TextureBase it encapsulates. */
FORCEINLINE RBD3D11TextureBase* GetD3D11TextureFromRHITexture(RBRHITexture* Texture)
{
  if (!Texture)
  {
    return NULL;
  }
  RBD3D11TextureBase* Result((RBD3D11TextureBase*)Texture->GetTextureBaseRHI());
  CHECK(Result);
  return Result;
}

extern size_t CalcTextureSize(uint32 SizeX, uint32 SizeY, EPixelFormat Format, uint32 MipCount);
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
extern void ReturnPooledTexture2D(int32 MipCount, EPixelFormat PixelFormat, ID3D11Texture2D* InResource);
extern void D3D11TextureAllocated2D(RBD3D11Texture2D& Texture);
template<typename BaseResourceType>
TD3D11Texture2D<BaseResourceType>::~TD3D11Texture2D()
{
  D3D11TextureDeleted(*this);
  if (bPooled)
  {
    ReturnPooledTexture2D(GetNumMips(), GetFormat(), GetResource());
  }
}
/** A ring allocation from the constants ring buffer. */
struct RBRingAllocation
{
  ID3D11Buffer* Buffer;
  void* DataPtr;
  uint32 Offset;
  uint32 Size;

  RBRingAllocation() : Buffer(NULL) {}
  inline bool IsValid() const { return Buffer != NULL; }
};
class RBD3D11UniformBuffer : public RBRHIUniformBuffer
{
public:

  /** The D3D11 constant buffer resource */
  TRefCountPtr<ID3D11Buffer> Resource;

  /** Allocation in the constants ring buffer if applicable. */
  RBRingAllocation RingAllocation;

  /** Resource table containing RHI references. */
  std::vector<TRefCountPtr<RBRHIResource> > ResourceTable;

  /** Initialization constructor. */
  RBD3D11UniformBuffer(class FD3D11DynamicRHI* InD3D11RHI, const RBRHIUniformBufferLayout& InLayout, ID3D11Buffer* InResource, const RBRingAllocation& InRingAllocation)
    : RBRHIUniformBuffer(InLayout)
    , Resource(InResource)
    , RingAllocation(InRingAllocation)
    , D3D11RHI(InD3D11RHI)
  {}

  virtual ~RBD3D11UniformBuffer();

private:
  class FD3D11DynamicRHI* D3D11RHI;
};
/** Index buffer resource class that stores stride information. */
class RBD3D11IndexBuffer : public RBRHIIndexBuffer, public RBD3D11BaseShaderResource
{
public:

  /** The index buffer resource */
  TRefCountPtr<ID3D11Buffer> Resource;

  RBD3D11IndexBuffer(ID3D11Buffer* InResource, uint32 InStride, uint32 InSize, uint32 InUsage)
    : RBRHIIndexBuffer(InStride, InSize, InUsage)
    , Resource(InResource)
  {}

  virtual ~RBD3D11IndexBuffer()
  {
    //UpdateBufferStats(Resource, false);
  }

  // IRefCountedObject interface.
  virtual uint32 AddRef() const
  {
    return RBRHIResource::AddRef();
  }
  virtual uint32 Release() const
  {
    return RBRHIResource::Release();
  }
  virtual uint32 GetRefCount() const
  {
    return RBRHIResource::GetRefCount();
  }
};

/** Structured buffer resource class. */
class RBD3D11StructuredBuffer : public RBRHIStructuredBuffer, public RBD3D11BaseShaderResource
{
public:

  TRefCountPtr<ID3D11Buffer> Resource;

  RBD3D11StructuredBuffer(ID3D11Buffer* InResource, uint32 InStride, uint32 InSize, uint32 InUsage)
    : RBRHIStructuredBuffer(InStride, InSize, InUsage)
    , Resource(InResource)
  {
    SetCurrentGPUAccess(EResourceTransitionAccess::ERWBarrier);
  }

  virtual ~RBD3D11StructuredBuffer()
  {
    //UpdateBufferStats(Resource, false);
  }

  // IRefCountedObject interface.
  virtual uint32 AddRef() const
  {
    return RBRHIResource::AddRef();
  }
  virtual uint32 Release() const
  {
    return RBRHIResource::Release();
  }
  virtual uint32 GetRefCount() const
  {
    return RBRHIResource::GetRefCount();
  }
};

/** Vertex buffer resource class. */
class RBD3D11VertexBuffer : public RBRHIVertexBuffer, public RBD3D11BaseShaderResource
{
public:

  TRefCountPtr<ID3D11Buffer> Resource;

  RBD3D11VertexBuffer(ID3D11Buffer* InResource, uint32 InSize, uint32 InUsage)
    : RBRHIVertexBuffer(InSize, InUsage)
    , Resource(InResource)
  {}

  virtual ~RBD3D11VertexBuffer()
  {
    //UpdateBufferStats(Resource, false);
  }

  // IRefCountedObject interface.
  virtual uint32 AddRef() const
  {
    return RBRHIResource::AddRef();
  }
  virtual uint32 Release() const
  {
    return RBRHIResource::Release();
  }
  virtual uint32 GetRefCount() const
  {
    return RBRHIResource::GetRefCount();
  }
};
/** Shader resource view class. */
class RBD3D11ShaderResourceView : public RBRHIShaderResourceView
{
public:

  TRefCountPtr<ID3D11ShaderResourceView> View;
  TRefCountPtr<RBD3D11BaseShaderResource> Resource;

  RBD3D11ShaderResourceView(ID3D11ShaderResourceView* InView, RBD3D11BaseShaderResource* InResource)
    : View(InView)
    , Resource(InResource)
  {}
};

/** Unordered access view class. */
class RBD3D11UnorderedAccessView : public RBRHIUnorderedAccessView
{
public:

  TRefCountPtr<ID3D11UnorderedAccessView> View;
  TRefCountPtr<RBD3D11BaseShaderResource> Resource;

  RBD3D11UnorderedAccessView(ID3D11UnorderedAccessView* InView, RBD3D11BaseShaderResource* InResource)
    : View(InView)
    , Resource(InResource)
  {}
};