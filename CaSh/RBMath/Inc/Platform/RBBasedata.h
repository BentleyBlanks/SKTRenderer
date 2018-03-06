#pragma once

//#define DISABLE_RBTYPE
#ifndef DISABLE_RBTYPE

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed __int64 i64;
typedef signed char int8;
typedef signed short int16;
typedef signed int int32;
typedef signed __int64 int64;

typedef unsigned char BYTE;
typedef unsigned char u8;
typedef unsigned char uint8;
typedef unsigned short u16;
typedef unsigned short uint16;
typedef unsigned int u32;
typedef unsigned int uint32;
typedef unsigned int uint;
typedef unsigned __int64 u64;
typedef unsigned __int64 uint64;
//typedef int u32;
typedef float float32;
typedef double double64;
typedef float f32;
typedef double f64;
typedef double real;
typedef float ft;

typedef unsigned long ul;
typedef long long ll;

#ifdef _WIN64
	typedef __int64 INT_PTR, *PINT_PTR;
    typedef unsigned __int64 UINT_PTR, *PUINT_PTR;

    typedef __int64 LONG_PTR, *PLONG_PTR;
    typedef unsigned __int64 ULONG_PTR, *PULONG_PTR;

#else
    typedef _w64 int INT_PTR, *PINT_PTR;
    typedef _w64 unsigned int UINT_PTR, *PUINT_PTR;

    typedef _w64 long LONG_PTR, *PLONG_PTR;
    typedef _w64 unsigned long ULONG_PTR, *PULONG_PTR;

#endif

typedef void* VOID_PTR;

//在文件中显示声明SAFE后禁用此宏
#ifndef SAFE_DELETE
#define SAFE_DELETE(p)			{ if(p) { delete (p);		(p)=NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p)	{ if(p) { delete [] (p);		(p)=NULL; } }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)			{ if(p) { (p)->Release();	(p)=NULL; } }
#endif

#define FORCEINLINE __forceinline
#define INLINE _inline

//0 success 1 fail
#define INI_RESULT u32
#define INI_SUCCESS 1
#define INI_FAIL 0
//定义错误代码
#define SYSTEM_MEMORY_LEAK 0
#define SYSTEM_MEMORY_SHORT 0
#define SYSTEM_MEMORY_SUCCESS 1 

//以下标识符
//#define RB_API 0


template<typename T>
class TIsPODType;

//template <> struct TIsPODType<RBObjectLoder> { enum { v = false }; };
//TIsPODType<RBVector4>::v

#endif
