#include "D3D11Viewport.h"
#include "RHID3D11.h"
#include "D3D11Resources.h"
#include "PixelFormat.h"
#include "RBMath.h"
#include "D3D11Util.h"
#include "Render.h"

/** Describes a uniform buffer in the free pool. */
struct FPooledUniformBuffer
{
  TRefCountPtr<ID3D11Buffer> Buffer;
  uint32 CreatedSize;
  uint32 FrameFreed;
};

/**
* Number of size buckets to use for the uniform buffer free pool.
* This needs to be enough to cover the valid uniform buffer size range combined with the heuristic used to map sizes to buckets.
*/
const int32 NumPoolBuckets = 17;

/**
* Number of frames that a uniform buffer will not be re-used for, after being freed.
* This is done as a workaround for what appears to be an AMD driver bug with 11.10 drivers and a 6970 HD,
* Where reusing a constant buffer with D3D11_MAP_WRITE_DISCARD still in use by the GPU will result in incorrect contents randomly.
*/
const int32 NumSafeFrames = 3;

/** Returns the size in bytes of the bucket that the given size fits into. */
uint32 GetPoolBucketSize(uint32 NumBytes)
{
  return RBMath::round_up_to_power_of_two(NumBytes);
}

/** Returns the index of the bucket that the given size fits into. */
uint32 GetPoolBucketIndex(uint32 NumBytes)
{
  return RBMath::ceil_log_two(NumBytes);
}

/** Pool of free uniform buffers, indexed by bucket for constant size search time. */
std::vector<FPooledUniformBuffer> UniformBufferPool[NumPoolBuckets];

/** Uniform buffers that have been freed more recently than NumSafeFrames ago. */
std::vector<FPooledUniformBuffer> SafeUniformBufferPools[NumSafeFrames][NumPoolBuckets];

/** Does per-frame global updating for the uniform buffer pool. */
void UniformBufferBeginFrame()
{
  int32 NumCleaned = 0;

  //SCOPE_CYCLE_COUNTER(STAT_D3D11CleanUniformBufferTime);

  // Clean a limited number of old entries to reduce hitching when leaving a large level
  for (int32 BucketIndex = 0; BucketIndex < NumPoolBuckets; BucketIndex++)
  {
    for (int32 EntryIndex = UniformBufferPool[BucketIndex].size() - 1; EntryIndex >= 0 && NumCleaned < 10; EntryIndex--)
    {
      FPooledUniformBuffer& PoolEntry = UniformBufferPool[BucketIndex][EntryIndex];

      CHECK(IsValidRef(PoolEntry.Buffer));

      // Clean entries that are unlikely to be reused
      if (GFrameNumberRenderThread - PoolEntry.FrameFreed > 30)
      {
        NumCleaned++;
        //UpdateBufferStats(PoolEntry.Buffer, false);

        //UniformBufferPool[BucketIndex].RemoveAtSwap(EntryIndex);
        //need really release memory?
        UniformBufferPool[BucketIndex].clear();
      }
    }
  }

  // Index of the bucket that is now old enough to be reused
  const int32 SafeFrameIndex = GFrameNumberRenderThread % NumSafeFrames;

  // Merge the bucket into the free pool array
  for (int32 BucketIndex = 0; BucketIndex < NumPoolBuckets; BucketIndex++)
  {
    int32 LastNum = UniformBufferPool[BucketIndex].size();
    UniformBufferPool[BucketIndex].insert(UniformBufferPool[BucketIndex].end(), SafeUniformBufferPools[SafeFrameIndex][BucketIndex].begin(), SafeUniformBufferPools[SafeFrameIndex][BucketIndex].end());

#define  DO_CHECK
#ifdef DO_CHECK
    while (LastNum < UniformBufferPool[BucketIndex].size())
    {
      CHECK(IsValidRef(UniformBufferPool[BucketIndex][LastNum].Buffer));
      LastNum++;
    }
#endif
#undef DO_CKECK
    SafeUniformBufferPools[SafeFrameIndex][BucketIndex].clear();
  }
}

static bool IsPoolingEnabled()
{
  if (true && /*IsInRenderingThread() && GRHICommandList.IsRHIThreadActive()*/true)
  {
    return false; // we can't currently use pooling if the RHI thread is active. 
  }
};

