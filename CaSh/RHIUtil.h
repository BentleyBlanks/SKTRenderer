#pragma once
#include "RHIDef.h"
#include "Logger.h"

/**
* Computes the vertex count for a given number of primitives of the specified type.
* @param NumPrimitives The number of primitives.
* @param PrimitiveType The type of primitives.
* @returns The number of vertices.
*/
inline uint32 GetVertexCountForPrimitiveCount(uint32 NumPrimitives, uint32 PrimitiveType)
{
  uint32 VertexCount = 0;
  switch (PrimitiveType)
  {
  case PT_TriangleList: VertexCount = NumPrimitives * 3; break;
  case PT_TriangleStrip: VertexCount = NumPrimitives + 2; break;
  case PT_LineList: VertexCount = NumPrimitives * 2; break;
  case PT_PointList: VertexCount = NumPrimitives; break;
  case PT_1_ControlPointPatchList:
  case PT_2_ControlPointPatchList:
  case PT_3_ControlPointPatchList:
  case PT_4_ControlPointPatchList:
  case PT_5_ControlPointPatchList:
  case PT_6_ControlPointPatchList:
  case PT_7_ControlPointPatchList:
  case PT_8_ControlPointPatchList:
  case PT_9_ControlPointPatchList:
  case PT_10_ControlPointPatchList:
  case PT_11_ControlPointPatchList:
  case PT_12_ControlPointPatchList:
  case PT_13_ControlPointPatchList:
  case PT_14_ControlPointPatchList:
  case PT_15_ControlPointPatchList:
  case PT_16_ControlPointPatchList:
  case PT_17_ControlPointPatchList:
  case PT_18_ControlPointPatchList:
  case PT_19_ControlPointPatchList:
  case PT_20_ControlPointPatchList:
  case PT_21_ControlPointPatchList:
  case PT_22_ControlPointPatchList:
  case PT_23_ControlPointPatchList:
  case PT_24_ControlPointPatchList:
  case PT_25_ControlPointPatchList:
  case PT_26_ControlPointPatchList:
  case PT_27_ControlPointPatchList:
  case PT_28_ControlPointPatchList:
  case PT_29_ControlPointPatchList:
  case PT_30_ControlPointPatchList:
  case PT_31_ControlPointPatchList:
  case PT_32_ControlPointPatchList:
    VertexCount = (PrimitiveType - PT_1_ControlPointPatchList + 1) * NumPrimitives;
    break;
  default: g_logger->debug(WIP_ERROR, TEXT("Unknown primitive type: %u"), PrimitiveType);
  };

  return VertexCount;
}