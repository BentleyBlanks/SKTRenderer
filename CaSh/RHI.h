#pragma once
#include "Colorf.h"
#include "RHIDef.h"
#include "ResourceArray.h"

struct RBRHIResourceCreateInfo
{
  RBRHIResourceCreateInfo()
    : BulkData(nullptr)
    , ResourceArray(nullptr)
    , ClearValueBinding(RBColorf::blank)
  {}

  // for CreateTexture calls
  RBRHIResourceCreateInfo(FResourceBulkDataInterface* InBulkData)
    : BulkData(InBulkData)
    , ResourceArray(nullptr)
    , ClearValueBinding(RBColorf::blank)
  {}

  // for CreateVertexBuffer/CreateStructuredBuffer calls
  RBRHIResourceCreateInfo(FResourceArrayInterface* InResourceArray)
    : BulkData(nullptr)
    , ResourceArray(InResourceArray)
    , ClearValueBinding(RBColorf::blank)
  {}

  RBRHIResourceCreateInfo(const RBClearValueBinding& InClearValueBinding)
    : BulkData(nullptr)
    , ResourceArray(nullptr)
    , ClearValueBinding(InClearValueBinding)
  {
  }

  // for CreateTexture calls
  FResourceBulkDataInterface* BulkData;
  // for CreateVertexBuffer/CreateStructuredBuffer calls
  FResourceArrayInterface* ResourceArray;

  // for binding clear colors to rendertargets.
  RBClearValueBinding ClearValueBinding;
};