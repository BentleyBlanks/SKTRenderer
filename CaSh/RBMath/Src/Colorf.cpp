#include "..\Inc\Colorf.h"
#include "..\Inc\RBMath.h"
#include "..\Inc\Color32.h"
#include <iostream>
#include "..\\Inc\\Vector4.h"

RBColorf RBColorf::operator/=(f32 c)
{
	
		if (RBMath::is_nearly_zero(c))
			return black;
		r /= c; g /= c; b /= c; this->a /= c;
		return *this;
	
}

RBColorf::RBColorf(f32 r, f32 g, f32 b, f32 a)
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}

RBColorf::RBColorf(f32 r, f32 g, f32 b)
{
	this->r = r;
	this->b = b;
	this->g = g;
	this->a = 1.0;
}

RBColorf::RBColorf(const RBColor32& color32)
{
	f32 f = 1.f / 255.f;
	r = color32.r*f;
	g = color32.g*f;
	b = color32.b*f;
	a = color32.a*f;
}

RBColorf::RBColorf()
{
	r = 1.f;
	b = 1.f;
	g = 1.f;
	a = 1.0;
}

RBColorf::RBColorf(const RBVector4& v)
{
	r = v.x;
	g = v.y;
	b = v.z;
	a = v.w;
}

bool RBColorf::equal(const RBColorf& other)
{
	return RBMath::is_nearly_equal(r, other.r) && RBMath::is_nearly_equal(g, other.g) && RBMath::is_nearly_equal(b, other.b) && RBMath::is_nearly_equal(a, other.a);
}

f32 RBColorf::get_grayscale()
{
	return (((0.299f * r) + (0.587f * g)) + (0.114f * b));
}

void RBColorf::to_linear()
{
	r = RBMath::pow(r,2.2f);
	g = RBMath::pow(g,2.2f);
	b = RBMath::pow(b,2.2f);
}

void RBColorf::out_cyan()
{
	std::cout << cyan.a << std::endl;
}

void RBColorf::out()
{
	std::cout<<"("<<r<<","<<g<<","<<b<<")"<<std::endl;
}

const RBColorf
RBColorf::red(1.f, 0.f, 0.f, 1.f),
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

