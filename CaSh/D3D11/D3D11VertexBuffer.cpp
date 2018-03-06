#include "D3D11Viewport.h"
#include "RHID3D11.h"
#include "D3D11Resources.h"
#include "PixelFormat.h"

RBVertexBufferRHIRef FD3D11DynamicRHI::RHICreateVertexBuffer(uint32 Size, uint32 InUsage, RBRHIResourceCreateInfo& CreateInfo)
{
  // Explicitly check that the size is nonzero before allowing CreateVertexBuffer to opaquely fail.
  CHECK(Size > 0);

  D3D11_BUFFER_DESC Desc;
  ZeroMemory(&Desc, sizeof(D3D11_BUFFER_DESC));
  Desc.ByteWidth = Size;
  Desc.Usage = (InUsage & BUF_AnyDynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
  Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  Desc.CPUAccessFlags = (InUsage & BUF_AnyDynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
  Desc.MiscFlags = 0;
  Desc.StructureByteStride = 0;

  if (InUsage & BUF_UnorderedAccess)
  {
    Desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

    static bool bRequiresRawView = false;
    if (bRequiresRawView)
    {
      Desc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
    }
  }

  if (InUsage & BUF_ByteAddressBuffer)
  {
    Desc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
  }

  if (InUsage & BUF_StreamOutput)
  {
    Desc.BindFlags |= D3D11_BIND_STREAM_OUTPUT;
  }

  if (InUsage & BUF_DrawIndirect)
  {
    Desc.MiscFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
  }

  if (InUsage & BUF_ShaderResource)
  {
    Desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
  }

  /*
  if (false)
  {
    if (InUsage & BUF_FastVRAM)
    {
      FFastVRAMAllocator::GetFastVRAMAllocator()->AllocUAVBuffer(Desc);
    }
  }
  */

  // If a resource array was provided for the resource, create the resource pre-populated
  D3D11_SUBRESOURCE_DATA InitData;
  D3D11_SUBRESOURCE_DATA* pInitData = NULL;
  if (CreateInfo.ResourceArray)
  {
    CHECK(Size == CreateInfo.ResourceArray->GetResourceDataSize());
    InitData.pSysMem = CreateInfo.ResourceArray->GetResourceData();
    InitData.SysMemPitch = Size;
    InitData.SysMemSlicePitch = 0;
    pInitData = &InitData;
  }

  TRefCountPtr<ID3D11Buffer> VertexBufferResource;
  VERIFYD3D11RESULT(Direct3DDevice->CreateBuffer(&Desc, pInitData, VertexBufferResource.GetInitReference()));

  //UpdateBufferStats(VertexBufferResource, true);

  if (CreateInfo.ResourceArray)
  {
    // Discard the resource array's contents.
    CreateInfo.ResourceArray->Discard();
  }

  return new RBD3D11VertexBuffer(VertexBufferResource, Size, InUsage);
}

void* FD3D11DynamicRHI::RHILockVertexBuffer(RBVertexBufferRHIParamRef VertexBufferRHI, uint32 Offset, uint32 Size, EResourceLockMode LockMode)
{
  CHECK(Size > 0);

  RBD3D11VertexBuffer* VertexBuffer = (RBD3D11VertexBuffer*)(VertexBufferRHI);

  // If this resource is bound to the device, unbind it
  ConditionalClearShaderResource(VertexBuffer);

  // Determine whether the vertex buffer is dynamic or not.
  D3D11_BUFFER_DESC Desc;
  VertexBuffer->Resource->GetDesc(&Desc);
  const bool bIsDynamic = (Desc.Usage == D3D11_USAGE_DYNAMIC);

  FD3D11LockedKey LockedKey(VertexBuffer->Resource);
  FD3D11LockedData LockedData;

  if (bIsDynamic)
  {
    CHECK(LockMode == RLM_WriteOnly);

    // If the buffer is dynamic, map its memory for writing.
    D3D11_MAPPED_SUBRESOURCE MappedSubresource;
    VERIFYD3D11RESULT(Direct3DDeviceIMContext->Map(VertexBuffer->Resource, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubresource));
    LockedData.SetData(MappedSubresource.pData);
    LockedData.Pitch = MappedSubresource.RowPitch;
  }
  else
  {
    if (LockMode == RLM_ReadOnly)
    {
      // If the static buffer is being locked for reading, create a staging buffer.
      D3D11_BUFFER_DESC StagingBufferDesc;
      ZeroMemory(&StagingBufferDesc, sizeof(D3D11_BUFFER_DESC));
      StagingBufferDesc.ByteWidth = Size;
      StagingBufferDesc.Usage = D3D11_USAGE_STAGING;
      StagingBufferDesc.BindFlags = 0;
      StagingBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
      StagingBufferDesc.MiscFlags = 0;
      TRefCountPtr<ID3D11Buffer> StagingVertexBuffer;
      VERIFYD3D11RESULT(Direct3DDevice->CreateBuffer(&StagingBufferDesc, NULL, StagingVertexBuffer.GetInitReference()));
      LockedData.StagingResource = StagingVertexBuffer;

      // Copy the contents of the vertex buffer to the staging buffer.
      D3D11_BOX SourceBox;
      SourceBox.left = Offset;
      SourceBox.right = Size;
      SourceBox.top = SourceBox.front = 0;
      SourceBox.bottom = SourceBox.back = 1;
      Direct3DDeviceIMContext->CopySubresourceRegion(StagingVertexBuffer, 0, 0, 0, 0, VertexBuffer->Resource, 0, &SourceBox);

      // Map the staging buffer's memory for reading.
      D3D11_MAPPED_SUBRESOURCE MappedSubresource;
      VERIFYD3D11RESULT(Direct3DDeviceIMContext->Map(StagingVertexBuffer, 0, D3D11_MAP_READ, 0, &MappedSubresource));
      LockedData.SetData(MappedSubresource.pData);
      LockedData.Pitch = MappedSubresource.RowPitch;
    }
    else
    {
      // If the static buffer is being locked for writing, allocate memory for the contents to be written to.
      LockedData.AllocData(Desc.ByteWidth);
      LockedData.Pitch = Desc.ByteWidth;
    }
  }

  // Add the lock to the lock map.
  OutstandingLocks.insert(std::pair<FD3D11LockedKey, FD3D11LockedData>(LockedKey, LockedData));

  // Return the offset pointer
  return (void*)((uint8*)LockedData.GetData() + Offset);
}

void FD3D11DynamicRHI::RHIUnlockVertexBuffer(RBVertexBufferRHIParamRef VertexBufferRHI)
{
  RBD3D11VertexBuffer* VertexBuffer = (RBD3D11VertexBuffer*)(VertexBufferRHI);

  // Determine whether the vertex buffer is dynamic or not.
  D3D11_BUFFER_DESC Desc;
  VertexBuffer->Resource->GetDesc(&Desc);
  const bool bIsDynamic = (Desc.Usage == D3D11_USAGE_DYNAMIC);

  // Find the outstanding lock for this VB.
  FD3D11LockedKey LockedKey(VertexBuffer->Resource);
  FD3D11LockedData* LockedData = &OutstandingLocks.find(LockedKey)->second;
  CHECK(LockedData);

  if (bIsDynamic)
  {
    // If the VB is dynamic, its memory was mapped directly; unmap it.
    Direct3DDeviceIMContext->Unmap(VertexBuffer->Resource, 0);
  }
  else
  {
    // If the static VB lock involved a staging resource, it was locked for reading.
    if (LockedData->StagingResource)
    {
      // Unmap the staging buffer's memory.
      ID3D11Buffer* StagingBuffer = (ID3D11Buffer*)LockedData->StagingResource.GetReference();
      Direct3DDeviceIMContext->Unmap(StagingBuffer, 0);
    }
    else
    {
      // Copy the contents of the temporary memory buffer allocated for writing into the VB.
      Direct3DDeviceIMContext->UpdateSubresource(VertexBuffer->Resource, LockedKey.Subresource, NULL, LockedData->GetData(), LockedData->Pitch, 0);

      // Free the temporary memory buffer.
      LockedData->FreeData();
    }
  }

  // Remove the FD3D11LockedData from the lock map.
  // If the lock involved a staging resource, this releases it.
  OutstandingLocks.erase(OutstandingLocks.find(LockedKey));
}

void FD3D11DynamicRHI::RHICopyVertexBuffer(RBVertexBufferRHIParamRef SourceBufferRHI, RBVertexBufferRHIParamRef DestBufferRHI)
{
  RBD3D11VertexBuffer* SourceBuffer = (RBD3D11VertexBuffer*)(SourceBufferRHI);
  RBD3D11VertexBuffer* DestBuffer = (RBD3D11VertexBuffer*)(DestBufferRHI);

  D3D11_BUFFER_DESC SourceBufferDesc;
  SourceBuffer->Resource->GetDesc(&SourceBufferDesc);

  D3D11_BUFFER_DESC DestBufferDesc;
  DestBuffer->Resource->GetDesc(&DestBufferDesc);

  CHECK(SourceBufferDesc.ByteWidth == DestBufferDesc.ByteWidth);

  Direct3DDeviceIMContext->CopyResource(DestBuffer->Resource, SourceBuffer->Resource);

  //GPUProfilingData.RegisterGPUWork(1);
}
