#pragma once
#include "math.h"
#include <cmath>
#include "stdlib.h"
#include<time.h>
#include "RBMathBase.h"//with math.h
#include "platform/WindowsPlatformTemp.h"

/*如果想要使用max，min函数，可能是因为windows.h包含了max/min的宏，请预定义宏：'NOMINMAX'
 *如果想使用get_max,get_min函数来代替，请定义下列宏NEWMAXMIN
 */
#define NEWMAXMIN

//常量定义
extern const f32 INV_RAND_MAX;
extern const f32 INV_RAND_MAX_ADD1;


struct RBBaseMath : public PlatformMath
{
	//定义枚举
	enum EForceInits
	{
		E_FORCEINIT,
		E_FORCETOZERO
	};

	//e的a次方
	static FORCEINLINE f32 exp(f32 a){return ::expf(a);}
	static FORCEINLINE f32 ln(f32 a){return ::logf(a);}
	static FORCEINLINE f32 log_x(f32 b,f32 v){return ln(v)/ln(b);}
	static FORCEINLINE f32 log_2(f32 a){return ln(a)*INV_LN2;}

  static FORCEINLINE f32 ceil(f32 a){ return std::ceilf(a); }
  static FORCEINLINE f32 floor(f32 a){ return std::floorf(a); }

	static FORCEINLINE f32 fmod(f32 a,f32 b){return ::fmodf(a,b);}
	//RAD
	static FORCEINLINE f32 cot(f32 a){return ::tanf(HALF_PI-a);}
	//RAD
	static FORCEINLINE f32 sin(f32 a){return ::sinf(a);}
	//RAD
	static FORCEINLINE f32 asin(f32 a){return ::asinf( (a<-1.f) ? -1.f : ((a<1.f) ? a : 1.f) );}
	//RAD
	static FORCEINLINE f32 cos(f32 a){return ::cosf(a);}
	//RAD
	static FORCEINLINE f32 acos(f32 a){return ::acosf( (a<-1.f) ? -1.f : ((a<1.f) ? a : 1.f));}
	//RAD
	static FORCEINLINE f32 tan(f32 a){return ::tanf(a);}
	//RAD
	static FORCEINLINE f32 atan(f32 a){return ::atanf(a);}
	//RAD
	static FORCEINLINE f32 atant2(f32 a,f32 b){return ::atan2f(b,a);}
	//RAD
	static FORCEINLINE f32 sqrt(f32 a){return ::sqrtf(a);}
	static FORCEINLINE f32 pow(f32 a,f32 b){return ::powf(a,b);}
	static FORCEINLINE f32 inv_sqrt(f32 a){return 1.f/::sqrtf(a);}

	/*reference to http://www.matrix67.com/data/InvSqrt.pdf
	  only 0.XXX
	*/
	static FORCEINLINE f32 inv_sqrt_fast(f32 a)
	{
		f32 ahalf = 0.5f*a;
		int i = *(int*)&a;
		i = 0x5f3759df - (i>>1);
		a = *(f32*)&i;
		a = a*(1.5f - ahalf*a*a);
		return a;
	}
	static FORCEINLINE bool is_NaN_f32(f32 a)
	{
		return ((*(u32*)&a) & 0x7FFFFFFF) > 0x7F800000;
	}
	static FORCEINLINE bool is_finite_f32(f32 a)
	{
		return ((*(u32*)&a) & 0x7F800000) != 0x7F800000;
	}
	static FORCEINLINE bool is_negative_f32(f32 a)
	{
		return ( (*(u32*)&a) >= (u32)0x80000000 );
	}
	static FORCEINLINE bool is_pow_2(i32 power)
	{
		return 0==(power&(power-1));
	}

	//设置随机种子
	static FORCEINLINE void rand_init(i32 s){::srand(s);}

	static FORCEINLINE i32 rand_i(){return ::rand();} 
	//0..1
	static FORCEINLINE f32 rand_f(){return ::rand()*INV_RAND_MAX;}

	template <class Ts>
	static FORCEINLINE void swap(Ts& a,Ts& b)
	{
		Ts temp = a;
		a = b;
		b = temp;
	}

	template< class Ts > 
	static FORCEINLINE Ts abs( const Ts a )
	{
		//实现下列运算
		return (a>=(Ts)0) ? a : -a;
	}


	//正：1/负：-1/零:0
	template< class Ts > 
	static FORCEINLINE Ts sign( const Ts a)
	{
		return (a > (Ts)0) ? (Ts)1 : ((a < (Ts)0) ? (Ts)-1 : (Ts)0);
	}

  

#ifdef NEWMAXMIN
	template< class Ts > 
	static FORCEINLINE Ts get_max( const Ts a, const Ts b)
	{
		return (a>=b) ? a : b;
	}

	template< class Ts > 
	static FORCEINLINE Ts get_min( const Ts a, const Ts b )
	{
		return (a<=b) ? a : b;
	}
#else
	template< class Ts > 
	static FORCEINLINE Ts max( const Ts a, const Ts b)
	{
		return (a>=b) ? a : b;
	}

	template< class Ts > 
	static FORCEINLINE Ts min( const Ts a, const Ts b )
	{
		return (a<=b) ? a : b;
	}
#endif // NEWMAXMIN
	/*
	//浮点绝对值
	static FORCEINLINE f32 abs(f32 a)
	{
		return ::fabsf(a);
	}
	*/

};

