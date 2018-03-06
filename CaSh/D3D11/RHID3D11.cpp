#include "RHID3D11.h"

RBD3D11BaseShaderResource* CurrentResourcesBoundAsSRVs[SF_NumFrequencies][D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
int32 MaxBoundShaderResourcesIndex[SF_NumFrequencies];

uint32 FD3D11DynamicRHI::GetMaxMSAAQuality(uint32 SampleCount)
{
  if (SampleCount <= 8)
  {
    // 0 has better quality (a more even distribution)
    // higher quality levels might be useful for non box filtered AA or when using weighted samples 
    return 0;
    //		return AvailableMSAAQualities[SampleCount];
  }
  // not supported
  return 0xffffffff;
}

void FD3D11DynamicRHI::ClearState()
{
	
	StateCache.ClearState();

	rb_memzero(CurrentResourcesBoundAsSRVs, sizeof(CurrentResourcesBoundAsSRVs));
	for (int32 Frequency = 0; Frequency < SF_NumFrequencies; Frequency++)
	{
		MaxBoundShaderResourcesIndex[Frequency] = -1;
	}
	
}

void FD3D11DynamicRHI::ConditionalClearShaderResource(RBD3D11BaseShaderResource* Resource)
{
  //SCOPE_CYCLE_COUNTER(STAT_D3D11ClearShaderResourceTime);
  CHECK(Resource);
  ClearShaderResourceViews<SF_Vertex>(Resource);
  ClearShaderResourceViews<SF_Hull>(Resource);
  ClearShaderResourceViews<SF_Domain>(Resource);
  ClearShaderResourceViews<SF_Pixel>(Resource);
  ClearShaderResourceViews<SF_Geometry>(Resource);
  ClearShaderResourceViews<SF_Compute>(Resource);
}

template <EShaderFrequency ShaderFrequency>
void FD3D11DynamicRHI::InternalSetShaderResourceView(RBD3D11BaseShaderResource* Resource, ID3D11ShaderResourceView* SRV, int32 ResourceIndex, const char* SRVName, RBD3D11StateCache::ESRV_Type SrvType)
{
  // Check either both are set, or both are null.
  CHECK((Resource && SRV) || (!Resource && !SRV));
  //CheckIfSRVIsResolved(SRV);

  //avoid state cache crash
  if (!((Resource && SRV) || (!Resource && !SRV)))
  {
    //UE_LOG(LogRHI, Warning, TEXT("Bailing on InternalSetShaderResourceView on resource: %i, %s"), ResourceIndex, *SRVName.ToString());
    return;
  }

  if (Resource)
  {
    const EResourceTransitionAccess CurrentAccess = Resource->GetCurrentGPUAccess();
    const bool bAccessPass = CurrentAccess == EResourceTransitionAccess::EReadable || (CurrentAccess == EResourceTransitionAccess::ERWBarrier && !Resource->IsDirty()) || CurrentAccess == EResourceTransitionAccess::ERWSubResBarrier;
    //CHECKF((GEnableDX11TransitionChecks == 0) || bAccessPass || Resource->GetLastFrameWritten() != PresentCounter, TEXT("Shader resource %s is not GPU readable.  Missing a call to RHITransitionResources()"), *SRVName.ToString());
  }

  RBD3D11BaseShaderResource*& ResourceSlot = CurrentResourcesBoundAsSRVs[ShaderFrequency][ResourceIndex];
  int32& MaxResourceIndex = MaxBoundShaderResourcesIndex[ShaderFrequency];

  if (Resource)
  {
    // We are binding a new SRV.
    // Update the max resource index to the highest bound resource index.
    MaxResourceIndex = RBMath::get_max(MaxResourceIndex, ResourceIndex);
    ResourceSlot = Resource;
  }
  else if (ResourceSlot != nullptr)
  {
    // Unbind the resource from the slot.
    ResourceSlot = nullptr;

    // If this was the highest bound resource...
    if (MaxResourceIndex == ResourceIndex)
    {
      // Adjust the max resource index downwards until we
      // hit the next non-null slot, or we've run out of slots.
      do
      {
        MaxResourceIndex--;
      } while (MaxResourceIndex >= 0 && CurrentResourcesBoundAsSRVs[ShaderFrequency][MaxResourceIndex] == nullptr);
    }
  }

  // Set the SRV we have been given (or null).
  StateCache.SetShaderResourceView<ShaderFrequency>(SRV, ResourceIndex, SrvType);
}
