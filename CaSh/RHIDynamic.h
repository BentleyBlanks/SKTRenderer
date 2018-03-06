#pragma once
#include "RHIDef.h"
#include "RHIResources.h"
#include "RHI.h"
#include "aabb.h"

/** Context that is capable of doing Compute work.  Can be async or compute on the gfx pipe. */
class IRHIComputeContext
{
public:
	/**
	* Compute queue will wait for the fence to be written before continuing.
	*/
	virtual void RHIWaitComputeFence(RBComputeFenceRHIParamRef InFence) = 0;

	/**
	*Sets the current compute shader.
	*/
	virtual void RHISetComputeShader(RBComputeShaderRHIParamRef ComputeShader) = 0;

	virtual void RHIDispatchComputeShader(uint32 ThreadGroupCountX, uint32 ThreadGroupCountY, uint32 ThreadGroupCountZ) = 0;

	virtual void RHIDispatchIndirectComputeShader(RBVertexBufferRHIParamRef ArgumentBuffer, uint32 ArgumentOffset) = 0;

	virtual void RHISetAsyncComputeBudget(EAsyncComputeBudget Budget) = 0;

	/**
	* Explicitly transition a UAV from readable -> writable by the GPU or vice versa.
	* Also explicitly states which pipeline the UAV can be used on next.  For example, if a Compute job just wrote this UAV for a Pixel shader to read
	* you would do EResourceTransitionAccess::Readable and EResourceTransitionPipeline::EComputeToGfx
	*
	* @param TransitionType - direction of the transition
	* @param EResourceTransitionPipeline - How this UAV is transitioning between Gfx and Compute, if at all.
	* @param InUAVs - array of UAV objects to transition
	* @param NumUAVs - number of UAVs to transition
	* @param WriteComputeFence - Optional ComputeFence to write as part of this transition
	*/
	virtual void RHITransitionResources(EResourceTransitionAccess TransitionType, EResourceTransitionPipeline TransitionPipeline, RBUnorderedAccessViewRHIParamRef* InUAVs, int32 NumUAVs, RBComputeFenceRHIParamRef WriteComputeFence) = 0;

	/** Set the shader resource view of a surface.  This is used for binding TextureMS parameter types that need a multi sampled view. */
	virtual void RHISetShaderTexture(RBComputeShaderRHIParamRef PixelShader, uint32 TextureIndex, RBTextureRHIParamRef NewTexture) = 0;

	/**
	* Sets sampler state.
	* @param GeometryShader	The geometry shader to set the sampler for.
	* @param SamplerIndex		The index of the sampler.
	* @param NewState			The new sampler state.
	*/
	virtual void RHISetShaderSampler(RBComputeShaderRHIParamRef ComputeShader, uint32 SamplerIndex, RBSamplerStateRHIParamRef NewState) = 0;

	/**
	* Sets a compute shader UAV parameter.
	* @param ComputeShader	The compute shader to set the UAV for.
	* @param UAVIndex		The index of the UAVIndex.
	* @param UAV			The new UAV.
	*/
	virtual void RHISetUAVParameter(RBComputeShaderRHIParamRef ComputeShader, uint32 UAVIndex, RBUnorderedAccessViewRHIParamRef UAV) = 0;

	/**
	* Sets a compute shader counted UAV parameter and initial count
	* @param ComputeShader	The compute shader to set the UAV for.
	* @param UAVIndex		The index of the UAVIndex.
	* @param UAV			The new UAV.
	* @param InitialCount	The initial number of items in the UAV.
	*/
	virtual void RHISetUAVParameter(RBComputeShaderRHIParamRef ComputeShader, uint32 UAVIndex, RBUnorderedAccessViewRHIParamRef UAV, uint32 InitialCount) = 0;

	virtual void RHISetShaderResourceViewParameter(RBComputeShaderRHIParamRef ComputeShader, uint32 SamplerIndex, RBShaderResourceViewRHIParamRef SRV) = 0;

	virtual void RHISetShaderUniformBuffer(RBComputeShaderRHIParamRef ComputeShader, uint32 BufferIndex, RBUniformBufferRHIParamRef Buffer) = 0;

	virtual void RHISetShaderParameter(RBComputeShaderRHIParamRef ComputeShader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue) = 0;

	virtual void RHIPushEvent(const char* Name, RBColorf Color) = 0;

	virtual void RHIPopEvent() = 0;

