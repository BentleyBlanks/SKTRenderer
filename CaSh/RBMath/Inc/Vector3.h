#pragma once
//#include "RBMathUtilities.h"
/*with:
 *     stdlib.h/
 *	   RBMathBase.h/math.h
 */
//#include "stdio.h"
#include "Vector4.h"

/*
 *3D行向量
 *f32
 */
class RBVector4;
class RBVector2;
class RBVector3
{
public:
	f32 x,y,z;
	FORCEINLINE RBVector3():x(0.f),y(0.f),z(0.f){}
	explicit FORCEINLINE RBVector3(f32 a):x(a),y(a),z(a){}

	/**
	 * Constructor
	 *
	 * @param InX X Coordinate.
	 * @param InY Y Coordinate.
	 * @param InZ Z Coordinate.
	 */
	FORCEINLINE RBVector3( f32 a, f32 b, f32 c)
		:x(a),y(b),z(c){}

  explicit FORCEINLINE RBVector3(f32* a):x(a[0]),y(a[1]),z(a[2]){};

	/**
	 * Constructor
	 * 
	 * @param V Vector to copy from.
	 * @param InZ Z Coordinate.
	 */
	//声明在Vector2.h
	explicit RBVector3( const RBVector2& v, f32 az);	


	/**
	 * Constructor
	 *
	 * @param V 4D Vector to copy from.
	 */
	FORCEINLINE RBVector3( const RBVector4& v4)
		:x(v4.x),y(v4.y),z(v4.z){}

	/**
	 * Constructor
	 *
	 * @param InColour Colour to copy from.
	 */
	//explicit RBVector3(const RBColorf& InColor);

	/**
	 * Constructor
	 *
	 * @param InVector FIntVector to copy from.
	 */
	//explicit RBVector3(FIntVector InVector);

	/**
	 * Constructor
	 *
	 * @param A Int Point used to set X and Y coords, Z is set to zero.
	 */
	//explicit RBVector3( RBVector2I A );

	/**
	 * Constructor
	 *
	 * @param EForceInit Force Init Enum
	 */
	explicit FORCEINLINE RBVector3(RBMath::EForceInits)
		:x(0.f),y(0.f),z(0.f){}


	/**
	* Copy another RBVector3 into this one
	*
	* @param Other The other vector.
	*
	* @return Reference to vector after copy.
	*/
	FORCEINLINE RBVector3& operator=(const RBVector3& b)
	{
		x = b.x;
		y = b.y;
		z = b.z;
		return *this;
	}

  FORCEINLINE RBVector3& operator=(float* a)
  {
    x = a[0];
    y = a[1];
    z = a[2];
    return *this;
  }

	/**
	 * Calculate Cross product between this and another vector.
	 *
	 * @param V The other vector.
	 *
	 * @return The Cross product.
	 */
	FORCEINLINE RBVector3 operator^( const RBVector3& a) const
	{
		return RBVector3(y*a.z-z*a.y,z*a.x-x*a.z,x*a.y-y*a.x);
	}

	/**
	 * Calculate the Cross product of two vectors.
	 *
	 * @param A The first vector.
	 * @param B The second vector.
	 *
	 * @return The Cross product.
	 */
	FORCEINLINE static RBVector3 cross_product( const RBVector3& a, const RBVector3& b)
	{
		return a^b;
	}

	/**
	 * Calculate the Dot Product between this and another vector.
	 *
	 * @param V The other vector.
	 *
	 * @return The Dot Product.
	 */
	FORCEINLINE f32 operator|( const RBVector3& a) const
	{
		return x*a.x+y*a.y+z*a.z;
	}

	/**
	 * Calculate the Dot product of two vectors.
	 *
	 * @param A The first vector.
	 * @param B The second vector.
	 *
	 * @return The Dot product.
	 */
	FORCEINLINE static f32 dot_product( const RBVector3& a, const RBVector3& b)
	{
		return a|b;
	}

	FORCEINLINE static f32 abs_dot(const RBVector3& a,const RBVector3& b)
	{
		return RBMath::abs(a|b);
	}

	/**
	 * Gets the result of adding a vector to this.
	 *
	 * @param V The vector to add.
	 *
	 * @return The result of vector addition.
	 */
	FORCEINLINE RBVector3 operator+( const RBVector3& a) const
	{
		return RBVector3(x+a.x,y+a.y,z+a.z);
	}

	/**
	 * Gets the result of subtracting a vector from this.
	 *
	 * @param V The vector to subtract.
	 *
	 * @return The result of vector subtraction.
	 */
	FORCEINLINE RBVector3 operator-( const RBVector3& a) const
	{
		return RBVector3(x-a.x,y-a.y,z-a.z);
	}

