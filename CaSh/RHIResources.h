#pragma once
#include "RefCount.h"
#include "Util.h"
#include "Thread.h"
#include "RHIDef.h"
#include "PixelFormat.h"
#include "SecureHash.h"



class RBRHIResource
{
public:
	RBRHIResource(bool InbDoNotDeferDelete = false)
		:bDoNotDeferDelete(InbDoNotDeferDelete)
	{
		MarkedForDelete = false;
	}

	virtual ~RBRHIResource()
	{
		CHECK(PlatformNeedsExtraDeletionLatency() || (NumRefs == 0 && (CurrentlyDeleting == this || bDoNotDeferDelete || ByPass()))); // this should not have any outstanding refs
	}

	u32 AddRef() const
	{
		i32 NewValue = ++NumRefs;
		CHECK(NewValue > 0);
		return u32(NewValue);
	}

	u32 Release() const
	{
		i32 NewValue = --NumRefs;
		if (NewValue == 0)
		{
			if (!DeferDelete())
			{
				delete this;
			}
			else
			{
				bool b = true;
				if ((MarkedForDelete.compare_exchange_weak(b, false)))
				{
					PendingDeletes.push(const_cast<RBRHIResource*>(this));
				}
			}
		}
		CHECK(NewValue >= 0);
		return u32(NewValue);
	}

	u32 GetRefCount() const
	{
		i32 CurrentValue = NumRefs;
		CHECK(CurrentValue >= 0);
		return u32(CurrentValue);
	}

	void DoNoDeferDelete()
	{
		CHECK(!MarkedForDelete);
		bDoNotDeferDelete = true;
		RBMemoryBarrier();
		CHECK(!MarkedForDelete);
	}

	static void FlushPendingDeletes(){
		//wait to implement
	}

	FORCEINLINE static bool PlatformNeedsExtraDeletionLatency()
	{
		/*
		return GRHINeedsExtraDeletionLatency && GIsRHIInitialized;
		*/
		return false;
	}

  static bool ByPass(){ return false; }

private:
	mutable std::atomic_int NumRefs;
	mutable std::atomic_bool MarkedForDelete;
	bool bDoNotDeferDelete;
	static RBThreadSafeQueue<RBRHIResource*> PendingDeletes;
	static RBRHIResource* CurrentlyDeleting;

	FORCEINLINE bool DeferDelete() const
	{
#define  DISABLE_RHI_DEFFERED_DELETE 
#ifdef  DISABLE_RHI_DEFFERED_DELETE
		return false;
#else
		// Defer if GRHINeedsExtraDeletionLatency or we are doing threaded rendering (unless otherwise requested).
		return !bDoNotDeferDelete && (GRHINeedsExtraDeletionLatency || !ByPass());
#endif
#undef DISABLE_RHI_DEFFERED_DELETE
	}
};



//
// State blocks
//

class RBRHISamplerState : public RBRHIResource{};
class RBRHIRasterizerState : public RBRHIResource{};
class RBRHIDepthStencilState : public RBRHIResource{};
class RBRHIBlendState : public RBRHIResource{};


//
// Shader bindings
//
class RBRHIVertexDeclaration : public RBRHIResource{};
class RBRHIBoundShaderState : public RBRHIResource{};

//
// Shaders
//

class RBRHIShader : public RBRHIResource
{
public:
  RBRHIShader(bool InbDoNotDeferDelete = false) : RBRHIResource(InbDoNotDeferDelete) {}

  void SetHash(FSHAHash InHash) { Hash = InHash; }
  FSHAHash GetHash() const { return Hash; }

//#ifdef _DEBUG
  // for debugging only e.g. MaterialName:ShaderFile.usf or ShaderFile.usf/EntryFunc
  std::string ShaderName;
//#endif

private:
  FSHAHash Hash;
};

class RBRHIVertexShader : public RBRHIShader {};
class RBRHIHullShader : public RBRHIShader {};
class RBRHIDomainShader : public RBRHIShader {};
class RBRHIPixelShader : public RBRHIShader {};
class RBRHIGeometryShader : public RBRHIShader {};
class RBRHIComputeShader : public RBRHIShader {};


//
// Buffers
//
struct RBRHIUniformBufferLayout
{
  /** The size of the constant buffer in bytes. */
  uint32 ConstantBufferSize;
  /** The offset to the beginning of the resource table. */
  uint32 ResourceOffset;
  /** The type of each resource (EUniformBufferBaseType). */
  std::vector<uint8> Resources;

  uint32 GetHash() const
  {
    if (!bComputedHash)
    {
      uint32 TmpHash = ConstantBufferSize << 16;
      // This is to account for 32vs64 bits difference in pointer sizes.
      TmpHash ^= Align(ResourceOffset, 8);
      uint32 N = Resources.size();
      while (N >= 4)
      {
        TmpHash ^= (Resources[--N] << 0);
        TmpHash ^= (Resources[--N] << 8);
        TmpHash ^= (Resources[--N] << 16);
        TmpHash ^= (Resources[--N] << 24);
      }
      while (N >= 2)
      {
        TmpHash ^= Resources[--N] << 0;
        TmpHash ^= Resources[--N] << 16;
      }
      while (N > 0)
      {
        TmpHash ^= Resources[--N];
      }
      Hash = TmpHash;
      bComputedHash = true;
    }
    return Hash;
  }

  explicit RBRHIUniformBufferLayout(std::string InName) :
    ConstantBufferSize(0),
    ResourceOffset(0),
    Name(InName),
    Hash(0),
    bComputedHash(false)
  {
  }

  enum EInit
  {
    Zero
  };
  explicit RBRHIUniformBufferLayout(EInit) :
    ConstantBufferSize(0),
    ResourceOffset(0),
    Name(""),
    Hash(0),
    bComputedHash(false)
  {
  }

