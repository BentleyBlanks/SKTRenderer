#pragma once
#include "Windows.h"
#include "AppTimer.h"
#include "RefCount.h"
#include "D3D11Viewport.h"

class AppBase
{
public:
  AppBase(){ init(); }
  virtual ~AppBase() = 0
  { 
    terminate(); 
  }
  virtual bool init_windows(const char* name, int w, int h) = 0;
  virtual bool init();
  virtual void terminate();
  virtual int run() = 0;
  virtual void resize(int w, int h, bool bfull) = 0;
  virtual class RBDynamicRHI* get_rhi() = 0;
  
  FORCEINLINE f32 get_time_in_millisecond(){ return _timer.TotalTime()*1000.f; }
protected:
  GameTimer _timer;
};

class AppD3D11 : public AppBase
{
public:
	AppD3D11();
	~AppD3D11();
	virtual bool init_windows(const char* name, int w, int h) override;
	virtual int run() override;
  void resize(int w,int h,bool bfull);
  FORCEINLINE RBDynamicRHI* get_rhi(){ return _rhid3d11; }
protected:
private:
	bool _init_window(const char* name, int& out_w, int& out_h);
	bool _init_d3d11(int adjusted_w, int adjusted_h);
	virtual void terminate();
  static LRESULT CALLBACK WndProcMain(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	HWND _hwnd;
	HDC _dc;
	class RBDynamicRHI* _rhid3d11;
	TRefCountPtr<RBRHIViewport> _d3d_viewport;
	class FastApp* fastapp;
	int ww, wh;
};

extern AppBase* g_base_app;