struct RBMath : public RBBaseMath
{
	static FORCEINLINE bool is_nearly_equal(f32 a,f32 b,f32 delta = SMALL_F)
	{
		return RBBaseMath::abs(a - b)<delta;
	}
	static FORCEINLINE bool is_nearly_equal(double a,double b,double delta = (double)SMALL_F)
	{
		return RBBaseMath::abs(a - b)<delta;
	}
	static FORCEINLINE bool is_nearly_zero(f32 v,f32 delta = SMALL_F)
	{
		return RBBaseMath::abs(v)<delta;
	}
	static FORCEINLINE bool is_nearly_zero(double v,double delta = (double)SMALL_F)
	{
		return RBBaseMath::abs(v)<delta;
	}


	/*求0到a随机数，包括0，除去a*/
	static FORCEINLINE i32 get_rand_i(i32 a)
	{
		return a>0 ? (int)((f32)(rand_i()*a)*INV_RAND_MAX_ADD1) : 0;
	}

	//求指定范围内的随机数，取到最大最小值
	static FORCEINLINE i32 get_rand_range_i(i32 minn, i32 maxn)
	{
		i32 _range = (maxn - minn) + 1;
		return minn + get_rand_i(_range);
	}

	static f32 get_rand_range_f(f32 minn, f32 maxn)
	{
		return minn + (maxn - minn) * rand_f();
	}

	template< class T > 
	static T max3( const T a, const T b, const T c)
	{
#ifdef NEWMAXMIN
		return get_max(get_max(a,b),c);
#else
		return max ( max( a, b ), c );
#endif
	}

	template< class T > 
	static T min3( const T a, const T b, const T c)
	{
#ifdef NEWMAXMIN
		return get_min(get_min(a,b),c);
#else
		return min ( min( a, b ), c );
#endif
	}

	//一般性平方
	template< class T > 
	static T square( const T a )
	{
		return a*a;
	}


	///** Return a uniformly distributed random unit length vector = point on the unit sphere surface. */
	//static RBVector3 VRand();
	//
	///*
	// * Returns a random unit vector, uniformly distributed, within the specified cone
	// * ConeHalfAngleRad is the half-angle of cone, in radians.  Returns a normalized vector. 
	// */
	//static RB_API RBVector3 VRandCone(RBVector3 const& Dir, f32 ConeHalfAngleRad);

	///** 
	// * This is a version of VRandCone that handles "squished" cones, i.e. with different angle limits in the Y and Z axes.
	// * Assumes world Y and Z, although this could be extended to handle arbitrary rotations.
	// */
	//static RB_API RBVector3 VRandCone(RBVector3 const& Dir, f32 HorizontalConeHalfAngleRad, f32 VerticalConeHalfAngleRad);


	//// Predicates

	///** Checks if value is within a range, exclusive on MaxValue) */
	//template< class U > 
	//static FORCEINLINE bool IsWithin(const U& TestValue, const U& MinValue, const U& MaxValue)
	//{
	//	return ((TestValue>=MinValue) && (TestValue < MaxValue));
	//}

	///** Checks if value is within a range, inclusive on MaxValue) */
	//template< class U > 
	//static FORCEINLINE bool IsWithinInclusive(const U& TestValue, const U& MinValue, const U& MaxValue)
	//{
	//	return ((TestValue>=MinValue) && (TestValue <= MaxValue));
	//}
	//
	///**
	// *	Checks if two f32ing point numbers are nearly equal.
	// *	@param A				First number to compare
	// *	@param B				Second number to compare
	// *	@param ErrorTolerance	Maximum allowed difference for considering them as 'nearly equal'
	// *	@return					true if A and B are nearly equal
	// */
	//static FORCEINLINE bool IsNearlyEqual(f32 A, f32 B, f32 ErrorTolerance = SMALLER_F)
	//{
	//	return Abs<f32>( A - B ) < ErrorTolerance;
	//}

	///**
	// *	Checks if two f32ing point numbers are nearly equal.
	// *	@param A				First number to compare
	// *	@param B				Second number to compare
	// *	@param ErrorTolerance	Maximum allowed difference for considering them as 'nearly equal'
	// *	@return					true if A and B are nearly equal
	// */
	//static FORCEINLINE bool IsNearlyEqual(double A, double B, double ErrorTolerance = SMALLER_F)
	//{
	//	return Abs<double>( A - B ) < ErrorTolerance;
	//}

	///**
	// *	Checks if a f32ing point number is nearly zero.
	// *	@param Value			Number to compare
	// *	@param ErrorTolerance	Maximum allowed difference for considering Value as 'nearly zero'
	// *	@return					true if Value is nearly zero
	// */
	//static FORCEINLINE bool IsNearlyZero(f32 Value, f32 ErrorTolerance = SMALLER_F)
	//{
	//	return Abs<f32>( Value ) < ErrorTolerance;
	//}

	///**
	// *	Checks if a f32ing point number is nearly zero.
	// *	@param Value			Number to compare
	// *	@param ErrorTolerance	Maximum allowed difference for considering Value as 'nearly zero'
	// *	@return					true if Value is nearly zero
	// */
	//static FORCEINLINE bool IsNearlyZero(double Value, double ErrorTolerance = SMALLER_F)
	//{
	//	return Abs<double>( Value ) < ErrorTolerance;
	//}

