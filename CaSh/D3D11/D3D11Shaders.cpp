#include "RHID3D11.h"
#include "D3D11Resources.h"
//#include "D3D11ShaderResources.h"
#include "D3D11BoundShaderStateCache.h"
#include "D3D11Util.h"



RBPixelShaderRHIRef FD3D11DynamicRHI::RHICreatePixelShader(const void* Code, uint32 CodeSize)
{
  FD3D11PixelShader* Shader = new FD3D11PixelShader;
  const uint8* CodePtr = static_cast<const uint8*>(Code);
  VERIFYD3D11SHADERRESULT(Direct3DDevice->CreatePixelShader((void*)CodePtr, CodeSize, NULL, Shader->Resource.GetInitReference()), Shader, Direct3DDevice);
  return Shader;
}

RBVertexShaderRHIRef FD3D11DynamicRHI::RHICreateVertexShader(const void* Code, uint32 CodeSize)
{
  FD3D11VertexShader* Shader = new FD3D11VertexShader;
  const uint8* CodePtr = static_cast<const uint8*>(Code);
  VERIFYD3D11SHADERRESULT(Direct3DDevice->CreateVertexShader((void*)CodePtr, CodeSize, NULL, Shader->Resource.GetInitReference()), Shader, Direct3DDevice);
  // TEMP
  Shader->Code.resize(CodeSize);
  memcpy(&(Shader->Code[0]), CodePtr,CodeSize);
  Shader->Offset = 0;
  return Shader;
}

RBHullShaderRHIRef FD3D11DynamicRHI::RHICreateHullShader(const void* Code, uint32 CodeSize)
{
  FD3D11HullShader* Shader = new FD3D11HullShader;
  const uint8* CodePtr = static_cast<const uint8*>(Code);
  VERIFYD3D11SHADERRESULT(Direct3DDevice->CreateHullShader((void*)CodePtr, CodeSize, NULL, Shader->Resource.GetInitReference()), Shader, Direct3DDevice);
  return Shader;
}

RBDomainShaderRHIRef FD3D11DynamicRHI::RHICreateDomainShader(const void* Code, uint32 CodeSize)
{
  FD3D11DomainShader* Shader = new FD3D11DomainShader;
  const uint8* CodePtr = static_cast<const uint8*>(Code);
  VERIFYD3D11SHADERRESULT(Direct3DDevice->CreateDomainShader((void*)CodePtr, CodeSize, NULL, Shader->Resource.GetInitReference()), Shader, Direct3DDevice);
  return Shader;
}

RBGeometryShaderRHIRef FD3D11DynamicRHI::RHICreateGeometryShader(const void* Code, uint32 CodeSize)
{
  FD3D11GeometryShader* Shader = new FD3D11GeometryShader;
  const uint8* CodePtr = static_cast<const uint8*>(Code);
  VERIFYD3D11SHADERRESULT(Direct3DDevice->CreateGeometryShader((void*)CodePtr, CodeSize, NULL, Shader->Resource.GetInitReference()), Shader, Direct3DDevice);
  return Shader;
}