	/**
	 * Gets the result of subtracting from each component of the vector.
	 *
	 * @param Bias What to subtract.
	 *
	 * @return The result of subtraction.
	 */
	FORCEINLINE RBVector3 operator-( f32 a) const
	{
		return RBVector3(x-a,y-a,z-a);
	}

	/**
	 * Gets the result of adding to each component of the vector.
	 *
	 * @param Bias What to add.
	 *
	 * @return The result of addition.
	 */
	FORCEINLINE RBVector3 operator+( f32 a) const
	{
		return RBVector3(x+a,y+a,z+a);
	}

	/**
	 * Gets the result of multiplying each component of the vector.
	 *
	 * @param Scale What to multiply by.
	 *
	 * @return The result of multiplication.
	 */
	FORCEINLINE RBVector3 operator*( f32 a) const
	{
		return RBVector3(x*a,y*a,z*a);
	}

	/**
	 * Gets the result of dividing each component of the vector.
	 *
	 * @param Scale What to divide by.
	 *
	 * @return The result of division.
	 */
	RBVector3 operator/( f32 a) const
	{
		const f32 s = 1.f/a;
		return RBVector3(x*s,y*s,z*s);
	}

	/**
	 * Gets the result of multiplying vector with this.
	 *
	 * @param V The vector to multiply with.
	 *
	 * @return The result of multiplication.
	 */
	FORCEINLINE RBVector3 operator*( const RBVector3& a) const
	{
		return RBVector3(x*a.x,y*a.y,z*a.z);
	}

	/**
	 * Gets the result of dividing this vector by another.
	 *
	 * @param V The vector to divide by.
	 *
	 * @return The result of division.
	 */
	FORCEINLINE RBVector3 operator/( const RBVector3& a) const
	{
		return RBVector3(x/a.x,y/a.y,z/a.z);
	}

	// Binary comparison operators.

	/**
	 * Check against another vector for equality.
	 *
	 * @param V The vector to check against.
	 *
	 * @return true if the vectors are equal, otherwise false.
	 */
	bool operator==( const RBVector3& a) const
	{
		return x==a.x&&y==a.y&&z==a.z;
	}

	/**
	 * Check against another vector for inequality.
	 *
	 * @param V The vector to check against.
	 *
	 * @return true if the vectors are not equal, otherwise false.
	 */
	bool operator!=( const RBVector3& a) const
	{
		return x!=a.x||y!=a.y||z!=a.z;
	}

	/**
	 * Check against another vector for equality, within specified error limits.
	 *
	 * @param V The vector to check against.
	 * @param Tolerance Error tolerance.
	 *
	 * @return true if the vectors are equal within tolerance limits, otherwise false.
	 */
	bool tolerance_equals(const RBVector3& a, f32 tolerance=SMALL_F) const
	{
		return 
			RBMath::abs(x-a.x)<tolerance &&
			RBMath::abs(y-a.y)<tolerance &&
			RBMath::abs(z-a.z)<tolerance;
	}

	/**
	 * Checks whether all components of the vector are the same, within a tolerance.
	 *
	 * @param Tolerance Error Tolerance.
	 *
	 * @return true if the vectors are equal within tolerance limits, otherwise false.
	 */
	bool all_equal(f32 tolerance=SMALL_F) const
	{
		return 
			RBMath::abs(x-y)<tolerance &&
			RBMath::abs(x-z)<tolerance &&
			RBMath::abs(y-z)<tolerance;
	}


	/**
	 * Get a negated copy of the vector.
	 *
	 * @return A negated copy of the vector.
	 */
	FORCEINLINE RBVector3 operator-() const
	{
		return RBVector3(-x,-y,-z);
	}


	/**
	 * Adds another vector to this.
	 *
	 * @param V Vector to add.
	 *
	 * @return Copy of the vector after addition.
	 */
	FORCEINLINE RBVector3 operator+=( const RBVector3& a)
	{
		x += a.x;
		y += a.y;
		z += a.z;
		return *this;
	}

	/**
	 * Subtracts another vector from this.
	 *
	 * @param V Vector to subtract.
	 *
	 * @return Copy of the vector after subtraction.
	 */
	FORCEINLINE RBVector3 operator-=( const RBVector3& a)
	{
		x -= a.x;
		y -= a.y;
		z -= a.z;
		return *this;
	}

	/**
	 * Scales the vector.
	 *
	 * @param Scale What to scale vector by.
	 *
	 * @return Copy of the vector after scaling.
	 */
	FORCEINLINE RBVector3 operator*=( f32 s)
	{
		x *= s;
		y *= s;
		z *= s;
		return *this;
	}

	/**
	 * Divides the vector by a number.
	 *
	 * @param V What to divide the vector by.
	 *
	 * @return Copy of the vector after division.
	 */
	RBVector3 operator/=( f32 a)
	{
		const f32 av = 1.f/a;
		x *= av;
		y *= av;
		z *= av;
		return *this;
	}

