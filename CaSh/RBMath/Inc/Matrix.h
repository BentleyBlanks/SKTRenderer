#pragma once

#include "Axis.h"
#include "Vector3.h"
/*with:
 *	   Vector4.h
 *	   RBMathUtilities.h
 *			stdlib.h/
 *			RBMathBase.h/math.h
 */
//#include "WindowsPlatformTemp.h"

class RBMatrix
{
public:
	//UE4 use a union to surround the data
	//not sure the reason
#if _MSC_VER > 1700
	ALIGN(16) f32 m[4][4];
#else
	f32 m[4][4];
#endif

	ALIGN(16) static const RBMatrix identity;

	FORCEINLINE RBMatrix();

	FORCEINLINE RBMatrix(const RBVector3& tx,const RBVector3& ty,const RBVector3& tz,const RBVector3& tw);
	
	FORCEINLINE RBMatrix(const RBVector4& tx,const RBVector4& ty,const RBVector4& tz,const RBVector4& tw);

	FORCEINLINE RBMatrix(f32 n00,f32 n01,f32 n02,f32 n03,f32 n10,f32 n11,f32 n12,f32 n13,f32 n20,f32 n21,f32 n22,f32 n23,f32 n30,f32 n31,f32 n32,f32 n33);

	INLINE void set_identity();

	FORCEINLINE RBMatrix operator* (const RBMatrix& other) const;

	FORCEINLINE void operator*=(const RBMatrix& other);

	FORCEINLINE RBMatrix operator+(const RBMatrix& other) const;

	FORCEINLINE void operator+=(const RBMatrix& other);

	FORCEINLINE RBMatrix operator*(f32 a) const;

	FORCEINLINE void operator*=(f32 a);

	INLINE bool operator==(const RBMatrix& other) const;

	INLINE bool equals(const RBMatrix& other, f32 tolerance=SMALL_F) const;

	INLINE bool operator!=(const RBMatrix& other) const;

	//for transform right mult
	FORCEINLINE void rotate(f32 degreex,f32 degreey,f32 degreez);
	
	FORCEINLINE void traslate(f32 x,f32 y,f32 z);

	FORCEINLINE void scale(f32 sx,f32 sy,f32 sz);

	FORCEINLINE void set_rotation(f32 degreex,f32 degreey,f32 degreez);

	FORCEINLINE void set_translation(f32 x,f32 y,f32 z);

	FORCEINLINE void set_translation(const RBVector3& v);

	FORCEINLINE void set_scale(f32 sx,f32 sy,f32 sz);

	//Homogeneous transform 齐次
	FORCEINLINE RBVector4 transform_vector4(const RBVector4& v) const
	{

	}

	FORCEINLINE RBVector4 transform_position(const RBVector3 &V) const
	{

	}

	FORCEINLINE RBVector3 inv_transform_position(const RBVector3 &V) const
	{}

	FORCEINLINE RBVector4 transform_vector3(const RBVector3& V) const
	{
		/*
		RBVector4 o(0,0,0,1);
		RBVector4 p(V,1);
		o = o* *this;
		p = p* *this;
		return (p - o);
		*/
		RBVector4 _r;

		_r.x = m[0][0] * V.x + m[1][0] * V.y + m[2][0] * V.z ;
		_r.y = m[0][1] * V.x + m[1][1] * V.y + m[2][1] * V.z ;
		_r.z = m[0][2] * V.x + m[1][2] * V.y + m[2][2] * V.z ;
		_r.w = 1;

		return _r;

	}

	FORCEINLINE RBVector3 inv_transform_vector3(const RBVector3 &V) const
	{}

	// Transpose.转置

	FORCEINLINE RBMatrix get_transposed() const;

	// @return determinant of this matrix.返回

	INLINE f32 get_determinant() const;

	/** @return the determinant of rotation 3x3 matrix */
	INLINE f32 rotation_determinant() const;

	/** Fast path, doesn't check for nil matrices in final release builds */
	INLINE RBMatrix get_inverse() const;
	/** Fast path, and handles nil matrices. */
	INLINE RBMatrix get_inverse_safe() const;
	/** Slow and safe path */
	INLINE RBMatrix get_inverse_slow() const;

