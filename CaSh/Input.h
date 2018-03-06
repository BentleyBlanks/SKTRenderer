#pragma once
#include "RBMath/Inc/Platform/RBBasedata.h"
#include "KeyDefinetions.h"

class InputManager;

//static
//TODO:We also provide input callbacks.
class Input
{
public:
	//sys_key 提供非字符按键检测，key为字符按键检测
	//down 按了一下 pressed 被按住 up 当前弹起了
	static bool get_sys_key_down(int key);
	static bool get_key_down(int key);
	static bool get_sys_key_up(int key);
	static bool get_key_up(int key);
	static bool get_sys_key_pressed(int key);
	static bool get_key_pressed(int key);
	static int get_mouse_x();
	static int get_mouse_y();
	static i32 get_mouse_scroller();

	static bool is_move();

	void print_current_keyinfo();

protected:
	Input();
	~Input();
private:


};