RBUniformBufferRHIRef FD3D11DynamicRHI::RHICreateUniformBuffer(const void* Contents, const RBRHIUniformBufferLayout& Layout, EUniformBufferUsage Usage)
{
  //CHECK(IsInRenderingThread());

  RBD3D11UniformBuffer* NewUniformBuffer = nullptr;
  const uint32 NumBytes = Layout.ConstantBufferSize;
  if (NumBytes > 0)
  {
    // Constant buffers must also be 16-byte aligned.
    CHECK(Align(NumBytes, 16) == NumBytes);
    CHECK(Align(Contents, 16) == Contents);
    CHECK(NumBytes <= D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16);
    CHECK(NumBytes < (1 << NumPoolBuckets));

    //SCOPE_CYCLE_COUNTER(STAT_D3D11UpdateUniformBufferTime);
    //false
#pragma region Pool
    if (IsPoolingEnabled())
    {
      TRefCountPtr<ID3D11Buffer> UniformBufferResource;
      RBRingAllocation RingAllocation;
      if (!RingAllocation.IsValid())
      {
        // Find the appropriate bucket based on size
        const uint32 BucketIndex = GetPoolBucketIndex(NumBytes);
        std::vector<FPooledUniformBuffer>& PoolBucket = UniformBufferPool[BucketIndex];

        if (PoolBucket.size() > 0)
        {
          // Reuse the last entry in this size bucket
          FPooledUniformBuffer FreeBufferEntry = PoolBucket.back();PoolBucket.pop_back();
          CHECK(IsValidRef(FreeBufferEntry.Buffer));
          UniformBufferResource = FreeBufferEntry.Buffer;
          CHECKF(FreeBufferEntry.CreatedSize >= NumBytes, TEXT("%u %u %u %u"), NumBytes, BucketIndex, FreeBufferEntry.CreatedSize, GetPoolBucketSize(NumBytes));
          //DEC_DWORD_STAT(STAT_D3D11NumFreeUniformBuffers);
          //DEC_MEMORY_STAT_BY(STAT_D3D11FreeUniformBufferMemory, FreeBufferEntry.CreatedSize);
        }

        // Nothing usable was found in the free pool, create a new uniform buffer
        if (!IsValidRef(UniformBufferResource))
        {
          D3D11_BUFFER_DESC Desc;
          // Allocate based on the bucket size, since this uniform buffer will be reused later
          Desc.ByteWidth = GetPoolBucketSize(NumBytes);
          // Use D3D11_USAGE_DYNAMIC, which allows multiple CPU writes for pool reuses
          // This is method of updating is vastly superior to creating a new constant buffer each time with D3D11_USAGE_IMMUTABLE, 
          // Since that inserts the data into the command buffer which causes GPU flushes
          Desc.Usage = D3D11_USAGE_DYNAMIC;
          Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
          Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
          Desc.MiscFlags = 0;
          Desc.StructureByteStride = 0;

          VERIFYD3D11RESULT(Direct3DDevice->CreateBuffer(&Desc, NULL, UniformBufferResource.GetInitReference()));

          //UpdateBufferStats(UniformBufferResource, true);
        }

        CHECK(IsValidRef(UniformBufferResource));

        D3D11_MAPPED_SUBRESOURCE MappedSubresource;
        // Discard previous results since we always do a full update
        VERIFYD3D11RESULT(Direct3DDeviceIMContext->Map(UniformBufferResource, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubresource));
        CHECK(MappedSubresource.RowPitch >= NumBytes);
        ::memcpy(MappedSubresource.pData, Contents, NumBytes);
        Direct3DDeviceIMContext->Unmap(UniformBufferResource, 0);
      }

      NewUniformBuffer = new RBD3D11UniformBuffer(this, Layout, UniformBufferResource, RingAllocation);
    }
#pragma endregion Pool
    else
    {
      // No pooling
      D3D11_BUFFER_DESC Desc;
      Desc.ByteWidth = NumBytes;
      Desc.Usage = D3D11_USAGE_IMMUTABLE;
      Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
      Desc.CPUAccessFlags = 0;
      Desc.MiscFlags = 0;
      Desc.StructureByteStride = 0;

      D3D11_SUBRESOURCE_DATA ImmutableData;
      ImmutableData.pSysMem = Contents;
      ImmutableData.SysMemPitch = ImmutableData.SysMemSlicePitch = 0;

      TRefCountPtr<ID3D11Buffer> UniformBufferResource;
      VERIFYD3D11RESULT(Direct3DDevice->CreateBuffer(&Desc, &ImmutableData, UniformBufferResource.GetInitReference()));

      NewUniformBuffer = new RBD3D11UniformBuffer(this, Layout, UniformBufferResource, RBRingAllocation());
    }
  }
  else
  {
    // This uniform buffer contains no constants, only a resource table.
    NewUniformBuffer = new RBD3D11UniformBuffer(this, Layout, nullptr, RBRingAllocation());
  }

  if (Layout.Resources.size())
  {
    int32 NumResources = Layout.Resources.size();
    RBRHIResource** InResources = (RBRHIResource**)((uint8*)Contents + Layout.ResourceOffset);

    uint32 p = NumResources;
    while (p--)
    {
      TRefCountPtr<RBRHIResource> res;
      NewUniformBuffer->ResourceTable.push_back(res);
    }

    CHECKF(InResources, TEXT("Invalid resources creating uniform buffer for %s [0x%x + %u]."), Layout.GetDebugName().c_str(), Contents, Layout.ResourceOffset);

    for (int32 i = 0; i < NumResources; ++i)
    {
      CHECKF(InResources[i], TEXT("Invalid resource entry creating uniform buffer, %s.Resources[%u], ResourceType 0x%x."), Layout.GetDebugName().c_str(), i, Layout.Resources[i]);
      NewUniformBuffer->ResourceTable[i] = InResources[i];
    }
  }

  return NewUniformBuffer;
}

