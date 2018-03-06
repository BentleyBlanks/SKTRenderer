#pragma once 
#include "Colorf.h"
#include "Assertion.h"
#include <vector>

enum class ERHIZBuffer
{
  // Before changing this, make sure all math & shader assumptions are correct! Also wrap your C++ assumptions with
  //		static_assert(ERHIZBuffer::IsInvertedZBuffer(), ...);
  // Shader-wise, make sure to update Definitions.usf, HAS_INVERTED_Z_BUFFER
  FarPlane = 0,
  NearPlane = 1,

  // 'bool' for knowing if the API is using Inverted Z buffer
  IsInverted = (int32)((int32)ERHIZBuffer::FarPlane < (int32)ERHIZBuffer::NearPlane),
};

enum ERenderQueryType
{
	// e.g. WaitForFrameEventCompletion()
	RQT_Undefined,
	// Result is the number of samples that are not culled (divide by MSAACount to get pixels)
	RQT_Occlusion,
	// Result is time in micro seconds = 1/1000 ms = 1/1000000 sec
	RQT_AbsoluteTime,
};

enum ERHITYPES
{
  RHI_D3D11 = 0,
  RHI_D3D12 ,
  RHI_TOTAL ,
};


enum ESamplerFilter
{
	SF_Point,
	SF_Bilinear,
	SF_Trilinear,
	SF_AnisotropicPoint,
	SF_AnisotropicLinear,
};

enum ESamplerAddressMode
{
	AM_Wrap,
	AM_Clamp,
	AM_Mirror,
	/** Not supported on all platforms */
	AM_Border
};

enum ESamplerCompareFunction
{
	SCF_Never,
	SCF_Less
};

enum ERasterizerFillMode
{
	FM_Point,
	FM_Wireframe,
	FM_Solid
};

enum ERasterizerCullMode
{
	CM_None,
	CM_CW,
	CM_CCW
};

enum EColorWriteMask
{
	CW_RED = 0x01,
	CW_GREEN = 0x02,
	CW_BLUE = 0x04,
	CW_ALPHA = 0x08,

	CW_NONE = 0,
	CW_RGB = CW_RED | CW_GREEN | CW_BLUE,
	CW_RGBA = CW_RED | CW_GREEN | CW_BLUE | CW_ALPHA,
	CW_RG = CW_RED | CW_GREEN,
	CW_BA = CW_BLUE | CW_ALPHA,
};

enum ECompareFunction
{
	CF_Less,
	CF_LessEqual,
	CF_Greater,
	CF_GreaterEqual,
	CF_Equal,
	CF_NotEqual,
	CF_Never,
	CF_Always,

	// Utility enumerations
	CF_DepthNearOrEqual = (((int32)ERHIZBuffer::IsInverted != 0) ? CF_GreaterEqual : CF_LessEqual),
	CF_DepthNear = (((int32)ERHIZBuffer::IsInverted != 0) ? CF_Greater : CF_Less),
	CF_DepthFartherOrEqual = (((int32)ERHIZBuffer::IsInverted != 0) ? CF_LessEqual : CF_GreaterEqual),
	CF_DepthFarther = (((int32)ERHIZBuffer::IsInverted != 0) ? CF_Less : CF_Greater),
};

enum EStencilOp
{
	SO_Keep,
	SO_Zero,
	SO_Replace,
	SO_SaturatedIncrement,
	SO_SaturatedDecrement,
	SO_Invert,
	SO_Increment,
	SO_Decrement
};

enum EBlendOperation
{
	BO_Add,
	BO_Subtract,
	BO_Min,
	BO_Max,
	BO_ReverseSubtract,
};

enum EBlendFactor
{
	BF_Zero,
	BF_One,
	BF_SourceColor,
	BF_InverseSourceColor,
	BF_SourceAlpha,
	BF_InverseSourceAlpha,
	BF_DestAlpha,
	BF_InverseDestAlpha,
	BF_DestColor,
	BF_InverseDestColor,
	BF_ConstantBlendFactor,
	BF_InverseConstantBlendFactor
};


