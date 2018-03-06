#pragma once
//#include "RBMathUtilities.h"
/*with:
 *     stdlib.h/
 *	   RBMathBase.h/math.h
 */
//#include "stdio.h"
#include "Vector3.h"
//#include "Vector4.h"

/*
 *2D行向量
 *f32
 */

/**
 *因为在RBVector2中要定义函数，所以暂时定义此类
 *位置：Vector2.h
 */
class RBVector2I
{
public:
  i32 x, y;
  FORCEINLINE RBVector2I() = default;
  FORCEINLINE RBVector2I(i32 ax, i32 ay) : x(ax), y(ay){}
  FORCEINLINE RBVector2I(const RBVector2I& o) = default;
};

class RBVector2
{
public:
	f32 x,y;

	//默认构造一个0向量
	FORCEINLINE RBVector2():x(0.f),y(0.f){}
	FORCEINLINE RBVector2(f32 ax,f32 ay) : x(ax),y(ay){}
	FORCEINLINE RBVector2(const RBVector4& v4) : x(v4.x), y(v4.y) {}
	/**
	 *暂留一个使用整型向量初始化的向量
	 */
	FORCEINLINE RBVector2(RBVector2I ivec){}


	explicit FORCEINLINE RBVector2(RBMath::EForceInits)
		:x(0.f),y(0.f){}

	//不禁止隐式复制，交给编译器优化
	//explicit FORCEINLINE RBVector2(const RBVector2& vec2):x(vec2.x),y(vec2.y){}

	//显式使用3D向量前两维进行构造
	FORCEINLINE RBVector2(const RBVector3& vec3):x(vec3.x),y(vec3.y){}

	FORCEINLINE RBVector2 operator+( const RBVector2& vec2 ) const
	{
		return RBVector2(x+vec2.x,y+vec2.y);
	}

	FORCEINLINE RBVector2 operator-( const RBVector2& vec2 ) const
	{
		return RBVector2(x-vec2.x,y-vec2.y);
	}

	FORCEINLINE RBVector2 operator*( f32 a) const
	{
		return RBVector2(x * a,y * a);
	}

	RBVector2 operator/( f32 a) const
	{
		const f32 s = 1.f/a;
		return RBVector2(x * s,y * s);
	}

	FORCEINLINE RBVector2 operator+( f32 a ) const
	{
		return RBVector2(x + a,y + a);
	}

	FORCEINLINE RBVector2 operator-( f32 a ) const
	{
		return RBVector2(x - a,y - a);
	}

	FORCEINLINE RBVector2 operator*( const RBVector2& vec2 ) const
	{
		return RBVector2(x * vec2.x,y * vec2.y);
	}

	FORCEINLINE RBVector2 operator/( const RBVector2& vec2 ) const
	{
		return RBVector2(x/vec2.x,y/vec2.y);
	}

	//点乘
	FORCEINLINE f32 operator|( const RBVector2& vec2) const
	{
		return x*vec2.x+y*vec2.y;
	}

	//交叉乘减
	FORCEINLINE f32 operator^( const RBVector2& vec2) const
	{
		return x*vec2.y - y*vec2.x;
	}

	FORCEINLINE static f32 dot_product( const RBVector2& a, const RBVector2& b )
	{
		//return a|b;
		return a.x*b.x + a.y*b.y;
	}

	FORCEINLINE static f32 squared_dist( const RBVector2& a, const RBVector2& b )
	{
		return RBMath::square(a.x-b.x)+RBMath::square(a.y-b.y);
	}

	FORCEINLINE static f32 dist(const RBVector2& a, const RBVector2& b)
	{
		return RBMath::sqrt(RBMath::square(a.x - b.x) + RBMath::square(a.y - b.y));
	}

	bool operator==( const RBVector2& a ) const
	{
		return x==a.x&&y==a.y;
	}

	bool operator!=( const RBVector2& a ) const
	{
		return x!=a.x||y!=a.y;
	}

	bool operator<( const RBVector2& a ) const
	{
		return x<a.x&&y<a.y;
	}

	bool operator>( const RBVector2& a ) const
	{
		return x>a.x&&y>a.y;
	}

	bool operator<=( const RBVector2& a ) const
	{
		return x<=a.x&&y<=a.y;
	}

	bool operator>=( const RBVector2& a ) const
	{
		return x>=a.x&&y>=a.y;
	}

