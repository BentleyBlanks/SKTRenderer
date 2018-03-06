#include "D3D11BoundShaderStateCache.h"
#include <map>

typedef std::map<FBoundShaderStateKey, FCachedBoundShaderStateLink*> FBoundShaderStateCache;
//typedef std::map<FBoundShaderStateKey, FCachedBoundShaderStateLink_Threadsafe*> FBoundShaderStateCache_Threadsafe;

static FBoundShaderStateCache GBoundShaderStateCache;
//static FBoundShaderStateCache_Threadsafe GBoundShaderStateCache_ThreadSafe;

/** Lazily initialized bound shader state cache singleton. */
static FBoundShaderStateCache& GetBoundShaderStateCache()
{
  return GBoundShaderStateCache;
}


/** Lazily initialized bound shader state cache singleton. */
/*
static FBoundShaderStateCache_Threadsafe& GetBoundShaderStateCache_Threadsafe()
{
return GBoundShaderStateCache_ThreadSafe;
}
*/

//static FCriticalSection BoundShaderStateCacheLock;


FCachedBoundShaderStateLink::FCachedBoundShaderStateLink(
  RBVertexDeclarationRHIParamRef VertexDeclaration,
  RBVertexShaderRHIParamRef VertexShader,
  RBPixelShaderRHIParamRef PixelShader,
  RBHullShaderRHIParamRef HullShader,
  RBDomainShaderRHIParamRef DomainShader,
  RBGeometryShaderRHIParamRef GeometryShader,
  RBBoundShaderStateRHIParamRef InBoundShaderState,
  bool bAddToSingleThreadedCache
  ) :
  BoundShaderState(InBoundShaderState),
  Key(VertexDeclaration, VertexShader, PixelShader, HullShader, DomainShader, GeometryShader),
  bAddedToSingleThreadedCache(bAddToSingleThreadedCache)
{
  if (bAddToSingleThreadedCache)
  {
    GetBoundShaderStateCache()[Key] = this;
  }
}

FCachedBoundShaderStateLink::FCachedBoundShaderStateLink(
  RBVertexDeclarationRHIParamRef VertexDeclaration,
  RBVertexShaderRHIParamRef VertexShader,
  RBPixelShaderRHIParamRef PixelShader,
  RBBoundShaderStateRHIParamRef InBoundShaderState,
  bool bAddToSingleThreadedCache
  ) :
  BoundShaderState(InBoundShaderState),
  Key(VertexDeclaration, VertexShader, PixelShader),
  bAddedToSingleThreadedCache(bAddToSingleThreadedCache)
{
  if (bAddToSingleThreadedCache)
  {
    GetBoundShaderStateCache()[Key] = this;
  }
}

FCachedBoundShaderStateLink::~FCachedBoundShaderStateLink()
{
  if (bAddedToSingleThreadedCache)
  {
    GetBoundShaderStateCache().erase(Key);
    bAddedToSingleThreadedCache = false;
  }
}

FCachedBoundShaderStateLink* GetCachedBoundShaderState(
  RBVertexDeclarationRHIParamRef VertexDeclaration,
  RBVertexShaderRHIParamRef VertexShader,
  RBPixelShaderRHIParamRef PixelShader,
  RBHullShaderRHIParamRef HullShader,
  RBDomainShaderRHIParamRef DomainShader,
  RBGeometryShaderRHIParamRef GeometryShader
  )
{
  // Find the existing bound shader state in the cache.
  auto it = GetBoundShaderStateCache().find(
    FBoundShaderStateKey(VertexDeclaration, VertexShader, PixelShader, HullShader, DomainShader, GeometryShader)
    );
  if (it == GetBoundShaderStateCache().end())
  {
    return nullptr;
  }
  return it->second;
}

/*
void FCachedBoundShaderStateLink_Threadsafe::AddToCache()
{
FScopeLock Lock(&BoundShaderStateCacheLock);
GetBoundShaderStateCache_Threadsafe().Add(Key, this);
}
void FCachedBoundShaderStateLink_Threadsafe::RemoveFromCache()
{
FScopeLock Lock(&BoundShaderStateCacheLock);
GetBoundShaderStateCache_Threadsafe().Remove(Key);
}


RBBoundShaderStateRHIRef GetCachedBoundShaderState_Threadsafe(
RBVertexDeclarationRHIParamRef VertexDeclaration,
RBVertexShaderRHIParamRef VertexShader,
RBPixelShaderRHIParamRef PixelShader,
RBHullShaderRHIParamRef HullShader,
RBDomainShaderRHIParamRef DomainShader,
RBGeometryShaderRHIParamRef GeometryShader
)
{
FScopeLock Lock(&BoundShaderStateCacheLock);
// Find the existing bound shader state in the cache.
FCachedBoundShaderStateLink_Threadsafe* CachedBoundShaderStateLink = GetBoundShaderStateCache_Threadsafe().FindRef(
FBoundShaderStateKey(VertexDeclaration, VertexShader, PixelShader, HullShader, DomainShader, GeometryShader)
);
if (CachedBoundShaderStateLink)
{
// If we've already created a bound shader state with these parameters, reuse it.
return CachedBoundShaderStateLink->BoundShaderState;
}
return RBBoundShaderStateRHIRef();
}

void EmptyCachedBoundShaderStates()
{
GetBoundShaderStateCache().clear();
//GetBoundShaderStateCache_Threadsafe().clear();
}
*/