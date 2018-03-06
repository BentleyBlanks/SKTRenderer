#pragma once

/**
 *only  Ax+By+Cz=D
 *be different from Ax+By+Cz+D=0
 */
//#include "RBMathUtilities.h"
//可以包含Vector3.h,完整后再做优化
//#include "Vector3.h"
//#include "Vector4.h"
#include "Matrix.h"


ALIGN(16) class RBPlane : public RBVector3
{
public:
	f32 w;//D

	FORCEINLINE RBPlane();

	FORCEINLINE RBPlane(const RBPlane& p);

	FORCEINLINE RBPlane(const RBVector4& v);

	FORCEINLINE RBPlane(f32 tx,f32 ty,f32 tz,f32 tw);

	FORCEINLINE RBPlane(RBVector3 normal,f32 tw);

	FORCEINLINE RBPlane(RBVector3 tbase,const RBVector3 &tnormal);

	/**
	 *Three points make a plane.
	 */
	RBPlane(RBVector3 a,RBVector3 b,RBVector3 c);

	/**
	 *Force to init
	 */
	explicit FORCEINLINE RBPlane(RBMath::EForceInits);

	// Functions.

	/**
	 * Calculates distance between plane and a point.
	 *
	 * @param P The other point.
	 *
	 * @return >0: point is in front of the plane, <0: behind, =0: on the plane */
	FORCEINLINE f32 plane_dot( const RBVector3 &p ) const;

	/**
	 * Get a flipped version of the plane.
	 *
	 * @return A flipped version of the plane.
	 */
	RBPlane get_flip() const;

	/**
	 * Get the result of transforming the plane by a Matrix.
	 *
	 * @param M The matrix to transform plane with.
	 *
	 * @return The result of transform.
	 */
	RBPlane transform_by( const RBMatrix& m ) const;

	/**
	 * You can optionally pass in the matrices transpose-adjoint, which save it recalculating it.
	 * MSM: If we are going to save the transpose-adjoint we should also save the more expensive
	 * determinant.
	 *
	 * @param M The Matrix to transform plane with.
	 * @param DetM Determinant of Matrix.
	 * @param TA Transpose-adjoint of Matrix.
	 *
	 * @return The result of transform.
	 */
	//RBPlane TransformByUsingAdjointT( const RBMatrix& M, f32 DetM, const RBMatrix& TA ) const;

	/**
	 * Check if two planes are identical.
	 *
	 * @param V The other plane.
	 *
	 * @return true if planes are identical, otherwise false.
	 */
	bool operator==( const RBPlane& v ) const;

	/**
	 * Check if two planes are different.
	 *
	 * @param V The other plane.
	 *
	 * @return true if planes are different, otherwise false.
	 */
	bool operator!=( const RBPlane& v ) const;

	/**
	 * Checks whether two planes are equal within specified tolerance.
	 *
	 * @param V The other plane.
	 * @param Tolerance Error Tolerance.
	 *
	 * @return true if the two planes are equal within specified tolerance, otherwise false.
	 */
	bool equals(const RBPlane& v, f32 tolerance=SMALL_F) const;

	/**
	 * Calculates dot product of two planes.
	 *
	 * @param V The other plane.
	 *
	 * @return The dot product.
	 */
	FORCEINLINE f32 operator|( const RBPlane& v ) const;

	/**
	 * Gets result of adding a plane to this.
	 *
	 * @param V The other plane.
	 *
	 * @return The result of adding a plane to this.
	 */
	RBPlane operator+( const RBPlane& v ) const;

	/**
	 * Gets result of subtracting a plane from this.
	 *
	 * @param V The other plane.
	 *
	 * @return The result of subtracting a plane from this.
	 */
	RBPlane operator-( const RBPlane& v ) const;

	/**
	 * Gets result of dividing a plane.
	 *
	 * @param Scale What to divide by.
	 *
	 * @return The result of division.
	 */
	RBPlane operator/( f32 scale ) const;

	/**
	 * Gets result of scaling a plane.
	 *
	 * @param Scale The scaling factor.
	 *
	 * @return The result of scaling.
	 */
	RBPlane operator*( f32 scale ) const;

	/**
	 * Gets result of multiplying a plane with this.
	 *
	 * @param V The other plane.
	 *
	 * @return The result of multiplying a plane with this.
	 */
	RBPlane operator*( const RBPlane& v );

	/**
	 * Add another plane to this.
	 *
	 * @param V The other plane.
	 *
	 * @return Copy of plane after addition.
	 */
	RBPlane operator+=( const RBPlane& v );

	/**
	 * Subtract another plane from this.
	 *
	 * @param V The other plane.
	 *
	 * @return Copy of plane after subtraction.
	 */
	RBPlane operator-=( const RBPlane& v );

	/**
	 * Scale this plane.
	 *
	 * @param Scale The scaling factor.
	 *
	 * @return Copy of plane after scaling.
	 */
	RBPlane operator*=( f32 scale );

	/**
	 * Multiply another plane with this.
	 *
	 * @param V The other plane.
	 *
	 * @return Copy of plane after multiplication.
	 */
	RBPlane operator*=( const RBPlane& v );

	/**
	 * Divide this plane.
	 *
	 * @param V What to divide by.
	 *
	 * @return Copy of plane after division.
	 */
	RBPlane operator/=( f32 v );

	/**
	 * Serializer.
	 *
	 * @param Ar Serialization Archive.
	 * @param P Plane to serialize.
	 *
	 * @return Reference to Archive after serialization.
	 */
	//friend FArchive& operator<<( FArchive& Ar, RBPlane &P )
	//{
	//	return Ar << (RBVector&)P << P.W;
	//}
	/**
	 * Serializes the vector compressed for e.g. network transmission.
	 * @param	Ar	Archive to serialize to/ from
	 * @return false to allow the ordinary struct code to run (this never happens)
	 */
	/*
	bool NetSerialize(FArchive& Ar, class UPackageMap*, bool& bOutSuccess)
	{
		if( Ar.IsLoading() )
		{
			int16 iX;
			int16 iY;
			int16 iZ;
			int16 iW;
			Ar << iX << iY << iZ << iW;
			*this = RBPlane(iX,iY,iZ,iW);
		}
		else
		{
			int16 iX(FMath::Round(X));
			int16 iY(FMath::Round(Y));
			int16 iZ(FMath::Round(Z));
			int16 iW(FMath::Round(W));
			Ar << iX << iY << iZ << iW;
		}
		bOutSuccess = true;
		return true;
	}
	*/
	FORCEINLINE RBMatrix get_reflect_mat();
};



