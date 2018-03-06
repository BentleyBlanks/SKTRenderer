#include "App.h"
#include "windows.h"
#include "windowsx.h"
#include "Logger.h"
#include "InputManager.h"
#include "RHID3D11.h"
#include "D3D11Viewport.h"
#include "thirdpart/imgui/imgui.h"
#include "thirdpart/imgui/imgui_impl_dx11.h"
#include "DeferredShadingFastApp.h"
#include "ConservativeRasterizationFastApp.h"
#include "HairShadingFastApp.h"
#include "SkinShadingFastApp.h"
#include "CRFWRAP.h"
#include "Input.h"
#include "D3D11ShaderResources.h"
#include "CRFastApp.h"
#include "CRFastApp_new.h"
#include "DSFastApp.h"
#include "SceneLoadFastApp.h"
#include "SceneEditorFastApp.h"
#include "SVOFastApp.h"
#include "ModelViewFastApp.h"

//CRTAllocator* g_crt_allcator = new CRTAllocator();
HMODULE D3D11ShaderCompiler::CompilerDLL = 0;
pD3DCompile D3D11ShaderCompiler::compile_shader = nullptr;
pD3DReflect D3D11ShaderCompiler::reflect_shader = nullptr;
AppBase* g_base_app = nullptr;

void OnMouseDown(WPARAM btnState, int x, int y)
{
	//16 for middle 1 for left 2 for right
	int bits = g_input_manager->get_last_key_info()->key_bit;
	int bits_c = g_input_manager->get_last_key_info()[1].key_bit;
	bits |= btnState;
	switch (btnState)
	{
	case WIP_MOUSE_LBUTTON:
		lmouse_keep_going = true;
		break;
	case WIP_MOUSE_RBUTTON:
		rmouse_keep_going = true;
		break;
	case WIP_MOUSE_MBUTTON:
		mmouse_keep_going = true;
		break;
	}
	g_input_manager->update(bits_c, bits);
}

void OnMouseUp(WPARAM btnState, int x, int y)
{
	int bits = g_input_manager->get_last_key_info()->key_bit;
	int bits_c = g_input_manager->get_last_key_info()[1].key_bit;
	bits &= ~btnState;
	switch (btnState)
	{
	case WIP_MOUSE_LBUTTON:
		lmouse_keep_going = false;
		break;
	case WIP_MOUSE_RBUTTON:
		rmouse_keep_going = false;
		break;
	case WIP_MOUSE_MBUTTON:
		mmouse_keep_going = false;
		break;
	}

	g_input_manager->update(bits_c, bits);
}

void OnMouseMove(WPARAM btnState, int x, int y)
{
	g_input_manager->update_mouse((int)(short)x, (int)(short)y);
	g_input_manager->set_move(true);
}

void OnKeyDown(WPARAM btnState)
{
	int bits = g_input_manager->get_last_key_info()->key_bit;
	int bits_c = g_input_manager->get_last_key_info()[1].key_bit;
	switch (btnState)
	{
	case 'A':bits_c |= WIP_A;
		break;
	case 'B':bits_c |= WIP_B;
		break;
	case 'C':bits_c |= WIP_C;
		break;
	case 'D':bits_c |= WIP_D;
		break;
	case 'E':bits_c |= WIP_E;
		break;
	case 'F':bits_c |= WIP_F;
		break;
	case 'G':bits_c |= WIP_G;
		break;
	case 'H':bits_c |= WIP_H;
		break;
	case 'I':bits_c |= WIP_I;
		break;
	case 'J':bits_c |= WIP_J;
		break;
	case 'K':bits_c |= WIP_K;
		break;
	case 'L':bits_c |= WIP_L;
		break;
	case 'M':bits_c |= WIP_M;
		break;
	case 'N':bits_c |= WIP_N;
		break;
	case 'O':bits_c |= WIP_O;
		break;
	case 'P':bits_c |= WIP_P;
		break;
	case 'Q':bits_c |= WIP_Q;
		break;
	case 'R':bits_c |= WIP_R;
		break;
	case 'S':bits_c |= WIP_S;
		break;
	case 'T':bits_c |= WIP_T;
		break;
	case 'U':bits_c |= WIP_U;
		break;
	case 'V':bits_c |= WIP_V;
		break;
	case 'W':bits_c |= WIP_W;
		break;
	case 'X':bits_c |= WIP_X;
		break;
	case 'Y':bits_c |= WIP_Y;
		break;
	case 'Z':bits_c |= WIP_Z;
		break;
	}
	g_input_manager->update(bits_c, bits);
}

