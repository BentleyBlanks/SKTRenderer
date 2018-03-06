// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

/*=============================================================================
BoundShaderStateCache.h: Bound shader state cache definition.
=============================================================================*/

#pragma once
#include "RHI.h"
#include "RHIResources.h"
#include "TypeHash.h"

/**
* Key used to map a set of unique decl/vs/ps combinations to
* a vertex shader resource
*/
class FBoundShaderStateKey
{
public:
  /** Initialization constructor. */
  FBoundShaderStateKey(
    RBVertexDeclarationRHIParamRef InVertexDeclaration,
    RBVertexShaderRHIParamRef InVertexShader,
    RBPixelShaderRHIParamRef InPixelShader,
    RBHullShaderRHIParamRef InHullShader = NULL,
    RBDomainShaderRHIParamRef InDomainShader = NULL,
    RBGeometryShaderRHIParamRef InGeometryShader = NULL
    )
    : VertexDeclaration(InVertexDeclaration)
    , VertexShader(InVertexShader)
    , PixelShader(InPixelShader)
    , HullShader(InHullShader)
    , DomainShader(InDomainShader)
    , GeometryShader(InGeometryShader)
  {}

  /**
  * Equality is based on decl, vertex shader and pixel shader
  * @param Other - instance to compare against
  * @return true if equal
  */
  friend bool operator ==(const FBoundShaderStateKey& A, const FBoundShaderStateKey& B)
  {
    return	A.VertexDeclaration == B.VertexDeclaration &&
      A.VertexShader == B.VertexShader &&
      A.PixelShader == B.PixelShader &&
      A.HullShader == B.HullShader &&
      A.DomainShader == B.DomainShader &&
      A.GeometryShader == B.GeometryShader;
  }

  friend bool operator< (const FBoundShaderStateKey& A, const FBoundShaderStateKey& B)
  {
	  return true;
  }

  /**
  * Get the hash for this type.
  * @param Key - struct to hash
  * @return dword hash based on type
  */
  friend uint32 GetTypeHash(const FBoundShaderStateKey &Key)
  {
	
    return GetTypeHash(Key.VertexDeclaration) ^
      GetTypeHash(Key.VertexShader) ^
      GetTypeHash(Key.PixelShader) ^
      GetTypeHash(Key.HullShader) ^
      GetTypeHash(Key.DomainShader) ^
      GetTypeHash(Key.GeometryShader);
	  
  }

  /**
  * Get the RHI shader for the given frequency.
  */
  FORCEINLINE RBVertexShaderRHIParamRef   GetVertexShader() const   { return VertexShader; }
  FORCEINLINE RBPixelShaderRHIParamRef    GetPixelShader() const    { return PixelShader; }
  FORCEINLINE RBHullShaderRHIParamRef     GetHullShader() const     { return HullShader; }
  FORCEINLINE RBDomainShaderRHIParamRef   GetDomainShader() const   { return DomainShader; }
  FORCEINLINE RBGeometryShaderRHIParamRef GetGeometryShader() const { return GeometryShader; }

private:
  /**
  * Note: We intentionally do use ...Ref, not ...ParamRef to get
  * AddRef() for object to prevent and rare issue that happened before.
  * When changing and recompiling a shader it got the same memory
  * pointer and because the caching is happening with pointer comparison
  * the BoundShaderstate cache was holding on to the old pointer
  * it was not creating a new entry.
  */

  /** vertex decl for this combination */
  RBVertexDeclarationRHIRef VertexDeclaration;
  /** vs for this combination */
  RBVertexShaderRHIRef VertexShader;
  /** ps for this combination */
  RBPixelShaderRHIRef PixelShader;
  /** hs for this combination */
  RBHullShaderRHIRef HullShader;
  /** ds for this combination */
  RBDomainShaderRHIRef DomainShader;
  /** gs for this combination */
  RBGeometryShaderRHIRef GeometryShader;
};

/**
* Encapsulates a bound shader state's entry in the cache.
* Handles removal from the bound shader state cache on destruction.
* RHIs that use cached bound shader states should create one for each bound shader state.
*/
class FCachedBoundShaderStateLink
{
public:
  /**
  * The cached bound shader state.  This is not a reference counted pointer because we rely on the RHI to destruct this object
  * when the bound shader state this references is destructed.
  */
  RBBoundShaderStateRHIParamRef BoundShaderState;

