#include "..\Inc\Color32.h"
#include "..\Inc\Colorf.h"

RBColor32::RBColor32(int color)
{
	r = color >> 24;
	g = (color & 0xff0000) >> 16;
	b = (color & 0xff00) >> 8;
	a = color & 0xff;
}

RBColor32::RBColor32(u8 r, u8 g, u8 b, u8 a)
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}

RBColor32::RBColor32(u8 r, u8 g, u8 b)
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = 255;
}

RBColor32::RBColor32(const RBColorf& colorf)
{
	r = static_cast<u8>(colorf.r * 255);
	g = static_cast<u8>(colorf.g * 255);
	b = static_cast<u8>(colorf.b * 255);
	a = static_cast<u8>(colorf.a * 255);
}

RBColor32::RBColor32()
{
	r = 0;
	g = 0;
	b = 0;
	a = 255;
}