	///**
	// *	Checks whether a number is a power of two.
	// *	@param Value	Number to check
	// *	@return			true if Value is a power of two
	// */
	//static FORCEINLINE bool IsPowerOfTwo( uint32 Value )
	//{
	//	return ((Value & (Value - 1)) == 0);
	//}


	//// Math Operations

	


	///** Clamps X to be between Min and Max, inclusive */
	template< class T > 
	static FORCEINLINE T clamp( const T X, const T Min, const T Max )
	{
		return X<Min ? Min : X<Max ? X : Max;
	}

	///** Snaps a value to the nearest grid multiple */
	//static FORCEINLINE f32 GridSnap( f32 Location, f32 Grid )
	//{
	//	if( Grid==0.f )	return Location;
	//	else			
	//	{
	//		return Floor((Location + 0.5*Grid)/Grid)*Grid;
	//	}
	//}

	///** Snaps a value to the nearest grid multiple */
	//static FORCEINLINE double GridSnap( double Location, double Grid )
	//{
	//	if( Grid==0.0 )	return Location;
	//	else			
	//	{
	//		return FloorDouble((Location + 0.5*Grid)/Grid)*Grid;
	//	}
	//}

	///** Divides two integers and rounds up */
	//template <class T>
	//static FORCEINLINE T DivideAndRoundUp(T Dividend,T Divisor)
	//{
	//	return (Dividend + Divisor - 1) / Divisor;
	//}

	///**
	// * Computes the base 2 logarithm of the specified value
	// *
	// * @param Value the value to perform the log on
	// *
	// * @return the base 2 log of the value
	// */
	//static FORCEINLINE f32 Log2(f32 Value)
	//{
	//	// Cached value for fast conversions
	//	static const f32 LogToLog2 = 1.f / Loge(2.f);
	//	// Do the platform specific log and convert using the cached value
	//	return Loge(Value) * LogToLog2;
	//}


	//// Conversion Functions

	///** 
	// * Converts radians to degrees.
	// * @param	RadVal			Value in radians.
	// * @return					Value in degrees.
	// */
	//template<class T>
	//static FORCEINLINE T RadiansToDegrees(T const& RadVal)
	//{
	//	return RadVal * (180.f / PI);
	//}

	///** 
	// * Converts degrees to radians.
	// * @param	DegVal			Value in degrees.
	// * @return					Value in radians.
	// */
	//template<class T>
	//static FORCEINLINE T DegreesToRadians(T const& DegVal)
	//{
	//	return DegVal * (PI / 180.f);
	//}

	///** 
	// * Clamps an arbitrary angle to be between the given angles.  Will clamp to nearest boundary.
	// * 
	// * @param MinAngleDegrees	"from" angle that defines the beginning of the range of valid angles (sweeping clockwise)
	// * @param MaxAngleDegrees	"to" angle that defines the end of the range of valid angles
	// * @return Returns clamped angle in the range -180..180.
	// */
	//static f32 RB_API ClampAngle(f32 AngleDegrees, f32 MinAngleDegrees, f32 MaxAngleDegrees);

	///** Find the smallest angle between two headings (in radians) */
	//static f32 FindDeltaAngle(f32 A1, f32 A2)
	//{
	//	// Find the difference
	//	f32 Delta = A2 - A1;

	//	// If change is larger than PI
	//	if(Delta > PI)
	//	{
	//		// Flip to negative equivalent
	//		Delta = Delta - (PI * 2.0f);
	//	}
	//	else if(Delta < -PI)
	//	{
	//		// Otherwise, if change is smaller than -PI
	//		// Flip to positive equivalent
	//		Delta = Delta + (PI * 2.0f);
	//	}

	//	// Return delta in [-PI,PI] range
	//	return Delta;
	//}

	///** Given a heading which may be outside the +/- PI range, 'unwind' it back into that range. */
	static f32 limit_radians(f32 a)
	{
		while(a > PI)
		{
			a -= ((f32)PI * 2.0f);
		}

		while(a < -PI)
		{
			a += ((f32)PI * 2.0f);
		}

		return a;
	}

	///** Utility to ensure angle is between +/- 180 degrees by unwinding. */
	static f32 limit_degrees(f32 a)
	{
		while(a > 180.f)
		{
			a -= 360.f;
		}

		while(a < -180.f)
		{
			a += 360.f;
		}

		return a;
	}

	///** Returns a new rotation component value
	// *
	// * @param InCurrent is the current rotation value
	// * @param InDesired is the desired rotation value
	// * @param InDeltaRate is the rotation amount to apply
	// *
	// * @return a new rotation component value
	// */
	//static RB_API f32 FixedTurn(f32 InCurrent, f32 InDesired, f32 InDeltaRate);

	///** Converts given Cartesian coordinate pair to Polar coordinate system. */
	//static FORCEINLINE void CartesianToPolar(f32 X, f32 Y, f32& OutRad, f32& OutAng)
	//{
	//	OutRad = Sqrt(Square(X) + Square(Y));
	//	OutAng = Atan2(Y, X);
	//}
	///** Converts given Polar coordinate pair to Cartesian coordinate system. */
	//static FORCEINLINE void PolarToCartesian(f32 Rad, f32 Ang, f32& OutX, f32& OutY)
	//{
	//	OutX = Rad * Cos(Ang);
	//	OutY = Rad * Sin(Ang);
	//}