	/**
	* Submit the current command buffer to the GPU if possible.
	*/
	virtual void RHISubmitCommandsHint() = 0;
};

/** The interface RHI command context. Sometimes the RHI handles these. On platforms that can processes command lists in parallel, it is a separate object. */
class IRHICommandContext : public IRHIComputeContext
{
public:
	virtual ~IRHICommandContext()
	{
	}

	/////// RHI Context Methods

	/**
	* Compute queue will wait for the fence to be written before continuing.
	*/
	virtual void RHIWaitComputeFence(RBComputeFenceRHIParamRef InFence) override
	{
		if (InFence)
		{
			CHECKF(InFence->GetWriteEnqueued(), TEXT("ComputeFence: %s waited on before being written. This will hang the GPU."), *InFence->GetName().c_str());
		}
	}

	/**
	*Sets the current compute shader.  Mostly for compliance with platforms
	*that require shader setting before resource binding.
	*/
	virtual void RHISetComputeShader(RBComputeShaderRHIParamRef ComputeShader) = 0;

	virtual void RHIDispatchComputeShader(uint32 ThreadGroupCountX, uint32 ThreadGroupCountY, uint32 ThreadGroupCountZ) = 0;

	virtual void RHIDispatchIndirectComputeShader(RBVertexBufferRHIParamRef ArgumentBuffer, uint32 ArgumentOffset) = 0;

	virtual void RHISetAsyncComputeBudget(EAsyncComputeBudget Budget) override
	{
	}

	virtual void RHIAutomaticCacheFlushAfterComputeShader(bool bEnable) = 0;

	virtual void RHIFlushComputeShaderCache() = 0;

	// Useful when used with geometry shader (emit polygons to different viewports), otherwise SetViewPort() is simpler
	// @param Count >0
	// @param Data must not be 0
	virtual void RHISetMultipleViewports(uint32 Count, const RBViewportBounds* Data) = 0;

	/** Clears a UAV to the multi-component value provided. */
	virtual void RHIClearUAV(RBUnorderedAccessViewRHIParamRef UnorderedAccessViewRHI, const uint32* Values) = 0;

	/**
	* Resolves from one texture to another.
	* @param SourceTexture - texture to resolve from, 0 is silenty ignored
	* @param DestTexture - texture to resolve to, 0 is silenty ignored
	* @param bKeepOriginalSurface - true if the original surface will still be used after this function so must remain valid
	* @param ResolveParams - optional resolve params
	*/
	//virtual void RHICopyToResolveTarget(RBTextureRHIParamRef SourceTexture, RBTextureRHIParamRef DestTexture, bool bKeepOriginalSurface, const FResolveParams& ResolveParams) = 0;

	/**
	* Explicitly transition a texture resource from readable -> writable by the GPU or vice versa.
	* We know rendertargets are only used as rendered targets on the Gfx pipeline, so these transitions are assumed to be implemented such
	* Gfx->Gfx and Gfx->Compute pipeline transitions are both handled by this call by the RHI implementation.  Hence, no pipeline parameter on this call.
	*
	* @param TransitionType - direction of the transition
	* @param InTextures - array of texture objects to transition
	* @param NumTextures - number of textures to transition
	
	virtual void RHITransitionResources(EResourceTransitionAccess TransitionType, RBTextureRHIParamRef* InTextures, int32 NumTextures)
	{
		if (TransitionType == EResourceTransitionAccess::EReadable)
		{
			const RBResolveParams ResolveParams;
			for (int32 i = 0; i < NumTextures; ++i)
			{
				RHICopyToResolveTarget(InTextures[i], InTextures[i], true, ResolveParams);
			}
		}
	}
	*/

	/**
	* Explicitly transition a UAV from readable -> writable by the GPU or vice versa.
	* Also explicitly states which pipeline the UAV can be used on next.  For example, if a Compute job just wrote this UAV for a Pixel shader to read
	* you would do EResourceTransitionAccess::Readable and EResourceTransitionPipeline::EComputeToGfx
	*
	* @param TransitionType - direction of the transition
	* @param EResourceTransitionPipeline - How this UAV is transitioning between Gfx and Compute, if at all.
	* @param InUAVs - array of UAV objects to transition
	* @param NumUAVs - number of UAVs to transition
	* @param WriteComputeFence - Optional ComputeFence to write as part of this transition
	*/
	virtual void RHITransitionResources(EResourceTransitionAccess TransitionType, EResourceTransitionPipeline TransitionPipeline, RBUnorderedAccessViewRHIParamRef* InUAVs, int32 NumUAVs, RBComputeFenceRHIParamRef WriteComputeFence)
	{
		if (WriteComputeFence)
		{
			WriteComputeFence->WriteFence();
		}
	}

