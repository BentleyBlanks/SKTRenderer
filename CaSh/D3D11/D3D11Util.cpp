#include "D3D11Util.h"
// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

/*=============================================================================
D3D11Util.h: D3D RHI utility implementation.
=============================================================================*/
#include "Logger.h"
#include "d3d11.h"
#include "Assertion.h"
#include "../RHIResources.h"

#define D3DERR(x) case x: ErrorCodeText = (#x); break;
#define LOCTEXT_NAMESPACE "Developer.MessageLog"

#ifndef _FACD3D 
#define _FACD3D  0x876
#endif	//_FACD3D 
#ifndef MAKE_D3DHRESULT
#define _FACD3D  0x876
#define MAKE_D3DHRESULT( code )  MAKE_HRESULT( 1, _FACD3D, code )
#endif	//MAKE_D3DHRESULT


static std::string GetD3D11DeviceHungErrorString(HRESULT ErrorCode)
{
  std::string ErrorCodeText;

  switch (ErrorCode)
  {
    D3DERR(DXGI_ERROR_DEVICE_HUNG)
      D3DERR(DXGI_ERROR_DEVICE_REMOVED)
      D3DERR(DXGI_ERROR_DEVICE_RESET)
      D3DERR(DXGI_ERROR_DRIVER_INTERNAL_ERROR)
      D3DERR(DXGI_ERROR_INVALID_CALL)
  default:
    {
      char s[9];
      sprintf(s, "%08X", (int32)ErrorCode);
      s[8] = '\0';
      ErrorCodeText = s;
    }
  }

  return ErrorCodeText;
}

static std::string GetD3D11ErrorString(HRESULT ErrorCode, ID3D11Device* Device)
{
  std::string ErrorCodeText;

  switch (ErrorCode)
  {
    D3DERR(S_OK);
    D3DERR(D3D11_ERROR_FILE_NOT_FOUND)
      D3DERR(D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS)
      D3DERR(E_FAIL)
      D3DERR(E_INVALIDARG)
      D3DERR(E_OUTOFMEMORY)
      D3DERR(DXGI_ERROR_INVALID_CALL)
      D3DERR(E_NOINTERFACE)
      D3DERR(DXGI_ERROR_DEVICE_REMOVED)
  default: 
    {
      char s[9];
      sprintf(s, "%08X", (int32)ErrorCode);
      s[8] = '\0';
      ErrorCodeText = s;
    }
  }

  if (ErrorCode == DXGI_ERROR_DEVICE_REMOVED && Device)
  {
    HRESULT hResDeviceRemoved = Device->GetDeviceRemovedReason();
    ErrorCodeText += std::string(" ") + GetD3D11DeviceHungErrorString(hResDeviceRemoved);
  }

  return ErrorCodeText;
}

#undef D3DERR

const char* GetD3D11TextureFormatString(DXGI_FORMAT TextureFormat)
{
  static const char* EmptyString = TEXT("");
  const char* TextureFormatText = EmptyString;
#define D3DFORMATCASE(x) case x: TextureFormatText = TEXT(#x); break;
  switch (TextureFormat)
  {
    D3DFORMATCASE(DXGI_FORMAT_R8G8B8A8_UNORM)
      D3DFORMATCASE(DXGI_FORMAT_B8G8R8A8_UNORM)
      D3DFORMATCASE(DXGI_FORMAT_B8G8R8X8_UNORM)
      D3DFORMATCASE(DXGI_FORMAT_BC1_UNORM)
      D3DFORMATCASE(DXGI_FORMAT_BC2_UNORM)
      D3DFORMATCASE(DXGI_FORMAT_BC3_UNORM)
      D3DFORMATCASE(DXGI_FORMAT_BC4_UNORM)
      D3DFORMATCASE(DXGI_FORMAT_R16G16B16A16_FLOAT)
      D3DFORMATCASE(DXGI_FORMAT_R32G32B32A32_FLOAT)
      D3DFORMATCASE(DXGI_FORMAT_UNKNOWN)
      D3DFORMATCASE(DXGI_FORMAT_R8_UNORM)
      D3DFORMATCASE(DXGI_FORMAT_R32G8X24_TYPELESS)
      D3DFORMATCASE(DXGI_FORMAT_D24_UNORM_S8_UINT)
      D3DFORMATCASE(DXGI_FORMAT_R24_UNORM_X8_TYPELESS)
      D3DFORMATCASE(DXGI_FORMAT_R32_FLOAT)
      D3DFORMATCASE(DXGI_FORMAT_R16G16_UINT)
      D3DFORMATCASE(DXGI_FORMAT_R16G16_UNORM)
      D3DFORMATCASE(DXGI_FORMAT_R16G16_SNORM)
      D3DFORMATCASE(DXGI_FORMAT_R16G16_FLOAT)
      D3DFORMATCASE(DXGI_FORMAT_R32G32_FLOAT)
      D3DFORMATCASE(DXGI_FORMAT_R10G10B10A2_UNORM)
      D3DFORMATCASE(DXGI_FORMAT_R16G16B16A16_UINT)
      D3DFORMATCASE(DXGI_FORMAT_R8G8_SNORM)
      D3DFORMATCASE(DXGI_FORMAT_BC5_UNORM)
      D3DFORMATCASE(DXGI_FORMAT_R1_UNORM)
      D3DFORMATCASE(DXGI_FORMAT_R8G8B8A8_TYPELESS)
      D3DFORMATCASE(DXGI_FORMAT_B8G8R8A8_TYPELESS)
      D3DFORMATCASE(DXGI_FORMAT_BC7_UNORM)
      D3DFORMATCASE(DXGI_FORMAT_BC6H_UF16)
  default: TextureFormatText = EmptyString;
  }
#undef D3DFORMATCASE
  return TextureFormatText;
}