enum EVertexElementType
{
	VET_None,
	VET_Float1,
	VET_Float2,
	VET_Float3,
	VET_Float4,
	VET_PackedNormal,	// FPackedNormal
	VET_UByte4,
	VET_UByte4N,
	VET_Color,
	VET_Short2,
	VET_Short4,
	VET_Short2N,		// 16 bit word normalized to (value/32767.0,value/32767.0,0,0,1)
	VET_Half2,			// 16 bit float using 1 bit sign, 5 bit exponent, 10 bit mantissa 
	VET_Half4,
	VET_Short4N,		// 4 X 16 bit word, normalized 
	VET_UShort2,
	VET_UShort4,
	VET_UShort2N,		// 16 bit word normalized to (value/65535.0,value/65535.0,0,0,1)
	VET_UShort4N,		// 4 X 16 bit word unsigned, normalized 
	VET_URGB10A2N,		// 10 bit r, g, b and 2 bit a normalized to (value/1023.0f, value/1023.0f, value/1023.0f, value/3.0f)
	VET_MAX
};

enum EShaderFrequency
{
  SF_Vertex = 0,
  SF_Hull = 1,
  SF_Domain = 2,
  SF_Pixel = 3,
  SF_Geometry = 4,
  SF_Compute = 5,

  SF_NumFrequencies = 6,

  SF_NumBits = 3,
};
static_assert(SF_NumFrequencies <= (1 << SF_NumBits), "SF_NumFrequencies will not fit on SF_NumBits");

enum EPrimitiveType
{
  PT_TriangleList,
  PT_TriangleStrip,
  PT_LineList,
  PT_QuadList,
  PT_PointList,
  PT_1_ControlPointPatchList,
  PT_2_ControlPointPatchList,
  PT_3_ControlPointPatchList,
  PT_4_ControlPointPatchList,
  PT_5_ControlPointPatchList,
  PT_6_ControlPointPatchList,
  PT_7_ControlPointPatchList,
  PT_8_ControlPointPatchList,
  PT_9_ControlPointPatchList,
  PT_10_ControlPointPatchList,
  PT_11_ControlPointPatchList,
  PT_12_ControlPointPatchList,
  PT_13_ControlPointPatchList,
  PT_14_ControlPointPatchList,
  PT_15_ControlPointPatchList,
  PT_16_ControlPointPatchList,
  PT_17_ControlPointPatchList,
  PT_18_ControlPointPatchList,
  PT_19_ControlPointPatchList,
  PT_20_ControlPointPatchList,
  PT_21_ControlPointPatchList,
  PT_22_ControlPointPatchList,
  PT_23_ControlPointPatchList,
  PT_24_ControlPointPatchList,
  PT_25_ControlPointPatchList,
  PT_26_ControlPointPatchList,
  PT_27_ControlPointPatchList,
  PT_28_ControlPointPatchList,
  PT_29_ControlPointPatchList,
  PT_30_ControlPointPatchList,
  PT_31_ControlPointPatchList,
  PT_32_ControlPointPatchList,
  PT_Num,
  PT_NumBits = 6
};

/**
* Action to take when a rendertarget is set.
*/
enum class ERenderTargetLoadAction
{
	ENoAction,
	ELoad,
	EClear,
};

/**
* Action to take when a rendertarget is unset or at the end of a pass.
*/
enum class ERenderTargetStoreAction
{
	ENoAction,
	EStore,
	EMultisampleResolve,
};

struct RBVRamAllocation
{
	RBVRamAllocation(uint32 InAllocationStart = 0, uint32 InAllocationSize = 0)
		: AllocationStart(InAllocationStart)
		, AllocationSize(InAllocationSize)
	{
	}

	bool IsValid() { return AllocationSize > 0; }

	// in bytes
	uint32 AllocationStart;
	// in bytes
	uint32 AllocationSize;
};

struct RBRHIResourceInfo
{
	RBVRamAllocation VRamAllocation;
};

enum class EClearBinding
{
	ENoneBound, //no clear color associated with this target.  Target will not do hardware clears on most platforms
	EColorBound, //target has a clear color bound.  Clears will use the bound color, and do hardware clears.
	EDepthStencilBound, //target has a depthstencil value bound.  Clears will use the bound values and do hardware clears.
};
enum EUniformBufferUsage
{
  // the uniform buffer is temporary, used for a single draw call then discarded
  UniformBuffer_SingleDraw = 0,
  // the uniform buffer is used for multiple draw calls but only for the current frame
  UniformBuffer_SingleFrame,
  // the uniform buffer is used for multiple draw calls, possibly across multiple frames
  UniformBuffer_MultiFrame,
};

