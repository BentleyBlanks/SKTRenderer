#pragma once 
#include "RBBasedata.h"

class FastApp
{
public:
	//before system 
	virtual bool preinit() = 0;
	//after RHI
  virtual bool init() = 0;
	//per frame
	virtual void update(f32 dt) = 0;
	virtual void draw() = 0;
	//for init
	virtual void ter() = 0;
	//for preinit
	virtual void postter() = 0;
	
  void set_window(int w, int h){ ww = w; wh = h; }
  void set_back_buffer(struct ID3D11RenderTargetView* rt){ back_buffer_view = rt; }
  virtual void resize(int w, int h, bool bfullscreen){}
  FastApp() = delete;
  FastApp(class AppBase* app);
	virtual ~FastApp() = 0{};
	FastApp(const FastApp& o) = delete;
	FastApp& operator=(const FastApp& o) = delete;

protected:
  i32 ww; i32 wh;
  struct ID3D11RenderTargetView* back_buffer_view;
  class AppBase* base_app;
};