static std::string GetD3D11TextureFlagString(uint32 TextureFlags)
{
  std::string TextureFormatText = TEXT("");

  if (TextureFlags & D3D11_BIND_RENDER_TARGET)
  {
    TextureFormatText += TEXT("D3D11_BIND_RENDER_TARGET ");
  }

  if (TextureFlags & D3D11_BIND_DEPTH_STENCIL)
  {
    TextureFormatText += TEXT("D3D11_BIND_DEPTH_STENCIL ");
  }

  if (TextureFlags & D3D11_BIND_SHADER_RESOURCE)
  {
    TextureFormatText += TEXT("D3D11_BIND_SHADER_RESOURCE ");
  }

  if (TextureFlags & D3D11_BIND_UNORDERED_ACCESS)
  {
    TextureFormatText += TEXT("D3D11_BIND_UNORDERED_ACCESS ");
  }

  return TextureFormatText;
}


static void TerminateOnDeviceRemoved(HRESULT D3DResult, ID3D11Device* Direct3DDevice)
{
  if (D3DResult == DXGI_ERROR_DEVICE_REMOVED)
  {
    if (Direct3DDevice)
    {
      HRESULT hRes = Direct3DDevice->GetDeviceRemovedReason();

      const TCHAR* Reason = TEXT("?");
      switch (hRes)
      {
      case DXGI_ERROR_DEVICE_HUNG:			Reason = TEXT("HUNG"); break;
      case DXGI_ERROR_DEVICE_REMOVED:			Reason = TEXT("REMOVED"); break;
      case DXGI_ERROR_DEVICE_RESET:			Reason = TEXT("RESET"); break;
      case DXGI_ERROR_DRIVER_INTERNAL_ERROR:	Reason = TEXT("INTERNAL_ERROR"); break;
      case DXGI_ERROR_INVALID_CALL:			Reason = TEXT("INVALID_CALL"); break;
      case S_OK:								Reason = TEXT("S_OK"); break;
      }

      // We currently don't support removed devices because FTexture2DResource can't recreate its RHI resources from scratch.
      // We would also need to recreate the viewport swap chains from scratch.
      g_logger->debug(WIP_ERROR, TEXT("Unreal Engine is exiting due to D3D device being lost. (Error: 0x%X - '%s')"), hRes, Reason);
    }
    else
    {
      g_logger->debug(WIP_ERROR, TEXT("Unreal Engine is exiting due to D3D device being lost. D3D device was not available to assertain DXGI cause."));
    }

    // Workaround for the fact that in non-monolithic builds the exe gets into a weird state and exception handling fails. 
    // @todo investigate why non-monolithic builds fail to capture the exception when graphics driver crashes.
//#if !IS_MONOLITHIC
//    FPlatformMisc::RequestExit(true);
//#endif
    g_logger->debug(WIP_ERROR, "Exited!\n");
    exit(0);
  }
}

static void TerminateOnOutOfMemory(HRESULT D3DResult, bool bCreatingTextures)
{
  if (D3DResult == E_OUTOFMEMORY)
  {
    if (bCreatingTextures)
    {
      ::MessageBox(NULL,"OutOfVideoMemoryTextures", "Out of video memory trying to allocate a texture! Make sure your video card has the minimum required memory, try lowering the resolution and/or closing other applications that are running. Exiting...", MB_OK);
    }
    else
    {
      ::MessageBox(NULL, "D3D11RHI,OutOfMemory", "Out of video memory trying to allocate a rendering resource. Make sure your video card has the minimum required memory, try lowering the resolution and/or closing other applications that are running. Exiting...", MB_OK);
    }
    g_logger->debug(WIP_ERROR, "Exited!\n");
    exit(0);
  }
}