  void CopyFrom(const RBRHIUniformBufferLayout& Source)
  {
    ConstantBufferSize = Source.ConstantBufferSize;
    ResourceOffset = Source.ResourceOffset;
    Resources = Source.Resources;
    Name = Source.Name;
    Hash = Source.Hash;
    bComputedHash = Source.bComputedHash;
  }

  const std::string GetDebugName() const { return Name; }

private:
  // for debugging / error message
  std::string Name;

  mutable uint32 Hash;
  mutable bool bComputedHash;
};

inline bool operator==(const RBRHIUniformBufferLayout& A, const RBRHIUniformBufferLayout& B)
{
return A.ConstantBufferSize == B.ConstantBufferSize
&& A.ResourceOffset == B.ResourceOffset
&& A.Resources == B.Resources;
}

class RBRHIUniformBuffer : public RBRHIResource
{
public:

  /** Initialization constructor. */
  RBRHIUniformBuffer(const RBRHIUniformBufferLayout& InLayout)
    : Layout(&InLayout)
  {}

  /** @return The number of bytes in the uniform buffer. */
  uint32 GetSize() const { return Layout->ConstantBufferSize; }
  const RBRHIUniformBufferLayout& GetLayout() const { return *Layout; }

private:
  /** Layout of the uniform buffer. */
  const RBRHIUniformBufferLayout* Layout;
};

class RBRHIIndexBuffer : public RBRHIResource
{
public:

  /** Initialization constructor. */
  RBRHIIndexBuffer(uint32 InStride, uint32 InSize, uint32 InUsage)
    : Stride(InStride)
    , Size(InSize)
    , Usage(InUsage)
  {}

  /** @return The stride in bytes of the index buffer; must be 2 or 4. */
  uint32 GetStride() const { return Stride; }

  /** @return The number of bytes in the index buffer. */
  uint32 GetSize() const { return Size; }

  /** @return The usage flags used to create the index buffer. */
  uint32 GetUsage() const { return Usage; }

private:
  uint32 Stride;
  uint32 Size;
  uint32 Usage;
};
class RBRHIVertexBuffer : public RBRHIResource
{
public:

  /**
  * Initialization constructor.
  * @apram InUsage e.g. BUF_UnorderedAccess
  */
  RBRHIVertexBuffer(uint32 InSize, uint32 InUsage)
    : Size(InSize)
    , Usage(InUsage)
  {}

  /** @return The number of bytes in the vertex buffer. */
  uint32 GetSize() const { return Size; }

  /** @return The usage flags used to create the vertex buffer. e.g. BUF_UnorderedAccess */
  uint32 GetUsage() const { return Usage; }

private:
  uint32 Size;
  // e.g. BUF_UnorderedAccess
  uint32 Usage;
};

class RBRHIStructuredBuffer : public RBRHIResource
{
public:

  /** Initialization constructor. */
  RBRHIStructuredBuffer(uint32 InStride, uint32 InSize, uint32 InUsage)
    : Stride(InStride)
    , Size(InSize)
    , Usage(InUsage)
  {}

  /** @return The stride in bytes of the structured buffer; must be 2 or 4. */
  uint32 GetStride() const { return Stride; }

  /** @return The number of bytes in the structured buffer. */
  uint32 GetSize() const { return Size; }

  /** @return The usage flags used to create the structured buffer. */
  uint32 GetUsage() const { return Usage; }

private:
  uint32 Stride;
  uint32 Size;
  uint32 Usage;
};


//
// Textures
//
class RBLastRenderTimeContainer
{
public:
	RBLastRenderTimeContainer() : _last_render_time(-FLT_MAX) {}

	double GetLastRenderTime() const { return _last_render_time; }
	void SetLastRenderTime(double InLastRenderTime)
	{
		// avoid dirty caches from redundant writes
		if (_last_render_time != InLastRenderTime)
		{
			_last_render_time = InLastRenderTime;
		}
	}

private:
	/** The last time the resource was rendered. */
	double _last_render_time;
};

class RBRHITexture : public RBRHIResource
{
public:

	/** Initialization constructor. */
	RBRHITexture(u32 InNumMips, u32 InNumSamples, EPixelFormat InFormat, uint32 InFlags, RBLastRenderTimeContainer* InLastRenderTime, const RBClearValueBinding& InClearValue)
		: ClearValue(InClearValue)
		, NumMips(InNumMips)
		, NumSamples(InNumSamples)
		, Format(InFormat)
		, Flags(InFlags)
		, LastRenderTime(InLastRenderTime ? *InLastRenderTime : DefaultLastRenderTime)
	{}

	// Dynamic cast methods.
	virtual class RBRHITexture2D* GetTexture2D() { return NULL; }
	virtual class RBRHITexture2DArray* GetTexture2DArray() { return NULL; }
	virtual class RBRHITexture3D* GetTexture3D() { return NULL; }
	virtual class RBRHITextureCube* GetTextureCube() { return NULL; }
	virtual class RBRHITextureReference* GetTextureReference() { return NULL; }

	/**
	* Returns access to the platform-specific native resource pointer.  This is designed to be used to provide plugins with access
	* to the underlying resource and should be used very carefully or not at all.
	*
	* @return	The pointer to the native resource or NULL if it not initialized or not supported for this resource type for some reason
	*/
	virtual void* GetNativeResource() const
	{
		// Override this in derived classes to expose access to the native texture resource
		return nullptr;
	}

	/**
	* Returns access to the platform-specific native shader resource view pointer.  This is designed to be used to provide plugins with access
	* to the underlying resource and should be used very carefully or not at all.
	*
	* @return	The pointer to the native resource or NULL if it not initialized or not supported for this resource type for some reason
	*/
	virtual void* GetNativeShaderResourceView() const
	{
		// Override this in derived classes to expose access to the native texture resource
		return nullptr;
	}

	/**
	* Returns access to the platform-specific RHI texture baseclass.  This is designed to provide the RHI with fast access to its base classes in the face of multiple inheritance.
	* @return	The pointer to the platform-specific RHI texture baseclass or NULL if it not initialized or not supported for this RHI
	*/
	virtual void* GetTextureBaseRHI()
	{
		// Override this in derived classes to expose access to the native texture resource
		return nullptr;
	}

