#pragma once
#include "stdafx.h"
#include "extension.h"
#include "./Utility/NativeThread.h"

const UINT WM_EXITLOOP = (WM_USER + 2333);
const UINT WM_PROCESS_FLAG = (WM_USER+2332);

class HFWindow : public Utility::ReferredObject{
	cstring title;
	HWND _native_handle;
	int _width, _height;

	static const UINT wstyle = WS_OVERLAPPEDWINDOW  & ~(WS_MAXIMIZEBOX | WS_THICKFRAME);
	static bool _native_inited;

public:
   static LRESULT CALLBACK _WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
    //pretend to be private.
    bool __ncl_button_down;
    RECT __ncl_down_rect;
    POINT __ncl_down_pos;
    //为了避免拖拽窗口导致消息处理过程阻塞。。。

	const HWND &native_handle = _native_handle;
	const int &width = _width, &height = _height;
    UINT style;


	HFWindow() {
		_native_handle = 0;
		_width = _height = 0;
        style = wstyle;
        __ncl_button_down = false;
	}
	HFWindow(const cstring &_title, int w, int h) :HFWindow() {
		Initialize(_title, w, h);
	}
	~HFWindow() {
		Uninitialize();
	}

	//this Initalize is for ruby extension, for RB_NEWOBJ_OF don't call the constructor function.
	template<typename ... _Arg>
	void Initialize(const _Arg & ..._arg) {
		_native_handle = 0;
		_width = _height = 0;
        style = wstyle;
        __ncl_button_down = false;
		Create(_arg ...);
	}
	//destructor.
	void Uninitialize() {
		title.~basic_string();
		if (_native_handle) {
			DestroyWindow(_native_handle);
			_native_handle = 0;
		}
	}
	virtual void Release() {
        Uninitialize();
	}

	void Create(const cstring &_title, int w, int h);

	cstring GetTitle() {
		return title;
	}
   
    void GetPosition(int *x, int *y) {
        assert(native_handle);
        RECT r;
        GetWindowRect(native_handle, &r);
        if(x)*x = r.left; if(y)*y = r.top;
    }
    void GetPosition(int &x, int &y) {
        assert(native_handle);
        RECT r;
        GetWindowRect(native_handle, &r);
        x = r.left, y = r.top;
    }


    void SetTitle(const cstring &t) {
        assert(_native_handle);

        title = t;
        SetWindowText(_native_handle, title.c_str());
    }
    void Show() {
        assert(_native_handle);

        ShowWindow(_native_handle, SW_NORMAL);
    }
    void Hide() {
        assert(_native_handle);

        ShowWindow(_native_handle, SW_HIDE);
    }
    void SetFixed(bool fixed) {
        UINT s = GetWindowLong(native_handle, GWL_STYLE);
        int w = _width, h = _height;
        if (fixed) {
            s &= ~(WS_MAXIMIZEBOX | WS_THICKFRAME);
            SetWindowLong(native_handle, GWL_STYLE, s);
        }
        else {
            s |= (WS_MAXIMIZEBOX | WS_THICKFRAME);
            SetWindowLong(native_handle, GWL_STYLE, s);
        }
        Resize(w, h);
    }
    void MoveTo(int x, int y) {
        SetWindowPos(native_handle, 0, x, y, 0, 0, SWP_NOSIZE);
    }

    void Resize(int w, int h) {
        assert(_native_handle);

        RECT r = { 0, 0, w, h };
        AdjustWindowRect(&r, style, false);
        SetWindowPos(_native_handle, 0, 0, 0, r.right - r.left, r.bottom - r.top, SWP_NOMOVE);
    }

	virtual void OnResized();
	virtual void OnClosed();
};

namespace Ext {
	namespace HFWindow{

		class RHFWindow : public HFWindow {
		public:
			VALUE self;
			
			virtual void OnResized();
			virtual void OnClosed();
			
			RHFWindow() : HFWindow() {
				self = 0;
			}
		};

		extern VALUE klass;
		void Init();
		VALUE New(VALUE klass);
	}
}