	void RHITransitionResources(EResourceTransitionAccess TransitionType, EResourceTransitionPipeline TransitionPipeline, RBUnorderedAccessViewRHIParamRef* InUAVs, int32 NumUAVs)
	{
		RHITransitionResources(TransitionType, TransitionPipeline, InUAVs, NumUAVs, nullptr);
	}


	virtual void RHIBeginRenderQuery(RBRenderQueryRHIParamRef RenderQuery) = 0;

	virtual void RHIEndRenderQuery(RBRenderQueryRHIParamRef RenderQuery) = 0;

	virtual void RHIBeginOcclusionQueryBatch() = 0;

	virtual void RHIEndOcclusionQueryBatch() = 0;

	virtual void RHISubmitCommandsHint() = 0;

	// This method is queued with an RHIThread, otherwise it will flush after it is queued; without an RHI thread there is no benefit to queuing this frame advance commands
	virtual void RHIBeginDrawingViewport(RBViewportRHIParamRef Viewport, RBTextureRHIParamRef RenderTargetRHI) = 0;

	// This method is queued with an RHIThread, otherwise it will flush after it is queued; without an RHI thread there is no benefit to queuing this frame advance commands
	virtual void RHIEndDrawingViewport(RBViewportRHIParamRef Viewport, bool bPresent, bool bLockToVsync) = 0;

	// This method is queued with an RHIThread, otherwise it will flush after it is queued; without an RHI thread there is no benefit to queuing this frame advance commands
	virtual void RHIBeginFrame() = 0;

	// This method is queued with an RHIThread, otherwise it will flush after it is queued; without an RHI thread there is no benefit to queuing this frame advance commands
	virtual void RHIEndFrame() = 0;

	/**
	* Signals the beginning of scene rendering. The RHI makes certain caching assumptions between
	* calls to BeginScene/EndScene. Currently the only restriction is that you can't update texture
	* references.
	*/
	// This method is queued with an RHIThread, otherwise it will flush after it is queued; without an RHI thread there is no benefit to queuing this frame advance commands
	virtual void RHIBeginScene() = 0;

	/**
	* Signals the end of scene rendering. See RHIBeginScene.
	*/
	// This method is queued with an RHIThread, otherwise it will flush after it is queued; without an RHI thread there is no benefit to queuing this frame advance commands
	virtual void RHIEndScene() = 0;

	virtual void RHISetStreamSource(uint32 StreamIndex, RBVertexBufferRHIParamRef VertexBuffer, uint32 Stride, uint32 Offset) = 0;

	virtual void RHISetRasterizerState(RBRasterizerStateRHIParamRef NewState) = 0;

	// @param MinX including like Win32 RECT
	// @param MinY including like Win32 RECT
	// @param MaxX excluding like Win32 RECT
	// @param MaxY excluding like Win32 RECT
	virtual void RHISetViewport(uint32 MinX, uint32 MinY, float MinZ, uint32 MaxX, uint32 MaxY, float MaxZ) = 0;

	// @param MinX including like Win32 RECT
	// @param MinY including like Win32 RECT
	// @param MaxX excluding like Win32 RECT
	// @param MaxY excluding like Win32 RECT
	virtual void RHISetScissorRect(bool bEnable, uint32 MinX, uint32 MinY, uint32 MaxX, uint32 MaxY) = 0;

	/**
	* Set bound shader state. This will set the vertex decl/shader, and pixel shader
	* @param BoundShaderState - state resource
	*/
	virtual void RHISetBoundShaderState(RBBoundShaderStateRHIParamRef BoundShaderState) = 0;

	/** Set the shader resource view of a surface.  This is used for binding TextureMS parameter types that need a multi sampled view. */
	virtual void RHISetShaderTexture(RBVertexShaderRHIParamRef VertexShader, uint32 TextureIndex, RBTextureRHIParamRef NewTexture) = 0;

	/** Set the shader resource view of a surface.  This is used for binding TextureMS parameter types that need a multi sampled view. */
	virtual void RHISetShaderTexture(RBHullShaderRHIParamRef HullShader, uint32 TextureIndex, RBTextureRHIParamRef NewTexture) = 0;