	INLINE void get_translate(RBVector3& out);

	INLINE RBMatrix get_rotation() const;

#ifdef _DEBUG
	/**output console**/
	void out() const;
#endif

	/**quickly set valves to the matrix**/
	INLINE void set_quickly(f32 a0,f32 a1,f32 a2,f32 a3,
							f32 b0,f32 b1,f32 b2,f32 b3,
							f32 c0,f32 c1,f32 c2,f32 c3,
							f32 d0,f32 d1,f32 d2,f32 d3)
	{

		m[0][0] = a0; m[0][1] = a1; m[0][2] = a2; m[0][3] = a3;
		m[1][0] = b0; m[1][1] = b1; m[1][2] = b2; m[1][3] = b3;
		m[2][0] = c0; m[2][1] = c1; m[2][2] = c2; m[2][3] = c3;
		m[3][0] = d0; m[3][1] = d1; m[3][2] = d2; m[3][3] = d3;

	}

	static FORCEINLINE RBMatrix get_lookat(const RBVector3& eye,const RBVector3& forward,const RBVector3& up);
	//static FORCEINLINE RBMatrix get_lookat(const RBVector3& eye,const RBVector3& target,const RBVector3& up);
	static FORCEINLINE RBMatrix get_frustum(f32 left,f32 right,f32 bottom,f32 top,f32 n,f32 f);
	static FORCEINLINE RBMatrix get_perspective(f32 fovy, f32 aspect, f32 n, f32 f);
	static FORCEINLINE RBMatrix get_ortho(f32 left,f32 right,f32 bottom,f32 top,f32 near,f32 far);

	static FORCEINLINE RBMatrix get_lookat_zup(const RBVector3& eye,const RBVector3& forward,const RBVector3& up);
	static FORCEINLINE RBMatrix get_frustum_zup(f32 left,f32 right,f32 bottom,f32 top,f32 n,f32 f);
	static FORCEINLINE RBMatrix get_perspective_zup(f32 fovy, f32 aspect, f32 n, f32 f);
	static FORCEINLINE RBMatrix get_ortho_zup(f32 left,f32 right,f32 bottom,f32 top,f32 near,f32 f);
};

RBMatrix RBMatrix::get_lookat(const RBVector3& eye,const RBVector3& forward,const RBVector3& up)
{
	RBVector3 za =   forward;
	za.normalize();
	RBVector3 xa = RBVector3::cross_product(up,za);
	xa.normalize();
	RBVector3 ya = RBVector3::cross_product(za,xa);

	return RBMatrix(
		xa.x,ya.x,za.x,0,
		xa.y,ya.y,za.y,0,
		xa.z,ya.z,za.z,0,
		-RBVector3::dot_product(xa,eye),
		-RBVector3::dot_product(ya,eye),
		-RBVector3::dot_product(za,eye),
		1
		);
}

RBMatrix RBMatrix::get_frustum(f32 left,f32 right,f32 bottom,f32 top,f32 n,f32 f)
{

}

RBMatrix RBMatrix::get_perspective(f32 fovy, f32 aspect, f32 n, f32 f)
{
	f32 ys = RBMath::cot(DEG2RAD(fovy)*0.5f);
	f32 xs = ys/aspect;
	return RBMatrix(
		xs,0,0,0,
		0,ys,0,0,
		0,0,f/(f-n),1,
		0,0,-n*f/(f-n),0
		);
}

RBMatrix RBMatrix::get_ortho(f32 left,f32 right,f32 bottom,f32 top,f32 n,f32 f)
{
	f32 w = right - left;
	f32 h = top - bottom;
	return RBMatrix(
		2/w,0,0,0,
		0,2/h,0,0,
		0,0,1/(f-n),0,
		0,0,n/(n-f),1
		);
}


