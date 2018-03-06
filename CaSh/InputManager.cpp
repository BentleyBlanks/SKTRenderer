#include "InputManager.h"
#include "KeyDefinetions.h"
#include "Util.h"

bool lmouse_keep_going = false;
bool rmouse_keep_going = false;
bool mmouse_keep_going = false;

InputManager* InputManager::_instance = 0;

InputManager* InputManager::instance()
{
	if (!_instance)
		_instance = new InputManager();
	return _instance;
}

InputManager::InputManager()
{
	up_bits = 0;
	_current_down_bit = 0;
	_current_up_bit = 0;
	_current_down_char = 0;
	_current_up_char = 0;
	bdown = false;
	_bactive = false;
}

InputManager::~InputManager()
{
	delete _instance;
}

bool InputManager::startup(char* mapfile)
{
	_bactive = true;
	_keyinfo[0].key_bit = _last_keyinfo[0].key_bit = 0;
	_keyinfo[1].key_bit = _last_keyinfo[1].key_bit = 0;
	init_key_map(mapfile);
	return true;
}


void InputManager::update(int ic, int ibits, bool flag)
{
	if (!_bactive)
		return;
	else
	{

		if (ibits != -1)
		{
			_keyinfo[0].key_bit = ibits;
			int changes = _keyinfo[0].key_bit ^ _last_keyinfo[0].key_bit;
			_current_down_bit = _keyinfo[0].key_bit & changes;
			_current_up_bit = changes & (~_keyinfo[0].key_bit);
			_last_keyinfo[0] = _keyinfo[0];
		}
		if (ic != -1)
		{
			_keyinfo[1].key_bit = ic;
			int changes = _keyinfo[1].key_bit ^ _last_keyinfo[1].key_bit;
			_current_down_char = _keyinfo[1].key_bit & changes;
			_current_up_char = changes & (~_keyinfo[1].key_bit);
			_last_keyinfo[1] = _keyinfo[1];
			
		}

	}



}

void InputManager::shutdown()
{
	_bactive = false;
	_keyinfo[0].key_bit = 0;
	_keyinfo[1].key_bit = 0;
}