	/**
	 * Multiplies the vector with another vector.
	 *
	 * @param V What to multiply vector with.
	 *
	 * @return Copy of the vector after multiplication.
	 */
	RBVector3 operator*=( const RBVector3& a)
	{
		x *= a.x;
		y *= a.y;
		z *= a.z;
		return *this;
	}

	/**
	 * Divides the vector by another vector.
	 *
	 * @param V What to divide vector by.
	 *
	 * @return Copy of the vector after division.
	 */
	RBVector3 operator/=( const RBVector3& a)
	{
		x /= a.x;
		y /= a.y;
		z /= a.z;
		return *this;
	}

	/**
	 * Gets specific component of the vector.
	 *
	 * @param Index the index of vector component
	 *
	 * @return reference to component.
	 */
	f32& operator[]( int32 index)
	{
		if(index==0)
			return x;
		else if(index==1)
			return y;
		else 
			return z;
	}

	/**
	 * Gets specific component of the vector.
	 *
	 * @param Index the index of vector component
	 *
	 * @return Copy of the component.
	 */
	f32 operator[]( int32 index)const
	{
		if(index==0)
			return x;
		else if(index==1)
			return y;
		else 
			return z;
	}

	// Simple functions.

	/**
	 * Set the values of the vector directly.
	 *
	 * @param InX New X coordinate.
	 * @param InY New Y coordinate.
	 * @param InZ New Z coordinate.
	 */
	void set( f32 x, f32 y, f32 z)
	{
		this->x = x;
		this->y = y;
		this->z = z;

	}

	/**
	 * Get the maximum of the vectors coordinates.
	 *
	 * @return The maximum of the vectors coordinates.
	 */
	f32 get_max() const
	{
		//return RBMath::max(RBMath::max(x,y),z);
		return RBMath::max3(x,y,z);
	}

	/**
	 * Get the absolute maximum of the vectors coordinates.
	 *
	 * @return The absolute maximum of the vectors coordinates.
	 */
	f32 get_abs_max() const
	{
		//return RBMath::max(RBMath::max(RBMath::abs(x),RBMath::abs(y)),RBMath::abs(z));
		return RBMath::max3(RBMath::abs(x),RBMath::abs(y),RBMath::abs(z));
	}

	/**
	 * Get the minimum of the vectors coordinates.
	 *
	 * @return The minimum of the vectors coordinates.
	 */
	f32 get_min() const
	{
		//return RBMath::min(RBMath::min(x,y),z);
		return RBMath::min3(x,y,z);
	}

	/**
	 * Get the absolute minimum of the vectors coordinates.
	 *
	 * @return The absolute minimum of the vectors coordinates.
	 */
	f32 get_abs_min() const
	{
		//return RBMath::min(RBMath::min(RBMath::abs(x),RBMath::abs(y)),RBMath::abs(z));
		return RBMath::min3(RBMath::abs(x),RBMath::abs(y),RBMath::abs(z));
	}

	/** Gets the component-wise min of two vectors. */
	RBVector3 both_min(const RBVector3& other) const
	{
#ifdef NEWMAXMIN
		return RBVector3(RBMath::get_min(x,other.x),RBMath::get_min(y,other.y),RBMath::get_min(z,other.z));
#else
		return RBVector3(RBMath::min(x,other.x),RBMath::min(y,other.y),RBMath::min(z,other.z));
#endif
	}

	/** Gets the component-wise max of two vectors. */
	RBVector3 both_max(const RBVector3& other) const
	{
#ifdef NEWMAXMIN
		return RBVector3(RBMath::get_max(x,other.x),RBMath::get_max(y,other.y),RBMath::get_max(z,other.z));
#else
		return RBVector3(RBMath::max(x,other.x),RBMath::max(y,other.y),RBMath::max(z,other.z));
#endif
	}

	/**
	 * Get a copy of this vector with absolute components.
	 *
	 * @return A copy of this vector with absolute components.
	 */
	RBVector3 get_abs() const
	{
		return RBVector3(RBMath::abs(x),RBMath::abs(y),RBMath::abs(z));
	}

	/**
	 * Get the length of this vector.
	 *
	 * @return The length of this vector.
	 */
	f32 size() const
	{
		return RBMath::sqrt(x*x+y*y+z*z);
	}

	/**
	 * Get the squared length of this vector.
	 *
	 * @return The squared length of this vector.
	 */
	f32 squared_size() const
	{
		return x*x+y*y+z*z;
	}

	/**
	 * Get the length of the 2D components of this vector.
	 *
	 * @return The 2D length of this vector.
	 */
	f32 size_2D() const
	{
		return RBMath::sqrt(x*x+y*y);
	}