	/** @return The number of mip-maps in the texture. */
	uint32 GetNumMips() const { return NumMips; }

	/** @return The format of the pixels in the texture. */
	EPixelFormat GetFormat() const { return Format; }

	/** @return The flags used to create the texture. */
	uint32 GetFlags() const { return Flags; }

	/* @return the number of samples for multi-sampling. */
	uint32 GetNumSamples() const { return NumSamples; }

	/** @return Whether the texture is multi sampled. */
	bool IsMultisampled() const { return NumSamples > 1; }

	RBRHIResourceInfo ResourceInfo;

	/** sets the last time this texture was cached in a resource table. */
	void SetLastRenderTime(float InLastRenderTime)
	{
		LastRenderTime.SetLastRenderTime(InLastRenderTime);
	}

	/** Returns the last render time container, or NULL if none were specified at creation. */
	RBLastRenderTimeContainer* GetLastRenderTimeContainer()
	{
		if (&LastRenderTime == &DefaultLastRenderTime)
		{
			return NULL;
		}
		return &LastRenderTime;
	}

	void SetName(std::string& InName)
	{
		TextureName = InName;
	}

	std::string GetName() const
	{
		return TextureName;
	}

	bool HasClearValue() const
	{
		return ClearValue.ColorBinding != EClearBinding::ENoneBound;
	}

	RBColorf GetClearColor() const
	{
		return ClearValue.GetClearColor();
	}

	void GetDepthStencilClearValue(float& OutDepth, uint32& OutStencil) const
	{
		return ClearValue.GetDepthStencil(OutDepth, OutStencil);
	}

	float GetDepthClearValue() const
	{
		float Depth;
		uint32 Stencil;
		ClearValue.GetDepthStencil(Depth, Stencil);
		return Depth;
	}

	uint32 GetStencilClearValue() const
	{
		float Depth;
		uint32 Stencil;
		ClearValue.GetDepthStencil(Depth, Stencil);
		return Stencil;
	}

	const RBClearValueBinding GetClearBinding() const
	{
		return ClearValue;
	}

private:
	RBClearValueBinding ClearValue;
	uint32 NumMips;
	uint32 NumSamples;
	EPixelFormat Format;
	uint32 Flags;
	RBLastRenderTimeContainer& LastRenderTime;
	RBLastRenderTimeContainer DefaultLastRenderTime;
	std::string TextureName;
};
class RBRHITexture2D : public RBRHITexture
{
public:

	/** Initialization constructor. */
	RBRHITexture2D(uint32 InSizeX, uint32 InSizeY, uint32 InNumMips, uint32 InNumSamples, EPixelFormat InFormat, uint32 InFlags, const RBClearValueBinding& InClearValue)
		: RBRHITexture(InNumMips, InNumSamples, InFormat, InFlags, NULL, InClearValue)
		, SizeX(InSizeX)
		, SizeY(InSizeY)
	{}

	// Dynamic cast methods.
	virtual RBRHITexture2D* GetTexture2D() { return this; }

	/** @return The width of the texture. */
	u32 GetSizeX() const { return SizeX; }

	/** @return The height of the texture. */
	u32 GetSizeY() const { return SizeY; }

private:

	u32 SizeX;
	u32 SizeY;
};
class RBRHITexture2DArray : public RBRHITexture
{
public:

  /** Initialization constructor. */
  RBRHITexture2DArray(uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, uint32 InNumMips, EPixelFormat InFormat, uint32 InFlags, const RBClearValueBinding& InClearValue)
    : RBRHITexture(InNumMips, 1, InFormat, InFlags, NULL, InClearValue)
    , SizeX(InSizeX)
    , SizeY(InSizeY)
    , SizeZ(InSizeZ)
  {}

  // Dynamic cast methods.
  virtual RBRHITexture2DArray* GetTexture2DArray() { return this; }

  /** @return The width of the textures in the array. */
  uint32 GetSizeX() const { return SizeX; }

  /** @return The height of the texture in the array. */
  uint32 GetSizeY() const { return SizeY; }

  /** @return The number of textures in the array. */
  uint32 GetSizeZ() const { return SizeZ; }

private:

  uint32 SizeX;
  uint32 SizeY;
  uint32 SizeZ;
};
class RBRHITexture3D : public RBRHITexture
{
public:

  /** Initialization constructor. */
  RBRHITexture3D(uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, uint32 InNumMips, EPixelFormat InFormat, uint32 InFlags, const RBClearValueBinding& InClearValue)
    : RBRHITexture(InNumMips, 1, InFormat, InFlags, NULL, InClearValue)
    , SizeX(InSizeX)
    , SizeY(InSizeY)
    , SizeZ(InSizeZ)
  {}

  // Dynamic cast methods.
  virtual RBRHITexture3D* GetTexture3D() { return this; }

  /** @return The width of the texture. */
  uint32 GetSizeX() const { return SizeX; }

  /** @return The height of the texture. */
  uint32 GetSizeY() const { return SizeY; }

  /** @return The depth of the texture. */
  uint32 GetSizeZ() const { return SizeZ; }

private:

  uint32 SizeX;
  uint32 SizeY;
  uint32 SizeZ;
};
class RBRHITextureCube : public RBRHITexture
{
public:

  /** Initialization constructor. */
  RBRHITextureCube(uint32 InSize, uint32 InNumMips, EPixelFormat InFormat, uint32 InFlags, const RBClearValueBinding& InClearValue)
    : RBRHITexture(InNumMips, 1, InFormat, InFlags, NULL, InClearValue)
    , Size(InSize)
  {}

  // Dynamic cast methods.
  virtual RBRHITextureCube* GetTextureCube() { return this; }

  /** @return The width and height of each face of the cubemap. */
  uint32 GetSize() const { return Size; }

private:

