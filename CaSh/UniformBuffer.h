#pragma once

#include <string>
#include "RBBasedata.h"

/** Each entry in a resource table is provided to the shader compiler for creating mappings. */
struct RBResourceTableEntry
{
  /** The name of the uniform buffer in which this resource exists. */
  std::string UniformBufferName;
  /** The type of the resource (EUniformBufferBaseType). */
  uint16 Type;
  /** The index of the resource in the table. */
  uint16 ResourceIndex;
};