	/** Set the shader resource view of a surface.  This is used for binding TextureMS parameter types that need a multi sampled view. */
	virtual void RHISetShaderTexture(RBDomainShaderRHIParamRef DomainShader, uint32 TextureIndex, RBTextureRHIParamRef NewTexture) = 0;

	/** Set the shader resource view of a surface.  This is used for binding TextureMS parameter types that need a multi sampled view. */
	virtual void RHISetShaderTexture(RBGeometryShaderRHIParamRef GeometryShader, uint32 TextureIndex, RBTextureRHIParamRef NewTexture) = 0;

	/** Set the shader resource view of a surface.  This is used for binding TextureMS parameter types that need a multi sampled view. */
	virtual void RHISetShaderTexture(RBPixelShaderRHIParamRef PixelShader, uint32 TextureIndex, RBTextureRHIParamRef NewTexture) = 0;

	/** Set the shader resource view of a surface.  This is used for binding TextureMS parameter types that need a multi sampled view. */
	virtual void RHISetShaderTexture(RBComputeShaderRHIParamRef PixelShader, uint32 TextureIndex, RBTextureRHIParamRef NewTexture) = 0;

	/**
	* Sets sampler state.
	* @param GeometryShader	The geometry shader to set the sampler for.
	* @param SamplerIndex		The index of the sampler.
	* @param NewState			The new sampler state.
	*/
	virtual void RHISetShaderSampler(RBComputeShaderRHIParamRef ComputeShader, uint32 SamplerIndex, RBSamplerStateRHIParamRef NewState) = 0;

	/**
	* Sets sampler state.
	* @param GeometryShader	The geometry shader to set the sampler for.
	* @param SamplerIndex		The index of the sampler.
	* @param NewState			The new sampler state.
	*/
	virtual void RHISetShaderSampler(RBVertexShaderRHIParamRef VertexShader, uint32 SamplerIndex, RBSamplerStateRHIParamRef NewState) = 0;

	/**
	* Sets sampler state.
	* @param GeometryShader	The geometry shader to set the sampler for.
	* @param SamplerIndex		The index of the sampler.
	* @param NewState			The new sampler state.
	*/
	virtual void RHISetShaderSampler(RBGeometryShaderRHIParamRef GeometryShader, uint32 SamplerIndex, RBSamplerStateRHIParamRef NewState) = 0;

	/**
	* Sets sampler state.
	* @param GeometryShader	The geometry shader to set the sampler for.
	* @param SamplerIndex		The index of the sampler.
	* @param NewState			The new sampler state.
	*/
	virtual void RHISetShaderSampler(RBDomainShaderRHIParamRef DomainShader, uint32 SamplerIndex, RBSamplerStateRHIParamRef NewState) = 0;

	/**
	* Sets sampler state.
	* @param GeometryShader	The geometry shader to set the sampler for.
	* @param SamplerIndex		The index of the sampler.
	* @param NewState			The new sampler state.
	*/
	virtual void RHISetShaderSampler(RBHullShaderRHIParamRef HullShader, uint32 SamplerIndex, RBSamplerStateRHIParamRef NewState) = 0;

	/**
	* Sets sampler state.
	* @param GeometryShader	The geometry shader to set the sampler for.
	* @param SamplerIndex		The index of the sampler.
	* @param NewState			The new sampler state.
	*/
	virtual void RHISetShaderSampler(RBPixelShaderRHIParamRef PixelShader, uint32 SamplerIndex, RBSamplerStateRHIParamRef NewState) = 0;

	/**
	* Sets a compute shader UAV parameter.
	* @param ComputeShader	The compute shader to set the UAV for.
	* @param UAVIndex		The index of the UAVIndex.
	* @param UAV			The new UAV.
	*/
	virtual void RHISetUAVParameter(RBComputeShaderRHIParamRef ComputeShader, uint32 UAVIndex, RBUnorderedAccessViewRHIParamRef UAV) = 0;

	/**
	* Sets a compute shader counted UAV parameter and initial count
	* @param ComputeShader	The compute shader to set the UAV for.
	* @param UAVIndex		The index of the UAVIndex.
	* @param UAV			The new UAV.
	* @param InitialCount	The initial number of items in the UAV.
	*/
	virtual void RHISetUAVParameter(RBComputeShaderRHIParamRef ComputeShader, uint32 UAVIndex, RBUnorderedAccessViewRHIParamRef UAV, uint32 InitialCount) = 0;