  uint32 Size;
};
class RBRHITextureReference : public RBRHITexture
{
public:
  explicit RBRHITextureReference(RBLastRenderTimeContainer* InLastRenderTime)
    : RBRHITexture(0, 0, PF_Unknown, 0, InLastRenderTime, RBClearValueBinding())
  {}

  virtual RBRHITextureReference* GetTextureReference() override { return this; }
  inline RBRHITexture* GetReferencedTexture() const { return ReferencedTexture.GetReference(); }

protected:
  void SetReferencedTexture(RBRHITexture* InTexture)
  {
    ReferencedTexture = InTexture;
  }

private:
  TRefCountPtr<RBRHITexture> ReferencedTexture;
};
class RBRHITextureReferenceNullImpl : public RBRHITextureReference
{
public:
  RBRHITextureReferenceNullImpl()
    : RBRHITextureReference(NULL)
  {}

  void SetReferencedTexture(RBRHITexture* InTexture)
  {
    RBRHITextureReference::SetReferencedTexture(InTexture);
  }
};


//
// Misc
//

class RBRHIRenderQuery : public RBRHIResource {};


class RBRHIComputeFence : public RBRHIResource
{
public:

  RBRHIComputeFence(std::string InName)
    : Name(InName)
    , bWriteEnqueued(false)
  {}

  FORCEINLINE std::string GetName() const
  {
    return Name;
  }

  FORCEINLINE bool GetWriteEnqueued() const
  {
    return bWriteEnqueued;
  }

  virtual void Reset()
  {
    bWriteEnqueued = false;
  }

  virtual void WriteFence()
  {
    CHECKF(!bWriteEnqueued, TEXT("ComputeFence: %s already written this frame. You should use a new label"), Name.c_str());
    bWriteEnqueued = true;
  }

private:
  //debug name of the label.
  std::string Name;

  //has the label been written to since being created.
  //check this when queuing waits to catch GPU hangs on the CPU at command creation time.
  bool bWriteEnqueued;
};

class RBRHIViewport : public RBRHIResource
{
public:
	RBRHIViewport()
		: RBRHIResource(true)
	{
	}
	/**
	* Returns access to the platform-specific native resource pointer.  This is designed to be used to provide plugins with access
	* to the underlying resource and should be used very carefully or not at all.
	*
	* @return	The pointer to the native resource or NULL if it not initialized or not supported for this resource type for some reason
	*/
	virtual void* GetNativeSwapChain() const { return nullptr; }
	/**
	* Returns access to the platform-specific native resource pointer to a backbuffer texture.  This is designed to be used to provide plugins with access
	* to the underlying resource and should be used very carefully or not at all.
	*
	* @return	The pointer to the native resource or NULL if it not initialized or not supported for this resource type for some reason
	*/
	virtual void* GetNativeBackBufferTexture() const { return nullptr; }
	/**
	* Returns access to the platform-specific native resource pointer to a backbuffer rendertarget. This is designed to be used to provide plugins with access
	* to the underlying resource and should be used very carefully or not at all.
	*
	* @return	The pointer to the native resource or NULL if it not initialized or not supported for this resource type for some reason
	*/
	virtual void* GetNativeBackBufferRT() const { return nullptr; }

	/**
	* Returns access to the platform-specific native window. This is designed to be used to provide plugins with access
	* to the underlying resource and should be used very carefully or not at all.
	*
	* @return	The pointer to the native resource or NULL if it not initialized or not supported for this resource type for some reason.
	* AddParam could represent any additional platform-specific data (could be null).
	*/
	virtual void* GetNativeWindow(void** AddParam = nullptr) const { return nullptr; }

	/**
	* Sets custom Present handler on the viewport
	*/
	virtual void SetCustomPresent(class RBRHICustomPresent*) {}

	/**
	* Returns currently set custom present handler.
	*/
	virtual class RBRHICustomPresent* GetCustomPresent() const { return nullptr; }
};


//
// Views
//
class RBRHIUnorderedAccessView : public RBRHIResource {};
class RBRHIShaderResourceView : public RBRHIResource {};

typedef RBRHISamplerState*              RBSamplerStateRHIParamRef;
typedef TRefCountPtr<RBRHISamplerState> RBSamplerStateRHIRef;

typedef RBRHIRasterizerState*              RBRasterizerStateRHIParamRef;
typedef TRefCountPtr<RBRHIRasterizerState> RBRasterizerStateRHIRef;

typedef RBRHIDepthStencilState*              RBDepthStencilStateRHIParamRef;
typedef TRefCountPtr<RBRHIDepthStencilState> RBDepthStencilStateRHIRef;

typedef RBRHIBlendState*              RBBlendStateRHIParamRef;
typedef TRefCountPtr<RBRHIBlendState> RBBlendStateRHIRef;

typedef RBRHIVertexDeclaration*              RBVertexDeclarationRHIParamRef;
typedef TRefCountPtr<RBRHIVertexDeclaration> RBVertexDeclarationRHIRef;

typedef RBRHIVertexShader*              RBVertexShaderRHIParamRef;
typedef TRefCountPtr<RBRHIVertexShader> RBVertexShaderRHIRef;

typedef RBRHIHullShader*              RBHullShaderRHIParamRef;
typedef TRefCountPtr<RBRHIHullShader> RBHullShaderRHIRef;

typedef RBRHIDomainShader*              RBDomainShaderRHIParamRef;
typedef TRefCountPtr<RBRHIDomainShader> RBDomainShaderRHIRef;

typedef RBRHIPixelShader*              RBPixelShaderRHIParamRef;
typedef TRefCountPtr<RBRHIPixelShader> RBPixelShaderRHIRef;

typedef RBRHIGeometryShader*              RBGeometryShaderRHIParamRef;
typedef TRefCountPtr<RBRHIGeometryShader> RBGeometryShaderRHIRef;

