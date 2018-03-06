#pragma once
#include "Vector2.h"

//参考标准DX坐标:x-w,z-l,y-h
struct RBAABB
{
  RBVector3 min;
  RBVector3 max;

  RBAABB()
  {
    this->min = RBVector3(MAX_F32,MAX_F32,MAX_F32);
    this->max = RBVector3(-(MAX_F32-1), -(MAX_F32-1), -(MAX_F32-1));
  }
  RBAABB(f32 min_x, f32 min_y, f32 min_z,f32 max_x, f32 max_y,f32 max_z)
  {
    this->set(min_x, min_y,min_z, max_x, max_y,max_z);
  }
  RBAABB(const RBVector3& min, const RBVector3& max)
  {
	  this->min = min;
	  this->max = max;
  }

  void reset()
  {
	  this->min = RBVector3(MAX_F32,MAX_F32,MAX_F32);
	  this->max = RBVector3(-(MAX_F32-1), -(MAX_F32-1), -(MAX_F32-1));
  }
  bool is_available() const
  {
	  return !(((this->min.x > this->max.x )||(this->min.y > this->max.y)||(this->min.z > this->max.z) ));
  }
  //zxy
  void set_size(f32 min_x, f32 min_y,f32 min_z, f32 l,f32 w, f32 h)
  {
    this->min.x = min_x;
    this->min.y = min_y;
    this->min.z = min_z;
    this->max.x = w + this->min.x;
    this->max.y = h + this->min.y;
    this->max.z = l + this->max.z;
  }

  void set(f32 min_x, f32 min_y, f32 min_z, f32 max_x, f32 max_y, f32 max_z)
  {
    this->min.x = min_x;
    this->min.y = min_y;
    this->min.z = min_z;
    this->max.x = max_x;
    this->max.y = max_y;
    this->max.z = max_z;
  }

  //012xyz,t 0~1
  void split(int axis,float t,RBAABB& a,RBAABB& b) const
  {
	  a = *this;
	  b = *this;
	  f32 tv;
	  auto d = max - min;
	  switch (axis)
	  {
	  case 0:
		  tv = t*d.x;
		  a.max.x -= d.x - tv;
		  b.min.x += tv;
		  break;
	  case 1:
		  tv = t*d.y;
		  a.max.y -= d.y - tv;
		  b.min.y += tv;		  
		  break;
	  case 2:
		  tv = t*d.z;
		  a.max.z -= d.z - tv;
		  b.min.z += tv;
		  break;
	  default:

		  break;
	  }
  }

  void bound_sphere(RBVector3& to,f32& tr) const
  {
	  to = (min + max )/2;
	  tr = RBVector3::dist(min,max)/2;
  }

  void transform(const RBMatrix& m)
  {

	  RBVector4 v[8];
	  v[0] = RBVector4(RBVector3(min.x,min.y,min.z),1.f);
	  v[0].apply_matrix(m);

	  v[1] = RBVector4(RBVector3(min.x,min.y,max.z),1.f);
	  v[1].apply_matrix(m);

	  v[2] = RBVector4(RBVector3(min.x,max.y,min.z),1.f);
	  v[2].apply_matrix(m);

	  v[3] = RBVector4(RBVector3(max.x,min.y,min.z),1.f);
	  v[3].apply_matrix(m);

	  v[4] = RBVector4(RBVector3(max.x,max.y,min.z),1.f);
	  v[4].apply_matrix(m);

	  v[5] = RBVector4(RBVector3(min.x,max.y,max.z),1.f);
	  v[5].apply_matrix(m);

	  v[6] = RBVector4(RBVector3(max.x,min.y,max.z),1.f);
	  v[6].apply_matrix(m);

	  v[7] = RBVector4(RBVector3(max.x,max.y,max.z),1.f);
	  v[7].apply_matrix(m);

	  reset();
	  for(int i=0;i<8;++i)
	  {
		  include(v[i]);
	  }

	  /*
	  RBVector4 v = this->min;
	  v.w = 1.f;
	  v.apply_matrix(m);
	  RBVector4 v1;
	  v1 = this->max;
	  v1.w = 1.f;
	  v1.apply_matrix(m);
	  reset();
	  include(v);
	  include(v1);
	  */
  }