	///**
	// * Calculates the dotted distance of vector 'Direction' to coordinate system O(AxisX,AxisY,AxisZ).
	// *
	// * Orientation: (consider 'O' the first person view of the player, and 'Direction' a vector pointing to an enemy)
	// * - positive azimuth means enemy is on the right of crosshair. (negative means left).
	// * - positive elevation means enemy is on top of crosshair, negative means below.
	// *
	// * @Note: 'Azimuth' (.X) sign is changed to represent left/right and not front/behind. front/behind is the funtion's return value.
	// *
	// * @param	OutDotDist	.X = 'Direction' dot AxisX relative to plane (AxisX,AxisZ). (== Cos(Azimuth))
	// *						.Y = 'Direction' dot AxisX relative to plane (AxisX,AxisY). (== Sin(Elevation))
	// * @param	Direction	direction of target.
	// * @param	AxisX		X component of reference system.
	// * @param	AxisY		Y component of reference system.
	// * @param	AxisZ		Z component of reference system.
	// *
	// * @return	true if 'Direction' is facing AxisX (Direction dot AxisX >= 0.f)
	// */
	//static RB_API bool GetDotDistance(RBVector32D &OutDotDist, const RBVector3 &Direction, const RBVector3 &AxisX, const RBVector3 &AxisY, const RBVector3 &AxisZ);

	///**
	// * Returns Azimuth and Elevation of vector 'Direction' in coordinate system O(AxisX,AxisY,AxisZ).
	// *
	// * Orientation: (consider 'O' the first person view of the player, and 'Direction' a vector pointing to an enemy)
	// * - positive azimuth means enemy is on the right of crosshair. (negative means left).
	// * - positive elevation means enemy is on top of crosshair, negative means below.
	// *
	// * @param	Direction		Direction of target.
	// * @param	AxisX			X component of reference system.
	// * @param	AxisY			Y component of reference system.
	// * @param	AxisZ			Z component of reference system.
	// *
	// * @return	RBVector32D	X = Azimuth angle (in radians) (-PI, +PI)
	// *						Y = Elevation angle (in radians) (-PI/2, +PI/2)
	// */
	//static RB_API RBVector32D GetAzimuthAndElevation(const RBVector3 &Direction, const RBVector3 &AxisX, const RBVector3 &AxisY, const RBVector3 &AxisZ);

	//// Interpolation Functions

	///** Calculates the percentage along a line from MinValue to MaxValue that Value is. */
	//static FORCEINLINE f32 GetRangePct(f32 MinValue, f32 MaxValue, f32 Value)
	//{
	//	return (Value - MinValue) / (MaxValue - MinValue);
	//}

	///** Same as above, but taking a 2d vector as the range. */
	//static f32 GetRangePct(RBVector32D const& Range, f32 Value);
	//
	///** Basically a Vector2d version of Lerp. */
	//static f32 GetRangeValue(RBVector32D const& Range, f32 Pct);

	///**
	// * For the given value in the input range, returns the corresponding value in the output range.
	// * Useful for mapping one value range to another value range.  Output is clamped to the OutputRange.
	// * e.g. given that velocities [50..100] correspond to a sound volume of [0.2..1.4], this makes it easy to 
	// *      find the volume for a velocity of 77.
	// */
	//static FORCEINLINE f32 GetMappedRangeValue(RBVector32D const& InputRange, RBVector32D const& OutputRange, f32 Value)
	//{
	//	f32 const ClampedPct = Clamp<f32>(GetRangePct(InputRange, Value), 0.f, 1.f);
	//	return GetRangeValue(OutputRange, ClampedPct);
	//}

	///** Performs a linear interpolation between two values, Alpha ranges from 0-1 */
	//template< class T, class U > 
	//static FORCEINLINE_DEBUGGABLE T Lerp( const T& A, const T& B, const U& Alpha )
	//{
	//	return (T)(A + Alpha * (B-A));
	//}

	///** Performs a linear interpolation between two values, Alpha ranges from 0-1. Handles full numeric range of T */
	//template< class T > 
	//static FORCEINLINE_DEBUGGABLE T LerpStable( const T& A, const T& B, double Alpha )
	//{
	//	return (T)((A * (1.0 - Alpha)) + (B * Alpha));
	//}

	///** Performs a 2D linear interpolation between four values values, FracX, FracY ranges from 0-1 */
	//template< class T, class U > 
	//static FORCEINLINE_DEBUGGABLE T BiLerp(const T& P00,const T& P10,const T& P01,const T& P11, const U& FracX, const U& FracY)
	//{
	//	return Lerp(
	//		Lerp(P00,P10,FracX),
	//		Lerp(P01,P11,FracX),
	//		FracY
	//		);
	//}

	///**
	// * Performs a cubic interpolation
	// *
	// * @param  P - end points
	// * @param  T - tangent directions at end points
	// * @param  Alpha - distance along spline
	// *
	// * @return  Interpolated value
	// */
	//template< class T, class U > 
	//static FORCEINLINE_DEBUGGABLE T CubicInterp( const T& P0, const T& T0, const T& P1, const T& T1, const U& A )
	//{
	//	const f32 A2 = A  * A;
	//	const f32 A3 = A2 * A;

	//	return (T)(((2*A3)-(3*A2)+1) * P0) + ((A3-(2*A2)+A) * T0) + ((A3-A2) * T1) + (((-2*A3)+(3*A2)) * P1);
	//}