/** Maximum number of miplevels in a texture. */
enum { MAX_TEXTURE_MIP_COUNT = 14 };

/** The maximum number of vertex elements which can be used by a vertex declaration. */
enum { MaxVertexElementCount = 16 };

/** The alignment in bytes between elements of array shader parameters. */
enum { ShaderArrayElementAlignBytes = 16 };

/** The number of render-targets that may be simultaneously written to. */
enum { MaxSimultaneousRenderTargets = 8 };

/** The number of UAVs that may be simultaneously bound to a shader. */
enum { MaxSimultaneousUAVs = 8 };



enum ECubeFace
{
	CubeFace_PosX = 0,
	CubeFace_NegX,
	CubeFace_PosY,
	CubeFace_NegY,
	CubeFace_PosZ,
	CubeFace_NegZ,
	CubeFace_MAX
};

/** The base type of a value in a uniform buffer. */
enum EUniformBufferBaseType
{
	UBMT_INVALID,
	UBMT_BOOL,
	UBMT_INT32,
	UBMT_UINT32,
	UBMT_FLOAT32,
	UBMT_STRUCT,
	UBMT_SRV,
	UBMT_UAV,
	UBMT_SAMPLER,
	UBMT_TEXTURE
};

/**
*	Resource usage flags - for vertex and index buffers.
*/
enum EBufferUsageFlags
{
	// Mutually exclusive write-frequency flags
	BUF_Static = 0x0001, // The buffer will be written to once.
	BUF_Dynamic = 0x0002, // The buffer will be written to occasionally, GPU read only, CPU write only.  The data lifetime is until the next update, or the buffer is destroyed.
	BUF_Volatile = 0x0004, // The buffer's data will have a lifetime of one frame.  It MUST be written to each frame, or a new one created each frame.

	// Mutually exclusive bind flags.
	BUF_UnorderedAccess = 0x0008, // Allows an unordered access view to be created for the buffer.

	/** Create a byte address buffer, which is basically a structured buffer with a uint32 type. */
	BUF_ByteAddressBuffer = 0x0020,
	/** Create a structured buffer with an atomic UAV counter. */
	BUF_UAVCounter = 0x0040,
	/** Create a buffer that can be bound as a stream output target. */
	BUF_StreamOutput = 0x0080,
	/** Create a buffer which contains the arguments used by DispatchIndirect or DrawIndirect. */
	BUF_DrawIndirect = 0x0100,
	/**
	* Create a buffer that can be bound as a shader resource.
	* This is only needed for buffer types which wouldn't ordinarily be used as a shader resource, like a vertex buffer.
	*/
	BUF_ShaderResource = 0x0200,

	/**
	* Request that this buffer is directly CPU accessible
	* (@todo josh: this is probably temporary and will go away in a few months)
	*/
	BUF_KeepCPUAccessible = 0x0400,

	/**
	* Provide information that this buffer will contain only one vertex, which should be delivered to every primitive drawn.
	* This is necessary for OpenGL implementations, which need to handle this case very differently (and can't handle GL_HALF_FLOAT in such vertices at all).
	*/
	BUF_ZeroStride = 0x0800,

	/** Buffer should go in fast vram (hint only) */
	BUF_FastVRAM = 0x1000,

	// Helper bit-masks
	BUF_AnyDynamic = (BUF_Dynamic | BUF_Volatile),
};

/** An enumeration of the different RHI reference types. */
enum ERHIResourceType
{
	RRT_None,

	RRT_SamplerState,
	RRT_RasterizerState,
	RRT_DepthStencilState,
	RRT_BlendState,
	RRT_VertexDeclaration,
	RRT_VertexShader,
	RRT_HullShader,
	RRT_DomainShader,
	RRT_PixelShader,
	RRT_GeometryShader,
	RRT_ComputeShader,
	RRT_BoundShaderState,
	RRT_UniformBuffer,
	RRT_IndexBuffer,
	RRT_VertexBuffer,
	RRT_StructuredBuffer,
	RRT_Texture,
	RRT_Texture2D,
	RRT_Texture2DArray,
	RRT_Texture3D,
	RRT_TextureCube,
	RRT_TextureReference,
	RRT_RenderQuery,
	RRT_Viewport,
	RRT_UnorderedAccessView,
	RRT_ShaderResourceView,