FORCEINLINE RBMatrix RBMatrix::get_lookat_zup(const RBVector3& eye,const RBVector3& forward,const RBVector3& up)
{
	RBVector3 ya =   forward;
	ya.normalize();
	RBVector3 xa = RBVector3::cross_product(ya,up);
	xa.normalize();
	RBVector3 za = RBVector3::cross_product(xa,ya);

	return RBMatrix(
		xa.x,ya.x,za.x,0,
		xa.y,ya.y,za.y,0,
		xa.z,ya.z,za.z,0,
		-RBVector3::dot_product(xa,eye),
		-RBVector3::dot_product(ya,eye),
		-RBVector3::dot_product(za,eye),
		1
		);
}

FORCEINLINE RBMatrix RBMatrix::get_frustum_zup(f32 left,f32 right,f32 bottom,f32 top,f32 n,f32 f)
{

}

FORCEINLINE RBMatrix RBMatrix::get_perspective_zup(f32 fovz, f32 aspect, f32 n, f32 f)
{
	f32 zs = RBMath::cot(DEG2RAD(fovz)*0.5f);
	f32 xs = zs/aspect;
	return RBMatrix(
		xs,0,0,0,
		0,f/(f-n),0,1,
		0,0,zs,0,
		0,-n*f/(f-n),0,0
		);
}

FORCEINLINE RBMatrix RBMatrix::get_ortho_zup(f32 left,f32 right,f32 bottom,f32 top,f32 n,f32 f)
{
	f32 w = right - left;
	f32 h = top - bottom;
	return RBMatrix(
		2/w,0,0,0,
		0,1/(f-n),0,0,
		0,0,2.f/h,0,
		0,n/(n-f),0,1
		);
}

//xyz
FORCEINLINE void RBMatrix::rotate(f32 degreex,f32 degreey,f32 degreez)
{
	//x,y,z
	set_identity();
	
	/*
	f32 rad_x = (degreex);
	f32 rad_y = (degreey);
	f32 rad_z = (degreez);
	*/
	
	f32 rad_x = DEG2RAD(degreex);
	f32 rad_y = DEG2RAD(degreey);
	f32 rad_z = DEG2RAD(degreez);
	

	f32 cx = RBMath::cos(rad_y);
	f32 cy = RBMath::cos(rad_z);
	f32 cz = RBMath::cos(rad_x);
	f32 sx = RBMath::sin(rad_y);
	f32 sy = RBMath::sin(rad_z);
	f32 sz = RBMath::sin(rad_x);

	/*
	set_quickly(cz*cy,sz*cy,sy,0,
		sx*sy*cz-sz*cx,sx*sy*sz+cz*cx,sx*cy,0,
		cx*sy*cz+sx*sz,-cx*sy*sz+sx*cz,cx*cy,0,
		0,0,0,1
		);
		*/
	
	set_quickly(cx*cy,sy*cx,-sx,0,
		sz*sx*cy-cz*sy,sz*sx*sy+cz*cy,sz*cx,0.f,
		(cz*sx*cy+sz*sy),-cy*sz+cz*sx*sy,cz*cx,0.f,
		0,0,0,1
		);
	/*
	m[0][0] = cx*cy;
	m[0][1] = -sy*cx;
	m[0][2] = -sx;
	m[1][0] = -sz*sx*cy+cz*sy;
	m[1][1] = sz*sx*sy+cz*cy;
	m[1][2] = -sz*cx;
	m[2][0] =	(cz*sx*cy+sz*sy);
	m[2][1] = cy*sz-cz*sx*sy;
	m[2][2] = cz*cx;
	*/
}

FORCEINLINE void RBMatrix::traslate(f32 x,f32 y,f32 z)
{
	set_identity();
	m[3][0] = x;
	m[3][1] = y;
	m[3][2] = z;
}

FORCEINLINE void RBMatrix::scale(f32 sx,f32 sy,f32 sz)
{
	set_identity();
	m[0][0] = sx;
	m[1][1] = sy;
	m[2][2] = sz;
}