	/**
	 * Get the squared length of the 2D components of this vector.
	 *
	 * @return The squared 2D length of this vector.
	 */
	f32 sdquared_size_2D() const
	{
		return x*x+y*y;
	}

	/**
	 * Checks whether vector is near to zero within a specified tolerance.
	 *
	 * @param Tolerance Error tolerance.
	 *
	 * @return true if the vector is near to zero, otherwise false.
	 */
	bool is_nearly_zero(f32 tolerance=SMALL_F) const
	{
		return 
			RBMath::abs(x)<tolerance &&
			RBMath::abs(y)<tolerance &&
			RBMath::abs(z)<tolerance;
	}

	/**
	 * Checks whether vector is exactly zero.
	 *
	 * @return true if the vector is exactly zero, otherwise false.
	 */
	bool is_zero() const
	{
		return x==0.f&&y==0.f&&z==0.f;
	}

	/**
	 * Normalize this vector if it is large enough.
	 *
	 * @param Tolerance Minimum squared length of vector for normalization.
	 *
	 * @return true if the vector was normalized correctly, otherwise false.
	 */
	void normalize(f32 tolerance=SMALLER_F)
	{
		const f32 s = x*x + y*y + z*z; 
		if(s>tolerance)
		{
			const f32 inv_s =  RBMath::inv_sqrt(s);
			x *= inv_s;
			y *= inv_s;
			z *= inv_s;
			return; 
		}
	}

	RBVector3 get_normalized(f32 tolerance = SMALLER_F) const
	{
		RBVector3 ret = *this;
		const f32 s = x*x + y*y + z*z;
		if (s>tolerance)
		{
			const f32 inv_s = RBMath::inv_sqrt(s);
			ret.x *= inv_s;
			ret.y *= inv_s;
			ret.z *= inv_s;
			return ret;
		}
		return RBVector3::zero_vector;
	}
	/**
	 * Checks whether vector is normalized.
	 *
	 * @return true if Normalized, false otherwise.
	 */
	bool is_normalized() const
	{
		return (RBMath::abs(1.f - squared_size())<=0.01);
	}

	//be sure normalized
	RBVector3 reflect(const RBVector3& normal) const
	{
		return *this - normal*2.f*dot_product(*this,normal);
	}
	/**
	 * Util to convert this vector into a unit direction vector, and its original length
	 *
	 * @param OutDir Reference passed in to store unit direction vector.
	 * @param OutLength Reference passed in to store length of the vector.
	 */
	void out_direction_and_length(RBVector3 &out_direction, f32 &out_length)
	{
		out_length = size();
		if(out_length>SMALLER_F)
		{
			f32 t = 1.f/out_length;
			out_direction = RBVector3(x*t,y*t,z*t);
		}
		else
		{
			out_direction = RBVector3::zero_vector;
		}
	}

	/**
	 * Get a copy of the vector as sign only.
	 *
	 * @param A copy of the vector with each component set to 1 or -1
	 */
	//获得符号单位向量
	FORCEINLINE RBVector3 get_sign_vector()
	{
		return RBVector3
			(
			x>0.f ? 1.f:-1.f,
			y>0.f ? 1.f:-1.f,
			z>0.f ? 1.f:-1.f
			);
	}

	/**
	 * Projects 2D components of vector based on Z.
	 *
	 * @return Projected version of vector.
	 */
	//沿y投射向量，即：把向量长度除以一个z值
	RBVector3 projection() const
	{
		const f32 t = 1.f/z;
		return RBVector3(x*t,y*t,1.f);
	}

	/**
	 * Calculates normalized version of vector without checking if it is non-zero.
	 *
	 * @return Normalized version of vector.
	 */
	FORCEINLINE RBVector3 unsafe_normal() const
	{
		const f32 t = RBMath::inv_sqrt(x*x+y*y+z*z);
		return RBVector3(x*t,y*t,z*t);
	}

	/**
	 * Gets a copy of this vector snapped to a grid.
	 *
	 * @param GridSz Grid dimension.
	 *
	 * @return A copy of this vector snapped to a grid.
	 */
	//RBVector3 GridSnap( const f32& GridSz ) const;

	/**
	 * Get a copy of this vector, clamped inside of a cube.
	 *
	 * @param Radius Half size of the cube.
	 *
	 * @return A copy of this vector, bound by cube.
	 */
	//RBVector3 BoundToCube( f32 Radius ) const;

	/** Create a copy of this vector, with its magnitude clamped between Min and Max. */
	//RBVector3 ClampSize(f32 Min, f32 Max) const;

	/** Create a copy of this vector, with the 2D magnitude clamped between Min and Max. Z is unchanged. */
	//RBVector3 ClampSize2D(f32 Min, f32 Max) const;