	RRT_Num
};

/** Flags used for texture creation */
enum ETextureCreateFlags
{
	TexCreate_None = 0,

	// Texture can be used as a render target
	TexCreate_RenderTargetable = 1 << 0,
	// Texture can be used as a resolve target
	TexCreate_ResolveTargetable = 1 << 1,
	// Texture can be used as a depth-stencil target.
	TexCreate_DepthStencilTargetable = 1 << 2,
	// Texture can be used as a shader resource.
	TexCreate_ShaderResource = 1 << 3,

	// Texture is encoded in sRGB gamma space
	TexCreate_SRGB = 1 << 4,
	// Texture will be created without a packed miptail
	TexCreate_NoMipTail = 1 << 5,
	// Texture will be created with an un-tiled format
	TexCreate_NoTiling = 1 << 6,
	// Texture that may be updated every frame
	TexCreate_Dynamic = 1 << 8,
	// Allow silent texture creation failure
	// @warning:	When you update this, you must update FTextureAllocations::FindTextureType() in Core/Private/UObject/TextureAllocations.cpp
	TexCreate_AllowFailure = 1 << 9,
	// Disable automatic defragmentation if the initial texture memory allocation fails.
	// @warning:	When you update this, you must update FTextureAllocations::FindTextureType() in Core/Private/UObject/TextureAllocations.cpp
	TexCreate_DisableAutoDefrag = 1 << 10,
	// Create the texture with automatic -1..1 biasing
	TexCreate_BiasNormalMap = 1 << 11,
	// Create the texture with the flag that allows mip generation later, only applicable to D3D11
	TexCreate_GenerateMipCapable = 1 << 12,
	// UnorderedAccessView (DX11 only)
	// Warning: Causes additional synchronization between draw calls when using a render target allocated with this flag, use sparingly
	// See: GCNPerformanceTweets.pdf Tip 37
	TexCreate_UAV = 1 << 16,
	// Render target texture that will be displayed on screen (back buffer)
	TexCreate_Presentable = 1 << 17,
	// Texture data is accessible by the CPU
	TexCreate_CPUReadback = 1 << 18,
	// Texture was processed offline (via a texture conversion process for the current platform)
	TexCreate_OfflineProcessed = 1 << 19,
	// Texture needs to go in fast VRAM if available (HINT only)
	TexCreate_FastVRAM = 1 << 20,
	// by default the texture is not showing up in the list - this is to reduce clutter, using the FULL option this can be ignored
	TexCreate_HideInVisualizeTexture = 1 << 21,
	// Texture should be created in virtual memory, with no physical memory allocation made
	// You must make further calls to RHIVirtualTextureSetFirstMipInMemory to allocate physical memory
	// and RHIVirtualTextureSetFirstMipVisible to map the first mip visible to the GPU
	TexCreate_Virtual = 1 << 22,
	// Creates a RenderTargetView for each array slice of the texture
	// Warning: if this was specified when the resource was created, you can't use SV_RenderTargetArrayIndex to route to other slices!
	TexCreate_TargetArraySlicesIndependently = 1 << 23,
	// Texture that may be shared with DX9 or other devices
	TexCreate_Shared = 1 << 24,
	// RenderTarget will not use full-texture fast clear functionality.
	TexCreate_NoFastClear = 1 << 25,
	// Texture is a depth stencil resolve target
	TexCreate_DepthStencilResolveTarget = 1 << 26,
	// RenderTarget will create with delta color compression
	TexCreate_DeltaColorCompression = 1 << 27,
};

struct RBClearValueBinding
{
	struct DSVAlue
	{
		float Depth;
		uint32 Stencil;
	};

	RBClearValueBinding()
		: ColorBinding(EClearBinding::EColorBound)
	{
		Value.Color[0] = 0.0f;
		Value.Color[1] = 0.0f;
		Value.Color[2] = 0.0f;
		Value.Color[3] = 0.0f;
	}

	RBClearValueBinding(EClearBinding NoBinding)
		: ColorBinding(NoBinding)
	{
		CHECK(ColorBinding == EClearBinding::ENoneBound);
	}