//xyz
FORCEINLINE void RBMatrix::set_rotation(f32 degreex,f32 degreey,f32 degreez)
{
	f32 rad_x = DEG2RAD(degreex);
	f32 rad_y = DEG2RAD(degreey);
	f32 rad_z = DEG2RAD(degreez);
	

	f32 cx = RBMath::cos(rad_y);
	f32 cy = RBMath::cos(rad_z);
	f32 cz = RBMath::cos(rad_x);
	f32 sx = RBMath::sin(rad_y);
	f32 sy = RBMath::sin(rad_z);
	f32 sz = RBMath::sin(rad_x);


	m[0][0] = cx*cy;
	m[0][1] = sy*cx;
	m[0][2] = -sx;
	m[1][0] = sz*sx*cy-cz*sy;
	m[1][1] = sz*sx*sy+cz*cy;
	m[1][2] = sz*cx;
	m[2][0] =	(cz*sx*cy+sz*sy);
	m[2][1] = -cy*sz+cz*sx*sy;
	m[2][2] = cz*cx;
}

FORCEINLINE void RBMatrix::set_translation(f32 x,f32 y,f32 z)
{
	m[3][0] = x;
	m[3][1] = y;
	m[3][2] = z;
}

FORCEINLINE void RBMatrix::set_translation(const RBVector3& v)
{
	m[3][0] = v.x;
	m[3][1] = v.y;
	m[3][2] = v.z;
}

FORCEINLINE void RBMatrix::set_scale(f32 sx,f32 sy,f32 sz)
{
	m[0][0] = sx;
	m[1][1] = sy;
	m[2][2] = sz;
}


FORCEINLINE  RBMatrix::RBMatrix()
{
	set_identity();
}

//行向量
FORCEINLINE RBMatrix::RBMatrix(const RBVector3& tx,const RBVector3& ty,const RBVector3& tz,const RBVector3& tw)
{
	m[0][0] = tx.x;m[0][1] = tx.y;m[0][2] = tx.z;m[0][3] = 0.f;
	m[1][0] = ty.x;m[1][1] = ty.y;m[1][2] = ty.z;m[1][3] = 0.f;
	m[2][0] = tz.x;m[2][1] = tz.y;m[2][2] = tz.z;m[2][3] = 0.f;
	m[3][0] = tw.x;m[3][1] = tw.y;m[3][2] = tw.z;m[3][3] = 1.f;
}

FORCEINLINE RBMatrix::RBMatrix(const RBVector4& tx,const RBVector4& ty,const RBVector4& tz,const RBVector4& tw)
{
	m[0][0] = tx.x;m[0][1] = tx.y;m[0][2] = tx.z;m[0][3] = tx.w;
	m[1][0] = ty.x;m[1][1] = ty.y;m[1][2] = ty.z;m[1][3] = ty.w;
	m[2][0] = tz.x;m[2][1] = tz.y;m[2][2] = tz.z;m[2][3] = tz.w;
	m[3][0] = tw.x;m[3][1] = tw.y;m[3][2] = tw.z;m[3][3] = tw.w;
}

FORCEINLINE RBMatrix::RBMatrix(f32 n00,f32 n01,f32 n02,f32 n03,f32 n10,f32 n11,f32 n12,f32 n13,f32 n20,f32 n21,f32 n22,f32 n23,f32 n30,f32 n31,f32 n32,f32 n33)
{
	
	m[0][0] = n00;m[0][1] = n01;m[0][2] = n02;m[0][3] = n03;
	m[1][0] = n10;m[1][1] = n11;m[1][2] = n12;m[1][3] = n13;
	m[2][0] = n20;m[2][1] = n21;m[2][2] = n22;m[2][3] = n23;
	m[3][0] = n30;m[3][1] = n31;m[3][2] = n32;m[3][3] = n33;

}

INLINE void RBMatrix::set_identity()
{
	m[0][0] = 1;m[0][1] = 0;m[0][2] = 0;m[0][3] = 0;
	m[1][0] = 0;m[1][1] = 1;m[1][2] = 0;m[1][3] = 0;
	m[2][0] = 0;m[2][1] = 0;m[2][2] = 1;m[2][3] = 0;
	m[3][0] = 0;m[3][1] = 0;m[3][2] = 0;m[3][3] = 1;
}