	/** Create a copy of this vector, with its magnitude clamped to MaxSize. */
	//RBVector3 ClampMaxSize(f32 MaxSize) const;

	/** Create a copy of this vector, with the 2D magnitude clamped to MaxSize. Z is unchanged. */
	//RBVector3 ClampMaxSize2D(f32 MaxSize) const;


	/**
	 * Add a vector to this and clamp the result in a cube.
	 *
	 * @param V Vector to add.
	 * @param Radius Half size of the cube.
	 */
	//void AddBounded( const RBVector3& V, f32 Radius=MAX_int16 );

	/**
	 * Gets a specific component of the vector.
	 *
	 * @param Index The index of the component required.
	 *
	 * @return Reference to the specified component.
	 */
	//f32& Component( int32 Index );

	/**
	 * Gets a specific component of the vector.
	 *
	 * @param Index The index of the component required.
	 *
	 * @return Copy of the specified component.
	 */
	//f32 Component( int32 Index ) const;

	/**
	 * Gets the reciprocal of this vector, avoiding division by zero.
	 * Zero components set to BIG_NUMBER.
	 *
	 * @return Reciprocal of this vector.
	 */
	//倒数
	RBVector3 reciprocal() const
	{
		RBVector3 v;
		if(x!=0.f)
			v.x = 1.f/x;
		else
			v.x = BIG_F;
		if(y!=0.f)
			v.y = 1.f/y;
		else
			v.y = BIG_F;
		if(z!=0.f)
			v.z = 1.f/z;
		else
			v.z = BIG_F;

		return v;
	}

	/**
	 * Check whether X, Y and Z are nearly equal.
	 *
	 * @param Tolerance Specified Tolerance.
	 *
	 * @return true if X == Y == Z within the specified tolerance.
	 */
	bool is_uniform(f32 tolerance=SMALL_F) const
	{
		return all_equal();
	}

	/**
	 * Mirror a vector about a normal vector.
	 *
	 * @param MirrorNormal Normal vector to mirror about.
	 *
	 * @return Mirrored vector.
	 */
	//对称向量是单位向量
	RBVector3 mirror_by_unit_normal( const RBVector3& mirror_normal ) const
	{
		return mirror_normal * (2.f * (*this | mirror_normal)) - *this;
	}


	RBVector3 mirror_by_normal( const RBVector3& mirror_normal ) const
	{
		return mirror_normal*((2.f * (*this | mirror_normal))/RBMath::square(mirror_normal.size())) - *this;
	}
	
//#ifdef _DEBUG
	void out() const;
//#endif
	///**
	// * Mirrors a vector about a plane.
	// *
	// * @param Plane Plane to mirror about.
	// *
	// * @return Mirrored vector.
	// */







	/*
	RBVector3 MirrorByPlane( const FPlane& Plane ) const
	{

	}
	*/









	///**
	// * Rotates around Axis (assumes Axis.Size() == 1).
	// *
	// * @param Angle Angle to rotate (in degrees).
	// * @param Axis Axis to rotate around.
	// *
	// * @return Rotated Vector.
	// */
	INLINE RBVector3 rotate_angle_around_axis( const f32 degree, const RBVector3& axis ) const
	{
		const f32 S	= RBMath::sin(DEG2RAD(degree));
		const f32 C	= RBMath::cos(DEG2RAD(degree));

		const f32 XX	= axis.x * axis.x;
		const f32 YY	= axis.y * axis.y;
		const f32 ZZ	= axis.z * axis.z;

		const f32 XY	= axis.x * axis.y;
		const f32 YZ	= axis.y * axis.z;
		const f32 ZX	= axis.z * axis.x;

		const f32 XS	= axis.x * S;
		const f32 YS	= axis.y * S;
		const f32 ZS	= axis.z * S;

		const f32 OMC	= 1.f - C;

		return RBVector3(
			(OMC * XX + C ) * x + (OMC * XY - ZS) * y + (OMC * ZX + YS) * z,
			(OMC * XY + ZS) * x + (OMC * YY + C ) * y + (OMC * YZ - XS) * z,
			(OMC * ZX - YS) * x + (OMC * YZ + XS) * y + (OMC * ZZ + C ) * z
			);
	}

	///**
	// * Gets a normalized copy of the vector, checking it is safe to do so.
	// *
	// * @param Tolerance Minimum squared vector length.
	// *
	// * @return Normalized copy if safe, otherwise returns zero vector.
	// */
	RBVector3 safe_normal(f32 tolerance=SMALLER_F) const
	{
		const f32 s = x*x+y*y+z*z;
		// Not sure if it's safe to add tolerance in there. Might introduce too many errors
		if(s==1.f)
			return *this;
		else if(s<tolerance)
			return RBVector3::zero_vector;
		const f32 scale = RBMath::inv_sqrt(s);
		return RBVector3(x*scale,y*scale,z*scale);
	}