	///**
	// * Performs a first derivative cubic interpolation
	// *
	// * @param  P - end points
	// * @param  T - tangent directions at end points
	// * @param  Alpha - distance along spline
	// *
	// * @return  Interpolated value
	// */
	//template< class T, class U > 
	//static FORCEINLINE_DEBUGGABLE T CubicInterpDerivative( const T& P0, const T& T0, const T& P1, const T& T1, const U& A )
	//{
	//	T a = 6.f*P0 + 3.f*T0 + 3.f*T1 - 6.f*P1;
	//	T b = -6.f*P0 - 4.f*T0 - 2.f*T1 + 6.f*P1;
	//	T c = T0;

	//	const f32 A2 = A  * A;

	//	return (a * A2) + (b * A) + c;
	//}

	///**
	// * Performs a second derivative cubic interpolation
	// *
	// * @param  P - end points
	// * @param  T - tangent directions at end points
	// * @param  Alpha - distance along spline
	// *
	// * @return  Interpolated value
	// */
	//template< class T, class U > 
	//static FORCEINLINE_DEBUGGABLE T CubicInterpSecondDerivative( const T& P0, const T& T0, const T& P1, const T& T1, const U& A )
	//{
	//	T a = 12.f*P0 + 6.f*T0 + 6.f*T1 - 12.f*P1;
	//	T b = -6.f*P0 - 4.f*T0 - 2.f*T1 + 6.f*P1;

	//	return (a * A) + b;
	//}

	///** Interpolate between A and B, applying an ease in/out function.  Exp controls the degree of the curve. */
	//template< class T > 
	//static FORCEINLINE_DEBUGGABLE T InterpEaseInOut( const T& A, const T& B, f32 Alpha, f32 Exp )
	//{
	//	f32 const ModifiedAlpha = ( Alpha < 0.5f ) ?
	//		0.5f * Pow(2.f * Alpha, Exp) :
	//	1.f - 0.5f * Pow(2.f * (1.f - Alpha), Exp);

	//	return Lerp<T>(A, B, ModifiedAlpha);
	//}

	//// Rotator specific interpolation
	//template< class U > static FRotator Lerp( const FRotator& A, const FRotator& B, const U& Alpha);

	//// Quat-specific interpolation

	//template< class U > static FQuat Lerp( const FQuat& A, const FQuat& B, const U& Alpha);
	//template< class U > static FQuat BiLerp(const FQuat& P00, const FQuat& P10, const FQuat& P01, const FQuat& P11, f32 FracX, f32 FracY);

	///**
	// * In the case of quaternions, we use a bezier like approach.
	// * T - Actual 'control' orientations.
	// */
	//template< class U > static FQuat CubicInterp( const FQuat& P0, const FQuat& T0, const FQuat& P1, const FQuat& T1, const U& A);

	//// Special-case interpolation

	///** Interpolate a normal vector Current to Target, by interpolating the angle between those vectors with constant step. */
	//static RB_API RBVector3 VInterpNormalRotationTo(const RBVector3& Current, const RBVector3& Target, f32 DeltaTime, f32 RotationSpeedDegrees);

	///** Interpolate vector from Current to Target with constant step */
	//static RB_API RBVector3 VInterpConstantTo(const RBVector3 Current, const RBVector3& Target, f32 DeltaTime, f32 InterpSpeed);

	///** Interpolate vector from Current to Target. Scaled by distance to Target, so it has a strong start speed and ease out. */
	//static RB_API RBVector3 VInterpTo( const RBVector3& Current, const RBVector3& Target, f32 DeltaTime, f32 InterpSpeed );
	//
	///** Interpolate vector2D from Current to Target with constant step */
	//static RB_API RBVector32D Vector2DInterpConstantTo( const RBVector32D& Current, const RBVector32D& Target, f32 DeltaTime, f32 InterpSpeed );

	///** Interpolate vector2D from Current to Target. Scaled by distance to Target, so it has a strong start speed and ease out. */
	//static RB_API RBVector32D Vector2DInterpTo( const RBVector32D& Current, const RBVector32D& Target, f32 DeltaTime, f32 InterpSpeed );

	///** Interpolate rotator from Current to Target with constant step */
	//static RB_API FRotator RInterpConstantTo( const FRotator& Current, const FRotator& Target, f32& DeltaTime, f32 InterpSpeed);

	///** Interpolate rotator from Current to Target. Scaled by distance to Target, so it has a strong start speed and ease out. */
	//static RB_API FRotator RInterpTo( const FRotator& Current, const FRotator& Target, const f32& DeltaTime, f32 InterpSpeed);

	///** Interpolate f32 from Current to Target with constant step */
	//static RB_API f32 FInterpConstantTo( f32 Current, f32 Target, f32 DeltaTime, f32 InterpSpeed );

	///** Interpolate f32 from Current to Target. Scaled by distance to Target, so it has a strong start speed and ease out. */
	//static RB_API f32 FInterpTo( f32 Current, f32 Target, f32 DeltaTime, f32 InterpSpeed );

	///**
	// * Simple function to create a pulsating scalar value
	// *
	// * @param  InCurrentTime  Current absolute time
	// * @param  InPulsesPerSecond  How many full pulses per second?
	// * @param  InPhase  Optional phase amount, between 0.0 and 1.0 (to synchronize pulses)
	// *
	// * @return  Pulsating value (0.0-1.0)
	// */
	//static f32 MakePulsatingValue( const double InCurrentTime, const f32 InPulsesPerSecond, const f32 InPhase = 0.0f )
	//{
	//	return 0.5f + 0.5f * (f32)sin( ( ( 0.25f + InPhase ) * PI * 2.0 ) + ( InCurrentTime * PI * 2.0 ) * InPulsesPerSecond );
	//}