RBD3D11UniformBuffer::~RBD3D11UniformBuffer()
{
  // Do not return the allocation to the pool if it is in the dynamic constant buffer!
  if (!RingAllocation.IsValid() && Resource != nullptr)
  {
    //CHECK(IsInRenderingThread());
    D3D11_BUFFER_DESC Desc;
    Resource->GetDesc(&Desc);

    // Return this uniform buffer to the free pool
    if (Desc.CPUAccessFlags == D3D11_CPU_ACCESS_WRITE && Desc.Usage == D3D11_USAGE_DYNAMIC)
    {
      CHECK(IsValidRef(Resource));
      FPooledUniformBuffer NewEntry;
      NewEntry.Buffer = Resource;
      NewEntry.FrameFreed = GFrameNumberRenderThread;
      NewEntry.CreatedSize = Desc.ByteWidth;

      // Add to this frame's array of free uniform buffers
      const int32 SafeFrameIndex = (GFrameNumberRenderThread - 1) % NumSafeFrames;
      const uint32 BucketIndex = GetPoolBucketIndex(Desc.ByteWidth);
      int32 LastNum = SafeUniformBufferPools[SafeFrameIndex][BucketIndex].size();
      CHECK(Desc.ByteWidth <= GetPoolBucketSize(Desc.ByteWidth));
      SafeUniformBufferPools[SafeFrameIndex][BucketIndex].push_back(NewEntry);
      //INC_DWORD_STAT(STAT_D3D11NumFreeUniformBuffers);
      //INC_MEMORY_STAT_BY(STAT_D3D11FreeUniformBufferMemory, Desc.ByteWidth);

      RBMemoryBarrier(); // check for unwanted concurrency

      CHECK(SafeUniformBufferPools[SafeFrameIndex][BucketIndex].size() == LastNum + 1);
    }
  }
}


void FD3D11DynamicRHI::ReleasePooledUniformBuffers()
{
  // Free D3D resources in the pool
  // Don't bother updating pool stats, this is only done on exit.
  for (int32 BucketIndex = 0; BucketIndex < NumPoolBuckets; BucketIndex++)
  {
    UniformBufferPool[BucketIndex].clear();
    std::vector<FPooledUniformBuffer>().swap(UniformBufferPool[BucketIndex]);
  }

  for (int32 SafeFrameIndex = 0; SafeFrameIndex < NumSafeFrames; SafeFrameIndex++)
  {
    for (int32 BucketIndex = 0; BucketIndex < NumPoolBuckets; BucketIndex++)
    {
      SafeUniformBufferPools[SafeFrameIndex][BucketIndex].clear();
      std::vector<FPooledUniformBuffer>().swap(SafeUniformBufferPools[SafeFrameIndex][BucketIndex]);
    }
  }
}