#pragma once
#include "./Platform/WindowsPlatformTemp.h"
//#include "RBMathUtilities.h"
///*with:
// *     stdlib.h/
// *	   RBMathBase.h/math.h
// */
//
///*
// *齐次行向量
// *f32
// */
//
//#include "stdio.h"
#include "RBMathUtilities.h"
#include "Colorf.h"
class RBVector2;
class RBMatrix;
class RBVector3;
#if _MSC_VER < 1700
ALIGN(16) class RBVector4
#else
class RBVector4
#endif
{
public:
	f32 x,y,z,w;
//	/**
//	 * Constructor.
//	 *
//	 * @param InVector 3D Vector to set first three components.
//	 * @param InW W Coordinate.
//	 */
	//定义在Vector3.h中
	RBVector4(const RBVector3& v,f32 az = 1.0f);

	RBVector4(const RBColorf& color) :x(color.r), y(color.g), z(color.b), w(color.a){}
//
//	/**
//	 * Constructor.
//	 *
//	 * @param InColour Colour used to set vector.
//	 */
//	RBVector4(const RBColorf& InColor);
//
//	/**
//	 * Constructor.
//	 *
//	 * @param InX X Coordinate.
//	 * @param InY Y Coordinate.
//	 * @param InZ Z Coordinate.
//	 * @param InW W Coordinate.
//	 */
	explicit RBVector4(f32 ax = 0.0f,f32 ay = 0.0f,f32 az = 0.0f,f32 aw = 1.0f)
		:x(ax),y(ay),z(az),w(aw){}
//
	//定义在Vector2.h中
	explicit RBVector4(RBVector2 axy, RBVector2 azw);
	explicit RBVector4(f32* a) :x(a[0]), y(a[1]), z(a[2]), w(a[3]){}
	/**
	 * Constructor.
	 *
	 * @param EForceInit Force Init Enum.
	 */
	explicit RBVector4(RBMath::EForceInits)
		:x(0.f),y(0.f),z(0.f),w(0.f){}
//
//	/**
//	 * Access a specific component of the vector.
//	 *
//	 * @param ComponentIndex The index of the component.
//	 *
//	 * @return Reference to the desired component.
//	 */
	FORCEINLINE f32 & operator[]( int32 index)
	{
		return (&x)[index];
	}
//
//	/**
//	 * Access a specific component of the vector.
//	 *
//	 * @param ComponentIndex The index of the component.
//	 *
//	 * @return Copy of the desired component.
//	 */
	FORCEINLINE f32 operator[]( int32 index) const
	{
		return (&x)[index];
	}
//
//	/**
//	 * Set all of the vectors coordinates.
//	 *
//	 * @param InX New X Coordinate.
//	 * @param InY New Y Coordinate.
//	 * @param InZ New Z Coordinate.
//	 * @param InW New W Coordinate.
//	 */
	FORCEINLINE void set( f32  ax, f32  ay, f32  az, f32  aw)
	{
		x = ax;
		y = ay;
		z = az;
		w = aw;
	}
//
//	// Unary operators.
//
//	/**
//	 * Gets a negated copy of the vector.
//	 *
//	 * @return A negated copy of the vector.
//	 */
	FORCEINLINE RBVector4 operator-() const
	{
		return RBVector4(-x,-y,-z,-w);
	}
//
//	/**
//	 * Gets the result of adding a vector to this.
//	 *
//	 * @param V The vector to add.
//	 *
//	 * @return The result of vector addition.
//	 */
	FORCEINLINE RBVector4 operator+( const RBVector4& v) const
	{
		return RBVector4(x+v.x,y+v.y,z+v.z,w+v.w);
	}
//
//	/**
//	 * Adds another vector to this one.
//	 *
//	 * @param V The other vector to add.
//	 *
//	 * @return Copy of the vector after addition.
//	 */
	FORCEINLINE RBVector4 operator+=( const RBVector4& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
		return *this;
	}
//
//	/**
//	 * Gets the result of subtracting a vector from this.
//	 *
//	 * @param V The vector to subtract.
//	 *
//	 * @return The result of vector subtraction.
//	 */
	FORCEINLINE RBVector4 operator-( const RBVector4& v) const
	{
		return RBVector4(x-v.x,y-v.y,z-v.z,w-v.w);
	}
//
//	/**
//	 * Gets the result of scaling this vector.
//	 *
//	 * @param Scale The scaling factor.
//	 *
//	 * @return The result of vector scaling.
//	 */
	FORCEINLINE RBVector4 operator*( f32  s) const
	{
		return RBVector4(x*s,y*s,z*s,w*s);
	}
//
//	/**
//	 * Gets the result of dividing this vector.
//	 *
//	 * @param Scale What to divide by.
//	 *
//	 * @return The result of division.
//	 */
	RBVector4 operator/( f32  s) const
	{
		const f32 s_inv = 1.f/s;
		return RBVector4(x*s_inv,y*s_inv,z*s_inv,w*s_inv);
	}

	void operator/=(f32 s)
	{
		const f32 s_inv = 1.f/s;
		x *= s_inv;
		y *= s_inv;
		z *= s_inv;
		w *= s_inv;
	}
//
//	/**
//	 * Gets the result of dividing this vector.
//	 *
//	 * @param V What to divide by.
//	 *
//	 * @return The result of division.
//	 */
	RBVector4 operator/( const RBVector4& v) const
	{
		return RBVector4(x/v.x,y/v.y,z/v.z,w/v.w); 
	}
//
//	/**
//	 * Gets the result of multiplying a vector with this.
//	 *
//	 * @param V The vector to multiply.
//	 *
//	 * @return The result of vector multiplication.
//	 */
	RBVector4 operator*( const RBVector4& v) const
	{
		return RBVector4(x*v.x,y*v.y,z*v.z,w*v.w); 
	}
//
//	/**
//	 * Gets the result of multiplying a vector with another Vector (component wise).
//	 *
//	 * @param V The vector to multiply.
//	 *
//	 * @return The result of vector multiplication.
//	 */
	RBVector4 operator*=( const RBVector4& v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		w *= v.w;
		return *this;
	}
//
//	/**
//	 * Gets the result of dividing a vector with another Vector (component wise).
//	 *
//	 * @param V The vector to divide with.
//	 *
//	 * @return The result of vector multiplication.
//	 */
	RBVector4 operator/=( const RBVector4& v)
	{
		x /= v.x;
		y /= v.y;
		z /= v.z;
		w /= v.w;
		return *this;
	}
//
//	/**
//	 * Gets the result of scaling this vector.
//	 *
//	 * @param Scale The scaling factor.
//	 *
//	 * @return The result of vector scaling.
//	 */
	RBVector4 operator*=( f32  s)
	{
		x *= s;
		y *= s;
		z *= s;
		w *= s;
		return *this;
	}
//
//	// Simple functions.
//
//	/**
//	 * Gets a specific component of the vector.
//	 *
//	 * @param Index The index of the component.
//	 *
//	 * @return Reference to the component.
//	 */
//	f32 & Component( int32 Index );
//
//	/**
//	 * Calculates 3D Dot product of two 4D vectors.
//	 *
//	 * @param V1 The first vector.
//	 * @param V2 The second vector.
//	 *
//	 * @return The 3D Dot product.
//	 */
	//static原本是friend
	static FORCEINLINE f32  dot3( const RBVector4& a, const RBVector4& b)
	{
		return a.x*b.x + a.y*b.y + a.z*b.z;
	}
//
//	/**
//	 * Calculates 4D Dot product.
//	 *
//	 * @param V1 The first vector.
//	 * @param V2 The second vector.
//	 *
//	 * @return The 4D Dot Product.
//	 */
	//static原本是friend
	static FORCEINLINE f32  dot4( const RBVector4& a, const RBVector4& b)
	{
		return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
	}
//
//	/**
//	 * Scales a vector.
//	 *
//	 * @param Scale The scaling factor.
//	 * @param V The vector to scale.
//	 *
//	 * @return The result of scaling.
//	 */
	friend FORCEINLINE RBVector4 operator*( f32  s, const RBVector4& v)
	{
		return v.operator*(s);
	}
//
//
//	/**
//	 * Basic == or != operators for FQuat
//	 */	
//
//	/**
//	 * Checks for equality against another vector.
//	 *
//	 * @param V The other vector.
//	 *
//	 * @return true if the two vectors are the same, otherwise false.
//	 */
	bool operator==(const RBVector4& v) const
	{
		return ((x == v.x) && (y == v.y) && (z == v.z) && (w == v.w));
	}
//
//	/**
//	 * Checks for inequality against another vector.
//	 *
//	 * @param V The other vector.
//	 *
//	 * @return true if the two vectors are different, otherwise false.
//	 */
	bool operator!=(const RBVector4& v) const
	{
		return ((x != v.x) || (y != v.y) || (z != v.z) || (w != v.w));
	}
//
//	/**
//	 * Calculate Cross product between this and another vector.
//	 *
//	 * @param V The other vector.
//	 *
//	 * @return The Cross product.
//	 */
	RBVector4 operator^( const RBVector4& v) const
	{
		return RBVector4(y*v.z-z*v.y,z*v.x-x*v.z,x*v.y-v.x*y,0.f);
	}
//
//	/**
//	 * Error tolerant comparison.
//	 *
//	 * @param V Vector to compare against.
//	 * @param Tolerance Error Tolerance.
//	 *
//	 * @return true if the two vectors are equal within specified tolerance, otherwise false.
//	 */
	bool tolerance_equals(const RBVector4& v, f32  tolerance=SMALL_F) const
	{
		return 
			RBMath::abs(x-v.x)<tolerance &&
			RBMath::abs(y-v.y)<tolerance &&
			RBMath::abs(z-v.z)<tolerance &&
			RBMath::abs(w-v.w)<tolerance;
	}

//
//	/**
//	 * Check if the vector is of unit length, with specified tolerance.
//	 *
//	 * @param LengthSquaredTolerance Tolerance against squared length.
//	 *
//	 * @return true if the vector is a unit vector within the specified tolerance.
//	 */
	bool is_unit3(f32  tolerance = SMALL_F) const
	{
		return RBMath::abs(1.f-squared_size3())<tolerance;
	}
//
//	/**
//	 * Get a textual representation of the vector.
//	 *
//	 * @return Text describing the vector.
//	 */
//	FString ToString() const;
//
//	/**
//	 * Initialize this Vector based on an FString. The String is expected to contain X=, Y=, Z=, W=.
//	 * The RBVector4 will be bogus when InitFromString returns false.
//	 *
//	 * @param	InSourceString	FString containing the vector values.
//	 *
//	 * @return true if the X,Y,Z values were read successfully; false otherwise.
//	 */
//	bool InitFromString( const FString & InSourceString );
//
//	/**
//	 * Returns a normalized copy of the vector if safe to normalize.
//	 *
//	 * @param Tolerance Minimum squared length of vector for normalization.
//	 *
//	 * @return A normalized copy of the vector or a zero vector.
//	 */
	FORCEINLINE RBVector4 safe_normal3(f32  tolerance=SMALLER_F) const
	{
		const f32 s = x*x+y*y+z*z;
		if(s>tolerance)
		{
			const f32 scale = RBMath::inv_sqrt(s);
			return RBVector4(x*scale,y*scale,z*scale,0.f);
		}
		return RBVector4(0.f);
	}
//
//	/**
//	 * Calculates normalized version of vector without checking if it is non-zero.
//	 *
//	 * @return Normalized version of vector.
//	 */
	FORCEINLINE RBVector4 unsafe_normal3() const
	{
		const f32 s = RBMath::inv_sqrt(x*x+y*y+z*z);
		return RBVector4(x*s,y*s,z*s,0.f);
	}
//
//	/**
//	 * Return the FRotator corresponding to the direction that the vector
//	 * is pointing in.  Sets Yaw and Pitch to the proper numbers, and sets
//	 * roll to zero because the roll can't be determined from a vector.
//	 *
//	 * @return The FRotator of the vector's direction.
//	 */
//	RB_API FRotator Rotation() const;
//
//	/**
//	 * Serializer.
//	 *
//	 * @param Ar The Serialization Archive.
//	 * @param V The vector being serialized.
//	 *
//	 * @return Reference to the Archive after serialization.
//	 */
//	friend FArchive& operator<<( FArchive& Ar, RBVector4& V )
//	{
//		return Ar << V.X << V.Y << V.Z << V.W;
//	}
//
//	/**
//	 * Get the length of this vector not taking W component into account.
//	 *
//	 * @return The length of this vector.
//	 */
	f32  size3() const
	{
		return RBMath::sqrt(x*x+y*y+z*z);
	}
//
//	/**
//	 * Get the squared length of this vector not taking W component into account.
//	 *
//	 * @return The squared length of this vector.
//	 */
	f32  squared_size3() const
	{
		return x*x+y*y+z*z;
	}
//
//	/** Utility to check if there are any NaNs in this vector. */
//	bool ContainsNaN() const;
//
//	/** Utility to check if all of the components of this vector are nearly zero given the tolerance. */
	bool is_nearly_zero3(f32  tolerance=SMALL_F) const
	{
		return 
			(
			RBMath::abs(x)<tolerance &&
			RBMath::abs(y)<tolerance &&
			RBMath::abs(z)<tolerance
			);
	}
//
//	/** Reflect vector. */
	//normal前三组件为单位向量
	RBVector4 reflect3(const RBVector4& normal) const
	{
		return *this - 2.f*dot3(*this,normal)*normal;
	}
//
//	/**
//	 * Find good arbitrary axis vectors to represent U and V axes of a plane,
//	 * given just the normal.
//	 */
//	void FindBestAxisVectors3( RBVector4& Axis1, RBVector4& Axis2 ) const;
//

	//Apply a matrix to this vector with side use,implemented in Matrix.h
	FORCEINLINE void apply_matrix(const RBMatrix& m);

	//Matrix multiple to right with a return value,implemented in Matrix.h
	FORCEINLINE RBVector4 operator*(const RBMatrix& m) const;

//#ifdef _DEBUG
	void out() const;
//#endif // _DEBUG

	static  const RBVector4 zero_vector;
	//static  const RBVector4 up_vector;
};

template <> struct TIsPODType<RBVector4> { enum { v = true }; };