	//// Geometry intersection 


	///**
	// * Find the intersection of an infinite line (defined by two points) and
	// * a plane.  Assumes that the line and plane do indeed intersect; you must
	// * make sure they're not parallel before calling.
	// */
	//static RBVector3 LinePlaneIntersection( const RBVector3 &Point1, const RBVector3 &Point2, const RBVector3 &PlaneOrigin, const RBVector3 &PlaneNormal);
	//static RBVector3 LinePlaneIntersection( const RBVector3 &Point1, const RBVector3 &Point2, const FPlane  &Plane);

	///**
	// * Determine if a plane and an AABB intersect
	// * @param P - the plane to test
	// * @param AABB - the axis aligned bounding box to test
	// * @return if collision occurs
	// */
	//static RB_API bool PlaneAABBIntersection(const FPlane& P, const FBox& AABB);

	///**
	// * Performs a sphere vs box intersection test using Arvo's algorithm:
	// *
	// *	for each i in (x, y, z)
	// *		if (SphereCenter(i) < BoxMin(i)) d2 += (SphereCenter(i) - BoxMin(i)) ^ 2
	// *		else if (SphereCenter(i) > BoxMax(i)) d2 += (SphereCenter(i) - BoxMax(i)) ^ 2
	// *
	// * @param Sphere the center of the sphere being tested against the AABB
	// * @param RadiusSquared the size of the sphere being tested
	// * @param AABB the box being tested against
	// *
	// * @return Whether the sphere/box intersect or not.
	// */
	//static bool SphereAABBIntersection(const RBVector3& SphereCenter,const f32 RadiusSquared,const FBox& AABB);

	///**
	// * Converts a sphere into a point plus radius squared for the test above
	// */
	//static bool SphereAABBIntersection(const FSphere& Sphere,const FBox& AABB);

	///** Determines whether a point is inside a box. */
	//static bool PointBoxIntersection( const RBVector3& Point, const FBox& Box );

	///** Determines whether a line intersects a box. */
	//static bool LineBoxIntersection( const FBox& Box, const RBVector3& Start, const RBVector3& End, const RBVector3& Direction, const RBVector3& OneOverDirection );

	///* Swept-Box vs Box test */
	//static RB_API bool LineExtentBoxIntersection(const FBox& inBox, const RBVector3& Start, const RBVector3& End, const RBVector3& Extent, RBVector3& HitLocation, RBVector3& HitNormal, f32& HitTime);

	///** Determines whether a line intersects a sphere. */
	//static bool LineSphereIntersection(const RBVector3& Start,const RBVector3& Dir,f32 Length,const RBVector3& Origin,f32 Radius);

	///**
	// * Assumes the cone tip is at 0,0,0 (means the SphereCenter is relative to the cone tip)
	// * @return true: cone and sphere do intersect, false otherwise
	// */
	//static RB_API bool SphereConeIntersection(const RBVector3& SphereCenter, f32 SphereRadius, const RBVector3& ConeAxis, f32 ConeAngleSin, f32 ConeAngleCos);

	///** Find the point on line segment from LineStart to LineEnd which is closest to Point */
	//static RB_API RBVector3 ClosestPointOnLine(const RBVector3& LineStart, const RBVector3& LineEnd, const RBVector3& Point);

	///** Compute intersection point of three planes. Return 1 if valid, 0 if infinite. */
	//static bool IntersectPlanes3( RBVector3& I, const FPlane& P1, const FPlane& P2, const FPlane& P3 );

	///**
	// * Compute intersection point and direction of line joining two planes.
	// * Return 1 if valid, 0 if infinite.
	// */
	//static bool IntersectPlanes2( RBVector3& I, RBVector3& D, const FPlane& P1, const FPlane& P2 );

	///**
	// * Calculates the distance of a given Point in world space to a given line,
	// * defined by the vector couple (Origin, Direction).
	// *
	// * @param	Point				point to check distance to Axis
	// * @param	Direction			unit vector indicating the direction to check against
	// * @param	Origin				point of reference used to calculate distance
	// * @param	OutClosestPoint	optional point that represents the closest point projected onto Axis
	// *
	// * @return	distance of Point from line defined by (Origin, Direction)
	// */
	//static RB_API f32 PointDistToLine(const RBVector3 &Point, const RBVector3 &Line, const RBVector3 &Origin, RBVector3 &OutClosestPoint);
	//static RB_API f32 PointDistToLine(const RBVector3 &Point, const RBVector3 &Line, const RBVector3 &Origin);

	///**
	// * Returns closest point on a segment to a given point.
	// * The idea is to project point on line formed by segment.
	// * Then we see if the closest point on the line is outside of segment or inside.
	// *
	// * @param	Point			point for which we find the closest point on the segment
	// * @param	StartPoint		StartPoint of segment
	// * @param	EndPoint		EndPoint of segment
	// *
	// * @return	point on the segment defined by (StartPoint, EndPoint) that is closest to Point.
	// */
	//static RB_API RBVector3 ClosestPointOnSegment(const RBVector3 &Point, const RBVector3 &StartPoint, const RBVector3 &EndPoint);

