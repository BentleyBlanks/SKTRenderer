// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

/*=============================================================================
RenderResource.cpp: Render resource implementation.
=============================================================================*/

#include "Util.h"
#include "RenderResources.h"
#include "Logger.h"

/** Whether to enable mip-level fading or not: +1.0f if enabled, -1.0f if disabled. */
float GEnableMipLevelFading = 1.0f;

std::list<FRenderResource*>& FRenderResource::GetResourceList()
{
  static std::list<FRenderResource*> FirstResourceLink;
  return FirstResourceLink;
}

void FRenderResource::InitResource()
{
  //check(IsInRenderingThread());
  if (!bInitialized)
  {
    auto li = GetResourceList();
    li.push_front(this);
    if (0)
    {
      InitDynamicRHI();
      InitRHI();
    }
    RBMemoryBarrier(); // there are some multithreaded reads of bInitialized
    bInitialized = true;
  }
}

void FRenderResource::ReleaseResource()
{
  if (1)
  {
    //check(IsInRenderingThread());
    if (bInitialized)
    {
      if (0)
      {
        ReleaseRHI();
        ReleaseDynamicRHI();
      }
      //ResourceLink.Unlink();
      auto li = GetResourceList();
      li.remove(this);
      bInitialized = false;
    }
  }
}

void FRenderResource::UpdateRHI()
{
  //check(IsInRenderingThread());
  if (bInitialized )
  {
    ReleaseRHI();
    ReleaseDynamicRHI();
    InitDynamicRHI();
    InitRHI();
  }
}

void FRenderResource::InitResourceFromPossiblyParallelRendering()
{
  InitResource();
}


FRenderResource::~FRenderResource()
{
  if (bInitialized )
  {
    // Deleting an initialized FRenderResource will result in a crash later since it is still linked
    g_logger->debug(WIP_ERROR, TEXT("A FRenderResource was deleted without being released first!"));
  }
}

void BeginInitResource(FRenderResource* Resource)
{

      Resource->InitResource();

}

void BeginUpdateResourceRHI(FRenderResource* Resource)
{

      Resource->UpdateRHI();

}

void BeginReleaseResource(FRenderResource* Resource)
{

      Resource->ReleaseResource();

}

void ReleaseResourceAndFlush(FRenderResource* Resource)
{
  // Send the release message.

      Resource->ReleaseResource();



}

/*
FTextureReference::FTextureReference()
  : TextureReferenceRHI(NULL)
{
}

FTextureReference::~FTextureReference()
{
}

void FTextureReference::BeginInit_GameThread()
{
  bInitialized_GameThread = true;
  BeginInitResource(this);
}

void FTextureReference::BeginRelease_GameThread()
{
  BeginReleaseResource(this);
  bInitialized_GameThread = false;
}

void FTextureReference::InvalidateLastRenderTime()
{
  LastRenderTimeRHI.SetLastRenderTime(-FLT_MAX);
}

void FTextureReference::InitRHI()
{
  TextureReferenceRHI = RHICreateTextureReference(&LastRenderTimeRHI);
}

void FTextureReference::ReleaseRHI()
{
  TextureReferenceRHI.SafeRelease();
}

FString FTextureReference::GetFriendlyName() const
{
  return TEXT("FTextureReference");
}
*/