void OnKeyUp(WPARAM btnState)
{
	int bits = g_input_manager->get_last_key_info()->key_bit;
	int bits_c = g_input_manager->get_last_key_info()[1].key_bit;
	switch (btnState)
	{
	case 'A':bits_c &= ~WIP_A;
		break;
	case 'B':bits_c &= ~WIP_B;
		break;
	case 'C':bits_c &= ~WIP_C;
		break;
	case 'D':bits_c &= ~WIP_D;
		break;
	case 'E':bits_c &= ~WIP_E;
		break;
	case 'F':bits_c &= ~WIP_F;
		break;
	case 'G':bits_c &= ~WIP_G;
		break;
	case 'H':bits_c &= ~WIP_H;
		break;
	case 'I':bits_c &= ~WIP_I;
		break;
	case 'J':bits_c &= ~WIP_J;
		break;
	case 'K':bits_c &= ~WIP_K;
		break;
	case 'L':bits_c &= ~WIP_L;
		break;
	case 'M':bits_c &= ~WIP_M;
		break;
	case 'N':bits_c &= ~WIP_N;
		break;
	case 'O':bits_c &= ~WIP_O;
		break;
	case 'P':bits_c &= ~WIP_P;
		break;
	case 'Q':bits_c &= ~WIP_Q;
		break;
	case 'R':bits_c &= ~WIP_R;
		break;
	case 'S':bits_c &= ~WIP_S;
		break;
	case 'T':bits_c &= ~WIP_T;
		break;
	case 'U':bits_c &= ~WIP_U;
		break;
	case 'V':bits_c &= ~WIP_V;
		break;
	case 'W':bits_c &= ~WIP_W;
		break;
	case 'X':bits_c &= ~WIP_X;
		break;
	case 'Y':bits_c &= ~WIP_Y;
		break;
	case 'Z':bits_c &= ~WIP_Z;
		break;
	}
	g_input_manager->update(bits_c, bits);
}

