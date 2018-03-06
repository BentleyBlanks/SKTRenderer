#pragma once
#include "./Platform/RBBasedata.h"

class RBColorf;

class RBColor32
{
public:
	u8 r, g, b, a;
	RBColor32();
	RBColor32(int color);
	RBColor32(u8 r, u8 g, u8 b, u8 a);
	RBColor32(u8 r, u8 g, u8 b);
	RBColor32(const RBColorf& colorf);

  uint32& dw_color(void) { return *((uint32*)this); }
  const uint32& dw_color(void) const { return *((uint32*)this); }
};

template <> struct TIsPODType <RBColor32 > { enum { v = true }; };