	explicit RBClearValueBinding(const RBColorf& InClearColor)
		: ColorBinding(EClearBinding::EColorBound)
	{
		Value.Color[0] = InClearColor.r;
		Value.Color[1] = InClearColor.g;
		Value.Color[2] = InClearColor.b;
		Value.Color[3] = InClearColor.a;
	}

	explicit RBClearValueBinding(float DepthClearValue, uint32 StencilClearValue = 0)
		: ColorBinding(EClearBinding::EDepthStencilBound)
	{
		Value.DSValue.Depth = DepthClearValue;
		Value.DSValue.Stencil = StencilClearValue;
	}

	RBColorf GetClearColor() const
	{
		CHECK(ColorBinding == EClearBinding::EColorBound);
		return RBColorf(Value.Color[0], Value.Color[1], Value.Color[2], Value.Color[3]);
	}

	void GetDepthStencil(float& OutDepth, uint32& OutStencil) const
	{
		CHECK(ColorBinding == EClearBinding::EDepthStencilBound);
		OutDepth = Value.DSValue.Depth;
		OutStencil = Value.DSValue.Stencil;
	}

	bool operator==(const RBClearValueBinding& Other) const
	{
		if (ColorBinding == Other.ColorBinding)
		{
			if (ColorBinding == EClearBinding::EColorBound)
			{
				return
					Value.Color[0] == Other.Value.Color[0] &&
					Value.Color[1] == Other.Value.Color[1] &&
					Value.Color[2] == Other.Value.Color[2] &&
					Value.Color[3] == Other.Value.Color[3];

			}
			if (ColorBinding == EClearBinding::EDepthStencilBound)
			{
				return
					Value.DSValue.Depth == Other.Value.DSValue.Depth &&
					Value.DSValue.Stencil == Other.Value.DSValue.Stencil;
			}
			return true;
		}
		return false;
	}

	EClearBinding ColorBinding;

	union ClearValueType
	{
		float Color[4];
		DSVAlue DSValue;
	} Value;

	// common clear values
	static const RBClearValueBinding None;
	static const RBClearValueBinding Black;
	static const RBClearValueBinding White;
	static const RBClearValueBinding Transparent;
	static const RBClearValueBinding DepthOne;
	static const RBClearValueBinding DepthZero;
	static const RBClearValueBinding DepthNear;
	static const RBClearValueBinding DepthFar;
};
enum class EResourceTransitionAccess
{
	EReadable, //transition from write-> read
	EWritable, //transition from read -> write	
	ERWBarrier, // Mostly for UAVs.  Transition to read/write state and always insert a resource barrier.
	ERWNoBarrier, //Mostly UAVs.  Indicates we want R/W access and do not require synchronization for the duration of the RW state.  The initial transition from writable->RWNoBarrier and readable->RWNoBarrier still requires a sync
	ERWSubResBarrier, //For special cases where read/write happens to different subresources of the same resource in the same call.  Inserts a barrier, but read validation will pass.  Temporary until we pass full subresource info to all transition calls.
	EMaxAccess,
};

enum EResourceLockMode
{
	RLM_ReadOnly,
	RLM_WriteOnly,
	RLM_Num
};

/**
* Hint to the driver on how to load balance async compute work.  On some platforms this may be a priority, on others actually masking out parts of the GPU for types of work.
*/
enum class EAsyncComputeBudget
{
	ELeast_0,			//Least amount of GPU allocated to AsyncCompute that still gets 'some' done.
	EGfxHeavy_1,		//Gfx gets most of the GPU.
	EBalanced_2,		//Async compute and Gfx share GPU equally.
	EComputeHeavy_3,	//Async compute can use most of the GPU
	EAll_4,				//Async compute can use the entire GPU.
};

enum class EResourceTransitionPipeline
{
	EGfxToCompute,
	EComputeToGfx,
	EGfxToGfx,
	EComputeToCompute,
};

#if 0
struct FResolveParams
{
	/** used to specify face when resolving to a cube map texture */
	ECubeFace CubeFace;
	/** resolve RECT bounded by [X1,Y1]..[X2,Y2]. Or -1 for fullscreen */
	FResolveRect Rect;
	/** The mip index to resolve in both source and dest. */
	int32 MipIndex;
	/** Array index to resolve in the source. */
	int32 SourceArrayIndex;
	/** Array index to resolve in the dest. */
	int32 DestArrayIndex;

