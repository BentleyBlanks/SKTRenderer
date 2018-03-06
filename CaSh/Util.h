#pragma once
#include "Platform/RBBasedata.h"
#include "AreTypesEqual.h"
#include "EnableIf.h"
#include "stdlib.h"
#include  <string>
#include <atomic>
#include "windows.h"

template <typename T, int N>
char(&ArrayCountHelper(const T(&)[N]))[N];

// Number of elements in an array.
#define ARRAY_COUNT( array ) (sizeof(ArrayCountHelper(array))+0)

//#define RBMemoryBarrier (__asm volatile ("" : : : "memory"))
#define RBMemoryBarrier MemoryBarrier

/**
* Aligns a value to the nearest higher multiple of 'Alignment', which must be a power of two.
*
* @param Ptr			Value to align
* @param Alignment		Alignment, must be a power of two
* @return				Aligned value
*/
template <typename T>
inline T Align(const T Ptr, int32 Alignment)
{
  return (T)(((LONG_PTR)Ptr + Alignment - 1) & ~(Alignment - 1));
}

/**
* Aligns a value to the nearest lower multiple of 'Alignment', which must be a power of two.
*
* @param Ptr			Value to align
* @param Alignment		Alignment, must be a power of two
* @return				Aligned value
*/
template <typename T>
inline T AlignDown(const T Ptr, int32 Alignment)
{
  return (T)(((LONG_PTR)Ptr) & ~(Alignment - 1));
}

/**
* Checks if a pointer is aligned to the specified alignment.
*
* @param Ptr - The pointer to check.
*
* @return true if the pointer is aligned, false otherwise.
*/
static FORCEINLINE bool IsAligned(const volatile void* Ptr, const uint32 Alignment)
{
  return !(ULONG_PTR(Ptr) & (Alignment - 1));
}

/**
* Aligns a value to the nearest higher multiple of 'Alignment'.
*
* @param Ptr			Value to align
* @param Alignment		Alignment, can be any arbitrary uint32
* @return				Aligned value
*/
template< class T > inline T AlignArbitrary(const T Ptr, uint32 Alignment)
{
  return (T)((((ULONG_PTR)Ptr + Alignment - 1) / Alignment) * Alignment);
}


/**
* Struct used to hold common memory constants for all platforms.
* These values don't change over the entire life of the executable.
*/
struct FGenericPlatformMemoryConstants
{
  /** The amount of actual physical memory, in bytes. */
  size_t TotalPhysical;

  /** The amount of virtual memory, in bytes. */
  size_t TotalVirtual;

  /** The size of a page, in bytes. */
  size_t PageSize;

  /**
  * For platforms that support multiple page sizes this is non-zero and smaller than PageSize.
  * If non-zero, then BinnedAllocFromOS will take allocation requests aligned to this size and return blocks aligned to PageSize
  */
  size_t OsAllocationGranularity;

  // AddressLimit - Second parameter is estimate of the range of addresses expected to be returns by BinnedAllocFromOS(). Binned
  // Malloc will adjust its internal structures to make lookups for memory allocations O(1) for this range. 
  // It is ok to go outside this range, lookups will just be a little slower
  uint64 AddressLimit;

  /** Approximate physical RAM in GB; 1 on everything except PC. Used for "course tuning", like FPlatformMisc::NumberOfCores(). */
  uint32 TotalPhysicalGB;

  /** Default constructor, clears all variables. */
  FGenericPlatformMemoryConstants()
    : TotalPhysical(0)
    , TotalVirtual(0)
    , PageSize(0)
    , OsAllocationGranularity(0)
    , AddressLimit((uint64)0xffffffff + 1)
    , TotalPhysicalGB(1)
  {}

  /** Copy constructor, used by the generic platform memory stats. */
  FGenericPlatformMemoryConstants(const FGenericPlatformMemoryConstants& Other)
    : TotalPhysical(Other.TotalPhysical)
    , TotalVirtual(Other.TotalVirtual)
    , PageSize(Other.PageSize)
    , OsAllocationGranularity(Other.OsAllocationGranularity)
    , AddressLimit(Other.AddressLimit)
    , TotalPhysicalGB(Other.TotalPhysicalGB)
  {}
};

//from gcc
static FORCEINLINE void* rb_align_malloc(size_t size, size_t align)
{
  void * malloc_ptr;
  void * aligned_ptr;

  /* Error if align is not a power of two.  */
  if (align & (align - 1))
  {
    errno = 22;// EINVAL;
    return ((void*)0);
  }

  if (size == 0)
    return ((void *)0);

  /* Assume malloc'd pointer is aligned at least to sizeof (void*).
  If necessary, add another sizeof (void*) to store the value
  returned by malloc. Effectively this enforces a minimum alignment
  of sizeof double. */
  if (align < 2 * sizeof(void *))
    align = 2 * sizeof(void *);

  malloc_ptr = ::malloc(size + align);
  if (!malloc_ptr)
    return ((void *)0);

  /* Align  We have at least sizeof (void *) space below malloc'd ptr. */
  aligned_ptr = (void *)(((size_t)malloc_ptr + align)
    & ~((size_t)(align)-1));

  /* Store the original pointer just before p.  */
  ((void **)aligned_ptr)[-1] = malloc_ptr;

  return aligned_ptr;
}

