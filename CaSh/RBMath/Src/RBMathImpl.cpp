#include "..\Inc\Vector4.h"
#include "..\Inc\Vector3.h"
#include "..\Inc\Vector2.h"
#include "..\Inc\Matrix.h"
#include "..\Inc\Colorf.h"
#include <iostream>
const f32 INV_RAND_MAX = 1.f/RAND_MAX;
const f32 INV_RAND_MAX_ADD1 = 1.f/(f32)((u32)RAND_MAX+1);

//RBVector4 definetion
RBVector4::RBVector4(RBVector2 axy,RBVector2 azw)
	:x(axy.x),y(axy.y),z(azw.x),w(azw.y){}
RBVector4::RBVector4(const RBVector3& v,f32 f)
	:x(v.x),y(v.y),z(v.z),w(f){}
const RBVector4 RBVector4::zero_vector(0.f,0.f,0.f,1.f);
//#ifdef _DEBUG
void RBVector4::out() const
{
	printf("(%3.3f,%3.3f,%3.3f,%3.3f)\n",x,y,z,w);
}
//#endif


//RBVector3 definetion
RBVector3::RBVector3(const RBVector2& v,f32 az):x(v.x),y(v.y),z(az){}
RBVector2 RBVector3::unit_cartesian_to_spherical() const
{
	const f32 t = RBMath::acos(z/size());
	const f32 p = RBMath::atant2(y,x);
	return RBVector2(t,p);
}

const RBVector3 RBVector3::zero_vector(0.f,0.f,0.f);
const RBVector3 RBVector3::up_vector(0.f,0.f,1.f);

//#ifdef _DEBUG
void RBVector3::out() const
{
	printf("(%3.3f,%3.3f,%3.3f)\n",x,y,z);
}
//#endif


//RBVector2 definetion
const RBVector2 RBVector2::zero_vector(0.0f, 0.0f);
const RBVector2 RBVector2::unit_vector(1.0f, 1.0f);
#ifdef _DEBUG
//×Ö·ûÊä³ö
void RBVector2::out() const
{
	printf("(%3.3f,%3.3f)\n",x,y);
}
#endif

//RBMatrix definetion
const RBMatrix RBMatrix::identity(RBVector4(1,0,0,0),RBVector4(0,1,0,0),RBVector4(0,0,1,0),RBVector4(0,0,0,1));
#ifdef _DEBUG
/**output console**/
void RBMatrix::out() const
{
	//std::cout<<std::endl;
	std::cout<<"|"<<m[0][0]<<","<<m[0][1]<<","<<m[0][2]<<","<<m[0][3]<<"|"<<std::endl;
	std::cout<<"|"<<m[1][0]<<","<<m[1][1]<<","<<m[1][2]<<","<<m[1][3]<<"|"<<std::endl;
	std::cout<<"|"<<m[2][0]<<","<<m[2][1]<<","<<m[2][2]<<","<<m[2][3]<<"|"<<std::endl;
	std::cout<<"|"<<m[3][0]<<","<<m[3][1]<<","<<m[3][2]<<","<<m[3][3]<<"|"<<std::endl;
}
#endif