	/** constructor */
	FResolveParams(
		const FResolveRect& InRect = FResolveRect(),
		ECubeFace InCubeFace = CubeFace_PosX,
		int32 InMipIndex = 0,
		int32 InSourceArrayIndex = 0,
		int32 InDestArrayIndex = 0)
		: CubeFace(InCubeFace)
		, Rect(InRect)
		, MipIndex(InMipIndex)
		, SourceArrayIndex(InSourceArrayIndex)
		, DestArrayIndex(InDestArrayIndex)
	{}

	FORCEINLINE FResolveParams(const FResolveParams& Other)
		: CubeFace(Other.CubeFace)
		, Rect(Other.Rect)
		, MipIndex(Other.MipIndex)
		, SourceArrayIndex(Other.SourceArrayIndex)
		, DestArrayIndex(Other.DestArrayIndex)
	{}
};
#endif

/**
*	Screen Resolution
*/
struct RBScreenResolutionRHI
{
	uint32	Width;
	uint32	Height;
	uint32	RefreshRate;
};

/**
*	Viewport bounds structure to set multiple view ports for the geometry shader
*  (needs to be 1:1 to the D3D11 structure)
*/
struct RBViewportBounds
{
	float	TopLeftX;
	float	TopLeftY;
	float	Width;
	float	Height;
	float	MinDepth;
	float	MaxDepth;

	RBViewportBounds() {}

	RBViewportBounds(float InTopLeftX, float InTopLeftY, float InWidth, float InHeight, float InMinDepth = 0.0f, float InMaxDepth = 1.0f)
		:TopLeftX(InTopLeftX), TopLeftY(InTopLeftY), Width(InWidth), Height(InHeight), MinDepth(InMinDepth), MaxDepth(InMaxDepth)
	{
	}

};

struct RBVertexElement
{
	uint8 StreamIndex;
	uint8 Offset;
	EVertexElementType Type;
	uint8 AttributeIndex;
	uint16 Stride;
	/**
	* Whether to use instance index or vertex index to consume the element.
	* eg if bUseInstanceIndex is 0, the element will be repeated for every instance.
	*/
	uint16 bUseInstanceIndex;

	RBVertexElement() {}
	RBVertexElement(uint8 InStreamIndex, uint8 InOffset, EVertexElementType InType, uint8 InAttributeIndex, uint16 InStride, bool bInUseInstanceIndex = false) :
		StreamIndex(InStreamIndex),
		Offset(InOffset),
		Type(InType),
		AttributeIndex(InAttributeIndex),
		Stride(InStride),
		bUseInstanceIndex(bInUseInstanceIndex)
	{}
	/**
	* Suppress the compiler generated assignment operator so that padding won't be copied.
	* This is necessary to get expected results for code that zeros, assigns and then CRC's the whole struct.
	*/
	void operator=(const RBVertexElement& Other)
	{
		StreamIndex = Other.StreamIndex;
		Offset = Other.Offset;
		Type = Other.Type;
		AttributeIndex = Other.AttributeIndex;
		bUseInstanceIndex = Other.bUseInstanceIndex;
	}

};

typedef std::vector<RBVertexElement > RBVertexDeclarationElementList;