	//容错等于
	bool tolerance_equals(const RBVector2& a,f32 tolerance) const
	{
		return RBMath::abs(x-a.x)<tolerance&&RBMath::abs(y-a.y)<tolerance;
	}
	
	//负号
	FORCEINLINE RBVector2 operator-() const
	{
		return RBVector2(-x,-y);
	}

	FORCEINLINE RBVector2 operator+=( const RBVector2& a)
	{
		x+=a.x;
		y+=a.y;
		return *this;
	}

	FORCEINLINE RBVector2 operator-=( const RBVector2& a)
	{
		x-=a.x;
		y-=a.y;
		return *this;
	}

	FORCEINLINE RBVector2 operator*=( f32 scale)
	{
		x*=scale;
		y*=scale;
		return *this;
	}

	RBVector2 operator/=( f32 a)
	{
		const f32 temp = 1.f/a;
		x*=temp;
		y*=temp;
		return *this;
	}

	RBVector2 operator*=( const RBVector2& a)
	{
		x*=a.x;
		y*=a.y;
		return *this;
	}

	RBVector2 operator/=( const RBVector2& a)
	{
		x/=a.x;
		y/=a.y;
		return *this;
	}
	
	//return reference to component
    f32& operator[]( int32 index)
	{
		//断言0或者1
		return ((index==0)?x:y);
	}
	//return copy to component.
	f32 operator[]( int32 index) const
	{
		return ((index==0)?x:y);
	}

	void set( f32 x,f32 y)
	{
		this->x = x;
		this->y = y;
	}

	f32 get_max() const
	{
#ifdef NEWMAXMIN
		return RBMath::get_max(x,y);
#else
		return RBMath::max(x,y);
#endif
	}

	f32 get_abs_max() const
	{
#ifdef NEWMAXMIN
		return RBMath::get_max(RBMath::abs(x),RBMath::abs(y));
#else
		return RBMath::max(RBMath::abs(x),RBMath::abs(y));
#endif
	}

	f32 get_min() const
	{
#ifdef NEWMAXMIN
		return RBMath::get_min(x,y);
#else
		return RBMath::min(x,y);
#endif
	}

	f32 get_abs_min() const
	{
#ifdef NEWMAXMIN
		return RBMath::get_min(RBMath::abs(x),RBMath::abs(y));
#else
		return RBMath::min(RBMath::abs(x),RBMath::abs(y));
#endif

	}

	f32 size() const
	{
		return RBMath::sqrt(x*x+y*y);
	}
	/**
	 * Get the maximum of the vectors coordinates.
	 *
	 * @return The maximum of the vectors coordinates.
	 */
	//f32 GetMax() const;

	/**
	 * Get the absolute maximum of the vectors coordinates.
	 *
	 * @return The absolute maximum of the vectors coordinates.
	 */
	//f32 GetAbsMax() const;

	/**
	 * Get the minimum of the vectors coordinates.
	 *
	 * @return The minimum of the vectors coordinates.
	 */
	//f32 GetMin() const;

	/**
	 * Get the length of this vector.
	 *
	 * @return The length of this vector.
	 */
	//f32 Size() const;

	/**
	 * Get the squared length of this vector.
	 *
	 * @return The squared length of this vector.
	 */
	f32 squared_size() const
	{
		return x*x+y*y;
	}

	/**
	 * Get a normalized copy of this vector if it is large enough.
	 *
	 * @param Tolerance Minimum squared length of vector for normalization.
	 *
	 * @return A normalized copy of the vector if safe, (0,0) otherwise.
	 */
	RBVector2 safe_normal(f32 tolerance=SMALLER_F) const
	{
		const f32 s = x*x + y*y;
		if(s>tolerance)
		{
			const f32 inv_s = RBMath::inv_sqrt(s);
			return RBVector2(x*inv_s,y*inv_s);
		}
		return RBVector2(0.f,0.f);

	}

	/**
	 * Normalize this vector if it is large enough, set it to (0,0) otherwise.
	 *
	 * @param Tolerance Minimum squared length of vector for normalization.
	 */
	void normalize(f32 tolerance=SMALLER_F)
	{
		const f32 s = x*x + y*y;
		if(s>tolerance)
		{
			const f32 inv_s = RBMath::inv_sqrt(s);
			x*=inv_s;
			y*=inv_s;
			return;
		}
		x = 0.f;
		y = 0.f;
	}