	///**
	// * Returns distance from a point to the closest point on a segment.
	// *
	// * @param	Point			point to check distance for
	// * @param	StartPoint		StartPoint of segment
	// * @param	EndPoint		EndPoint of segment
	// *
	// * @return	closest distance from Point to segment defined by (StartPoint, EndPoint).
	// */
	//static RB_API f32 PointDistToSegment(const RBVector3 &Point, const RBVector3 &StartPoint, const RBVector3 &EndPoint);

	///**
	// * Returns square of the distance from a point to the closest point on a segment.
	// *
	// * @param	Point			point to check distance for
	// * @param	StartPoint		StartPoint of segment
	// * @param	EndPoint		EndPoint of segment
	// *
	// * @return	square of the closest distance from Point to segment defined by (StartPoint, EndPoint).
	// */
	//static RB_API f32 PointDistToSegmentSquared(const RBVector3 &Point, const RBVector3 &StartPoint, const RBVector3 &EndPoint);

	///** 
	// * Find closest points between 2 segments.
	// * @param	(A1, B1)	defines the first segment.
	// * @param	(A2, B2)	defines the second segment.
	// * @param	OutP1		Closest point on segment 1 to segment 2.
	// * @param	OutP2		Closest point on segment 2 to segment 1.
	// */
	//static RB_API void SegmentDistToSegment(RBVector3 A1, RBVector3 B1, RBVector3 A2, RBVector3 B2, RBVector3& OutP1, RBVector3& OutP2);

	///** 
	// * Find closest points between 2 segments.
	// * @param	(A1, B1)	defines the first segment.
	// * @param	(A2, B2)	defines the second segment.
	// * @param	OutP1		Closest point on segment 1 to segment 2.
	// * @param	OutP2		Closest point on segment 2 to segment 1.
	// */
	//static RB_API void SegmentDistToSegmentSafe(RBVector3 A1, RBVector3 B1, RBVector3 A2, RBVector3 B2, RBVector3& OutP1, RBVector3& OutP2);

	///**
	// * returns the time (t) of the intersection of the passed segment and a plane (could be <0 or >1)
	// * @param StartPoint - start point of segment
	// * @param EndPoint   - end point of segment
	// * @param Plane		- plane to intersect with
	// * @return time(T) of intersection
	// */
	//static RB_API f32 GetTForSegmentPlaneIntersect(const RBVector3& StartPoint, const RBVector3& EndPoint, const FPlane& Plane);

	///**
	// * Returns true if there is an intersection between the segment specified by StartPoint and Endpoint, and
	// * the plane on which polygon Plane lies. If there is an intersection, the point is placed in IntersectionPoint
	// * @param StartPoint - start point of segment
	// * @param EndPoint   - end point of segment
	// * @param Plane		- plane to intersect with
	// * @param out_InterSectPoint - out var for the point on the segment that intersects the mesh (if any)
	// * @return true if intersection occured
	// */
	//static RB_API bool SegmentPlaneIntersection(const RBVector3& StartPoint, const RBVector3& EndPoint, const FPlane& Plane, RBVector3& out_IntersectPoint);

	///**
	// * Returns closest point on a triangle to a point.
	// * The idea is to identify the halfplanes that the point is
	// * in relative to each triangle segment "plane"
	// *
	// * @param	Point			point to check distance for
	// * @param	A,B,C			counter clockwise ordering of points defining a triangle
	// *
	// * @return	Point on triangle ABC closest to given point
	// */
	//static RB_API RBVector3 ClosestPointOnTriangleToPoint(const RBVector3& Point, const RBVector3& A, const RBVector3& B, const RBVector3& C);

	///**
	// * Returns closest point on a tetrahedron to a point.
	// * The idea is to identify the halfplanes that the point is
	// * in relative to each face of the tetrahedron
	// *
	// * @param	Point			point to check distance for
	// * @param	A,B,C,D			four points defining a tetrahedron
	// *
	// * @return	Point on tetrahedron ABCD closest to given point
	// */
	//static RB_API RBVector3 ClosestPointOnTetrahedronToPoint(const RBVector3& Point, const RBVector3& A, const RBVector3& B, const RBVector3& C, const RBVector3& D);

	///** 
	// * Find closest point on a Sphere to a Line.
	// * When line intersects		Sphere, then closest point to LineOrigin is returned.
	// * @param SphereOrigin		Origin of Sphere
	// * @param SphereRadius		Radius of Sphere
	// * @param LineOrigin		Origin of line
	// * @param LineDir			Direction of line. Needs to be normalized!!
	// * @param OutClosestPoint	Closest point on sphere to given line.
	// */
	//static RB_API void SphereDistToLine(RBVector3 SphereOrigin, f32 SphereRadius, RBVector3 LineOrigin, RBVector3 LineDir, RBVector3& OutClosestPoint);

	///**
	// * Calculates whether a Point is within a cone segment, and also what percentage within the cone (100% is along the center line, whereas 0% is along the edge)
	// *
	// * @param Point - The Point in question
	// * @param ConeStartPoint - the beginning of the cone (with the smallest radius)
	// * @param ConeLine - the line out from the start point that ends at the largest radius point of the cone
	// * @param radiusAtStart - the radius at the ConeStartPoint (0 for a 'proper' cone)
	// * @param radiusAtEnd - the largest radius of the cone
	// * @param percentageOut - output variable the holds how much within the cone the point is (1 = on center line, 0 = on exact edge or outside cone).
	// *
	// * @return true if the point is within the cone, false otherwise.
	// */
	//static RB_API bool GetDistanceWithinConeSegment(RBVector3 Point, RBVector3 ConeStartPoint, RBVector3 ConeLine, f32 RadiusAtStart, f32 RadiusAtEnd, f32 &PercentageOut);