struct RBSamplerStateInitializerRHI
{
	RBSamplerStateInitializerRHI() {}
	RBSamplerStateInitializerRHI(
		ESamplerFilter InFilter,
		ESamplerAddressMode InAddressU = AM_Wrap,
		ESamplerAddressMode InAddressV = AM_Wrap,
		ESamplerAddressMode InAddressW = AM_Wrap,
		int32 InMipBias = 0,
		int32 InMaxAnisotropy = 0,
		f32 InMinMipLevel = 0,
		f32 InMaxMipLevel = MAX_F32,
		uint32 InBorderColor = 0,
		/** Only supported in D3D11 */
		ESamplerCompareFunction InSamplerComparisonFunction = SCF_Never
		)
		: Filter(InFilter)
		, AddressU(InAddressU)
		, AddressV(InAddressV)
		, AddressW(InAddressW)
		, MipBias(InMipBias)
		, MinMipLevel(InMinMipLevel)
		, MaxMipLevel(InMaxMipLevel)
		, MaxAnisotropy(InMaxAnisotropy)
		, BorderColor(InBorderColor)
		, SamplerComparisonFunction(InSamplerComparisonFunction)
	{
	}
	ESamplerFilter Filter;
	ESamplerAddressMode AddressU;
	ESamplerAddressMode AddressV;
	ESamplerAddressMode AddressW;
	int32 MipBias;
	/** Smallest mip map level that will be used, where 0 is the highest resolution mip level. */
	f32 MinMipLevel;
	/** Largest mip map level that will be used, where 0 is the highest resolution mip level. */
	f32 MaxMipLevel;
	int32 MaxAnisotropy;
	uint32 BorderColor;
	ESamplerCompareFunction SamplerComparisonFunction;

};
struct RBRasterizerStateInitializerRHI
{
	ERasterizerFillMode FillMode;
	ERasterizerCullMode CullMode;
	float DepthBias;
	float SlopeScaleDepthBias;
	bool bAllowMSAA;
	bool bEnableLineAA;

};
struct RBDepthStencilStateInitializerRHI
{
	bool bEnableDepthWrite;
	ECompareFunction DepthTest;

	bool bEnableFrontFaceStencil;
	ECompareFunction FrontFaceStencilTest;
	EStencilOp FrontFaceStencilFailStencilOp;
	EStencilOp FrontFaceDepthFailStencilOp;
	EStencilOp FrontFacePassStencilOp;
	bool bEnableBackFaceStencil;
	ECompareFunction BackFaceStencilTest;
	EStencilOp BackFaceStencilFailStencilOp;
	EStencilOp BackFaceDepthFailStencilOp;
	EStencilOp BackFacePassStencilOp;
	uint8 StencilReadMask;
	uint8 StencilWriteMask;

	RBDepthStencilStateInitializerRHI(
		bool bInEnableDepthWrite = true,
		ECompareFunction InDepthTest = CF_LessEqual,
		bool bInEnableFrontFaceStencil = false,
		ECompareFunction InFrontFaceStencilTest = CF_Always,
		EStencilOp InFrontFaceStencilFailStencilOp = SO_Keep,
		EStencilOp InFrontFaceDepthFailStencilOp = SO_Keep,
		EStencilOp InFrontFacePassStencilOp = SO_Keep,
		bool bInEnableBackFaceStencil = false,
		ECompareFunction InBackFaceStencilTest = CF_Always,
		EStencilOp InBackFaceStencilFailStencilOp = SO_Keep,
		EStencilOp InBackFaceDepthFailStencilOp = SO_Keep,
		EStencilOp InBackFacePassStencilOp = SO_Keep,
		uint8 InStencilReadMask = 0xFF,
		uint8 InStencilWriteMask = 0xFF
		)
		: bEnableDepthWrite(bInEnableDepthWrite)
		, DepthTest(InDepthTest)
		, bEnableFrontFaceStencil(bInEnableFrontFaceStencil)
		, FrontFaceStencilTest(InFrontFaceStencilTest)
		, FrontFaceStencilFailStencilOp(InFrontFaceStencilFailStencilOp)
		, FrontFaceDepthFailStencilOp(InFrontFaceDepthFailStencilOp)
		, FrontFacePassStencilOp(InFrontFacePassStencilOp)
		, bEnableBackFaceStencil(bInEnableBackFaceStencil)
		, BackFaceStencilTest(InBackFaceStencilTest)
		, BackFaceStencilFailStencilOp(InBackFaceStencilFailStencilOp)
		, BackFaceDepthFailStencilOp(InBackFaceDepthFailStencilOp)
		, BackFacePassStencilOp(InBackFacePassStencilOp)
		, StencilReadMask(InStencilReadMask)
		, StencilWriteMask(InStencilWriteMask)
	{}

};
class RBBlendStateInitializerRHI
{
public:

	struct FRenderTarget
	{
		EBlendOperation ColorBlendOp;
		EBlendFactor ColorSrcBlend;
		EBlendFactor ColorDestBlend;
		EBlendOperation AlphaBlendOp;
		EBlendFactor AlphaSrcBlend;
		EBlendFactor AlphaDestBlend;
		EColorWriteMask ColorWriteMask;

