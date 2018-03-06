#include "Input.h"
#include "InputManager.h"
//#include "Assertion.h"



Input::Input()
{
}

Input::~Input()
{
}


bool Input::get_sys_key_down(int key)
{

	return (bool)(g_input_manager->get_current_down_bit()&key);
	/*
	bool last = (bool)(_manager->get_last_key_info()[0].key_bit&key);
	bool current = (bool)(_manager->get_key_info()[0].key_bit&key);
	return !last&&current;
	*/
}

bool Input::get_key_down(int key)
{
	//return (bool)(g_input_manager->get_key_info()[1].key_bit&key);
	return (bool)(g_input_manager->get_current_down_char()&key);
}

bool Input::get_sys_key_up(int key)
{
	return (bool)(g_input_manager->get_current_up_bit()&key);
	/*
	bool last = (bool)(_manager->get_last_key_info()[0].key_bit&key);
	bool current = (bool)(_manager->get_key_info()[0].key_bit&key);
	return last&&!current;
	*/
}

bool Input::get_key_up(int key)
{
	return (bool)(g_input_manager->get_current_up_char()&key);
}

bool Input::get_sys_key_pressed(int key)
{
	return (bool)(g_input_manager->get_key_info()[0].key_bit&key);
}

bool Input::get_key_pressed(int key)
{
	return (bool)(g_input_manager->get_key_info()[1].key_bit&key);
}

i32 Input::get_mouse_scroller()
{
	if (g_input_manager->get_key_info()->key_bit&WIP_MOUSE_SCROLLER_DOWN)
		return -1;
	else if (g_input_manager->get_key_info()->key_bit&WIP_MOUSE_SCROLLER_UP)
	{
		return 1;
	}
	return 0;
}

i32 Input::get_mouse_x()
{
	//because of friend
	return g_input_manager->_mouse_x;
}

i32 Input::get_mouse_y()
{
	return g_input_manager->_mouse_y;
}


void Input::print_current_keyinfo()
{
	printf("%d\n", g_input_manager->get_key_info()[0].key_bit);
}

bool Input::is_move()
{
	return g_input_manager->get_move();
}