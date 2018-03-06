#pragma once
#include "RBBasedata.h"
#include "PixelFormat.h"

/**
* Calculates the height of a mip, in blocks.
*
* @param TextureSizeY		Number of vertical texels (for the base mip-level)
* @param Format			Texture format
* @param MipIndex			The index of the mip-map to compute the size of.
*/
size_t CalcTextureMipHeightInBlocks(uint32 TextureSizeY, EPixelFormat Format, uint32 MipIndex);

/**
* Calculates the amount of memory used for a single mip-map of a texture.
*
* @param TextureSizeX		Number of horizontal texels (for the base mip-level)
* @param TextureSizeY		Number of vertical texels (for the base mip-level)
* @param Format	Texture format
* @param MipIndex	The index of the mip-map to compute the size of.
*/
size_t CalcTextureMipMapSize(uint32 TextureSizeX, uint32 TextureSizeY, EPixelFormat Format, uint32 MipIndex);


/**
* Calculates the amount of memory used for a single mip-map of a texture.
*
* @param TextureSizeX		Number of horizontal texels (for the base mip-level)
* @param TextureSizeY		Number of vertical texels (for the base mip-level)
* @param Format	Texture format
* @param MipIndex	The index of the mip-map to compute the size of.
*/
size_t CalcTextureMipMapSize(uint32 TextureSizeX, uint32 TextureSizeY, EPixelFormat Format, uint32 MipIndex);

/**
* Calculates the amount of memory used for a texture.
*
* @param SizeX		Number of horizontal texels (for the base mip-level)
* @param SizeY		Number of vertical texels (for the base mip-level)
* @param Format	Texture format
* @param MipCount	Number of mip-levels (including the base mip-level)
*/
size_t CalcTextureSize(uint32 SizeX, uint32 SizeY, EPixelFormat Format, uint32 MipCount);