FORCEINLINE RBPlane::RBPlane()
{}

FORCEINLINE RBPlane::RBPlane( const RBPlane& p )
	:	RBVector3(p)
	,	w(p.w)
{}

FORCEINLINE RBPlane::RBPlane( const RBVector4& v )
	:	RBVector3(v)
	,	w(v.w)
{}

FORCEINLINE RBPlane::RBPlane( f32 tx, f32 ty, f32 tz, f32 tw )
	:	RBVector3(tx,ty,tz)
	,	w(tw)
{}

FORCEINLINE RBPlane::RBPlane( RBVector3 tnormal, f32 tw )
	:	RBVector3(tnormal), w(tw)
{}

FORCEINLINE RBPlane::RBPlane( RBVector3 tbase, const RBVector3 &tnormal )
	:	RBVector3(tnormal)
	,	w(tbase | tnormal)
{}

FORCEINLINE RBPlane::RBPlane( RBVector3 a, RBVector3 b, RBVector3 c )
	:	RBVector3( ((b-a)^(c-a)).safe_normal() )
	,	w( a | ((b-a)^(c-a)).safe_normal() )
{}

FORCEINLINE RBPlane::RBPlane(RBMath::EForceInits)
	: RBVector3(RBMath::EForceInits::E_FORCEINIT), w(0.f)
{}


FORCEINLINE f32 RBPlane::plane_dot( const RBVector3 &p ) const
{
	return x*p.x + y*p.y + z*p.z - w;
}

FORCEINLINE RBPlane RBPlane::get_flip() const
{
	return RBPlane(-x,-y,-z,-w);
}

FORCEINLINE bool RBPlane::operator==( const RBPlane& v ) const
{
	return x==v.x && y==v.y && z==v.z && w==v.w;
}

FORCEINLINE bool RBPlane::operator!=( const RBPlane& v ) const
{
	return x!=v.x || y!=v.y || z!=v.z || w!=v.w;
}

FORCEINLINE bool RBPlane::equals(const RBPlane& v, f32 tolerance) const
{
	return RBMath::abs(x-v.x) < tolerance && RBMath::abs(y-v.y) < tolerance && RBMath::abs(z-v.z) < tolerance && RBMath::abs(w-v.w) < tolerance;
}

FORCEINLINE f32 RBPlane::operator|( const RBPlane& v ) const
{
	return x*v.x + y*v.y + z*v.z + w*v.w;
}

FORCEINLINE RBPlane RBPlane::operator+( const RBPlane& v ) const
{
	return RBPlane( x + v.x, y + v.y, z + v.z, w + v.w );
}

FORCEINLINE RBPlane RBPlane::operator-( const RBPlane& v ) const
{
	return RBPlane( x - v.x, y - v.y, z - v.z, w - v.w );
}

FORCEINLINE RBPlane RBPlane::operator/( f32 scale ) const
{
	const f32 rscale = 1.f/scale;
	return RBPlane( x * rscale, y * rscale, z * rscale, w * rscale );
}

FORCEINLINE RBPlane RBPlane::operator*( f32 scale ) const
{
	return RBPlane( x * scale, y * scale, z * scale, w * scale );
}

FORCEINLINE RBPlane RBPlane::operator*( const RBPlane& v )
{
	return RBPlane ( x*v.x,y*v.y,z*v.z,w*v.w );
}

FORCEINLINE RBMatrix RBPlane::get_reflect_mat()
{
	f32 D = -w;
	f32 A = x;
	f32 B = y;
	f32 C = z;
	RBMatrix mat;
	mat.set_quickly(
		B*B+C*C-A*A,-A*B,-A*C,-A*D,
		-A*B,A*A+C*C-B*B,-B*C,-B*D,
		-A*C,-B*C,B*B+A*A-C*C,-C*D,
		0.f,0.f,0.f,A*A+B*B+C*C
		);
	return mat.get_transposed();
}

FORCEINLINE RBPlane RBPlane::operator+=(const RBPlane& v)
{
	x += v.x; y += v.y; z += v.z; w += v.w;
	return *this;
}

FORCEINLINE RBPlane RBPlane::operator-=( const RBPlane& v )
{
	x -= v.x; y -= v.y; z -= v.z; w -= v.w;
	return *this;
}

FORCEINLINE RBPlane RBPlane::operator*=( f32 scale )
{
	x *= scale; y *= scale; z *= scale; w *= scale;
	return *this;
}

FORCEINLINE RBPlane RBPlane::operator*=( const RBPlane& v )
{
	x *= v.x; y *= v.y; z *= v.z; w *= v.w;
	return *this;
}

FORCEINLINE RBPlane RBPlane::operator/=( f32 v )
{
	const f32 rv = 1.f/v;
	x *= rv; y *= rv; z *= rv; w *= rv;
	return *this;
}

//template <> struct TIsPODType<RBPlane> { enum { Value = true }; };