typedef RBRHIComputeShader*              RBComputeShaderRHIParamRef;
typedef TRefCountPtr<RBRHIComputeShader> RBComputeShaderRHIRef;

typedef RBRHIComputeFence*				RBComputeFenceRHIParamRef;
typedef TRefCountPtr<RBRHIComputeFence>	RBComputeFenceRHIRef;


typedef RBRHIBoundShaderState*              RBBoundShaderStateRHIParamRef;
typedef TRefCountPtr<RBRHIBoundShaderState> RBBoundShaderStateRHIRef;

typedef RBRHIUniformBuffer*              RBUniformBufferRHIParamRef;
typedef TRefCountPtr<RBRHIUniformBuffer> RBUniformBufferRHIRef;

typedef RBRHIIndexBuffer*              RBIndexBufferRHIParamRef;
typedef TRefCountPtr<RBRHIIndexBuffer> RBIndexBufferRHIRef;

typedef RBRHIVertexBuffer*              RBVertexBufferRHIParamRef;
typedef TRefCountPtr<RBRHIVertexBuffer> RBVertexBufferRHIRef;

typedef RBRHIStructuredBuffer*              RBStructuredBufferRHIParamRef;
typedef TRefCountPtr<RBRHIStructuredBuffer> RBStructuredBufferRHIRef;

typedef RBRHITexture*              RBTextureRHIParamRef;
typedef TRefCountPtr<RBRHITexture> RBTextureRHIRef;

typedef RBRHITexture2D*              RBTexture2DRHIParamRef;
typedef TRefCountPtr<RBRHITexture2D> RBTexture2DRHIRef;

typedef RBRHITexture2DArray*              RBTexture2DArrayRHIParamRef;
typedef TRefCountPtr<RBRHITexture2DArray> RBTexture2DArrayRHIRef;

typedef RBRHITexture3D*              RBTexture3DRHIParamRef;
typedef TRefCountPtr<RBRHITexture3D> RBTexture3DRHIRef;

typedef RBRHITextureCube*              RBTextureCubeRHIParamRef;
typedef TRefCountPtr<RBRHITextureCube> RBTextureCubeRHIRef;

typedef RBRHITextureReference*              RBTextureReferenceRHIParamRef;
typedef TRefCountPtr<RBRHITextureReference> RBTextureReferenceRHIRef;

typedef RBRHIRenderQuery*              RBRenderQueryRHIParamRef;
typedef TRefCountPtr<RBRHIRenderQuery> RBRenderQueryRHIRef;

typedef RBRHIViewport*              RBViewportRHIParamRef;
typedef TRefCountPtr<RBRHIViewport> RBViewportRHIRef;

typedef RBRHIUnorderedAccessView*              RBUnorderedAccessViewRHIParamRef;
typedef TRefCountPtr<RBRHIUnorderedAccessView> RBUnorderedAccessViewRHIRef;

typedef RBRHIShaderResourceView*              RBShaderResourceViewRHIParamRef;
typedef TRefCountPtr<RBRHIShaderResourceView> RBShaderResourceViewRHIRef;


class RBRHIRenderTargetView
{
public:
	RBRHITexture* Texture;
	u32 MipIndex;

	/** Array slice or texture cube face.  Only valid if texture resource was created with TexCreate_TargetArraySlicesIndependently! */
	u32 ArraySliceIndex;

	ERenderTargetLoadAction LoadAction;
	ERenderTargetStoreAction StoreAction;

	RBRHIRenderTargetView() :
		Texture(NULL),
		MipIndex(0),
		ArraySliceIndex(-1),
		LoadAction(ERenderTargetLoadAction::ELoad),
		StoreAction(ERenderTargetStoreAction::EStore)
	{}

	RBRHIRenderTargetView(const RBRHIRenderTargetView& Other) :
		Texture(Other.Texture),
		MipIndex(Other.MipIndex),
		ArraySliceIndex(Other.ArraySliceIndex),
		LoadAction(Other.LoadAction),
		StoreAction(Other.StoreAction)
	{}

	RBRHIRenderTargetView(RBRHITexture* InTexture) :
		Texture(InTexture),
		MipIndex(0),
		ArraySliceIndex(-1),
		LoadAction(ERenderTargetLoadAction::ELoad),
		StoreAction(ERenderTargetStoreAction::EStore)
	{}

	RBRHIRenderTargetView(RBRHITexture* InTexture, uint32 InMipIndex, uint32 InArraySliceIndex) :
		Texture(InTexture),
		MipIndex(InMipIndex),
		ArraySliceIndex(InArraySliceIndex),
		LoadAction(ERenderTargetLoadAction::ELoad),
		StoreAction(ERenderTargetStoreAction::EStore)
	{}

	RBRHIRenderTargetView(RBRHITexture* InTexture, uint32 InMipIndex, uint32 InArraySliceIndex, ERenderTargetLoadAction InLoadAction, ERenderTargetStoreAction InStoreAction) :
		Texture(InTexture),
		MipIndex(InMipIndex),
		ArraySliceIndex(InArraySliceIndex),
		LoadAction(InLoadAction),
		StoreAction(InStoreAction)
	{}

	bool operator==(const RBRHIRenderTargetView& Other) const
	{
		return
			Texture == Other.Texture &&
			MipIndex == Other.MipIndex &&
			ArraySliceIndex == Other.ArraySliceIndex &&
			LoadAction == Other.LoadAction &&
			StoreAction == Other.StoreAction;
	}

};
class RBExclusiveDepthStencil
{
public:
  enum Type
  {
    // don't use those directly, use the combined versions below
    // 4 bits are used for depth and 4 for stencil to make the hex value readable and non overlapping
    DepthNop = 0x00,
    DepthRead = 0x01,
    DepthWrite = 0x02,
    DepthMask = 0x0f,
    StencilNop = 0x00,
    StencilRead = 0x10,
    StencilWrite = 0x20,
    StencilMask = 0xf0,