	virtual void RHISetShaderResourceViewParameter(RBPixelShaderRHIParamRef PixelShader, uint32 SamplerIndex, RBShaderResourceViewRHIParamRef SRV) = 0;

	virtual void RHISetShaderResourceViewParameter(RBVertexShaderRHIParamRef VertexShader, uint32 SamplerIndex, RBShaderResourceViewRHIParamRef SRV) = 0;

	virtual void RHISetShaderResourceViewParameter(RBComputeShaderRHIParamRef ComputeShader, uint32 SamplerIndex, RBShaderResourceViewRHIParamRef SRV) = 0;

	virtual void RHISetShaderResourceViewParameter(RBHullShaderRHIParamRef HullShader, uint32 SamplerIndex, RBShaderResourceViewRHIParamRef SRV) = 0;

	virtual void RHISetShaderResourceViewParameter(RBDomainShaderRHIParamRef DomainShader, uint32 SamplerIndex, RBShaderResourceViewRHIParamRef SRV) = 0;

	virtual void RHISetShaderResourceViewParameter(RBGeometryShaderRHIParamRef GeometryShader, uint32 SamplerIndex, RBShaderResourceViewRHIParamRef SRV) = 0;

	virtual void RHISetShaderUniformBuffer(RBVertexShaderRHIParamRef VertexShader, uint32 BufferIndex, RBUniformBufferRHIParamRef Buffer) = 0;

	virtual void RHISetShaderUniformBuffer(RBHullShaderRHIParamRef HullShader, uint32 BufferIndex, RBUniformBufferRHIParamRef Buffer) = 0;

	virtual void RHISetShaderUniformBuffer(RBDomainShaderRHIParamRef DomainShader, uint32 BufferIndex, RBUniformBufferRHIParamRef Buffer) = 0;

	virtual void RHISetShaderUniformBuffer(RBGeometryShaderRHIParamRef GeometryShader, uint32 BufferIndex, RBUniformBufferRHIParamRef Buffer) = 0;

	virtual void RHISetShaderUniformBuffer(RBPixelShaderRHIParamRef PixelShader, uint32 BufferIndex, RBUniformBufferRHIParamRef Buffer) = 0;

	virtual void RHISetShaderUniformBuffer(RBComputeShaderRHIParamRef ComputeShader, uint32 BufferIndex, RBUniformBufferRHIParamRef Buffer) = 0;

	virtual void RHISetShaderParameter(RBVertexShaderRHIParamRef VertexShader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue) = 0;

	virtual void RHISetShaderParameter(RBPixelShaderRHIParamRef PixelShader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue) = 0;

	virtual void RHISetShaderParameter(RBHullShaderRHIParamRef HullShader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue) = 0;

	virtual void RHISetShaderParameter(RBDomainShaderRHIParamRef DomainShader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue) = 0;

	virtual void RHISetShaderParameter(RBGeometryShaderRHIParamRef GeometryShader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue) = 0;

	virtual void RHISetShaderParameter(RBComputeShaderRHIParamRef ComputeShader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue) = 0;

	virtual void RHISetDepthStencilState(RBDepthStencilStateRHIParamRef NewState, uint32 StencilRef) = 0;

	// Allows to set the blend state, parameter can be created with RHICreateBlendState()
	virtual void RHISetBlendState(RBBlendStateRHIParamRef NewState, const RBColorf& BlendFactor) = 0;

	virtual void RHISetRenderTargets(uint32 NumSimultaneousRenderTargets, const RBRHIRenderTargetView* NewRenderTargets, const RBRHIDepthRenderTargetView* NewDepthStencilTarget, uint32 NumUAVs, const RBUnorderedAccessViewRHIParamRef* UAVs) = 0;

	virtual void RHISetRenderTargetsAndClear(const RBRHISetRenderTargetsInfo& RenderTargetsInfo) = 0;

	// Bind the clear state of the currently set rendertargets.  This is used by platforms which
	// need the state of the target when finalizing a hardware clear or a resource transition to SRV
	// The explicit bind is needed to support parallel rendering (propagate state between contexts).
	virtual void RHIBindClearMRTValues(bool bClearColor, bool bClearDepth, bool bClearStencil){}

	virtual void RHIDrawPrimitive(uint32 PrimitiveType, uint32 BaseVertexIndex, uint32 NumPrimitives, uint32 NumInstances) = 0;

