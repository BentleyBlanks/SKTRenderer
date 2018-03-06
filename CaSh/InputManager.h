#pragma once
#include "RBMath/Inc/Platform/RBBasedata.h"
#include <string>
#include <map>
enum WIPKEY;

class Input;

extern bool lmouse_keep_going;
extern bool rmouse_keep_going;
extern bool mmouse_keep_going;
//加载按键映射表
//从PlatformInput接收原始按键数据
//处理分发按键数据
//检测输入位串的掩码应该由映射表来映射
//使用hash表来实现动态映射
union KeyInfo
{
	KeyInfo()
	{
		key = 0x00;
		key_bit = 0x00000000;
	}
	char key;
	i32 key_bit;
};

//TODO:Use long type to write this class over
//compare current input with last input not the last frame
////Singleton
class InputManager
{
public:
	static InputManager* instance();

	~InputManager();
	bool startup(char* mapfile);
	//flag true for bits,false for char,but may be useless
	void update(int ic, int ibits, bool flag = true);

	inline void update_mouse(i32 x, i32 y)
	{
		_mouse_x = x;
		_mouse_y = y;
	}
	inline KeyInfo* get_key_info()
	{
		return _keyinfo;
	}
	inline KeyInfo* get_last_key_info()
	{
		return _last_keyinfo;
	}
	void clear_states();
	/*clear mouse scroller state,just use for some system*/
	void clear_scroller();
	//
	void shutdown();
	int get_current_up_bit();
	int get_current_down_bit();
	int get_current_down_char();
	int get_current_up_char();
	FORCEINLINE bool get_move()
	{
		return _move;
	}

	void set_move(bool v);

	bool bdown;
	int up_bits;

	std::map<std::string, int> input_map;
protected:

private:
	friend Input;
	InputManager();
	int _current_down_bit;
	int _current_up_bit;
	int _current_down_char;
	int _current_up_char;

	KeyInfo _keyinfo[2];
	KeyInfo _last_keyinfo[2];
	bool init_key_map(char *mapfile);
	std::map<WIPKEY, int> _key_map;

	bool _bactive;

	i32 _mouse_x, _mouse_y;

	bool _move;
	static InputManager* _instance;
};

extern InputManager* g_input_manager;