//be sure that the memory to be freed is allocated by rb_align_malloc
static FORCEINLINE void rb_align_free(void * aligned_ptr)
{
  if (aligned_ptr)
    ::free(((void **)aligned_ptr)[-1]);
}

static FORCEINLINE void rb_memzero(void* ptr, size_t size)
{
  ::memset(ptr, 0, size);
}

/**
* Reverses the order of the bits of a value.
* This is an TEnableIf'd template to ensure that no undesirable conversions occur.  Overloads for other types can be added in the same way.
*
* @param Bits - The value to bit-swap.
* @return The bit-swapped value.
*/
template <typename T>
FORCEINLINE typename TEnableIf<TAreTypesEqual<T, uint32>::Value, T>::Type ReverseBits(T Bits)
{
  Bits = (Bits << 16) | (Bits >> 16);
  Bits = ((Bits & 0x00ff00ff) << 8) | ((Bits & 0xff00ff00) >> 8);
  Bits = ((Bits & 0x0f0f0f0f) << 4) | ((Bits & 0xf0f0f0f0) >> 4);
  Bits = ((Bits & 0x33333333) << 2) | ((Bits & 0xcccccccc) >> 2);
  Bits = ((Bits & 0x55555555) << 1) | ((Bits & 0xaaaaaaaa) >> 1);
  return Bits;
}


// These macros are not safe to use unless data is UNSIGNED!
#define BYTESWAP_ORDER16_unsigned(x) ((((x) >> 8) & 0xff) + (((x) << 8) & 0xff00))
#define BYTESWAP_ORDER32_unsigned(x) (((x) >> 24) + (((x) >> 8) & 0xff00) + (((x) << 8) & 0xff0000) + ((x) << 24))


static FORCEINLINE uint16 BYTESWAP_ORDER16(uint16 val)
{
  return(BYTESWAP_ORDER16_unsigned(val));
}

static FORCEINLINE int16 BYTESWAP_ORDER16(int16 val)
{
  uint16 uval = *((uint16*)&val);
  uval = BYTESWAP_ORDER16_unsigned(uval);

  return *((int16*)&uval);
}

static FORCEINLINE uint32 BYTESWAP_ORDER32(uint32 val)
{
  return (BYTESWAP_ORDER32_unsigned(val));
}

static FORCEINLINE int32 BYTESWAP_ORDER32(int32 val)
{
  uint32 uval = *((uint32*)&val);
  uval = BYTESWAP_ORDER32_unsigned(uval);

  return *((int32*)&uval);
}

static FORCEINLINE float BYTESWAP_ORDERF(float val)
{
  uint32 uval = *((uint32*)&val);
  uval = BYTESWAP_ORDER32_unsigned(uval);

  return *((float*)&uval);
}

static FORCEINLINE uint64 BYTESWAP_ORDER64(uint64 Value)
{
  uint64 Swapped = 0;
  uint8* Forward = (uint8*)&Value;
  uint8* Reverse = (uint8*)&Swapped + 7;
  for (int32 i = 0; i < 8; i++)
  {
    *(Reverse--) = *(Forward++); // copy into Swapped
  }
  return Swapped;
}

static FORCEINLINE int64 BYTESWAP_ORDER64(int64 Value)
{
  int64 Swapped = 0;
  uint8* Forward = (uint8*)&Value;
  uint8* Reverse = (uint8*)&Swapped + 7;

  for (int32 i = 0; i < 8; i++)
  {
    *(Reverse--) = *(Forward++); // copy into Swapped
  }

  return Swapped;
}

static FORCEINLINE void BYTESWAP_ORDER_TCHARARRAY(char* str)
{
  for (char* c = str; *c; ++c)
  {
    *c = BYTESWAP_ORDER16_unsigned(*c);
  }
}


// General byte swapping.
#ifdef PLATFORM_LITTLE_ENDIAN
#define INTEL_ORDER16(x)   (x)
#define INTEL_ORDER32(x)   (x)
#define INTEL_ORDERF(x)    (x)
#define INTEL_ORDER64(x)   (x)
#define INTEL_ORDER_TCHARARRAY(x)
#else
#define INTEL_ORDER16(x)			BYTESWAP_ORDER16(x)
#define INTEL_ORDER32(x)			BYTESWAP_ORDER32(x)
#define INTEL_ORDERF(x)				BYTESWAP_ORDERF(x)
#define INTEL_ORDER64(x)			BYTESWAP_ORDER64(x)
#define INTEL_ORDER_TCHARARRAY(x)	BYTESWAP_ORDER_TCHARARRAY(x)
#endif