	//// Formatting functions

	///**
	// * Formats an integer value into a human readable string (i.e. 12345 becomes "12,345")
	// *
	// * @param	Val		The value to use
	// * @return	FString	The human readable string
	// */
	//static FString FormatIntToHumanReadable(int32 Val);


	//// Utilities

	///**
	// * Tests a memory region to see that it's working properly.
	// *
	// * @param BaseAddress	Starting address
	// * @param NumBytes		Number of bytes to test (will be rounded down to a multiple of 4)
	// * @return				true if the memory region passed the test
	// */
	//static RB_API bool MemoryTest( void* BaseAddress, uint32 NumBytes );

	///**
	// * Evaluates a numerical equation.
	// *
	// * Operators and precedence: 1:+- 2:/% 3:* 4:^ 5:&|
	// * Unary: -
	// * Types: Numbers (0-9.), Hex ($0-$f)
	// * Grouping: ( )
	// *
	// * @param	Str			String containing the equation.
	// * @param	OutValue		Pointer to storage for the result.
	// * @return				1 if successful, 0 if equation fails.
	// */
	//static RB_API bool Eval( FString Str, f32& OutValue );

	///*
	// * Computes the barycentric coordinates for a given point in a triangle - simpler version
	// *
	// * @param	Point			point to convert to barycentric coordinates (in plane of ABC)
	// * @param	A,B,C			three non-colinear points defining a triangle in CCW
	// * 
	// * @return Vector containing the three weights a,b,c such that Point = a*A + b*B + c*C
	// *							                                or Point = A + b*(B-A) + c*(C-A) = (1-b-c)*A + b*B + c*C
	// */
	//static RB_API RBVector3 GetBaryCentric2D(const RBVector3& Point, const RBVector3& A, const RBVector3& B, const RBVector3& C);

	///*
	// * Computes the barycentric coordinates for a given point in a triangle
	// *
	// * @param	Point			point to convert to barycentric coordinates (in plane of ABC)
	// * @param	A,B,C			three non-collinear points defining a triangle in CCW
	// * 
	// * @return Vector containing the three weights a,b,c such that Point = a*A + b*B + c*C
	// *							                               or Point = A + b*(B-A) + c*(C-A) = (1-b-c)*A + b*B + c*C
	// */
	//static RB_API RBVector3 ComputeBaryCentric2D(const RBVector3& Point, const RBVector3& A, const RBVector3& B, const RBVector3& C);

	///*
	// * Computes the barycentric coordinates for a given point on a tetrahedron (3D)
	// *
	// * @param	Point			point to convert to barycentric coordinates
	// * @param	A,B,C,D			four points defining a tetrahedron
	// *
	// * @return Vector containing the four weights a,b,c,d such that Point = a*A + b*B + c*C + d*D
	// */
	//static RB_API RBVector4 ComputeBaryCentric3D(const RBVector3& Point, const RBVector3& A, const RBVector3& B, const RBVector3& C, const RBVector3& D);

	///** 32 bit values where BitFlag[x] == (1<<x) */
	//static RB_API const uint32 BitFlag[32];

	///** 
	// * Returns a smooth Hermite interpolation between 0 and 1 for the value X (where X ranges between A and B)
	// * Clamped to 0 for X <= A and 1 for X >= B.
	// *
	// * @param A Minimum value of X
	// * @param B Maximum value of X
	// * @param X Parameter
	// *
	// * @return Smoothed value between 0 and 1
	// */
	//static f32 SmoothStep(f32 A, f32 B, f32 X)
	//{
	//	if (X < A)
	//	{
	//		return 0.0f;
	//	}
	//	else if (X >= B)
	//	{
	//		return 1.0f;
	//	}
	//	const f32 InterpFraction = (X - A) / (B - A);
	//	return InterpFraction * InterpFraction * (3.0f - 2.0f * InterpFraction);
	//}
	//
	///**
	// * Get a bit in memory created from bitflags (uint32 Value:1), used for EngineShowFlags,
	// * TestBitFieldFunctions() tests the implementation
	// */
	//static INLINE bool ExtractBoolFromBitfield(uint8* Ptr, uint32 Index)
	//{
	//	uint8* BytePtr = Ptr + Index / 8;
	//	uint8 Mask = 1 << (Index & 0x7);

	//	return (*BytePtr & Mask) != 0;
	//}

	///**
	// * Set a bit in memory created from bitflags (uint32 Value:1), used for EngineShowFlags,
	// * TestBitFieldFunctions() tests the implementation
	// */
	//static INLINE void SetBoolInBitField(uint8* Ptr, uint32 Index, bool bSet)
	//{
	//	uint8* BytePtr = Ptr + Index / 8;
	//	uint8 Mask = 1 << (Index & 0x7);

	//	if(bSet)
	//	{
	//		*BytePtr |= Mask;
	//	}
	//	else
	//	{
	//		*BytePtr &= ~Mask;
	//	}
	//}

	///**
	// * Handy to apply scaling in the editor
	// * @param Dst in and out
	// */
	//static RB_API void ApplyScaleTof32(f32& Dst, const RBVector3& DeltaScale, f32 Magnitude = 1.0f);
};