//Res = Mat1.operator*(Mat2) means Res = Mat1 * Mat2
FORCEINLINE RBMatrix RBMatrix::operator*(const RBMatrix& other) const
{
	//UE4 optimize it with SSE，refer to "UnrealMathSSE.h".
	RBMatrix _m;

	_m.m[0][0] = other.m[0][0]*m[0][0]+other.m[1][0]*m[0][1]+other.m[2][0]*m[0][2]+other.m[3][0]*m[0][3];
	_m.m[0][1] = other.m[0][1]*m[0][0]+other.m[1][1]*m[0][1]+other.m[2][1]*m[0][2]+other.m[3][1]*m[0][3];
	_m.m[0][2] = other.m[0][2]*m[0][0]+other.m[1][2]*m[0][1]+other.m[2][2]*m[0][2]+other.m[3][2]*m[0][3];
	_m.m[0][3] = other.m[0][3]*m[0][0]+other.m[1][3]*m[0][1]+other.m[2][3]*m[0][2]+other.m[3][3]*m[0][3];

	_m.m[1][0] = other.m[0][0]*m[1][0]+other.m[1][0]*m[1][1]+other.m[2][0]*m[1][2]+other.m[3][0]*m[1][3];
	_m.m[1][1] = other.m[0][1]*m[1][0]+other.m[1][1]*m[1][1]+other.m[2][1]*m[1][2]+other.m[3][1]*m[1][3];
	_m.m[1][2] = other.m[0][2]*m[1][0]+other.m[1][2]*m[1][1]+other.m[2][2]*m[1][2]+other.m[3][2]*m[1][3];
	_m.m[1][3] = other.m[0][3]*m[1][0]+other.m[1][3]*m[1][1]+other.m[2][3]*m[1][2]+other.m[3][3]*m[1][3];

	_m.m[2][0] = other.m[0][0]*m[2][0]+other.m[1][0]*m[2][1]+other.m[2][0]*m[2][2]+other.m[3][0]*m[2][3];
	_m.m[2][1] = other.m[0][1]*m[2][0]+other.m[1][1]*m[2][1]+other.m[2][1]*m[2][2]+other.m[3][1]*m[2][3];
	_m.m[2][2] = other.m[0][2]*m[2][0]+other.m[1][2]*m[2][1]+other.m[2][2]*m[2][2]+other.m[3][2]*m[2][3];
	_m.m[2][3] = other.m[0][3]*m[2][0]+other.m[1][3]*m[2][1]+other.m[2][3]*m[2][2]+other.m[3][3]*m[2][3];

	_m.m[3][0] = other.m[0][0]*m[3][0]+other.m[1][0]*m[3][1]+other.m[2][0]*m[3][2]+other.m[3][0]*m[3][3];
	_m.m[3][1] = other.m[0][1]*m[3][0]+other.m[1][1]*m[3][1]+other.m[2][1]*m[3][2]+other.m[3][1]*m[3][3];
	_m.m[3][2] = other.m[0][2]*m[3][0]+other.m[1][2]*m[3][1]+other.m[2][2]*m[3][2]+other.m[3][2]*m[3][3];
	_m.m[3][3] = other.m[0][3]*m[3][0]+other.m[1][3]*m[3][1]+other.m[2][3]*m[3][2]+other.m[3][3]*m[3][3];

	return _m;

}