  /** Adds the bound shader state to the cache. */
  FCachedBoundShaderStateLink(
    RBVertexDeclarationRHIParamRef VertexDeclaration,
    RBVertexShaderRHIParamRef VertexShader,
    RBPixelShaderRHIParamRef PixelShader,
    RBBoundShaderStateRHIParamRef InBoundShaderState,
    bool bAddToSingleThreadedCache = true
    );

  /** Adds the bound shader state to the cache. */
  FCachedBoundShaderStateLink(
    RBVertexDeclarationRHIParamRef VertexDeclaration,
    RBVertexShaderRHIParamRef VertexShader,
    RBPixelShaderRHIParamRef PixelShader,
    RBHullShaderRHIParamRef HullShader,
    RBDomainShaderRHIParamRef DomainShader,
    RBGeometryShaderRHIParamRef GeometryShader,
    RBBoundShaderStateRHIParamRef InBoundShaderState,
    bool bAddToSingleThreadedCache = true
    );

  /** Destructor.  Removes the bound shader state from the cache. */
  ~FCachedBoundShaderStateLink();

  /**
  * Get the RHI shader for the given frequency.
  */
  FORCEINLINE RBVertexShaderRHIParamRef   GetVertexShader() const   { return Key.GetVertexShader(); }
  FORCEINLINE RBPixelShaderRHIParamRef    GetPixelShader() const    { return Key.GetPixelShader(); }
  FORCEINLINE RBHullShaderRHIParamRef     GetHullShader() const     { return Key.GetHullShader(); }
  FORCEINLINE RBDomainShaderRHIParamRef   GetDomainShader() const   { return Key.GetDomainShader(); }
  FORCEINLINE RBGeometryShaderRHIParamRef GetGeometryShader() const { return Key.GetGeometryShader(); }
protected:
  FBoundShaderStateKey Key;
  bool bAddedToSingleThreadedCache;
};


/**
* Searches for a cached bound shader state.
* @return If a bound shader state matching the parameters is cached, it is returned; otherwise NULL is returned.
*/
extern FCachedBoundShaderStateLink* GetCachedBoundShaderState(
  RBVertexDeclarationRHIParamRef VertexDeclaration,
  RBVertexShaderRHIParamRef VertexShader,
  RBPixelShaderRHIParamRef PixelShader,
  RBHullShaderRHIParamRef HullShader = NULL,
  RBDomainShaderRHIParamRef DomainShader = NULL,
  RBGeometryShaderRHIParamRef GeometryShader = NULL
  );

extern void EmptyCachedBoundShaderStates();

#if 0
class FCachedBoundShaderStateLink_Threadsafe : public FCachedBoundShaderStateLink
{
public:
  /** Adds the bound shader state to the cache. */
  FCachedBoundShaderStateLink_Threadsafe(
    RBVertexDeclarationRHIParamRef VertexDeclaration,
    RBVertexShaderRHIParamRef VertexShader,
    RBPixelShaderRHIParamRef PixelShader,
    RBBoundShaderStateRHIParamRef InBoundShaderState
    )
    : FCachedBoundShaderStateLink(VertexDeclaration, VertexShader, PixelShader, InBoundShaderState, false)
  {
  }

  /** Adds the bound shader state to the cache. */
  FCachedBoundShaderStateLink_Threadsafe(
    RBVertexDeclarationRHIParamRef VertexDeclaration,
    RBVertexShaderRHIParamRef VertexShader,
    RBPixelShaderRHIParamRef PixelShader,
    RBHullShaderRHIParamRef HullShader,
    RBDomainShaderRHIParamRef DomainShader,
    RBGeometryShaderRHIParamRef GeometryShader,
    RBBoundShaderStateRHIParamRef InBoundShaderState
    )
    : FCachedBoundShaderStateLink(VertexDeclaration, VertexShader, PixelShader, HullShader, DomainShader, GeometryShader, InBoundShaderState, false)
  {
  }

  void AddToCache();
  void RemoveFromCache();
};

/**
* Searches for a cached bound shader state. Threadsafe version.
* @return If a bound shader state matching the parameters is cached, it is returned; otherwise NULL is returned.
*/
extern RBBoundShaderStateRHIRef GetCachedBoundShaderState_Threadsafe(
  RBVertexDeclarationRHIParamRef VertexDeclaration,
  RBVertexShaderRHIParamRef VertexShader,
  RBPixelShaderRHIParamRef PixelShader,
  RBHullShaderRHIParamRef HullShader = NULL,
  RBDomainShaderRHIParamRef DomainShader = NULL,
  RBGeometryShaderRHIParamRef GeometryShader = NULL
  );
#endif