#ifndef MAKE_D3DHRESULT
#define _FACD3D						0x876
#define MAKE_D3DHRESULT( code)		MAKE_HRESULT( 1, _FACD3D, code )
#endif	//MAKE_D3DHRESULT

void VerifyD3D11Result(HRESULT D3DResult, const char* Code, const char* Filename, uint32 Line, ID3D11Device* Device)
{
  CHECK(FAILED(D3DResult));

  const std::string& ErrorString = GetD3D11ErrorString(D3DResult, Device);

  g_logger->debug(WIP_ERROR, TEXT("%s failed \n at %s:%u \n with error %s"), Code, Filename, Line, ErrorString.c_str());

  TerminateOnDeviceRemoved(D3DResult, Device);
  TerminateOnOutOfMemory(D3DResult, false);

  //g_logger->debug(WIP_ERROR, TEXT("%s failed \n at %s:%u \n with error %s"), (Code), (Filename), Line, ErrorString.c_str());
}


void VerifyD3D11ShaderResult(RBRHIShader* Shader, HRESULT D3DResult, const char* Code, const char* Filename, uint32 Line, ID3D11Device* Device)
{
	CHECK(FAILED(D3DResult));
	const std::string& ErrorString = GetD3D11ErrorString(D3DResult, Device);

	if (Shader->ShaderName.size())
	{
		g_logger->debug(WIP_ERROR, TEXT("%s failed trying to create shader %s\n at %s:%u \n with error %s"), (Code), Shader->ShaderName.c_str(), (Filename), Line, ErrorString.c_str());
		TerminateOnDeviceRemoved(D3DResult, Device);
		TerminateOnOutOfMemory(D3DResult, false);

		g_logger->debug(WIP_ERROR, TEXT("%s failed trying to create shader %s \n at %s:%u \n with error %s"), (Code), Shader->ShaderName.c_str(), (Filename), Line, ErrorString.c_str());
	}
	else
	{
		VerifyD3D11Result(D3DResult, Code, Filename, Line, Device);
	}
}

void VerifyD3D11CreateTextureResult(HRESULT D3DResult, const char* Code, const char* Filename, uint32 Line, uint32 SizeX, uint32 SizeY, uint32 SizeZ, uint8 Format, uint32 NumMips, uint32 Flags, ID3D11Device* Device)
{
  CHECK(FAILED(D3DResult));

  const std::string ErrorString = GetD3D11ErrorString(D3DResult, 0);
  const TCHAR* D3DFormatString = GetD3D11TextureFormatString((DXGI_FORMAT)Format);

  g_logger->debug(WIP_ERROR,
    TEXT("%s failed \n at %s:%u \n with error %s, \n Size=%ix%ix%i Format=%s(0x%08X), NumMips=%i, Flags=%s"),
    (Code),
    (Filename),
    Line,
    ErrorString.c_str(),
    SizeX,
    SizeY,
    SizeZ,
    D3DFormatString,
    Format,
    NumMips,
    GetD3D11TextureFlagString(Flags).c_str());

  TerminateOnDeviceRemoved(D3DResult, Device);
  TerminateOnOutOfMemory(D3DResult, true);

  g_logger->debug(WIP_ERROR,
    TEXT("%s failed \n at %s:%u \n with error %s, \n Size=%ix%ix%i Format=%s(0x%08X), NumMips=%i, Flags=%s"),
    (Code),
    (Filename),
    Line,
    ErrorString.c_str(),
    SizeX,
    SizeY,
    SizeZ,
    D3DFormatString,
    Format,
    NumMips,
    GetD3D11TextureFlagString(Flags).c_str());
}

void VerifyComRefCount(IUnknown* Object, int32 ExpectedRefs, const TCHAR* Code, const TCHAR* Filename, int32 Line)
{
  int32 NumRefs;

  if (Object)
  {
    Object->AddRef();
    NumRefs = Object->Release();

    CHECK(NumRefs == ExpectedRefs);

    if (NumRefs != ExpectedRefs)
    {
      g_logger->debug(WIP_ERROR,
        TEXT("%s:(%d): %s has %d refs, expected %d"),
        Filename,
        Line,
        Code,
        NumRefs,
        ExpectedRefs
        );
    }
  }
}


FD3D11BoundRenderTargets::FD3D11BoundRenderTargets(ID3D11DeviceContext* InDeviceContext)
{
  ::memset(RenderTargetViews,0, sizeof(RenderTargetViews));
  DepthStencilView = NULL;
  InDeviceContext->OMGetRenderTargets(
    MaxSimultaneousRenderTargets,
    &RenderTargetViews[0],
    &DepthStencilView
    );
  for (NumActiveTargets = 0; NumActiveTargets < MaxSimultaneousRenderTargets; ++NumActiveTargets)
  {
    if (RenderTargetViews[NumActiveTargets] == NULL)
    {
      break;
    }
  }
}