	virtual void RHIDrawPrimitiveIndirect(uint32 PrimitiveType, RBVertexBufferRHIParamRef ArgumentBuffer, uint32 ArgumentOffset) = 0;

	virtual void RHIDrawIndexedIndirect(RBIndexBufferRHIParamRef IndexBufferRHI, uint32 PrimitiveType, RBStructuredBufferRHIParamRef ArgumentsBufferRHI, int32 DrawArgumentsIndex, uint32 NumInstances) = 0;

	// @param NumPrimitives need to be >0 
	virtual void RHIDrawIndexedPrimitive(RBIndexBufferRHIParamRef IndexBuffer, uint32 PrimitiveType, int32 BaseVertexIndex, uint32 FirstInstance, uint32 NumVertices, uint32 StartIndex, uint32 NumPrimitives, uint32 NumInstances) = 0;

	virtual void RHIDrawIndexedPrimitiveIndirect(uint32 PrimitiveType, RBIndexBufferRHIParamRef IndexBuffer, RBVertexBufferRHIParamRef ArgumentBuffer, uint32 ArgumentOffset) = 0;

	/**
	* Preallocate memory or get a direct command stream pointer to fill up for immediate rendering . This avoids memcpys below in DrawPrimitiveUP
	* @param PrimitiveType The type (triangles, lineloop, etc) of primitive to draw
	* @param NumPrimitives The number of primitives in the VertexData buffer
	* @param NumVertices The number of vertices to be written
	* @param VertexDataStride Size of each vertex
	* @param OutVertexData Reference to the allocated vertex memory
	*/
	virtual void RHIBeginDrawPrimitiveUP(uint32 PrimitiveType, uint32 NumPrimitives, uint32 NumVertices, uint32 VertexDataStride, void*& OutVertexData) = 0;

	/**
	* Draw a primitive using the vertex data populated since RHIBeginDrawPrimitiveUP and clean up any memory as needed
	*/
	virtual void RHIEndDrawPrimitiveUP() = 0;

	/**
	* Preallocate memory or get a direct command stream pointer to fill up for immediate rendering . This avoids memcpys below in DrawIndexedPrimitiveUP
	* @param PrimitiveType The type (triangles, lineloop, etc) of primitive to draw
	* @param NumPrimitives The number of primitives in the VertexData buffer
	* @param NumVertices The number of vertices to be written
	* @param VertexDataStride Size of each vertex
	* @param OutVertexData Reference to the allocated vertex memory
	* @param MinVertexIndex The lowest vertex index used by the index buffer
	* @param NumIndices Number of indices to be written
	* @param IndexDataStride Size of each index (either 2 or 4 bytes)
	* @param OutIndexData Reference to the allocated index memory
	*/
	virtual void RHIBeginDrawIndexedPrimitiveUP(uint32 PrimitiveType, uint32 NumPrimitives, uint32 NumVertices, uint32 VertexDataStride, void*& OutVertexData, uint32 MinVertexIndex, uint32 NumIndices, uint32 IndexDataStride, void*& OutIndexData) = 0;

	/**
	* Draw a primitive using the vertex and index data populated since RHIBeginDrawIndexedPrimitiveUP and clean up any memory as needed
	*/
	virtual void RHIEndDrawIndexedPrimitiveUP() = 0;

	/*
	* This method clears all MRT's, but to only one color value
	* @param ExcludeRect within the viewport in pixels, is only a hint to optimize - if a fast clear can be done this is preferred
	*/
	virtual void RHIClear(bool bClearColor, const RBColorf& Color, bool bClearDepth, float Depth, bool bClearStencil, uint32 Stencil, RBAABBI ExcludeRect) = 0;

	/*
	* This method clears all MRT's to potentially different color values
	* @param ExcludeRect within the viewport in pixels, is only a hint to optimize - if a fast clear can be done this is preferred
	*/
	virtual void RHIClearMRT(bool bClearColor, int32 NumClearColors, const RBColorf* ColorArray, bool bClearDepth, float Depth, bool bClearStencil, uint32 Stencil, RBAABBI ExcludeRect) = 0;

	/**
	* Enabled/Disables Depth Bounds Testing with the given min/max depth.
	* @param bEnable	Enable(non-zero)/disable(zero) the depth bounds test
	* @param MinDepth	The minimum depth for depth bounds test
	* @param MaxDepth	The maximum depth for depth bounds test.
	*					The valid values for fMinDepth and fMaxDepth are such that 0 <= fMinDepth <= fMaxDepth <= 1
	*/
	virtual void RHIEnableDepthBoundsTest(bool bEnable, float MinDepth, float MaxDepth) = 0;