	///**
	// * Gets a normalized copy of the 2D components of the vector, checking it is safe to do so.
	// *
	// * @param Tolerance Minimum squared vector length.
	// *
	// * @return Normalized copy if safe, otherwise returns zero vector.
	// */
	RBVector3 safe_normal2D(f32 tolerance=SMALLER_F) const
	{
		const f32 s = x*x+y*y;
		// Not sure if it's safe to add tolerance in there. Might introduce too many errors
		if(s==1.f)
			return RBVector3(x,y,0.f);
		else if(s<tolerance)
			return RBVector3::zero_vector;
		const f32 scale = RBMath::inv_sqrt(s);
		return RBVector3(x*scale,y*scale,0.f);
		
	}

	///**
	// * Returns the cosine of the angle between this vector and another projected onto the XY plane (no z)
	// *
	// * @param B the other vector to find the 2D cosine of the angle with.
	// *
	// * @return The cosine.
	// */
	FORCEINLINE f32 cos_angle_2D(RBVector3 b) const
	{
		RBVector3 a(*this);
		a.z = b.z = 0.f;
		a.normalize();
		b.normalize();
		return a|b;
	}

	///**
	// * Projects this vector onto the input vector.
	// *
	// * @param A Vector to project onto, does not assume it is unnormalized.
	// *
	// * @return Projected vector.
	// */
	//投射向量到输入向量上
	FORCEINLINE RBVector3 project_onto( const RBVector3& a) const
	{
		return (a*(*this|a)/(a|a));
	}

	///**
	// * Return the FRotator corresponding to the direction that the vector
	// * is pointing in.  Sets Yaw and Pitch to the proper numbers, and sets
	// * roll to zero because the roll can't be determined from a vector.
	// *
	// * @return The FRotator from the vector's direction.
	// */
	// FRotator Rotation() const;

	///**
	// * Find good arbitrary axis vectors to represent U and V axes of a plane,
	// * given just the normal.
	// *
	// * @param Axis1 Reference to first axis.
	// * @param Axis2 Reference to second axis.
	// */
	// void FindBestAxisVectors( RBVector3& Axis1, RBVector3& Axis2 ) const;

	///** When this vector contains Euler angles (degrees), ensure that angles are between +/-180 */
	void limit_euler()
	{
		x = RBMath::limit_degrees(x);
		y = RBMath::limit_degrees(y);
		z = RBMath::limit_degrees(z);
	}

	///**
	// * Utility to check if there are any NaNs in this vector.
	// *
	// * @return true if there are any NaNs in this vector, otherwise false.
	// */
	//bool ContainsNaN() const;

	///**
	// * Check if the vector is of unit length, with specified tolerance.
	// *
	// * @param LengthSquaredTolerance Tolerance against squared length.
	// *
	// * @return true if the vector is a unit vector within the specified tolerance.
	// */
	FORCEINLINE bool is_unit(f32 tolerance = SMALL_F ) const
	{
		return RBMath::abs(1.f - squared_size())<tolerance;
	}

	///**
	// * Get a textual representation of this vector.
	// *
	// * @return A string describing the vector.
	// */
	//FString ToString() const;

	///** Get a short textural representation of this vector, for compact readable logging. */
	//FString ToCompactString() const;

	///**
	// * Initialize this Vector based on an FString. The String is expected to contain X=, Y=, Z=.
	// * The RBVector3 will be bogus when InitFromString returns false.
	// *
	// * @param	InSourceString	FString containing the vector values.
	// *
	// * @return true if the X,Y,Z values were read successfully; false otherwise.
	// */
	//bool InitFromString( const FString & InSourceString );

	///** 
	// * Converts a cartesian unit vector into spherical coordinates on the unit sphere.
	// * @return Output Theta will be in the range [0, PI], and output Phi will be in the range [-PI, PI]. 
	// */
	//定义在Vector2.h中
	RBVector2 unit_cartesian_to_spherical() const;

	///**
	// * Convert a direction vector into a 'heading' angle.
	// *
	// * @return 'Heading' angle between +/-PI. 0 is pointing down +X.
	// */
	//f32 HeadingAngle() const;

	///**
	// * Create an orthonormal basis from a basis with at least two orthogonal vectors.
	// * It may change the directions of the X and Y axes to make the basis orthogonal,
	// * but it won't change the direction of the Z axis.
	// * All axes will be normalized.
	// *
	// * @param XAxis - The input basis' XAxis, and upon return the orthonormal basis' XAxis.
	// * @param YAxis - The input basis' YAxis, and upon return the orthonormal basis' YAxis.
	// * @param ZAxis - The input basis' ZAxis, and upon return the orthonormal basis' ZAxis.
	// */

