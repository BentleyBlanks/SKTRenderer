#pragma once
#include "Platform/RBBasedata.h"
#include "RBMath.h"

class RBComplex
{
public:
	f32 img,real;

	FORCEINLINE RBComplex():real(0.f),img(0.f){};
	FORCEINLINE RBComplex(f32 treal,f32 timg):real(treal),img(timg){};
	FORCEINLINE RBComplex(f32 exp):real(RBMath::cos(exp)),img(RBMath::sin(exp)){};

	FORCEINLINE RBComplex operator+( const RBComplex& vec2 ) const
	{
		return RBComplex(real+vec2.real,img+vec2.img);
	}

	FORCEINLINE RBComplex operator-( const RBComplex& vec2 ) const
	{
		return RBComplex(real-vec2.real,img-vec2.img);
	}

	FORCEINLINE RBComplex operator*( f32 a) const
	{
		return RBComplex(real + a,img + a);
	}

	FORCEINLINE RBComplex operator*(const RBComplex& complex) const 
	{
		f32 r = real*complex.real - img*complex.img;
		f32 i = real*complex.img + img*complex.real;
		return RBComplex(r,i);
	}


};

FORCEINLINE RBComplex operator*( f32 scale, const RBComplex& a)
{
	return a.operator*( scale);
}

template <> struct TIsPODType<RBComplex> { enum { v = true }; };

//e^2*PI*i
class RBUNITComplexRoot : public RBComplex
{

};

template <> struct TIsPODType<RBUNITComplexRoot> { enum { v = true }; };