FORCEINLINE void RBMatrix::operator*=(const RBMatrix& other)
{
	*this = *this*other;
	/*
	RBMatrix _m;
	_m.m[0][0] = other.m[0][0]*m[0][0]+other.m[1][0]*m[0][1]+other.m[2][0]*m[0][2]+other.m[3][0]*m[0][3];
	_m.m[0][1] = other.m[0][1]*m[0][0]+other.m[1][1]*m[0][1]+other.m[2][1]*m[0][2]+other.m[3][1]*m[0][3];
	_m.m[0][2] = other.m[0][2]*m[0][0]+other.m[1][2]*m[0][1]+other.m[2][2]*m[0][2]+other.m[3][2]*m[0][3];
	_m.m[0][3] = other.m[0][3]*m[0][0]+other.m[1][3]*m[0][1]+other.m[2][3]*m[0][2]+other.m[3][3]*m[0][3];

	_m.m[1][0] = other.m[0][0]*m[1][0]+other.m[1][0]*m[1][1]+other.m[2][0]*m[1][2]+other.m[3][0]*m[1][3];
	_m.m[1][1] = other.m[0][1]*m[1][0]+other.m[1][1]*m[1][1]+other.m[2][1]*m[1][2]+other.m[3][1]*m[1][3];
	_m.m[1][2] = other.m[0][2]*m[1][0]+other.m[1][2]*m[1][1]+other.m[2][2]*m[1][2]+other.m[3][2]*m[1][3];
	_m.m[1][3] = other.m[0][3]*m[1][0]+other.m[1][3]*m[1][1]+other.m[2][3]*m[1][2]+other.m[3][3]*m[1][3];

	_m.m[2][0] = other.m[0][0]*m[2][0]+other.m[1][0]*m[2][1]+other.m[2][0]*m[2][2]+other.m[3][0]*m[2][3];
	_m.m[2][1] = other.m[0][1]*m[2][0]+other.m[1][1]*m[2][1]+other.m[2][1]*m[2][2]+other.m[3][1]*m[2][3];
	_m.m[2][2] = other.m[0][2]*m[2][0]+other.m[1][2]*m[2][1]+other.m[2][2]*m[2][2]+other.m[3][2]*m[2][3];
	_m.m[2][3] = other.m[0][3]*m[2][0]+other.m[1][3]*m[2][1]+other.m[2][3]*m[2][2]+other.m[3][3]*m[2][3];

	_m.m[3][0] = other.m[0][0]*m[3][0]+other.m[1][0]*m[3][1]+other.m[2][0]*m[3][2]+other.m[3][0]*m[3][3];
	_m.m[3][1] = other.m[0][1]*m[3][0]+other.m[1][1]*m[3][1]+other.m[2][1]*m[3][2]+other.m[3][1]*m[3][3];
	_m.m[3][2] = other.m[0][2]*m[3][0]+other.m[1][2]*m[3][1]+other.m[2][2]*m[3][2]+other.m[3][2]*m[3][3];
	_m.m[3][3] = other.m[0][3]*m[3][0]+other.m[1][3]*m[3][1]+other.m[2][3]*m[3][2]+other.m[3][3]*m[3][3];

	for(int i=0;i<4;i++)
		for(int j=0;j<4;j++)
		{
			m[i][j] = _m.m[i][j];
		}
	*/
}



FORCEINLINE RBMatrix RBMatrix::operator+(const RBMatrix& other) const
{
	RBMatrix _m;

	for(int i=0;i<4;i++)
		for(int j=0;j<4;j++)
		{
			_m.m[i][j] = m[i][j] + other.m[i][j];
		}

	return _m;

}

FORCEINLINE void RBMatrix::operator+=(const RBMatrix& other)
{
	*this = *this + other;
}

INLINE bool RBMatrix::operator==(const RBMatrix& other) const
{
	for(i32 i=0;i<4;i++)
	{
		for(i32 j=0;j<4;j++)
		{
			if(m[i][j]!=other.m[i][j])
				return false;

		}
	}
	return true;
}

INLINE bool RBMatrix::equals(const RBMatrix& other, f32 tolerance/*=SMALL_F*/) const
{
	for(i32 i=0;i<4;i++)
	{
		for(i32 j=0;j<4;j++)
		{
			if(RBMath::abs(m[i][j] - other.m[i][j]) > tolerance)
				return false;

		}
	}
	return true;
}

INLINE bool RBMatrix::operator!=(const RBMatrix& other) const
{
	return !(*this==other);
}

FORCEINLINE RBMatrix RBMatrix::get_transposed() const
{
	RBMatrix _r;
	_r.m[0][0] = m[0][0];
	_r.m[0][1] = m[1][0];
	_r.m[0][2] = m[2][0];
	_r.m[0][3] = m[3][0];

	_r.m[1][0] = m[0][1];
	_r.m[1][1] = m[1][1];
	_r.m[1][2] = m[2][1];
	_r.m[1][3] = m[3][1];

	_r.m[2][0] = m[0][2];
	_r.m[2][1] = m[1][2];
	_r.m[2][2] = m[2][2];
	_r.m[2][3] = m[3][2];

	_r.m[3][0] = m[0][3];
	_r.m[3][1] = m[1][3];
	_r.m[3][2] = m[2][3];
	_r.m[3][3] = m[3][3];

	return _r;
}