    // use those:
    DepthNop_StencilNop = DepthNop + StencilNop,
    DepthRead_StencilNop = DepthRead + StencilNop,
    DepthWrite_StencilNop = DepthWrite + StencilNop,
    DepthNop_StencilRead = DepthNop + StencilRead,
    DepthRead_StencilRead = DepthRead + StencilRead,
    DepthWrite_StencilRead = DepthWrite + StencilRead,
    DepthNop_StencilWrite = DepthNop + StencilWrite,
    DepthRead_StencilWrite = DepthRead + StencilWrite,
    DepthWrite_StencilWrite = DepthWrite + StencilWrite,
  };

private:
  Type Value;

public:
  // constructor
  RBExclusiveDepthStencil(Type InValue = DepthNop_StencilNop)
    : Value(InValue)
  {
  }

  inline bool IsUsingDepthStencil() const
  {
    return Value != DepthNop_StencilNop;
  }
  inline bool IsDepthWrite() const
  {
    return ExtractDepth() == DepthWrite;
  }
  inline bool IsStencilWrite() const
  {
    return ExtractStencil() == StencilWrite;
  }

  inline bool IsAnyWrite() const
  {
    return IsDepthWrite() || IsStencilWrite();
  }

  inline void SetDepthWrite()
  {
    Value = (Type)(ExtractStencil() | DepthWrite);
  }
  inline void SetStencilWrite()
  {
    Value = (Type)(ExtractDepth() | StencilWrite);
  }
  inline void SetDepthStencilWrite(bool bDepth, bool bStencil)
  {
    Value = DepthNop_StencilNop;

    if (bDepth)
    {
      SetDepthWrite();
    }
    if (bStencil)
    {
      SetStencilWrite();
    }
  }
  bool operator==(const RBExclusiveDepthStencil& rhs) const
  {
    return Value == rhs.Value;
  }
  inline bool IsValid(RBExclusiveDepthStencil& Current) const
  {
    Type Depth = ExtractDepth();

    if (Depth != DepthNop && Depth != Current.ExtractDepth())
    {
      return false;
    }

    Type Stencil = ExtractStencil();

    if (Stencil != StencilNop && Stencil != Current.ExtractStencil())
    {
      return false;
    }

    return true;
  }

  uint32 GetIndex() const
  {
    // Note: The array to index has views created in that specific order.

    // we don't care about the Nop versions so less views are needed
    // we combine Nop and Write
    switch (Value)
    {
    case DepthWrite_StencilNop:
    case DepthNop_StencilWrite:
    case DepthWrite_StencilWrite:
    case DepthNop_StencilNop:
      return 0; // old DSAT_Writable

    case DepthRead_StencilNop:
    case DepthRead_StencilWrite:
      return 1; // old DSAT_ReadOnlyDepth

    case DepthNop_StencilRead:
    case DepthWrite_StencilRead:
      return 2; // old DSAT_ReadOnlyStencil

    case DepthRead_StencilRead:
      return 3; // old DSAT_ReadOnlyDepthAndStencil
    }
    // should never happen
    CHECK(0);
    return -1;
  }
  static const uint32 MaxIndex = 4;

private:
  inline Type ExtractDepth() const
  {
    return (Type)(Value & DepthMask);
  }
  inline Type ExtractStencil() const
  {
    return (Type)(Value & StencilMask);
  }
};
class RBRHIDepthRenderTargetView
{
public:
  RBTextureRHIParamRef Texture;

  ERenderTargetLoadAction		DepthLoadAction;
  ERenderTargetStoreAction	DepthStoreAction;
  ERenderTargetLoadAction		StencilLoadAction;

private:
  ERenderTargetStoreAction	StencilStoreAction;
  RBExclusiveDepthStencil		DepthStencilAccess;
public:

  // accessor to prevent write access to StencilStoreAction
  ERenderTargetStoreAction GetStencilStoreAction() const { return StencilStoreAction; }
  // accessor to prevent write access to DepthStencilAccess
  RBExclusiveDepthStencil GetDepthStencilAccess() const { return DepthStencilAccess; }

  RBRHIDepthRenderTargetView() :
    Texture(nullptr),
    DepthLoadAction(ERenderTargetLoadAction::EClear),
    DepthStoreAction(ERenderTargetStoreAction::EStore),
    StencilLoadAction(ERenderTargetLoadAction::EClear),
    StencilStoreAction(ERenderTargetStoreAction::EStore),
    DepthStencilAccess(RBExclusiveDepthStencil::DepthWrite_StencilWrite)
  {
    Validate();
  }

  RBRHIDepthRenderTargetView(RBTextureRHIParamRef InTexture) :
    Texture(InTexture),
    DepthLoadAction(ERenderTargetLoadAction::EClear),
    DepthStoreAction(ERenderTargetStoreAction::EStore),
    StencilLoadAction(ERenderTargetLoadAction::EClear),
    StencilStoreAction(ERenderTargetStoreAction::EStore),
    DepthStencilAccess(RBExclusiveDepthStencil::DepthWrite_StencilWrite)
  {
    Validate();
  }

  RBRHIDepthRenderTargetView(RBTextureRHIParamRef InTexture, ERenderTargetLoadAction InLoadAction, ERenderTargetStoreAction InStoreAction) :
    Texture(InTexture),
    DepthLoadAction(InLoadAction),
    DepthStoreAction(InStoreAction),
    StencilLoadAction(InLoadAction),
    StencilStoreAction(InStoreAction),
    DepthStencilAccess(RBExclusiveDepthStencil::DepthWrite_StencilWrite)
  {
    Validate();
  }

  RBRHIDepthRenderTargetView(RBTextureRHIParamRef InTexture, ERenderTargetLoadAction InLoadAction, ERenderTargetStoreAction InStoreAction, RBExclusiveDepthStencil InDepthStencilAccess) :
    Texture(InTexture),
    DepthLoadAction(InLoadAction),
    DepthStoreAction(InStoreAction),
    StencilLoadAction(InLoadAction),
    StencilStoreAction(InStoreAction),
    DepthStencilAccess(InDepthStencilAccess)
  {
    Validate();
  }

