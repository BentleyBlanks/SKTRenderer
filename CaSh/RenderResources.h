#pragma once
#include <list>
#include <vector>
#include "RBBasedata.h"

/**
* A rendering resource which is owned by the rendering thread.
*/
class FRenderResource
{
public:

  /** @return The global initialized resource list. */
  static std::list<FRenderResource*>& GetResourceList();

  /** Default constructor. */
  FRenderResource():bInitialized(false)
  {}


  /** Destructor used to catch unreleased resources. */
  virtual ~FRenderResource();

  /**
  * Initializes the dynamic RHI resource and/or RHI render target used by this resource.
  * Called when the resource is initialized, or when reseting all RHI resources.
  * Resources that need to initialize after a D3D device reset must implement this function.
  * This is only called by the rendering thread.
  */
  virtual void InitDynamicRHI() {}

  /**
  * Releases the dynamic RHI resource and/or RHI render target resources used by this resource.
  * Called when the resource is released, or when reseting all RHI resources.
  * Resources that need to release before a D3D device reset must implement this function.
  * This is only called by the rendering thread.
  */
  virtual void ReleaseDynamicRHI() {}

  /**
  * Initializes the RHI resources used by this resource.
  * Called when entering the state where both the resource and the RHI have been initialized.
  * This is only called by the rendering thread.
  */
  virtual void InitRHI() {}

  /**
  * Releases the RHI resources used by this resource.
  * Called when leaving the state where both the resource and the RHI have been initialized.
  * This is only called by the rendering thread.
  */
  virtual void ReleaseRHI() {}

  /**
  * Initializes the resource.
  * This is only called by the rendering thread.
  */
  virtual void InitResource();

  /**
  * Prepares the resource for deletion.
  * This is only called by the rendering thread.
  */
  virtual void ReleaseResource();

  /**
  * If the resource's RHI resources have been initialized, then release and reinitialize it.  Otherwise, do nothing.
  * This is only called by the rendering thread.
  */
  void UpdateRHI();

  // Probably temporary code that sends a task back to renderthread_local and blocks waiting for it to call InitResource
  void InitResourceFromPossiblyParallelRendering();

  /** @return The resource's friendly name.  Typically a UObject name. */
  virtual std::string GetFriendlyName() const { return ("undefined"); }

  // Accessors.
  FORCEINLINE bool IsInitialized() const { return bInitialized; }


protected:

private:

  /** This resource's link in the global resource list. */
  std::list<FRenderResource*> ResourceLink;

  /** True if the resource has been initialized. */
  bool bInitialized;
};

