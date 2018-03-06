#include "RenderUtils.h"
#include "RBMath.h"


size_t CalcTextureMipWidthInBlocks(uint32 TextureSizeX, EPixelFormat Format, uint32 MipIndex)
{
  const uint32 BlockSizeX = GPixelFormats[Format].BlockSizeX;
  const uint32 WidthInTexels = RBMath::get_max<uint32>(TextureSizeX >> MipIndex, 1);
  const uint32 WidthInBlocks = (WidthInTexels + BlockSizeX - 1) / BlockSizeX;
  return WidthInBlocks;
}

size_t CalcTextureMipHeightInBlocks(uint32 TextureSizeY, EPixelFormat Format, uint32 MipIndex)
{
  const uint32 BlockSizeY = GPixelFormats[Format].BlockSizeY;
  const uint32 HeightInTexels = RBMath::get_max<uint32>(TextureSizeY >> MipIndex, 1);
  const uint32 HeightInBlocks = (HeightInTexels + BlockSizeY - 1) / BlockSizeY;
  return HeightInBlocks;
}

size_t CalcTextureMipMapSize(uint32 TextureSizeX, uint32 TextureSizeY, EPixelFormat Format, uint32 MipIndex)
{
  const uint32 WidthInBlocks = CalcTextureMipWidthInBlocks(TextureSizeX, Format, MipIndex);
  const uint32 HeightInBlocks = CalcTextureMipHeightInBlocks(TextureSizeY, Format, MipIndex);
  return WidthInBlocks * HeightInBlocks * GPixelFormats[Format].BlockBytes;
}

size_t CalcTextureSize(uint32 SizeX, uint32 SizeY, EPixelFormat Format, uint32 MipCount)
{
  size_t Size = 0;
  for (uint32 MipIndex = 0; MipIndex < MipCount; ++MipIndex)
  {
    Size += CalcTextureMipMapSize(SizeX, SizeY, Format, MipIndex);
  }
  return Size;
}