	/**
	 * Checks whether vector is near to zero within a specified tolerance.
	 *
	 * @param Tolerance Error tolerance.
	 *
	 * @return true if vector is in tolerance to zero, otherwise false.
	 */
	bool is_nearly_zero(f32 tolerance=SMALL_F) const
	{
		return RBMath::abs(x)<tolerance && RBMath::abs(y)<tolerance;
	}

	/**
	 * Checks whether vector is exactly zero.
	 *
	 * @return true if vector is exactly zero, otherwise false.
	 */
	bool is_zero() const
	{
		return x==0&&y==0;
	}

	/**
	 * Gets a specific component of the vector.
	 *
	 * @param Index The index of the component required.
	 *
	 * @return Reference to the specified component.
	 */
	//不知所用，暂记
	f32& Component( int32 Index )
	{
		return (&x)[Index];
	}

	/**
	 * Gets a specific component of the vector.
	 *
	 * @param Index The index of the component required.
	 *
	 * @return Copy of the specified component.
	 */
	//f32 Component( int32 Index ) const;

	/**
	 * Get this vector as an Int Point.
	 *
	 * @return New Int Point from this vector.
	 */
	//RBVector2I IntPoint() const;

	/**
	 * Creates a copy of this vector with both axes clamped to the given range.
	 * @return New vector with clamped axes.
	 */
	//RBVector2 ClampAxes( f32 MinAxisVal, f32 MaxAxisVal ) const;

	/**
	 * Get a textual representation of the vector.
	 *
	 * @return Text describing the vector.
	 */
	//FString ToString() const;

	/**
	 * Initialize this Vector based on an FString. The String is expected to contain X=, Y=.
	 * The RBVector2 will be bogus when InitFromString returns false.
	 *
	 * @param	InSourceString	FString containing the vector values.
	 *
	 * @return true if the X,Y values were read successfully; false otherwise.
	 */
	//bool InitFromString( const FString & InSourceString );

	/**
	 * Serialize a vector.
	 *
	 * @param Ar Serialization archive.
	 * @param V Vector being serialized.
	 *
	 * @return Reference to Archive after serialization.
	 */
	/*
	friend FArchive& operator<<( FArchive& Ar, RBVector2& V )
	{
		// @warning BulkSerialize: RBVector2 is serialized as memory dump
		// See std::vector::BulkSerialize for detailed description of implied limitations.
		return Ar << V.X << V.Y;
	}
	*/

	/**
	 * Utility to check if there are any NaNs in this vector.
	 *
	 * @return true if there are any NaNs in this vector, otherwise false.
	 */
	/*
	FORCEINLINE bool ContainsNaN() const
	{
		return (FMath::IsNaN(X) || !FMath::IsFinite(X) || 
			FMath::IsNaN(Y) || !FMath::IsFinite(Y));
	}
	
	RB_API bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
	*/
	/** Converts spherical coordinates on the unit sphere into a cartesian unit length vector. */
	//INLINE RBVector3 SphericalToUnitCartesian() const;



#ifdef _DEBUG
	//字符输出
	void out() const;
#endif

	/** Global 2D zero vector constant */
	static const RBVector2 zero_vector;

	/** Global 2D unit vector constant */
	static const RBVector2 unit_vector;

};

FORCEINLINE RBVector2 operator*( f32 scale, const RBVector2& a)
{
	return a.operator*( scale);
}

/*
const RBVector2 RBVector2::zero_vector(0.0f, 0.0f);
const RBVector2 RBVector2::unit_vector(1.0f, 1.0f);
*/

/*
//RBVector3定义区

RBVector3::RBVector3(const RBVector2 v,f32 az):x(v.x),y(v.y),z(az){}
RBVector2 RBVector3::unit_cartesian_to_spherical() const
{
	const f32 t = RBMath::acos(z/size());
	const f32 p = RBMath::atant2(y,x);
	return RBVector2(t,p);
}
*/


/*
//RBVector4定义区

RBVector4::RBVector4(RBVector2 axy,RBVector2 azw)
	:x(axy.x),y(axy.y),z(azw.x),w(azw.y){}
	*/

template <> struct TIsPODType<RBVector2> { enum { v = true }; };