#ifdef _WIN64

struct RBThreadSafeCounter
{

  RBThreadSafeCounter(){ val = 0; }
  RBThreadSafeCounter(const RBThreadSafeCounter& other)
  {
    val.store(other.val);
  }

  RBThreadSafeCounter& operator=(const RBThreadSafeCounter& other)
  {
    if (this == &other) return *this;
    Set(other.val);
    return *this;
  }
  RBThreadSafeCounter(i64 va)
  {
    val = va;
  }
  i64 Increment()
  {
    return ++val;
  }
  i64 Add(i64 amount)
  {
    val += amount;
    return val.load();
  }
  i64 Decrement()
  {
    return --val;
  }
  i64 Subtract(i64 amount)
  {
    val -= amount;
    return val.load();
  }

  i64 Set(i64 va)
  {
    val.store(va);
    return val;
  }

  i64 Reset()
  {
    val.store(0);
  }

  i64 GetValue() const
  {
    return val.load();
  }

  std::atomic<i64> val;

};

#endif


/** @return Char value of Nibble */
inline TCHAR NibbleToTChar(uint8 Num)
{
  if (Num > 9)
  {
    return TEXT('A') + TCHAR(Num - 10);
  }
  return TEXT('0') + TCHAR(Num);
}

/**
* Convert a byte to hex
* @param In byte value to convert
* @param Result out hex value output
*/
inline void ByteToHex(uint8 In, std::string& Result)
{
  Result += NibbleToTChar(In >> 4);
  Result += NibbleToTChar(In & 15);
}
/**
* Convert an array of bytes to hex
* @param In byte array values to convert
* @param Count number of bytes to convert
* @return Hex value in string.
*/
inline std::string BytesToHex(const uint8* In, int32 Count)
{
  std::string Result;
  Result.resize(Count * 2);

  while (Count)
  {
    ByteToHex(*In++, Result);
    Count--;
  }
  return Result;
}


#if 0
#pragma region BXMEM

#	define BX_ALLOC(_allocator, _size)                         alloc(_allocator, _size, 0)
#	define BX_REALLOC(_allocator, _ptr, _size)                 realloc(_allocator, _ptr, _size, 0)
#	define BX_FREE(_allocator, _ptr)                           free(_allocator, _ptr, 0)
#	define BX_ALIGNED_ALLOC(_allocator, _size, _align)         alloc(_allocator, _size, _align)
#	define BX_ALIGNED_REALLOC(_allocator, _ptr, _size, _align) realloc(_allocator, _ptr, _size, _align)
#	define BX_ALIGNED_FREE(_allocator, _ptr, _align)           free(_allocator, _ptr, _align)
#	define BX_NEW(_allocator, _type)                           ::new(BX_ALLOC(_allocator, sizeof(_type) ) ) _type
#	define BX_DELETE(_allocator, _ptr)                         deleteObject(_allocator, _ptr, 0)
#	define BX_ALIGNED_NEW(_allocator, _type, _align)           ::new(BX_ALIGNED_ALLOC(_allocator, sizeof(_type), _align) ) _type
#	define BX_ALIGNED_DELETE(_allocator, _ptr, _align)         deleteObject(_allocator, _ptr, _align)