	//创建正交基

	//static  void CreateOrthonormalBasis(RBVector3& XAxis,RBVector3& YAxis,RBVector3& ZAxis);

	///**
	// * Compare two points and see if they're the same, using a threshold.
	// *
	// * @param P First vector.
	// * @param Q Second vector.
	// *
	// * @return 1=yes, 0=no.  Uses fast distance approximation.
	// */
	//这里的命名如果是are_same_points可能产生歧义
	static bool points_are_same( const RBVector3 &a, const RBVector3 &b)
	{
		f32 t;
		t=a.x-b.x;
		if((t>-POINTS_ARE_SAME)&&(t<POINTS_ARE_SAME))
		{
			t=a.y-b.y;
			if((t>-POINTS_ARE_SAME)&&(t<POINTS_ARE_SAME))
			{
				t=a.z-b.z;
				if((t>POINTS_ARE_SAME)&&(t<POINTS_ARE_SAME))
				{
					return 1;
				}
			}
		}
		return 0;
	}
	//
	///**
	// * Compare two points and see if they're within specified distance.
	// *
	// * @param Point1 First vector.
	// * @param Point2 Second vector.
	// * @param Dist Specified distance.
	// *
	// * @return 1=yes, 0=no.  Uses fast distance approximation.
	// */
	static bool points_are_near( const RBVector3 &p1, const RBVector3 &p2, f32 dist)
	{
		f32 t;
		t = p1.x-p2.x;
		if(RBMath::abs(t)>=dist) return 0;
		t = p1.y-p2.y;
		if(RBMath::abs(t)>=dist) return 0;
		t = p1.z-p2.z;
		if(RBMath::abs(t)>=dist) return 0;
		return 1;
	}

	///**
	// * Calculate the signed distance (in the direction of the normal) between
	// * a point and a plane.
	// *
	// * @param Point The Point we are checking.
	// * @param PlaneBase The Base Point in the plane.
	// * @param PlaneNormal The Normal of the plane.
	// *
	// * @return Signed distance  between point and plane.
	// */
	//平面法线是单位向量
	static f32 point_plane_dist( const RBVector3 &point, const RBVector3 &point_in_plane, const RBVector3 &plane_normal)
	{
		return (point-point_in_plane)|plane_normal;
	}

	//平面发现是任意长度的向量
	static f32 point_plane_dist_normal(const RBVector3 &point, const RBVector3 &point_in_plane, const RBVector3 &plane_normal)
	{
		return ((point-point_in_plane)|plane_normal)/plane_normal.size();
	}
	///**
	// * Calculate a the projection of a point on the given plane
	// *
	// * @param Point - the point to project onto the plane
	// * @param Plane - the plane
	// *
	// * @return Projection of Point onto Plane
	// */
	//static RBVector3 PointPlaneProject(const RBVector3& Point, const FPlane& Plane);

	///**
	// * Calculate a the projection of a point on the plane defined by CCW points A,B,C
	// *
	// * @param Point - the point to project onto the plane
	// * @param A,B,C - three points in CCW order defining the plane 
	// *
	// * @return Projection of Point onto plane ABC
	// */
	//A/B/C逆时针顺序
	//static RBVector3 point_plane_project(const RBVector3& point, const RBVector3& a, const RBVector3& b, const RBVector3& c)
	//{

	//}
	///**
	//* Calculate a the projection of a point on the plane defined by PlaneBase, and PlaneNormal
	//*
	//* @param Point - the point to project onto the plane
	//* @param PlaneBase - point on the plane
	//* @param PlaneNorm - normal of the plane
	//*
	//* @return Projection of Point onto plane ABC
	//*/
	static RBVector3 point_plane_project(const RBVector3 &point, const RBVector3 &point_in_plane, const RBVector3 &plane_normal)
	{
		return point - plane_normal * RBVector3::point_plane_dist(point,point_in_plane,plane_normal);
	}

	///**
	// * Euclidean distance between two points.
	// *
	// * @param V1 The first point.
	// * @param V2 The second point.
	// *
	// * @return The distance between two points.
	// */
	//欧氏距离
	static FORCEINLINE f32 dist( const RBVector3 &a, const RBVector3 &b)
	{
		return RBMath::sqrt(RBMath::square(a.x-b.x)+RBMath::square(a.y-b.y)+RBMath::square(a.z-b.z));
	}

	///**
	// * Squared distance between two points.
	// *
	// * @param V1 The first point.
	// * @param V2 The second point.
	// *
	// * @return The squared distance between two points.
	// */
	static FORCEINLINE f32 squared_dist( const RBVector3 &a, const RBVector3 &b)
	{
		return RBMath::square(a.x-b.x)+RBMath::square(a.y-b.y)+RBMath::square(a.z-b.z);
	}