INLINE f32 RBMatrix::get_determinant() const
{
	return 
		m[0][0] * (
				m[1][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
				m[2][1] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) +
				m[3][1] * (m[1][2] * m[2][3] - m[1][3] * m[2][2])
				) -
			m[1][0] * (
				m[0][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
				m[2][1] * (m[0][2] * m[3][3] - m[0][3] * m[3][2]) +
				m[3][1] * (m[0][2] * m[2][3] - m[0][3] * m[2][2])
				) +
			m[2][0] * (
				m[0][1] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) -
				m[1][1] * (m[0][2] * m[3][3] - m[0][3] * m[3][2]) +
				m[3][1] * (m[0][2] * m[1][3] - m[0][3] * m[1][2])
				) -
			m[3][0] * (
				m[0][1] * (m[1][2] * m[2][3] - m[1][3] * m[2][2]) -
				m[1][1] * (m[0][2] * m[2][3] - m[0][3] * m[2][2]) +
				m[2][1] * (m[0][2] * m[1][3] - m[0][3] * m[1][2])
				);
}

INLINE f32 RBMatrix::rotation_determinant() const
{
	return	
		m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
		m[1][0] * (m[0][1] * m[2][2] - m[0][2] * m[2][1]) +
		m[2][0] * (m[0][1] * m[1][2] - m[0][2] * m[1][1]);
}

INLINE RBMatrix RBMatrix::get_inverse_slow() const
{
	RBMatrix _r;
	const f32	Det = get_determinant();

	if(Det == 0.0f)
		return RBMatrix::identity;

	const f32	RDet = 1.0f / Det;

	_r.m[0][0] = RDet * (
			m[1][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
			m[2][1] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) +
			m[3][1] * (m[1][2] * m[2][3] - m[1][3] * m[2][2])
			);
	_r.m[0][1] = -RDet * (
			m[0][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
			m[2][1] * (m[0][2] * m[3][3] - m[0][3] * m[3][2]) +
			m[3][1] * (m[0][2] * m[2][3] - m[0][3] * m[2][2])
			);
	_r.m[0][2] = RDet * (
			m[0][1] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) -
			m[1][1] * (m[0][2] * m[3][3] - m[0][3] * m[3][2]) +
			m[3][1] * (m[0][2] * m[1][3] - m[0][3] * m[1][2])
			);
	_r.m[0][3] = -RDet * (
			m[0][1] * (m[1][2] * m[2][3] - m[1][3] * m[2][2]) -
			m[1][1] * (m[0][2] * m[2][3] - m[0][3] * m[2][2]) +
			m[2][1] * (m[0][2] * m[1][3] - m[0][3] * m[1][2])
			);

	_r.m[1][0] = -RDet * (
			m[1][0] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
			m[2][0] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) +
			m[3][0] * (m[1][2] * m[2][3] - m[1][3] * m[2][2])
			);
	_r.m[1][1] = RDet * (
			m[0][0] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
			m[2][0] * (m[0][2] * m[3][3] - m[0][3] * m[3][2]) +
			m[3][0] * (m[0][2] * m[2][3] - m[0][3] * m[2][2])
			);
	_r.m[1][2] = -RDet * (
			m[0][0] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) -
			m[1][0] * (m[0][2] * m[3][3] - m[0][3] * m[3][2]) +
			m[3][0] * (m[0][2] * m[1][3] - m[0][3] * m[1][2])
			);
	_r.m[1][3] = RDet * (
			m[0][0] * (m[1][2] * m[2][3] - m[1][3] * m[2][2]) -
			m[1][0] * (m[0][2] * m[2][3] - m[0][3] * m[2][2]) +
			m[2][0] * (m[0][2] * m[1][3] - m[0][3] * m[1][2])
			);

	_r.m[2][0] = RDet * (
			m[1][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) -
			m[2][0] * (m[1][1] * m[3][3] - m[1][3] * m[3][1]) +
			m[3][0] * (m[1][1] * m[2][3] - m[1][3] * m[2][1])
			);
	_r.m[2][1] = -RDet * (
			m[0][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) -
			m[2][0] * (m[0][1] * m[3][3] - m[0][3] * m[3][1]) +
			m[3][0] * (m[0][1] * m[2][3] - m[0][3] * m[2][1])
			);
	_r.m[2][2] = RDet * (
			m[0][0] * (m[1][1] * m[3][3] - m[1][3] * m[3][1]) -
			m[1][0] * (m[0][1] * m[3][3] - m[0][3] * m[3][1]) +
			m[3][0] * (m[0][1] * m[1][3] - m[0][3] * m[1][1])
			);
	_r.m[2][3] = -RDet * (
			m[0][0] * (m[1][1] * m[2][3] - m[1][3] * m[2][1]) -
			m[1][0] * (m[0][1] * m[2][3] - m[0][3] * m[2][1]) +
			m[2][0] * (m[0][1] * m[1][3] - m[0][3] * m[1][1])
			);

	_r.m[3][0] = -RDet * (
			m[1][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]) -
			m[2][0] * (m[1][1] * m[3][2] - m[1][2] * m[3][1]) +
			m[3][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
			);
	_r.m[3][1] = RDet * (
			m[0][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]) -
			m[2][0] * (m[0][1] * m[3][2] - m[0][2] * m[3][1]) +
			m[3][0] * (m[0][1] * m[2][2] - m[0][2] * m[2][1])
			);
	_r.m[3][2] = -RDet * (
			m[0][0] * (m[1][1] * m[3][2] - m[1][2] * m[3][1]) -
			m[1][0] * (m[0][1] * m[3][2] - m[0][2] * m[3][1]) +
			m[3][0] * (m[0][1] * m[1][2] - m[0][2] * m[1][1])
			);
	_r.m[3][3] = RDet * (
			m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
			m[1][0] * (m[0][1] * m[2][2] - m[0][2] * m[2][1]) +
			m[2][0] * (m[0][1] * m[1][2] - m[0][2] * m[1][1])
			);

	return _r;
}