  //set w to 1
  void transform_aff(const RBMatrix& m)
  {

	  RBVector4 v[8];
	  v[0] = RBVector4(RBVector3(min.x,min.y,min.z),1.f);
	  v[0].apply_matrix(m);
	  v[0]/=v[0].w;
	  v[1] = RBVector4(RBVector3(min.x,min.y,max.z),1.f);
	  v[1].apply_matrix(m);
	  v[1]/=v[1].w;
	  v[2] = RBVector4(RBVector3(min.x,max.y,min.z),1.f);
	  v[2].apply_matrix(m);
	  v[2]/=v[2].w;
	  v[3] = RBVector4(RBVector3(max.x,min.y,min.z),1.f);
	  v[3].apply_matrix(m);
	  v[3]/=v[3].w;
	  v[4] = RBVector4(RBVector3(max.x,max.y,min.z),1.f);
	  v[4].apply_matrix(m);
	  v[4]/=v[4].w;
	  v[5] = RBVector4(RBVector3(min.x,max.y,max.z),1.f);
	  v[5].apply_matrix(m);
	  v[5]/=v[5].w;
	  v[6] = RBVector4(RBVector3(max.x,min.y,max.z),1.f);
	  v[6].apply_matrix(m);
	  v[6]/=v[6].w;
	  v[7] = RBVector4(RBVector3(max.x,max.y,max.z),1.f);
	  v[7].apply_matrix(m);
	  v[7]/=v[7].w;
	  reset();
	  for(int i=0;i<8;++i)
	  {
		  include(v[i]);
	  }
	  /*
	  RBVector4 v = this->min;
	  v.w = 1.f;
	  v.apply_matrix(m);
	  v /= v.w;
	  RBVector4 v1 = this->max;
	  v1.w = 1.f;
	  v1.apply_matrix(m);
	  v1 /= v1.w;
	  reset();
	  include(v);
	  include(v1);
	  */
  }
  void include(const RBVector3& v)
  {
	  min.z = min.z > v.z ? v.z : min.z;
	  min.y = min.y > v.y ? v.y : min.y;
	  min.x = min.x > v.x ? v.x : min.x;
	  max.z = max.z < v.z ? v.z : max.z;
	  max.y = max.y < v.y ? v.y : max.y;
	  max.x = max.x < v.x ? v.x : max.x;
  }
  bool intersection(const RBAABB& o)
  {
	  return (o.max.x > min.x) && (o.max.y > min.y) && (o.min.x < max.x) && (o.min.y < max.y) && (o.min.z <max.z) && (o.max.z>min.z);
  }
  f32 get_xy_area()
  {
	  return (max.x - min.x)*(max.y - min.y);
  }
  void include(const RBAABB& o)
  {
	  include(o.min);
	  include(o.max);
  }
  f32 get_surface_area() const
  {
	  if(!is_available()) return -1.f;
	  f32 a = max.x - min.x;
	  f32 b = max.y - min.y;
	  f32 c = max.z - min.z;
	  f32 d = a*b + b*c + a*c;
	  return d + d;
  }

  //d归一化
  bool intersection(const RBVector3& o,const RBVector3& d,RBVector3& p,float& mint) const
  {
	  //《碰撞检测算法技术》p.124
	  //原书代码有误
	  mint = 0.f;
	  f32 maxt = MAX_F32;
	  for(int i=0;i<3;i++)
	  {
		  if(RBMath::abs(d[i])<VECTORS_ARE_NEAR)
		  {
			  if(o[i]<min[i]||o[i]>max[i]) 
				  return false;
		  }
		  else
		  {
			  f32 ood = 1.f/d[i];
			  f32 t1 = (min[i]-o[i])*ood;
			  f32 t2 = (max[i]-o[i])*ood;
			  if(t1>t2)
			  {
				  f32 temp = t1;
				  t1 = t2;
				  t2 = temp;
			  }
			  if(t1>mint) 
				  mint = t1;
			  if(t2<maxt)
				  maxt = t2;
			  if(mint>maxt)
				  return false;
		  }
	  }
	  p = o + d*mint;
	  return true;
  }

  //d归一化
  bool intersection(const RBVector3& o,const RBVector3& d,float& mint,float& maxt) const
  {
	  mint = 0.f;
	  maxt = MAX_F32;
	  for(int i=0;i<3;i++)
	  {
		  if(RBMath::abs(d[i])<VECTORS_ARE_NEAR)
		  {
			  if(o[i]<min[i]||o[i]>max[i]) 
				  return false;
		  }
		  else
		  {
			  f32 ood = 1.f/d[i];
			  f32 t1 = (min[i]-o[i])*ood;
			  f32 t2 = (max[i]-o[i])*ood;
			  if(t1>t2)
			  {
				  f32 temp = t1;
				  t1 = t2;
				  t2 = temp;
			  }
			  if(t1>mint) 
				  mint = t1;
			  if(t2<maxt)
				  maxt = t2;
			  if(mint>maxt)
				  return false;
		  }
	  }
	  return true;
  }
};
template <> struct TIsPODType<RBAABB> { enum { v = true }; };

struct RBAABB2D
{
	RBVector2 min;
	RBVector2 max;