	///**
	// * Compute pushout of a box from a plane.
	// *
	// * @param Normal The plane normal.
	// * @param Size The size of the box.
	// *
	// * @return Pushout required.
	// */
	//static FORCEINLINE f32 BoxPushOut( const RBVector3 & Normal, const RBVector3 & Size );

	///**
	// * See if two normal vectors (or plane normals) are nearly parallel.
	// *
	// * @param Normal1 First normalized vector.
	// * @param Normal1 Second normalized vector.
	// *
	// * @return true if vectors are parallel, otherwise false.
	// */
	static bool is_parallel( const RBVector3 &a, const RBVector3 &b)
	{

		const f32 t = a|b;
		return (RBMath::abs(RBMath::abs(t) - 1.f) <= VECTORS_ARE_PARALLEL);
	}

	static bool is_parallel( const RBVector3 &a, const RBVector3 &b,float epsilon)
	{

		const f32 t = a|b;
		return (RBMath::abs(RBMath::abs(t) - 1.f) <= epsilon);
	}

	///**
	// * See if two planes are coplanar.
	// *
	// * @param Base1 The base point in the first plane.
	// * @param Normal1 The normal of the first plane.
	// * @param Base2 The base point in the second plane.
	// * @param Normal2 The normal of the second plane.
	// *
	// * @return true if the planes are coplanar, otherwise false.
	// */
	//检测两平面共面
	static bool is_coplanar( const RBVector3 &base1, const RBVector3 &normal1, const RBVector3 &base2, const RBVector3 &normal2)
	{
		if      (!RBVector3::is_parallel(normal1,normal2)) return 0;
		else if (RBVector3::point_plane_dist(base2,base1,normal1) > POINT_ON_PLANE) return 0;
		else    return 1;

	}

	///**
	// * Triple product of three vectors.
	// *
	// * @param X The first vector.
	// * @param Y The second vector.
	// * @param Z The third vector.
	// *
	// * @return The triple product.
	// */
	//三重积
	static f32 triple_product( const RBVector3& a, const RBVector3& b, const RBVector3& c)
	{
		return
		(	(a.x * (b.y * c.z - b.z * c.y))+
			(a.y * (b.z * c.x - b.z * c.z))+
			(a.z * (b.x * c.y - b.y * c.x)) );
	}

	///**
	// * Generates a list of sample points on a Bezier curve defined by 2 points.
	// *
	// * @param	ControlPoints	Array of 4 RBVector3s (vert1, controlpoint1, controlpoint2, vert2).
	// * @param	NumPoints		Number of samples.
	// * @param	OutPoints		Receives the output samples.
	// *
	// * @return					Path length.
	// */
	//static  f32 EvaluateBezier(const RBVector3* ControlPoints, int32 NumPoints, std::vector<RBVector3>& OutPoints);

	///**
	// * Given a current set of cluster centers, a set of points, iterate N times to move clusters to be central. 
	// *
	// * @param Clusters Reference to array of Clusters.
	// * @param Points Set of points.
	// * @param NumIterations Number of iterations.
	// * @param NumConnectionsToBeValid Sometimes you will have long strings that come off the mass of points
	// * which happen to have been chosen as Cluster starting points.  You want to be able to disregard those.
	// */
	//static  void GenerateClusterCenters(std::vector<RBVector3>& Clusters, const std::vector<RBVector3>& Points, int32 NumIterations, int32 NumConnectionsToBeValid);

	///**
	// * Serializer.
	// *
	// * @param Ar Serialization Archive.
	// * @param V Vector to serialize.
	// *
	// * @return Reference to Archive after serialization.
	// */
	//friend FArchive& operator<<( FArchive& Ar, RBVector3& V )
	//{
	//	// @warning BulkSerialize: RBVector3 is serialized as memory dump
	//	// See std::vector::BulkSerialize for detailed description of implied limitations.
	//	return Ar << V.X << V.Y << V.Z;
	//}

	// bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
	// 
	static  const RBVector3 zero_vector;
	static  const RBVector3 up_vector;
};

/**
 * Multiplies a vector by a scaling factor.
 *
 * @param Scale Scaling factor.
 * @param V Vector to scale.
 *
 * @return Result of multiplication.
 */
FORCEINLINE RBVector3 operator*( f32 Scale, const RBVector3& V )
{
	return V.operator*( Scale );
}

/*
const RBVector3 RBVector3::zero_vector(0.f,0.f,0.f);
const RBVector3 RBVector3::up_vector(0.f,0.f,1.f);
*/

template <> struct TIsPODType<RBVector3> { enum { v = true }; };