FD3D11BoundShaderState::FD3D11BoundShaderState(
  RBVertexDeclarationRHIParamRef InVertexDeclarationRHI,
  RBVertexShaderRHIParamRef InVertexShaderRHI,
  RBPixelShaderRHIParamRef InPixelShaderRHI,
  RBHullShaderRHIParamRef InHullShaderRHI,
  RBDomainShaderRHIParamRef InDomainShaderRHI,
  RBGeometryShaderRHIParamRef InGeometryShaderRHI,
  ID3D11Device* Direct3DDevice
  ) :
  CacheLink(InVertexDeclarationRHI, InVertexShaderRHI, InPixelShaderRHI, InHullShaderRHI, InDomainShaderRHI, InGeometryShaderRHI, this)
{
	RBD3D11VertexDeclaration* InVertexDeclaration = static_cast<RBD3D11VertexDeclaration*>(InVertexDeclarationRHI);
	FD3D11VertexShader* InVertexShader = static_cast<FD3D11VertexShader*>(InVertexShaderRHI);
	FD3D11PixelShader* InPixelShader = static_cast<FD3D11PixelShader*>(InPixelShaderRHI);
	FD3D11HullShader* InHullShader = static_cast<FD3D11HullShader*>(InHullShaderRHI);
	FD3D11DomainShader* InDomainShader = static_cast<FD3D11DomainShader*>(InDomainShaderRHI);
	FD3D11GeometryShader* InGeometryShader = static_cast<FD3D11GeometryShader*>(InGeometryShaderRHI);

  // Create an input layout for this combination of vertex declaration and vertex shader.
  D3D11_INPUT_ELEMENT_DESC NullInputElement;
  rb_memzero(&NullInputElement, sizeof(D3D11_INPUT_ELEMENT_DESC));

  VERIFYD3D11RESULT(Direct3DDevice->CreateInputLayout(
    InVertexDeclaration ? InVertexDeclaration->VertexElements.data() : &NullInputElement,
    InVertexDeclaration ? InVertexDeclaration->VertexElements.size() : 0,
    &InVertexShader->Code[0],
    InVertexShader->Code.size(),
    InputLayout.GetInitReference()
    ));

  VertexShader = InVertexShader->Resource;
  PixelShader = InPixelShader ? InPixelShader->Resource : NULL;
  HullShader = InHullShader ? InHullShader->Resource : NULL;
  DomainShader = InDomainShader ? InDomainShader->Resource : NULL;
  GeometryShader = InGeometryShader ? InGeometryShader->Resource : NULL;

  rb_memzero(&bShaderNeedsGlobalConstantBuffer, sizeof(bShaderNeedsGlobalConstantBuffer));

  bShaderNeedsGlobalConstantBuffer[SF_Vertex] = InVertexShader->bShaderNeedsGlobalConstantBuffer;
  bShaderNeedsGlobalConstantBuffer[SF_Hull] = InHullShader ? InHullShader->bShaderNeedsGlobalConstantBuffer : false;
  bShaderNeedsGlobalConstantBuffer[SF_Domain] = InDomainShader ? InDomainShader->bShaderNeedsGlobalConstantBuffer : false;
  bShaderNeedsGlobalConstantBuffer[SF_Pixel] = InPixelShader ? InPixelShader->bShaderNeedsGlobalConstantBuffer : false;
  bShaderNeedsGlobalConstantBuffer[SF_Geometry] = InGeometryShader ? InGeometryShader->bShaderNeedsGlobalConstantBuffer : false;

  static_assert(ARRAY_COUNT(bShaderNeedsGlobalConstantBuffer) == SF_NumFrequencies, "EShaderFrequency size should match with array count of bShaderNeedsGlobalConstantBuffer.");
}

FD3D11BoundShaderState::~FD3D11BoundShaderState(){}

RBBoundShaderStateRHIRef FD3D11DynamicRHI::RHICreateBoundShaderState(
  RBVertexDeclarationRHIParamRef VertexDeclarationRHI,
  RBVertexShaderRHIParamRef VertexShaderRHI,
  RBHullShaderRHIParamRef HullShaderRHI,
  RBDomainShaderRHIParamRef DomainShaderRHI,
  RBPixelShaderRHIParamRef PixelShaderRHI,
  RBGeometryShaderRHIParamRef GeometryShaderRHI
  )
{
  CHECKF(GIsRHIInitialized && Direct3DDeviceIMContext, (TEXT("Bound shader state RHI resource was created without initializing Direct3D first")));

  // Check for an existing bound shader state which matches the parameters
  FCachedBoundShaderStateLink* CachedBoundShaderStateLink = GetCachedBoundShaderState(
    VertexDeclarationRHI,
    VertexShaderRHI,
    PixelShaderRHI,
    HullShaderRHI,
    DomainShaderRHI,
    GeometryShaderRHI
    );
  if (CachedBoundShaderStateLink)
  {
    // If we've already created a bound shader state with these parameters, reuse it.
    return CachedBoundShaderStateLink->BoundShaderState;
  }
  else
  {
    return new FD3D11BoundShaderState(VertexDeclarationRHI, VertexShaderRHI, PixelShaderRHI, HullShaderRHI, DomainShaderRHI, GeometryShaderRHI, Direct3DDevice);
  }
}