  RBRHIDepthRenderTargetView(RBTextureRHIParamRef InTexture, ERenderTargetLoadAction InDepthLoadAction, ERenderTargetStoreAction InDepthStoreAction, ERenderTargetLoadAction InStencilLoadAction, ERenderTargetStoreAction InStencilStoreAction) :
    Texture(InTexture),
    DepthLoadAction(InDepthLoadAction),
    DepthStoreAction(InDepthStoreAction),
    StencilLoadAction(InStencilLoadAction),
    StencilStoreAction(InStencilStoreAction),
    DepthStencilAccess(RBExclusiveDepthStencil::DepthWrite_StencilWrite)
  {
    Validate();
  }

  RBRHIDepthRenderTargetView(RBTextureRHIParamRef InTexture, ERenderTargetLoadAction InDepthLoadAction, ERenderTargetStoreAction InDepthStoreAction, ERenderTargetLoadAction InStencilLoadAction, ERenderTargetStoreAction InStencilStoreAction, RBExclusiveDepthStencil InDepthStencilAccess) :
    Texture(InTexture),
    DepthLoadAction(InDepthLoadAction),
    DepthStoreAction(InDepthStoreAction),
    StencilLoadAction(InStencilLoadAction),
    StencilStoreAction(InStencilStoreAction),
    DepthStencilAccess(InDepthStencilAccess)
  {
    Validate();
  }

  void Validate() const
  {
    CHECKF(DepthStencilAccess.IsDepthWrite() || DepthStoreAction == ERenderTargetStoreAction::ENoAction, TEXT("Depth is read-only, but we are performing a store.  This is a waste on mobile.  If depth can't change, we don't need to store it out again"));
    CHECKF(DepthStencilAccess.IsStencilWrite() || StencilStoreAction == ERenderTargetStoreAction::ENoAction, TEXT("Stencil is read-only, but we are performing a store.  This is a waste on mobile.  If stencil can't change, we don't need to store it out again"));
  }

  bool operator==(const RBRHIDepthRenderTargetView& Other) const
  {
    return
      Texture == Other.Texture &&
      DepthLoadAction == Other.DepthLoadAction &&
      DepthStoreAction == Other.DepthStoreAction &&
      StencilLoadAction == Other.StencilLoadAction &&
      StencilStoreAction == Other.StencilStoreAction &&
      DepthStencilAccess == Other.DepthStencilAccess;
  }
};
class RBRHISetRenderTargetsInfo
{
public:
  // Color Render Targets Info
  RBRHIRenderTargetView ColorRenderTarget[MaxSimultaneousRenderTargets];
  int32 NumColorRenderTargets;
  bool bClearColor;

  // Depth/Stencil Render Target Info
  RBRHIDepthRenderTargetView DepthStencilRenderTarget;
  bool bClearDepth;
  bool bClearStencil;

  // UAVs info.
  RBUnorderedAccessViewRHIRef UnorderedAccessView[MaxSimultaneousUAVs];
  int32 NumUAVs;

  RBRHISetRenderTargetsInfo() :
    NumColorRenderTargets(0),
    bClearColor(false),
    bClearDepth(false),
    bClearStencil(false),
    NumUAVs(0)
  {}

  RBRHISetRenderTargetsInfo(int32 InNumColorRenderTargets, const RBRHIRenderTargetView* InColorRenderTargets, const RBRHIDepthRenderTargetView& InDepthStencilRenderTarget) :
    NumColorRenderTargets(InNumColorRenderTargets),
    bClearColor(InNumColorRenderTargets > 0 && InColorRenderTargets[0].LoadAction == ERenderTargetLoadAction::EClear),
    DepthStencilRenderTarget(InDepthStencilRenderTarget),
    bClearDepth(InDepthStencilRenderTarget.Texture && InDepthStencilRenderTarget.DepthLoadAction == ERenderTargetLoadAction::EClear),
    bClearStencil(InDepthStencilRenderTarget.Texture && InDepthStencilRenderTarget.StencilLoadAction == ERenderTargetLoadAction::EClear),
    NumUAVs(0)
  {
    CHECK(InNumColorRenderTargets <= 0 || InColorRenderTargets);
    for (int32 Index = 0; Index < InNumColorRenderTargets; ++Index)
    {
      ColorRenderTarget[Index] = InColorRenderTargets[Index];
    }
  }
  // @todo metal mrt: This can go away after all the cleanup is done
  void SetClearDepthStencil(bool bInClearDepth, bool bInClearStencil = false)
  {
    if (bInClearDepth)
    {
      DepthStencilRenderTarget.DepthLoadAction = ERenderTargetLoadAction::EClear;
    }
    if (bInClearStencil)
    {
      DepthStencilRenderTarget.StencilLoadAction = ERenderTargetLoadAction::EClear;
    }
    bClearDepth = bInClearDepth;
    bClearStencil = bInClearStencil;
  }
#if 0
  uint32 CalculateHash() const
  {
    // Need a separate struct so we can memzero/remove dependencies on reference counts
    struct FHashableStruct
    {
      // Depth goes in the last slot
      FRHITexture* Texture[MaxSimultaneousRenderTargets + 1];
      uint32 MipIndex[MaxSimultaneousRenderTargets];
      uint32 ArraySliceIndex[MaxSimultaneousRenderTargets];
      ERenderTargetLoadAction LoadAction[MaxSimultaneousRenderTargets];
      ERenderTargetStoreAction StoreAction[MaxSimultaneousRenderTargets];

      ERenderTargetLoadAction		DepthLoadAction;
      ERenderTargetStoreAction	DepthStoreAction;
      ERenderTargetLoadAction		StencilLoadAction;
      ERenderTargetStoreAction	StencilStoreAction;
      FExclusiveDepthStencil		DepthStencilAccess;

      bool bClearDepth;
      bool bClearStencil;
      bool bClearColor;
      FRHIUnorderedAccessView* UnorderedAccessView[MaxSimultaneousUAVs];

      void Set(const FRHISetRenderTargetsInfo& RTInfo)
      {
        FMemory::Memzero(*this);
        for (int32 Index = 0; Index < RTInfo.NumColorRenderTargets; ++Index)
        {
          Texture[Index] = RTInfo.ColorRenderTarget[Index].Texture;
          MipIndex[Index] = RTInfo.ColorRenderTarget[Index].MipIndex;
          ArraySliceIndex[Index] = RTInfo.ColorRenderTarget[Index].ArraySliceIndex;
          LoadAction[Index] = RTInfo.ColorRenderTarget[Index].LoadAction;
          StoreAction[Index] = RTInfo.ColorRenderTarget[Index].StoreAction;
        }

        Texture[MaxSimultaneousRenderTargets] = RTInfo.DepthStencilRenderTarget.Texture;
        DepthLoadAction = RTInfo.DepthStencilRenderTarget.DepthLoadAction;
        DepthStoreAction = RTInfo.DepthStencilRenderTarget.DepthStoreAction;
        StencilLoadAction = RTInfo.DepthStencilRenderTarget.StencilLoadAction;
        StencilStoreAction = RTInfo.DepthStencilRenderTarget.GetStencilStoreAction();
        DepthStencilAccess = RTInfo.DepthStencilRenderTarget.GetDepthStencilAccess();

        bClearDepth = RTInfo.bClearDepth;
        bClearStencil = RTInfo.bClearStencil;
        bClearColor = RTInfo.bClearColor;

        for (int32 Index = 0; Index < MaxSimultaneousUAVs; ++Index)
        {
          UnorderedAccessView[Index] = RTInfo.UnorderedAccessView[Index];
        }
      }
    };

    FHashableStruct RTHash;
    rb_memzero(RTHash,sizeof(RTHash));
    RTHash.Set(*this);
    return FCrc::MemCrc32(&RTHash, sizeof(RTHash));
  }
#endif
};
class RBRHICustomPresent : public RBRHIResource
{
public:
  explicit RBRHICustomPresent(RBRHIViewport* InViewport)
    : RBRHIResource(true)
    , ViewportRHI(InViewport)
  {
  }