INLINE void RBMatrix::get_translate(RBVector3& out)
{
	out.x = m[3][0];
	out.y = m[3][1];
	out.z = m[3][2];
}

INLINE RBMatrix RBMatrix::get_rotation() const
{
	return RBMatrix(
		m[0][0], m[0][1], m[0][2], 0,
		m[1][0], m[1][1], m[1][2], 0,
		m[2][0], m[2][1], m[2][2], 0,
		0, 0, 0, 1);
}

//Below..Implementation for class RBVector4

//Apply a matrix to this vector with side use,implemented in Matrix.h
FORCEINLINE void RBVector4::apply_matrix(const RBMatrix& tm)
{
	RBVector4 _r;

	_r.x = tm.m[0][0]*x+tm.m[1][0]*y+tm.m[2][0]*z+tm.m[3][0]*w;
	_r.y = tm.m[0][1]*x+tm.m[1][1]*y+tm.m[2][1]*z+tm.m[3][1]*w;
	_r.z = tm.m[0][2]*x+tm.m[1][2]*y+tm.m[2][2]*z+tm.m[3][2]*w;
	_r.w = tm.m[0][3]*x+tm.m[1][3]*y+tm.m[2][3]*z+tm.m[3][3]*w;

	*this = _r;
}

//Matrix multiple to right with a return value,implemented in Matrix.h
FORCEINLINE RBVector4 RBVector4::operator*(const RBMatrix& tm) const
{
	RBVector4 _r;

	_r.x = tm.m[0][0]*x+tm.m[1][0]*y+tm.m[2][0]*z+tm.m[3][0]*w;
	_r.y = tm.m[0][1]*x+tm.m[1][1]*y+tm.m[2][1]*z+tm.m[3][1]*w;
	_r.z = tm.m[0][2]*x+tm.m[1][2]*y+tm.m[2][2]*z+tm.m[3][2]*w;
	_r.w = tm.m[0][3]*x+tm.m[1][3]*y+tm.m[2][3]*z+tm.m[3][3]*w;

	return _r;
}