		FRenderTarget(
			EBlendOperation InColorBlendOp = BO_Add,
			EBlendFactor InColorSrcBlend = BF_One,
			EBlendFactor InColorDestBlend = BF_Zero,
			EBlendOperation InAlphaBlendOp = BO_Add,
			EBlendFactor InAlphaSrcBlend = BF_One,
			EBlendFactor InAlphaDestBlend = BF_Zero,
			EColorWriteMask InColorWriteMask = CW_RGBA
			)
			: ColorBlendOp(InColorBlendOp)
			, ColorSrcBlend(InColorSrcBlend)
			, ColorDestBlend(InColorDestBlend)
			, AlphaBlendOp(InAlphaBlendOp)
			, AlphaSrcBlend(InAlphaSrcBlend)
			, AlphaDestBlend(InAlphaDestBlend)
			, ColorWriteMask(InColorWriteMask)
		{}

	};

	RBBlendStateInitializerRHI() {}

	RBBlendStateInitializerRHI(const FRenderTarget& InRenderTargetBlendState)
		: bUseIndependentRenderTargetBlendStates(false)
	{
		RenderTargets[0] = InRenderTargetBlendState;
	}

	template<uint32 NumRenderTargets>
	RBBlendStateInitializerRHI(const std::vector<FRenderTarget>& InRenderTargetBlendStates)
		: bUseIndependentRenderTargetBlendStates(NumRenderTargets > 1)
	{
		static_assert(NumRenderTargets <= MaxSimultaneousRenderTargets, "Too many render target blend states.");
    RenderTargets.resize(NumRenderTargets);
		for (uint32 RenderTargetIndex = 0; RenderTargetIndex < NumRenderTargets; ++RenderTargetIndex)
		{
			RenderTargets[RenderTargetIndex] = InRenderTargetBlendStates[RenderTargetIndex];
		}
	}

	std::vector<FRenderTarget> RenderTargets;
	bool bUseIndependentRenderTargetBlendStates;


};

struct FRHIResourceTableEntry
{
public:
  static uint32 GetEndOfStreamToken()
  {
    return 0xffffffff;
  }

  static uint32 Create(uint16 UniformBufferIndex, uint16 ResourceIndex, uint16 BindIndex)
  {
    return ((UniformBufferIndex & RTD_Mask_UniformBufferIndex) << RTD_Shift_UniformBufferIndex) |
      ((ResourceIndex & RTD_Mask_ResourceIndex) << RTD_Shift_ResourceIndex) |
      ((BindIndex & RTD_Mask_BindIndex) << RTD_Shift_BindIndex);
  }

  static inline uint16 GetUniformBufferIndex(uint32 Data)
  {
    return (Data >> RTD_Shift_UniformBufferIndex) & RTD_Mask_UniformBufferIndex;
  }

  static inline uint16 GetResourceIndex(uint32 Data)
  {
    return (Data >> RTD_Shift_ResourceIndex) & RTD_Mask_ResourceIndex;
  }

  static inline uint16 GetBindIndex(uint32 Data)
  {
    return (Data >> RTD_Shift_BindIndex) & RTD_Mask_BindIndex;
  }

private:
  enum EResourceTableDefinitions
  {
    RTD_NumBits_UniformBufferIndex = 8,
    RTD_NumBits_ResourceIndex = 16,
    RTD_NumBits_BindIndex = 8,

    RTD_Mask_UniformBufferIndex = (1 << RTD_NumBits_UniformBufferIndex) - 1,
    RTD_Mask_ResourceIndex = (1 << RTD_NumBits_ResourceIndex) - 1,
    RTD_Mask_BindIndex = (1 << RTD_NumBits_BindIndex) - 1,

    RTD_Shift_BindIndex = 0,
    RTD_Shift_ResourceIndex = RTD_Shift_BindIndex + RTD_NumBits_BindIndex,
    RTD_Shift_UniformBufferIndex = RTD_Shift_ResourceIndex + RTD_NumBits_ResourceIndex,
  };
  static_assert(RTD_NumBits_UniformBufferIndex + RTD_NumBits_ResourceIndex + RTD_NumBits_BindIndex <= sizeof(uint32) * 8, "RTD_* values must fit in 32 bits");
};