	virtual void RHIPushEvent(const char* Name, RBColorf Color) = 0;

	virtual void RHIPopEvent() = 0;

	virtual void RHIUpdateTextureReference(RBTextureReferenceRHIParamRef TextureRef, RBTextureRHIParamRef NewTexture) = 0;


};

/** The interface which is implemented by the dynamically bound RHI. */
class RBDynamicRHI
{
public:
  static RBDynamicRHI* GetRHI(ERHITYPES rhi_tp);
  /** Declare a virtual destructor, so the dynamic RHI can be deleted without knowing its type. */
  virtual ~RBDynamicRHI() {}

  /** Initializes the RHI; separate from IDynamicRHIModule::CreateRHI so that GDynamicRHI is set when it is called. */
  virtual void Init() = 0;

  /** Called after the RHI is initialized; before the render thread is started. */
  virtual void PostInit() {}

  /** Shutdown the RHI; handle shutdown and resource destruction before the RHI's actual destructor is called (so that all resources of the RHI are still available for shutdown). */
  virtual void Shutdown() = 0;



  // FlushType: Thread safe
  virtual RBSamplerStateRHIRef RHICreateSamplerState(const RBSamplerStateInitializerRHI& Initializer) = 0;

  // FlushType: Thread safe
  virtual RBRasterizerStateRHIRef RHICreateRasterizerState(const RBRasterizerStateInitializerRHI& Initializer) = 0;

  // FlushType: Thread safe
  virtual RBDepthStencilStateRHIRef RHICreateDepthStencilState(const RBDepthStencilStateInitializerRHI& Initializer) = 0;

  // FlushType: Thread safe
  virtual RBBlendStateRHIRef RHICreateBlendState(const RBBlendStateInitializerRHI& Initializer) = 0;

  // FlushType: Wait RHI Thread
  virtual RBVertexDeclarationRHIRef RHICreateVertexDeclaration(const RBVertexDeclarationElementList& Elements) = 0;
  /**
  * Creates a uniform buffer.  The contents of the uniform buffer are provided in a parameter, and are immutable.
  * CAUTION: Even though this is marked as threadsafe, it is only valid to call from the render thread or the RHI thread. Thus is need not be threadsafe on platforms that do not support or aren't using an RHIThread
  * @param Contents - A pointer to a memory block of size NumBytes that is copied into the new uniform buffer.
  * @param NumBytes - The number of bytes the uniform buffer should contain.
  * @return The new uniform buffer.
  */
  // FlushType: Thread safe, but varies depending on the RHI
  virtual RBUniformBufferRHIRef RHICreateUniformBuffer(const void* Contents, const RBRHIUniformBufferLayout& Layout, EUniformBufferUsage Usage) = 0;

  // FlushType: Wait RHI Thread
  virtual RBIndexBufferRHIRef RHICreateIndexBuffer(uint32 Stride, uint32 Size, uint32 InUsage, RBRHIResourceCreateInfo& CreateInfo) = 0;

  // FlushType: Flush RHI Thread
  virtual void* RHILockIndexBuffer(RBIndexBufferRHIParamRef IndexBuffer, uint32 Offset, uint32 Size, EResourceLockMode LockMode) = 0;

  // FlushType: Flush RHI Thread
  virtual void RHIUnlockIndexBuffer(RBIndexBufferRHIParamRef IndexBuffer) = 0;

  /**
  * @param ResourceArray - An optional pointer to a resource array containing the resource's data.
  */
  // FlushType: Wait RHI Thread
  virtual RBVertexBufferRHIRef RHICreateVertexBuffer(uint32 Size, uint32 InUsage, RBRHIResourceCreateInfo& CreateInfo) = 0;

  // FlushType: Flush RHI Thread
  virtual void* RHILockVertexBuffer(RBVertexBufferRHIParamRef VertexBuffer, uint32 Offset, uint32 SizeRHI, EResourceLockMode LockMode) = 0;

  // FlushType: Flush RHI Thread
  virtual void RHIUnlockVertexBuffer(RBVertexBufferRHIParamRef VertexBuffer) = 0;

