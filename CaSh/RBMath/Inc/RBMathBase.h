#pragma once

#include "./Platform/RBBasedata.h"
//


#ifdef PI
#undef PI
#endif
#define PI (3.1415926535897932f)
#define SMALLER_F (1.e-8f)
#define SMALL_F (1.e-4f)
#define BIG_F (3.4e+38f)
#ifdef E
#undef E
#endif
#define E (2.71828182845904523536f)

#define MAX_F 3.402823466e+38F

#define INV_PI      (0.31830988618f)
#define  INV_2PI (0.15915494309189533577f)
#ifndef HALF_PI
#define HALF_PI     (1.57079632679f)
#endif
#define POW2_PI     (9.86960440109f)
#define ROOT_PI		(1.77245385091f)
#define TIWCE_PI	(6.28318530718f)
#define QUARTER_PI	(0.78539816340f)

#define SQRT2       (1.41421356237f)
#define SQRT3       (1.73205080757f)
#define LN2         (0.69314718056f) // ln(2)
#define INV_LN2		(1.44269504089f) // 1.0 / ln(2)

//判断精度，近似为零
/*从UE4直接拷贝，参考UnrealMathUtility.h*/

// Magic numbers for numerical precision.
#define DELTA			(0.00001f)
/**
 * Lengths of normalized vectors (These are half their maximum values
 * to assure that dot products with normalized vectors don't overflow).
 */
#define f32_NORMAL_THRESH				(0.0001f)

//
// Magic numbers for numerical precision.
//
#define POINT_ON_PLANE			(0.10f)		/* Thickness of plane for front/back/inside test */
#define POINT_ON_SIDE			(0.20f)		/* Thickness of polygon side's side-plane for point-inside/outside/on side test */
#define POINTS_ARE_SAME			(0.00002f)	/* Two points are same if within this distance */
#define POINTS_ARE_NEAR			(0.015f)	/* Two points are near if within this distance and can be combined if imprecise math is ok */
#define NORMALS_ARE_SAME			(0.00002f)	/* Two normal points are same if within this distance */
													/* Making this too large results in incorrect CSG classification and disaster */
#define VECTORS_ARE_NEAR			(0.0004f)	/* Two vectors are near if within this distance and can be combined if imprecise math is ok */
													/* Making this too large results in lighting problems due to inaccurate texture coordinates */
#define SPLIT_POLY_WITH_PLANE	(0.25f)		/* A plane splits a polygon in half */
#define SPLIT_POLY_PRECISELY		(0.01f)		/* A plane exactly splits a polygon */
#define ZERO_NORM_SQUARED		(0.0001f)	/* Size of a unit normal that is considered "zero", squared */
#define VECTORS_ARE_PARALLEL		(0.02f)		/* Vectors are parallel if dot product varies less than this */

/*拷贝结束*/


#define MIN_U8  ((u8)  0x00)
#define MIN_U16 ((u16) 0x0000)
#define MIN_U32 ((u32) 0x00000000)
#define MIN_U64 ((u64) 0x0000000000000000)
#define MIN_I8  ((i8)  -128)
#define MIN_I16 ((i16) 0x8000)
#define MIN_I32 ((i32) 0x80000000)
#define MIN_I64 ((i64) 0x8000000000000000)

#define MAX_U8  ((u8)  0xff)
#define MAX_U16 ((u16) 0xffff)
#define MAX_U32 ((u32) 0xffffffff)
#define MAX_U64 ((u64) 0xffffffffffffffff)
#define MAX_I8  ((i8)  127)
#define MAX_I16 ((i16) 0x7fff)
#define MAX_I32 ((i32) 0x7fffffff)
#define MAX_I64 ((i64) 0x7fffffffffffffff)

//8位防止有时候解析成为char

/*全是正数*/
#define MIN_F32			(1.175494351e-38F)	
#define MAX_F32			(3.402823466e+38F)
#define MIN_F64			(2.2250738585072014e-308)
#define MAX_F64			(1.7976931348623158e+308)

//必须重载过比较运算符
//RBMath中有类似功能
#define MAX(a,b)     (((a)>(b))?(a):(b))
#define MIN(a,b)     (((a)<(b))?(a):(b))

#define DEG2RAD(a)   ((a)*(PI/180.0f))
#define RAD2DEG(a)   ((a)*(180.0f/PI))
