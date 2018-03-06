#pragma once
#include "RefCount.h"
#include "RHIResources.h"
#include "d3d11.h"

class RBD3D11SamplerState : public RBRHISamplerState
{
public:

	TRefCountPtr<ID3D11SamplerState> Resource;
};

class RBD3D11RasterizerState : public RBRHIRasterizerState
{
public:

	TRefCountPtr<ID3D11RasterizerState> Resource;
};

class RBD3D11DepthStencilState : public RBRHIDepthStencilState
{
public:

	TRefCountPtr<ID3D11DepthStencilState> Resource;

	/* Describes the read/write state of the separate depth and stencil components of the DSV. */
	RBExclusiveDepthStencil AccessType;
};

class RBD3D11BlendState : public RBRHIBlendState
{
public:

	TRefCountPtr<ID3D11BlendState> Resource;
};

