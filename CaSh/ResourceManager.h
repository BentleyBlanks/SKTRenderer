#pragma once
#include <map>
#include <string>

#define SUCC_SUB 0
#define FAIL 1
#define SUCC_RELEASE 2

struct TextureData
{
	int width;
	int height;
	int size;
	unsigned int channel /* = 4(currently)*/;
	unsigned int bpp;
	//refer to FreeImage type,not use now
	unsigned int image_type;
	unsigned int color_type;
};

struct SoundData
{

};

struct TextData
{
	enum TextContentType
	{
		E_BIN,
		E_ASCII,
		E_UNICODE,
    E_MEM,
    E_STRING
	};
	TextContentType conten_type;
};

struct FontData
{

};

struct OtherData
{

};


enum WIPResourceType
{
	TEXT,
	TEXTURE,
	SOUND,
	Font,
	OTHER
};

//note that all the handler instances are produced in WIPResourceManager,impossible for other places.
struct ResHandler
{
	//resource name
	std::string file_name;
	//类型
	//type
	WIPResourceType type;
	//引用计数
	//reference count
	unsigned int nref;
	//资源大小
	//size of resource
	unsigned int size;
	//指针
	//pointer to memory
	void* ptr;
	//附加数据
	//Extra data
	void* extra;
};
/*
资源类型:
文本/图片/声音
此类职责：
制造资源，统计内存使用，检测泄露，
这些资源没有使用定制的内存分配器，因为这些资源不会在游戏中频繁的分配和释放
一般是在需要load的时候进行load，直到游戏结束才释放。
克隆资源仅仅使用clone方法去复制资源，拒绝一切的句柄指针赋值，直接复制等等
Resource types:
text/texture/sound
Responsibility:
create resources,resource statistics,check memory leaks,
These resources will not use custom memory allocator,because of they will not alloc and release  frequently in memory
They are loaded when needed and keep in memory until the game exit.
Be sure only use clone method to copy ResHandler and its pointer.
DO NOT allow any assignment between handler and handler or the pointer and pointer.
*/
class WIPResourceManager
{
public:

	static WIPResourceManager* get_instance();
	bool startup();
	void shutdown();
	//return NULL if failed
	ResHandler* load_resource(const char* filename, WIPResourceType type = WIPResourceType::TEXTURE);
	//Force Delete ALL the Resource,it can be unsafe sometimes.
	void free_all();
	//be sure all the references has been DEREFERENCED.
	//free will return the state to check if the resouces had beeen deleted from memory
	int free(ResHandler* file, int size);
	ResHandler* clone(ResHandler* handler);

	//Manager information

protected:
	WIPResourceManager();
	~WIPResourceManager();
private:
	void add_ref(ResHandler* file);
	void remove_ref(ResHandler* file);
	void set_ref(ResHandler* file, int num);
	void delete_handler(ResHandler* handler);
	void free_resource(ResHandler* handler);
	ResHandler* alloc(const char* filename, WIPResourceType type);
	//don't set char* as the key of map,because map regard the key as a pointer not a string.
	//When the function pass "XXXX" as parameter,the pointer to "XXXX" may be the same because the optimization of compiler
	typedef std::map<std::string, ResHandler*> file2ptr_map;
	typedef std::map<ResHandler*, int> ref_map;
	file2ptr_map _map;
	ref_map _ref;
};

extern WIPResourceManager* g_res_manager;