#ifndef BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT
#	define BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT 8
#endif // BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT

  /// Aligns pointer to nearest next aligned address. _align must be power of two.
  inline void* alignPtr(void* _ptr, size_t _extra, size_t _align = BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT)
  {
    union { void* ptr; size_t addr; } un;
    un.ptr = _ptr;
    size_t unaligned = un.addr + _extra; // space for header
    size_t mask = _align - 1;
    size_t aligned = Align(unaligned, (u32)mask);
    un.addr = aligned;
    return un.ptr;
  }

  struct __declspec(novtable) AllocatorI
  {
    virtual ~AllocatorI() = 0;

    /// Allocated, resizes memory block or frees memory.
    ///
    /// @param[in] _ptr If _ptr is NULL new block will be allocated.
    /// @param[in] _size If _ptr is set, and _size is 0, memory will be freed.
    /// @param[in] _align Alignment.
    /// @param[in] _file Debug file path info.
    /// @param[in] _line Debug file line info.
    virtual void* realloc(void* _ptr, size_t _size, size_t _align, const char* _file, uint32 _line) = 0;
  };

  inline AllocatorI::~AllocatorI()
  {
  }

  struct CRTAllocator : public AllocatorI
  {
    void* realloc(void* _ptr, size_t _size, size_t _align, const char* _file, uint32 _line) override
    {
      return _ptr ? ::realloc(_ptr, _size) : malloc(_size);
    }
  };

  inline void* alloc(AllocatorI* _allocator, size_t _size, size_t _align = 0, const char* _file = NULL, uint32 _line = 0)
  {
    return _allocator->realloc(NULL, _size, _align, _file, _line);
  }

  inline void free(AllocatorI* _allocator, void* _ptr, size_t _align = 0, const char* _file = NULL, uint32 _line = 0)
  {
    _allocator->realloc(_ptr, 0, _align, _file, _line);
  }

  inline void* realloc(AllocatorI* _allocator, void* _ptr, size_t _size, size_t _align = 0, const char* _file = NULL, uint32 _line = 0)
  {
    return _allocator->realloc(_ptr, _size, _align, _file, _line);
  }

  static inline void* alignedAlloc(AllocatorI* _allocator, size_t _size, size_t _align, const char* _file = NULL, uint32 _line = 0)
  {
    size_t total = _size + _align;
    uint8* ptr = (uint8*)alloc(_allocator, total, 0, _file, _line);
    uint8* aligned = (uint8*)alignPtr(ptr, sizeof(uint32), _align);
    uint32* header = (uint32*)aligned - 1;
    *header = uint32(aligned - ptr);
    return aligned;
  }

  static inline void alignedFree(AllocatorI* _allocator, void* _ptr, size_t /*_align*/, const char* _file = NULL, uint32 _line = 0)
  {
    uint8* aligned = (uint8*)_ptr;
    uint32* header = (uint32*)aligned - 1;
    uint8* ptr = aligned - *header;
    free(_allocator, ptr, 0, _file, _line);
  }

  static inline void* alignedRealloc(AllocatorI* _allocator, void* _ptr, size_t _size, size_t _align, const char* _file = NULL, uint32 _line = 0)
  {
    if (NULL == _ptr)
    {
      return alignedAlloc(_allocator, _size, _align, _file, _line);
    }

    uint8* aligned = (uint8*)_ptr;
    uint32 offset = *((uint32*)aligned - 1);
    uint8* ptr = aligned - offset;
    size_t total = _size + _align;
    ptr = (uint8*)realloc(_allocator, ptr, total, 0, _file, _line);
    uint8* newAligned = (uint8*)alignPtr(ptr, sizeof(uint32), _align);

    if (newAligned == aligned)
    {
      return aligned;
    }

    aligned = ptr + offset;
    ::memmove(newAligned, aligned, _size);
    uint32* header = (uint32*)newAligned - 1;
    *header = uint32(newAligned - ptr);
    return newAligned;
  }

  template <typename ObjectT>
  inline void deleteObject(AllocatorI* _allocator, ObjectT* _object, size_t _align = 0, const char* _file = NULL, uint32 _line = 0)
  {
    if (NULL != _object)
    {
      _object->~ObjectT();
      free(_allocator, _object, _align, _file, _line);
    }
  }

  extern CRTAllocator* g_crt_allcator;

#pragma endregion BXMEM

#endif

  class RBPathTool{
  public:
    static std::string char2string(const char* tstring)
    {
      return std::string(tstring);
    }

    static std::string get_file_directory(const std::string& file_path)
    {
      int len = file_path.size();
      const char* chars = file_path.c_str();
      int sep = 0;
      for (int i = 0; i < len; ++i)
      {
        if (file_path[i] == '/')
        {
          sep = i;
        }
      }

      return get_n1_to_n2(file_path, 0, sep);
    }

    static std::string get_file_extension(const std::string& file_path)
    {

      int pos = file_path.rfind('.');
      std::string res = file_path.substr(pos + 1);
      return res;
    }

    static std::string get_file_name(const std::string& file_path0)
    {
      int len = file_path0.size();
      const char* chars = file_path0.c_str();
      int sep = 0;
      int point = 0;
      for (int i = 0; i < len; ++i)
      {
        if (file_path0[i] == '/')
        {
          sep = i;
        }
        if (file_path0[i] == '.')
        {
          point = i;
        }

      }

      return get_n1_to_n2(file_path0, sep + 1, point - 1);
    }

    static std::string get_separator()
    {
      std::string separator;
#ifdef _WIN32
      separator = "\\";
#else
      separator = "/";
#endif

      return separator;
    }

    static const char* string2char(const std::string& tstring)
    {
      return tstring.c_str();
    }

    static std::string get_n1_to_n2(const std::string& tstring, int n1, int n2)
    {
      if (n1 > n2)
      {
        return "\0";
      }
      std::string ret;
      char temp = tstring[n1];
      int i = 0;
      do
      {
        ret.push_back(temp);
        temp = tstring[n1 + (++i)];
      } while (i <= n2 - n1);
      return ret;
    }


  };