  /** Copies the contents of one vertex buffer to another vertex buffer.  They must have identical sizes. */
  // FlushType: Flush Immediate (seems dangerous)
  virtual void RHICopyVertexBuffer(RBVertexBufferRHIParamRef SourceBuffer, RBVertexBufferRHIParamRef DestBuffer) = 0;

  /**
  * @param ResourceArray - An optional pointer to a resource array containing the resource's data.
  */
  // FlushType: Wait RHI Thread
  virtual RBStructuredBufferRHIRef RHICreateStructuredBuffer(uint32 Stride, uint32 Size, uint32 InUsage, RBRHIResourceCreateInfo& CreateInfo) = 0;

  // FlushType: Flush RHI Thread
  virtual void* RHILockStructuredBuffer(RBStructuredBufferRHIParamRef StructuredBuffer, uint32 Offset, uint32 SizeRHI, EResourceLockMode LockMode) = 0;

  // FlushType: Flush RHI Thread
  virtual void RHIUnlockStructuredBuffer(RBStructuredBufferRHIParamRef StructuredBuffer) = 0;

  //  must be called from the main thread.
  // FlushType: Thread safe
  virtual RBViewportRHIRef RHICreateViewport(void* WindowHandle, uint32 SizeX, uint32 SizeY, bool bIsFullscreen, EPixelFormat PreferredPixelFormat) = 0;

  //  must be called from the main thread.
  // FlushType: Thread safe
  virtual void RHIResizeViewport(RBViewportRHIParamRef Viewport, uint32 SizeX, uint32 SizeY, bool bIsFullscreen) = 0;

  // FlushType: Wait RHI Thread
  virtual RBPixelShaderRHIRef RHICreatePixelShader(const void* Code,uint32 CodeSize) = 0;

  // FlushType: Wait RHI Thread
  virtual RBVertexShaderRHIRef RHICreateVertexShader(const void* Code, uint32 CodeSize) = 0;

  // FlushType: Wait RHI Thread
  virtual RBHullShaderRHIRef RHICreateHullShader(const void* Code, uint32 CodeSize) = 0;

  // FlushType: Wait RHI Thread
  virtual RBDomainShaderRHIRef RHICreateDomainShader(const void* Code, uint32 CodeSize) = 0;

  // FlushType: Wait RHI Thread
  virtual RBGeometryShaderRHIRef RHICreateGeometryShader(const void* Code, uint32 CodeSize) = 0;

  /**
  * Creates a bound shader state instance which encapsulates a decl, vertex shader, hull shader, domain shader and pixel shader
  * CAUTION: Even though this is marked as threadsafe, it is only valid to call from the render thread or the RHI thread. It need not be threadsafe unless the RHI support parallel translation.
  * CAUTION: Platforms that support RHIThread but don't actually have a threadsafe implementation must flush internally with FScopedRHIThreadStaller StallRHIThread(FRHICommandListExecutor::GetImmediateCommandList()); when the call is from the render thread
  * @param VertexDeclaration - existing vertex decl
  * @param VertexShader - existing vertex shader
  * @param HullShader - existing hull shader
  * @param DomainShader - existing domain shader
  * @param GeometryShader - existing geometry shader
  * @param PixelShader - existing pixel shader
  */
  // FlushType: Thread safe, but varies depending on the RHI
  virtual RBBoundShaderStateRHIRef RHICreateBoundShaderState(RBVertexDeclarationRHIParamRef VertexDeclaration, RBVertexShaderRHIParamRef VertexShader, RBHullShaderRHIParamRef HullShader, RBDomainShaderRHIParamRef DomainShader, RBPixelShaderRHIParamRef PixelShader, RBGeometryShaderRHIParamRef GeometryShader) = 0;


  /** Creates a geometry shader with stream output ability, defined by ElementList. */
  // FlushType: Wait RHI Thread
  //virtual RBGeometryShaderRHIRef RHICreateGeometryShaderWithStreamOutput(const std::vector<uint8>& Code, const FStreamOutElementList& ElementList, uint32 NumStrides, const uint32* Strides, int32 RasterizedStream) = 0;

  virtual RBTexture2DRHIRef RHICreateTexture2D(uint32 SizeX, uint32 SizeY, uint8 Format, uint32 NumMips, uint32 NumSamples, uint32 Flags, RBRHIResourceCreateInfo& CreateInfo) = 0;

};

/** A global pointer to the dynamically bound RHI implementation. */
extern RBDynamicRHI* GDynamicRHI;