bool InputManager::init_key_map(char* mapfile)
{
	/*
	inifile::IniFile ini;
	ini.open(mapfile);
	*/
	/*
	_key_map[WIPKEY_ESC] = 0;
	_key_map[WIPKEY_F1] = 0;
	_key_map[WIPKEY_F2] = 0;
	_key_map[WIPKEY_F3] = 0;
	_key_map[WIPKEY_F4] = 0;
	_key_map[WIPKEY_F5] = 0;
	_key_map[WIPKEY_F6] = 0;
	_key_map[WIPKEY_F7] = 0;
	_key_map[WIPKEY_F8] = 0;
	_key_map[WIPKEY_F9] = 0;
	_key_map[WIPKEY_F10] = 0;
	_key_map[WIPKEY_F11] = 0;
	_key_map[WIPKEY_F12] = 0;
	_key_map[WIPKEY_BACKSPACE] = WIP_MENUBACKWARD;
	_key_map[WIPKEY_ENTER] = WIP_OK;
	_key_map[WIPKEY_RSHIFT] = 0;
	_key_map[WIPKEY_LSHIFT] = 0;
	_key_map[WIPKEY_RALT] = 0;
	_key_map[WIPKEY_RCTRL] = 0;
	_key_map[WIPKEY_LALT] = 0;
	_key_map[WIPKEY_LCTRL] = 0;
	_key_map[WIPKEY_TAB] = 0;
	_key_map[WIPKEY_UP] = WIP_UP;
	_key_map[WIPKEY_DOWN] = WIP_DOWN;
	_key_map[WIPKEY_LEFT] = WIP_LEFT;
	_key_map[WIPKEY_RIGHT] = WIP_RIGHT;
	*/
	input_map["f1"] = WIP_F1;
	input_map["f2"] = WIP_F2;
	input_map["f3"] = WIP_F3;
	input_map["f4"] = WIP_F4;
	input_map["f5"] = WIP_F5;
	input_map["f6"] = WIP_F6;
	input_map["f7"] = WIP_F7;
	input_map["f8"] = WIP_F8;
	input_map["f9"] = WIP_F9;
	input_map["f10"] = WIP_F10;
	input_map["f11"] = WIP_F11;
	input_map["f12"] = WIP_F12;
	input_map["esc"] = WIP_ESC;
	input_map["backspace"] = WIP_BACKSPACE;
	input_map["enter"] = WIP_ENTER;
	input_map["rshift"] = WIP_RSHIFT;
	input_map["lshift"] = WIP_LSHIFT;
	input_map["lctrl"] = WIP_LCTRL;
	input_map["rctrl"] = WIP_RCTRL;
	input_map["lalt"] = WIP_LALT;
	input_map["ralt"] = WIP_RALT;
	input_map["tab"] = WIP_TAB;
	input_map["up"] = WIP_UP;
	input_map["down"] = WIP_DOWN;
	input_map["left"] = WIP_LEFT;
	input_map["right"] = WIP_RIGHT;
	input_map["space"] = WIP_SPACE;
	input_map["lmouse"] = WIP_MOUSE_LBUTTON;
	input_map["rmouse"] = WIP_MOUSE_RBUTTON;
	input_map["mmouse"] = WIP_MOUSE_MBUTTON;
	input_map["scrollerup"] = WIP_MOUSE_SCROLLER_UP;
	input_map["scrollerdown"] = WIP_MOUSE_SCROLLER_DOWN;
	input_map["a"] = WIP_A;
	input_map["b"] = WIP_B;
	input_map["c"] = WIP_C;
	input_map["d"] = WIP_D;
	input_map["e"] = WIP_E;
	input_map["f"] = WIP_F;
	input_map["g"] = WIP_G;
	input_map["h"] = WIP_H;
	input_map["i"] = WIP_I;
	input_map["j"] = WIP_J;
	input_map["k"] = WIP_K;
	input_map["l"] = WIP_L;
	input_map["m"] = WIP_M;
	input_map["n"] = WIP_N;
	input_map["o"] = WIP_O;
	input_map["p"] = WIP_P;
	input_map["q"] = WIP_Q;
	input_map["r"] = WIP_R;
	input_map["s"] = WIP_S;
	input_map["t"] = WIP_T;
	input_map["u"] = WIP_U;
	input_map["v"] = WIP_V;
	input_map["w"] = WIP_W;

	input_map["x"] = WIP_X;
	input_map["y"] = WIP_Y;
	input_map["z"] = WIP_Z;
	return true;
}

void InputManager::clear_states()
{


	//if up_bits != 0,zero that bit.
	//_keyinfo[0].key_bit = 0;
	//_keyinfo[1].key_bit = 0;
	_current_up_bit = _current_down_bit = 0;
	_current_down_char = _current_up_char = 0;
	
	set_move(false);
}

void InputManager::clear_scroller()
{
	_keyinfo[0].key_bit &= ~WIP_MOUSE_SCROLLER_UP;
	_keyinfo[0].key_bit &= ~WIP_MOUSE_SCROLLER_DOWN;
	_last_keyinfo[0].key_bit &= ~WIP_MOUSE_SCROLLER_UP;
	_last_keyinfo[0].key_bit &= ~WIP_MOUSE_SCROLLER_DOWN;
}

int InputManager::get_current_down_bit()
{
	return _current_down_bit;
}

int InputManager::get_current_up_bit()
{
	return _current_up_bit;
}

int InputManager::get_current_down_char()
{
	return _current_down_char;
}

int InputManager::get_current_up_char()
{
	return _current_up_char;
}

void InputManager::set_move(bool v)
{
	_move = v;
}

InputManager* g_input_manager = InputManager::instance();