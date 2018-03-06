#pragma once

#include "./Platform/RBBasedata.h"
#include "RBMath.h"


class RBColor32;
class RBVector4;

//ClampÇëÊÖ¶¯
class RBColorf
{
public:
	//0.0f-1.0f
	f32 r, g, b, a;
	RBColorf(f32 val):r(val),g(val),b(val),a(val){}
	RBColorf();
	RBColorf(f32 r, f32 g, f32 b, f32 a);
	RBColorf(f32 r, f32 g, f32 b);
	RBColorf(const RBVector4& v);
	bool equal(const RBColorf& other);
	f32 get_grayscale();
	void to_linear();
	//[Kinds of operators(+,-,*,/)] 

	bool operator==(const RBColorf& o) const
	{
		return r == o.r&&g == o.g&&b == o.g&&a == o.a;
	}

	FORCEINLINE RBColorf& operator=(const RBColorf& b)
	{
		this->r = b.r;
		this->g = b.g;
		this->b = b.b;
		this->a = b.a;
		return *this;
	}

	FORCEINLINE RBColorf operator+(f32 a) const
	{
		return RBColorf(r+a,g+a,b+a,this->a+a);
	}

	FORCEINLINE RBColorf operator+(const RBColorf& o) const
	{
		return RBColorf(r + o.r, g + o.g, b + o.b, a + o.a);
	}

	FORCEINLINE RBColorf operator-(f32 a) const
	{
		return RBColorf(r - a, g - a, b - a, this->a - a);
	}

	FORCEINLINE RBColorf operator-(const RBColorf& o) const
	{
		return RBColorf(r - o.r, g - o.g, b - o.b, a - o.a);
	}

	FORCEINLINE RBColorf operator*(f32 a) const
	{
		return RBColorf(r * a, g * a, b * a, this->a * a);
	}

	FORCEINLINE RBColorf operator*(const RBColorf& o) const
	{
		return RBColorf(r * o.r, g * o.g, b * o.b, a * o.a);
	}

	FORCEINLINE RBColorf operator+=(f32 a)
	{
		r += a; g += a; b += a; this->a += a;
		return *this;
	}

	FORCEINLINE RBColorf operator+=(const RBColorf& o)
	{
		r += o.r; g += o.g; b += o.b; this->a += o.a;
		return *this;
	}

	FORCEINLINE RBColorf operator-=(f32 a)
	{
		r -= a; g -= a; b -= a; this->a -= a;
		return *this;
	}

	FORCEINLINE RBColorf operator-=(const RBColorf& o)
	{
		r -= o.r; g -= o.g; b -= o.b; this->a -= o.a;
		return *this;
	}

	FORCEINLINE RBColorf operator*=(f32 a)
	{
		r *= a; g *= a; b *= a; this->a *= a;
		return *this;
	}

	FORCEINLINE RBColorf operator*=(const RBColorf& o)
	{
		r *= o.r; g *= o.g; b *= o.b; this->a *= o.a;
		return *this;
	}

	RBColorf operator/=(f32 c);

	FORCEINLINE RBColorf operator/(const RBColorf& o)
	{
		r /= o.r; g /= o.g; b /= o.b; this->a /= o.a;
		return *this;
	}

	FORCEINLINE f32 y() const
	{
		const float YWeight[3] = { 0.212671f, 0.715160f, 0.072169f };
		return YWeight[0] * r + YWeight[1] * g + YWeight[2] * b;
	}

	FORCEINLINE f32 power_y(ft times) const
	{
		return y()*times;
	}

	bool is_black() const
	{
		return RBMath::is_nearly_zero(r) && RBMath::is_nearly_zero(g) && RBMath::is_nearly_zero(b);
	}



	RBColorf(const RBColor32& color32);

	
	static const RBColorf
		red,
		green,
		blue,
		white,
		black,
		yellow,
		cyan,
		magenta,
		gray,
		grey,
		blank;
		

	void out_cyan();

	void out();
};

FORCEINLINE RBColorf operator*(f32 scale, const RBColorf& v)
{
	return v.operator*(scale);
}

/*
RBColorf::green(0.f, 1.f, 0.f, 1.f),
RBColorf::blue(0.f, 0.f, 1.f, 1.f),
RBColorf::white(1.f, 1.f, 1.f, 1.f),
RBColorf::black(0.f, 0.f, 0.f, 1.f),
RBColorf::yellow(1.f, 0.9215686f, 0.01568628f, 1.f),
//Çà
RBColorf::cyan(0.f, 1.f, 1.f, 1.f),
//Ñóºì
RBColorf::magenta(1.f, 0.f, 1.f, 1.f),
//»Ò
RBColorf::grey(0.5f, 0.5f, 0.5f, 1.f),
//»Ò
RBColorf::gray(0.5f, 0.5f, 0.5f, 1.f),
//clear
RBColorf::blank(0.f, 0.f, 0.f, 0.f);
*/
template <> struct TIsPODType <RBColorf> { enum { v = true }; };