extern LRESULT ImGui_ImplDX11_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK AppD3D11::WndProcMain(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ImGui_ImplDX11_WndProcHandler(hWnd, message, wParam, lParam);

	switch (message) {
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		OnKeyDown(wParam);

		break;
	case WM_KEYUP:
		OnKeyUp(wParam);

		break;
	case WM_LBUTTONDOWN:
		OnMouseDown(WIP_MOUSE_LBUTTON, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_LBUTTONDBLCLK:
		break;
	case WM_LBUTTONUP:
		OnMouseUp(WIP_MOUSE_LBUTTON, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

		break;
	case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
		OnMouseDown(WIP_MOUSE_MBUTTON, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_MBUTTONUP:
		OnMouseUp(WIP_MOUSE_MBUTTON, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
		OnMouseDown(WIP_MOUSE_RBUTTON, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_RBUTTONUP:
		OnMouseUp(WIP_MOUSE_RBUTTON, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_MOUSEMOVE: {
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

		break;
	}
	case WM_MOUSEWHEEL:

		break;
	case WM_ACTIVATE:

		break;
	case WM_SIZE: {
		break;
		//not surpport currentlly
		{
			
			FD3D11DynamicRHI* _rhid3d11 = (FD3D11DynamicRHI*)RBDynamicRHI::GetRHI(ERHITYPES::RHI_D3D11);
			auto device = _rhid3d11->GetDevice();
			auto context = _rhid3d11->GetDeviceContext();
			if (device&&wParam != SIZE_MAXIMIZED)
			{
				ImGui_ImplDX11_InvalidateDeviceObjects();
				g_base_app->resize((UINT)LOWORD(lParam), (UINT)HIWORD(lParam), false);
				ImGui_ImplDX11_CreateDeviceObjects();
				
			}
			

		}
		break;
	}
				  /*
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
		break;
		*/
	case WM_CHAR:

		break;
	case WM_IME_SETCONTEXT:
	case WM_IME_STARTCOMPOSITION:
	case WM_IME_COMPOSITION:
	case WM_IME_ENDCOMPOSITION:
	case WM_INPUTLANGCHANGE:
	case WM_IME_NOTIFY:

		break;
	case WM_HOTKEY:

		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

static bool ComputeWindowRect(RECT& out_rect, int w, int h)
{
	int monitor_width, monitor_height;
	int width, height;
	RECT work_area;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &work_area, 0);
	monitor_width = work_area.right;
	monitor_height = work_area.bottom;
	width = w;// (AppConfig::RESOLUTION[0] <= 1) ? (int)(AppConfig::RESOLUTION[0] * monitor_width) : (int)AppConfig::RESOLUTION[0];
	height = h;// (AppConfig::RESOLUTION[1] <= 1) ? (int)(AppConfig::RESOLUTION[1] * monitor_height) : (int)AppConfig::RESOLUTION[1];

	out_rect.left = (monitor_width - width) / 2;
	out_rect.right = out_rect.left + width;
	out_rect.top = (monitor_height - height) / 2;
	out_rect.bottom = out_rect.top + height;
	return true;
}


AppD3D11::AppD3D11()
{
	//create fast app
	//fastapp = new SkinShadingFastApp(this);
  //fastapp = new HairShadingFastApp(this);
	//fastapp = new DefferedShadingFastApp(this);
  //fastapp = new ConservativeRasterizationFastApp(this);
  //fastapp = new CRFastApp(this);
  //fastapp = new DSFastApp(this);
  //fastapp = new SceneLoadFastApp(this);
  fastapp = new SceneEditorFastApp(this);
  //fastapp = new CRFastApp_new(this);
  //fastapp = new SVOFastApp(this);
  //fastapp = new ModelViewFastApp(this);
  g_logger->debug(WIP_ERROR, "Release Reflect failed!!!!!!!!!!!!!\n!!!可能是写叶子节点的时候出现了bug！");

}

bool AppD3D11::init_windows(const char* name, int w, int h)
{
	if (!fastapp->preinit()) return false;
	if (!_init_window(name, w, h)) return false;
	if (!_init_d3d11(w, h)) return false;
  if (!D3D11ShaderCompiler::load_compiler("D3Dcompiler_47-x64.dll")) return false;
	if (!fastapp->init()) return false;
	
	
	if (!ImGui_ImplDX11_Init(_hwnd, ((FD3D11DynamicRHI*)_rhid3d11)->GetDevice(), ((FD3D11DynamicRHI*)_rhid3d11)->GetDeviceContext()))
	{
		g_logger->debug(WIP_ERROR, "Imgui init failed!");
		return false;
	}
	ww = w;
	wh = h;

	return true;
}

bool AppD3D11::_init_window(const char* name, int& w, int& h)
{
	const char* WINDOW_CLASS_NAME = "Maind3d";
	HINSTANCE hInstance = GetModuleHandle(0);
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_CLASSDC;
	wcex.lpfnWndProc = WndProcMain;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = 0;// icon_filename ? (HICON)LoadImage(nullptr, icon_filename, IMAGE_ICON, 64, 64, LR_LOADFROMFILE) : nullptr;
	wcex.hIconSm = 0;// icon_filename ? (HICON)LoadImage(nullptr, icon_filename, IMAGE_ICON, 16, 16, LR_LOADFROMFILE) : nullptr;
	wcex.hCursor = 0;// AppConfig::USE_SYSTEM_CURSOR ? LoadCursor(nullptr, IDC_ARROW) : nullptr;
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr; // MAKEINTRESOURCE(IDC_TRYWIN32);
	wcex.lpszClassName = WINDOW_CLASS_NAME;

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(0, "RegisterClass Failed.", 0, 0);
		return false;
	}
	DWORD style = WS_OVERLAPPEDWINDOW;
	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = w;
	rect.bottom = h;
	//回忆窗口上次关闭位置
	ComputeWindowRect(rect, w, h);
	//按照窗口风格调整窗口
	AdjustWindowRect(&rect, style, FALSE);
	_hwnd = CreateWindow(WINDOW_CLASS_NAME, name, style,
		rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top,
		NULL, NULL, hInstance, NULL);
	if (!_hwnd) return false;
	_dc = GetDC(_hwnd);
	RegisterHotKey(_hwnd, 0, MOD_ALT, VK_F4);
	ShowWindow(_hwnd, SW_SHOW);
	UpdateWindow(_hwnd);

	GetClientRect(_hwnd, &rect);

	fastapp->set_window(rect.right - rect.left, rect.bottom - rect.top);

	w = rect.right - rect.left;
	h = rect.bottom - rect.top;

	return true;
}

bool AppD3D11::_init_d3d11(int adjusted_w, int adjusted_h)
{
	_rhid3d11 = FD3D11DynamicRHI::GetRHI(ERHITYPES::RHI_D3D11);
	_d3d_viewport = _rhid3d11->RHICreateViewport(_hwnd, adjusted_w, adjusted_h, false, EPixelFormat::PF_R8G8B8A8);

	fastapp->set_back_buffer((ID3D11RenderTargetView*)_d3d_viewport->GetNativeBackBufferRT());

	/*
	TResourceArray<u16> rsa;
	rsa.push_back(0);
	rsa.push_back(1);
	rsa.push_back(2);
	RBRHIResourceCreateInfo cif(&rsa);
	auto index_buffer = _rhid3d11->RHICreateIndexBuffer(3, 3 * sizeof(u16), EBufferUsageFlags::BUF_Static, cif).DeRef();
	RBRHIResourceCreateInfo rf;
	auto vertex_buffer = _rhid3d11->RHICreateVertexBuffer(9 * sizeof(f32), EBufferUsageFlags::BUF_Static, rf).DeRef();
	f32* vptr = (f32*)_rhid3d11->RHILockVertexBuffer(vertex_buffer, 0, 9 * sizeof(f32), EResourceLockMode::RLM_WriteOnly);
	std::vector<f32> vs = { 0.f, 1.f, 1.f, -1.f, 0.f, 1.f, 1.f, 0.f, 1.f };
	memcpy(vptr, &vs[0], 9 * sizeof(f32));
	_rhid3d11->RHIUnlockVertexBuffer(vertex_buffer);
	*/

	return true;
}

int AppD3D11::run()
{
	MSG msg = { 0 };
	_timer.Reset();
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			f32 t = _timer.TotalTime();
			f32 dt;
			//120fps
			f32 fps = 1.f / 120.f;
			while (_timer.TotalTime() - t <= fps)
			{
				_timer.Tick();
			}
			dt = _timer.TotalTime() - t;
			ImGui_ImplDX11_NewFrame(ww, wh);
			bool mAppPaused = false;
			if (!mAppPaused)
			{
				/*
				CalculateFrameStats();
				UpdateScene(mTimer.DeltaTime());
				DrawScene();
				*/
				//fastapp->set_back_buffer((ID3D11RenderTargetView*)_d3d_viewport->GetNativeBackBufferRT());
				fastapp->update(dt);
				
				fastapp->draw();

				ImGui::Render();
				((RBD3D11Viewport*)_d3d_viewport.GetReference())->Present(false);
				//g_input_manager->clear_states();
				//g_input_manager->clear_scroller();

				int bits = g_input_manager->get_last_key_info()->key_bit;
				int bits_c = g_input_manager->get_last_key_info()[1].key_bit;
				g_input_manager->update(bits_c, bits);
				g_input_manager->set_move(false);
			}
			else
			{
				Sleep(100);
			}
		}
	}
	ImGui_ImplDX11_Shutdown();
	fastapp->ter();

	return (int)msg.wParam;
}



void AppD3D11::terminate()
{
  D3D11ShaderCompiler::unload();
	_rhid3d11->Shutdown();
	//AppBase::terminate();
	fastapp->postter();
}

AppD3D11::~AppD3D11()
{
	terminate();
	delete fastapp;
}

void AppD3D11::resize(int w, int h,bool bfullscreen)
{
	if (!_d3d_viewport) return;
  _rhid3d11->RHIResizeViewport(_d3d_viewport,w, h, bfullscreen);
  fastapp->set_back_buffer((ID3D11RenderTargetView*)_d3d_viewport->GetNativeBackBufferRT());
  //((RBD3D11Viewport*)_d3d_viewport.GetReference())->Resize(w, h, bfullscreen);
}

bool AppBase::init()
{
	g_logger->startup("./log/");
	if (!g_input_manager->startup("")) return false;
  g_base_app = this;
	return true;
}

void AppBase::terminate()
{
	g_input_manager->shutdown();
	g_logger->shutdown();
}

/** Fast app impl */
FastApp::FastApp(AppBase* app)
{
  base_app = app;
}