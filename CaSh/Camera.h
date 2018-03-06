#pragma once
#pragma warning(disable : 4005)
#include "RBMath/Inc/Vector3.h"
#include "RBMath/Inc/Vector2.h"

class RBCamera
{
public:
	RBCamera(float x=0,float y=0,float z=0) 
	{
		_position.x = x;
		_position.y = y;
		_position.z = z;
		_up = RBVector3(0,1,0);
		_forward = RBVector4(0,0,1,1);
		_near_panel = 1;
		_far_panel = 100;
		_fovy = 90;
		_ratio = 9.f/16.f;

    _cam_move_speed = RBVector2(10, 10);
    _cam_rotate_speed = RBVector2(50, 50);
		
	}

  void set_move_speed(const RBVector2& speed)
  {
    _cam_move_speed = speed;
  }

  void set_rotate_speed(const RBVector2& speed)
  {
    _cam_rotate_speed = speed;
  }

	void translate(float x,float y,float z);
	void rotate(float x_deg,float y_deg,float  z_deg);

	void set_position(float x,float y,float z);
	void set_rotation(float x_deg,float y_deg,float  z_deg);

	void set_target(RBVector3 tar);

	RBVector3 get_position() const;
	RBVector4 get_rotaion();

	void update(float dt);
	void get_view_matrix(RBMatrix& out_mat) const;
	void get_perspective_matrix(RBMatrix& out_mat) const;
	void get_ortho(RBMatrix& out_mat) const;

	inline void set_near_panel(float n)
	{
		_near_panel = n;
	}

	inline void set_far_panel(float f)
	{
		_far_panel = f;
	}

	inline void set_fov_y(float deg)
	{
		_fovy = deg;
	}

	//x/y
	inline void set_ratio(float r)
	{
		_ratio = r;
	}

	inline float get_near_panel()
	{
		return _near_panel;
	}

	inline float get_far_panel()
	{
		return _far_panel;
	}

	inline float get_fovy()
	{
		return _fovy;
	}

  inline float get_tan_half_fovy()
  {
    return RBMath::tan(DEG2RAD( _fovy*0.5));
  }

  inline float get_tan_half_fovx()
  {
    return get_tan_half_fovy()*_ratio;
  }

	inline float get_ratio()
	{
		return _ratio;
	}

	inline RBVector4 get_forward()
	{
		return _forward;
	}

	inline RBVector3 get_up()
	{
		return _up;
	}

	inline RBVector3 get_right()
	{
		RBVector3 v = _forward;
		return RBVector3::cross_product(_up, v);
	}

  void control_update(f32 dt);

	void pan(const RBVector3& target,float degree);
	void rotate_by_axis(float degree,RBVector3 axis); 
  RBVector2 world_to_screen(const RBVector3& a);

protected:

public:
	RBVector3 _position;
	RBVector3 _up;
	RBVector4 _forward;
	float _near_panel;
	float _far_panel;
	float _fovy;
	//w/h
	float _ratio;

  RBVector2 _cam_move_speed;
  RBVector2 _cam_rotate_speed;
  private:
    int old_x, old_y;
};