FD3D11BoundRenderTargets::~FD3D11BoundRenderTargets()
{
  // OMGetRenderTargets calls AddRef on each RTV/DSV it returns. We need
  // to make a corresponding call to Release.
  for (int32 TargetIndex = 0; TargetIndex < NumActiveTargets; ++TargetIndex)
  {
    RenderTargetViews[TargetIndex]->Release();
  }
  if (DepthStencilView)
  {
    DepthStencilView->Release();
  }
}

/*
RBD3D11DynamicBuffer::RBD3D11DynamicBuffer(RBD3D11DynamicRHI* InD3DRHI, D3D11_BIND_FLAG InBindFlags, uint32* InBufferSizes)
  : D3DRHI(InD3DRHI)
  , BindFlags(InBindFlags)
  , LockedBufferIndex(-1)
{
  while (BufferSizes.size() < MAX_BUFFER_SIZES && *InBufferSizes > 0)
  {
    uint32 Size = *InBufferSizes++;
    BufferSizes.push_back(Size);
  }
  CHECK(*InBufferSizes == 0);
  //InitResource();
}

RBD3D11DynamicBuffer::~RBD3D11DynamicBuffer()
{
  //ReleaseResource();
}

void RBD3D11DynamicBuffer::InitRHI()
{
  D3D11_BUFFER_DESC Desc;
  ZeroMemory(&Desc, sizeof(D3D11_BUFFER_DESC));
  Desc.Usage = D3D11_USAGE_DYNAMIC;
  Desc.BindFlags = BindFlags;
  Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  Desc.MiscFlags = 0;

  while (Buffers.size() < BufferSizes.size())
  {
    TRefCountPtr<ID3D11Buffer> Buffer;
    Desc.ByteWidth = BufferSizes[Buffers.size()];
    VERIFYD3D11RESULT(D3DRHI->GetDevice()->CreateBuffer(&Desc, NULL, Buffer.GetInitReference()));
    UpdateBufferStats(Buffer, true);
    Buffers.push_back(Buffer);
  }
}

void RBD3D11DynamicBuffer::ReleaseRHI()
{
  for (int32 i = 0; i < Buffers.size(); ++i)
  {
    UpdateBufferStats(Buffers[i], false);
  }
  Buffers.clear();
  //should release memory??
}

//Map一个指定尺寸的buffer
void* RBD3D11DynamicBuffer::Lock(uint32 Size)
{
  CHECK(LockedBufferIndex == -1 && Buffers.size() > 0);

  int32 BufferIndex = 0;
  int32 NumBuffers = Buffers.size();
  while (BufferIndex < NumBuffers && BufferSizes[BufferIndex] < Size)
  {
    BufferIndex++;
  }
  
  if (BufferIndex == NumBuffers)
  {
    //没有buffer满足锁定条件，创建之
    BufferIndex--;

    TRefCountPtr<ID3D11Buffer> Buffer;
    D3D11_BUFFER_DESC Desc;
    ZeroMemory(&Desc, sizeof(D3D11_BUFFER_DESC));
    Desc.Usage = D3D11_USAGE_DYNAMIC;
    Desc.BindFlags = BindFlags;
    Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    Desc.MiscFlags = 0;
    Desc.ByteWidth = Size;
    VERIFYD3D11RESULT(D3DRHI->GetDevice()->CreateBuffer(&Desc, NULL, Buffer.GetInitReference()));
    UpdateBufferStats(Buffers[BufferIndex], false);
    UpdateBufferStats(Buffer, true);
    Buffers[BufferIndex] = Buffer;
    BufferSizes[BufferIndex] = Size;
  }

  LockedBufferIndex = BufferIndex;
  D3D11_MAPPED_SUBRESOURCE MappedSubresource;
  VERIFYD3D11RESULT(D3DRHI->GetDeviceContext()->Map(Buffers[BufferIndex], 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubresource));
  return MappedSubresource.pData;
}

ID3D11Buffer* RBD3D11DynamicBuffer::Unlock()
{
  CHECK(LockedBufferIndex != -1);
  ID3D11Buffer* LockedBuffer = Buffers[LockedBufferIndex];
  D3DRHI->GetDeviceContext()->Unmap(LockedBuffer, 0);
  LockedBufferIndex = -1;
  return LockedBuffer;
}
*/
//
// Stat declarations.
//
#undef LOCTEXT_NAMESPACE