  virtual ~RBRHICustomPresent() {} // should release any references to D3D resources.

  // Called when viewport is resized.
  virtual void OnBackBufferResize() = 0;

  // @param InOutSyncInterval - in out param, indicates if vsync is on (>0) or off (==0).
  // @return	true if normal Present should be performed; false otherwise. If it returns
  // true, then InOutSyncInterval could be modified to switch between VSync/NoVSync for the normal Present.
  virtual bool Present(int32& InOutSyncInterval) = 0;

  // Called when rendering thread is acquired
  virtual void OnAcquireThreadOwnership() {}
  // Called when rendering thread is released
  virtual void OnReleaseThreadOwnership() {}

protected:
  // Weak reference, don't create a circular dependency that would prevent the viewport from being destroyed.
  RBRHIViewport* ViewportRHI;
};

typedef RBRHICustomPresent*              FCustomPresentRHIParamRef;
typedef TRefCountPtr<RBRHICustomPresent> FCustomPresentRHIRef;

// Template magic to convert an FRHI*Shader to its enum
template<typename TRHIShader> struct TRHIShaderToEnum {};
template<> struct TRHIShaderToEnum<RBRHIVertexShader>	{ enum { ShaderFrequency = SF_Vertex }; };
template<> struct TRHIShaderToEnum<RBRHIHullShader>		{ enum { ShaderFrequency = SF_Hull }; };
template<> struct TRHIShaderToEnum<RBRHIDomainShader>	{ enum { ShaderFrequency = SF_Domain }; };
template<> struct TRHIShaderToEnum<RBRHIPixelShader>		{ enum { ShaderFrequency = SF_Pixel }; };
template<> struct TRHIShaderToEnum<RBRHIGeometryShader>	{ enum { ShaderFrequency = SF_Geometry }; };
template<> struct TRHIShaderToEnum<RBRHIComputeShader>	{ enum { ShaderFrequency = SF_Compute }; };
template<> struct TRHIShaderToEnum<RBVertexShaderRHIParamRef>	{ enum { ShaderFrequency = SF_Vertex }; };
template<> struct TRHIShaderToEnum<RBHullShaderRHIParamRef>		{ enum { ShaderFrequency = SF_Hull }; };
template<> struct TRHIShaderToEnum<RBDomainShaderRHIParamRef>	{ enum { ShaderFrequency = SF_Domain }; };
template<> struct TRHIShaderToEnum<RBPixelShaderRHIParamRef>		{ enum { ShaderFrequency = SF_Pixel }; };
template<> struct TRHIShaderToEnum<RBGeometryShaderRHIParamRef>	{ enum { ShaderFrequency = SF_Geometry }; };
template<> struct TRHIShaderToEnum<RBComputeShaderRHIParamRef>	{ enum { ShaderFrequency = SF_Compute }; };
template<> struct TRHIShaderToEnum<RBVertexShaderRHIRef>		{ enum { ShaderFrequency = SF_Vertex }; };
template<> struct TRHIShaderToEnum<RBHullShaderRHIRef>		{ enum { ShaderFrequency = SF_Hull }; };
template<> struct TRHIShaderToEnum<RBDomainShaderRHIRef>		{ enum { ShaderFrequency = SF_Domain }; };
template<> struct TRHIShaderToEnum<RBPixelShaderRHIRef>		{ enum { ShaderFrequency = SF_Pixel }; };
template<> struct TRHIShaderToEnum<RBGeometryShaderRHIRef>	{ enum { ShaderFrequency = SF_Geometry }; };
template<> struct TRHIShaderToEnum<RBComputeShaderRHIRef>	{ enum { ShaderFrequency = SF_Compute }; };