	RBAABB2D()
	{
		this->min = RBVector2(MAX_F32, MAX_F32);
		this->max = RBVector2(-(MAX_F32-1), -(MAX_F32-1));
	}
	RBAABB2D(const RBVector2& min, const RBVector2& max)
	{
		this->min = min;
		this->max = max;
	}
	RBAABB2D(f32 min_x, f32 min_y, f32 max_x, f32 max_y)
	{
		this->set(min_x,min_y,max_x,max_y);
	}
	void reset()
	{
		this->min = RBVector2(MAX_F32, MAX_F32);
		this->max = RBVector2(-(MAX_F32-1), -(MAX_F32-1));
	}
	void set_size(f32 min_x,f32 min_y,f32 w,f32 h)
	{
		this->min.x = min_x;
		this->min.y = min_y;
		this->max.x = w + this->min.x;
		this->max.y = h + this->min.y;
	}

	void set(f32 min_x, f32 min_y, f32 max_x, f32 max_y)
	{
		this->min.x = min_x;
		this->min.y = min_y;
		this->max.x = max_x;
		this->max.y = max_y;
	}
	bool intersection(const RBAABB2D& o)
	{
		/*
		if (o.max.x < min.x) return false;
		if (o.max.y < min.y) return false;
		if (o.min.x > max.x) return false;
		if (o.min.y > max.y) return false;
		return true;
		*/
		return (o.max.x > min.x) && (o.max.y > min.y) && (o.min.x < max.x) && (o.min.y < max.y);
	}
	//保守相交测试，用于规避误差
	bool intersection_consv(const RBAABB2D& o)
	{
		return (o.max.x >= min.x-SMALL_F) && (o.max.y >= min.y-SMALL_F) && (o.min.x <= max.x+SMALL_F) && (o.min.y <= max.y+SMALL_F);
	}
	bool check()
	{
		return (max.x > min.x) && (max.y>min.y);
	}
  void include(const RBVector2& v)
  {
    min.y = min.y > v.y ? v.y : min.y;
    min.x = min.x > v.x ? v.x : min.x;
    max.y = max.y < v.y ? v.y : max.y;
    max.x = max.x < v.x ? v.x : max.x;
  }
  void transform(const RBMatrix& m)
  {

	  RBVector4 v[4];
	  v[0] = RBVector4(RBVector3(min.x,min.y,0),1.f);
	  v[0].apply_matrix(m);

	  v[1] = RBVector4(RBVector3(min.x,min.y,0),1.f);
	  v[1].apply_matrix(m);

	  v[2] = RBVector4(RBVector3(min.x,max.y,0),1.f);
	  v[2].apply_matrix(m);

	  v[3] = RBVector4(RBVector3(max.x,min.y,0),1.f);
	  v[3].apply_matrix(m);

	  reset();
	  for (int i=0;i<4;++i)
	  {
		  include(v[i]);
	  }

	/*
    RBVector4 v(this->min,RBVector2(0,1));
    v.w = 1.f;
    v.apply_matrix(m);
    RBVector4 v1 = RBVector4(this->max,RBVector2(0,1));
    v1.w = 1.f;
    v1.apply_matrix(m);
	reset();
    include(v);
    include(v1);
	*/
  }

  //set w to 1
  void transform_aff(const RBMatrix& m)
  {
	  RBVector4 v[4];
	  v[0] = RBVector4(RBVector3(min.x,min.y,0),1.f);
	  v[0].apply_matrix(m);
	  v[0]/=v[0].w;
	  v[1] = RBVector4(RBVector3(min.x,min.y,0),1.f);
	  v[1].apply_matrix(m);
	  v[1]/=v[1].w;
	  v[2] = RBVector4(RBVector3(min.x,max.y,0),1.f);
	  v[2].apply_matrix(m);
	  v[2]/=v[2].w;
	  v[3] = RBVector4(RBVector3(max.x,min.y,0),1.f);
	  v[3].apply_matrix(m);
	  v[3]/=v[3].w;
	  reset();
	  for (int i=0;i<4;++i)
	  {
		  include(v[i]);
	  }
	  /*
    RBVector4 v(this->min,RBVector2(0,1));
    v.w = 1.f;
    v.apply_matrix(m);
    v /= v.w;
    RBVector4 v1 = RBVector4(this->max,RBVector2(0,1));
    v1.w = 1.f;
    v1.apply_matrix(m);
    v1 /= v.w;
	reset();
    include(v);
    include(v1);
	*/
  }
  bool is_nearly_same(const RBAABB2D& aabb)
  {
	  return RBMath::is_nearly_zero(RBVector2::dist(min,aabb.min))&&
	  RBMath::is_nearly_zero( RBVector2::dist(max,aabb.max));
  }
};

template <> struct TIsPODType<RBAABB2D> { enum { v = true }; };

struct RBAABBI
{
  RBVector2I min, max;
  RBAABBI() = default;
  RBAABBI(const RBVector2I& inv1, const RBVector2I& inv2)
  {
	  this->min = inv1;
	  this->max = inv2;
  }
  FORCEINLINE i32 get_width(){ return max.x - min.x; }
  FORCEINLINE i32 get_height(){ return max.y - min.y; }


};