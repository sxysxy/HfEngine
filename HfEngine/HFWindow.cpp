#include "HFWindow.h"
#include "extension.h"

#ifdef _WIN64
#define SetWindowLongV SetWindowLongPtr
#define GetWindowLongV GetWindowLongPtr
#else 
#define SetWindowLongV SetWindowLong
#define GetWindowLongV GetWindowLong
#endif

bool HFWindow::_native_inited = false;
LRESULT CALLBACK HFWindow::_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
	case WM_CREATE:
		{
			CREATESTRUCT * pc = (CREATESTRUCT *)lParam;
            HFWindow * w = (HFWindow *)pc->lpCreateParams;
			SetWindowLongV(hWnd, GWLP_USERDATA, (PTR_VALUE_T)pc->lpCreateParams);
			return 0;
		}
	case WM_DESTROY:
	case WM_CLOSE:
		{
			HFWindow * w = (HFWindow *)GetWindowLongV(hWnd, GWLP_USERDATA);
            w->OnClosed();
            return 0;
		}
		return 0;
	case WM_SIZE:
		{
			HFWindow * w = (HFWindow *)GetWindowLongV(hWnd, GWLP_USERDATA);
            w->OnResized();
			return 0;
		}
    case WM_SYSCOMMAND: 
        {
            HFWindow * w = (HFWindow *)GetWindowLongV(hWnd, GWLP_USERDATA);
            switch (wParam & 0xfff0){
            case SC_MOVE:
                if (!w->__ncl_button_down) {
                    w->__ncl_button_down = true;
                    GetWindowRect(hWnd, &w->__ncl_down_rect);
                    GetCursorPos(&w->__ncl_down_pos);
                    SetCapture(hWnd);
                    return 0;
                }
            
            case SC_KEYMENU:
            case SC_MOUSEMENU:
                return 0;
            default:
                break;
            }
            break;  //not return 0;
        }
    case WM_LBUTTONUP:
    case WM_NCLBUTTONUP:
        {
            HFWindow *w = (HFWindow *)GetWindowLongV(hWnd, GWLP_USERDATA);
            w->__ncl_button_down = false;
            ReleaseCapture();
            break;
        }
    case WM_NCMOUSEMOVE:
    case WM_MOUSEMOVE:
        {
            HFWindow *w = (HFWindow *)GetWindowLongV(hWnd, GWLP_USERDATA);
            if (w->__ncl_button_down) {
                POINT pt;
                GetCursorPos(&pt);
                const int dx = w->__ncl_down_pos.x - pt.x;
                const int dy = w->__ncl_down_pos.y - pt.y;
                w->__ncl_down_rect.left -= dx;
                w->__ncl_down_rect.top -= dy;
                w->__ncl_down_pos = pt;
                w->MoveTo(w->__ncl_down_rect.left, w->__ncl_down_rect.top);
            }
            break;
        }
    case WM_NCRBUTTONDOWN:
        return 0;
    default:
        break;
	}
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
void HFWindow::OnResized() {
	assert(_native_handle);

	RECT r;
	GetClientRect(_native_handle, &r);
	_width = r.right - r.left;
	_height = r.bottom - r.top;
}

void HFWindow::OnClosed() {
    PostQuitMessage(0);
}

void HFWindow::Create(const cstring &_title, int w, int h) {
	_width = w, _height = h;
	title = _title;
	HINSTANCE instance = GetModuleHandle(0);

	if(!_native_inited){
		WNDCLASS wc;
		RtlZeroMemory(&wc, sizeof(wc));
		wc.hbrBackground = (HBRUSH)(GetStockObject(BLACK_BRUSH));
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
		wc.hInstance = instance;
		wc.lpfnWndProc = _WndProc;
		wc.lpszClassName = TEXT("23333");
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		RegisterClass(&wc);
		_native_inited = true;
	}

    RECT crect = { 0, 0, width, height };
    AdjustWindowRect(&crect, style, false);
    int cw = crect.right - crect.left;
    int ch = crect.bottom - crect.top;
    _native_handle = CreateWindow(TEXT("23333"), title.c_str(), style,
        (GetSystemMetrics(SM_CXSCREEN) - cw) >> 1,
        (GetSystemMetrics(SM_CYSCREEN) - ch) >> 1,
        cw, ch,
        0, 0, instance, this);
}

namespace Ext {
	namespace HFWindow{
		using namespace Utility;

		void RHFWindow::OnResized() {
			HFWindow::OnResized();
			rb_funcall(self, rb_intern("call_handler"), 1, ID2SYM(rb_intern("on_resized")));
		}

		void RHFWindow::OnClosed() {
			//HFWindow::OnClosed();
			int s = 0;
			rb_protect([](VALUE obj) -> VALUE {return rb_funcall(obj, rb_intern("call_handler"), 1, ID2SYM(rb_intern("on_closed"))); }, self, &s);
			if (s) {
				rb_funcall(rb_mKernel, rb_intern("msgbox"), 1, rb_errinfo());
			}
		}
	

		VALUE klass;
		void Delete(RHFWindow *wnd) {
			wnd->SubRefer();
		}

		VALUE New(VALUE klass) {
			RHFWindow *wnd = new RHFWindow;
            wnd->AddRefer();
			return Data_Wrap_Struct(klass, nullptr, Delete, wnd);
		}

		static VALUE initialize(VALUE self, VALUE title, VALUE w, VALUE h) {
			auto *wnd = GetNativeObject<RHFWindow>(self);
			wnd->self = self;
			std::wstring wtitle;
			U8ToU16(rb_string_value_cstr(&title), wtitle);
			wnd->Initialize(wtitle.c_str(), FIX2INT(w), FIX2INT(h));
			rb_iv_set(self, "@handlers", rb_hash_new());
			//---
			if (rb_block_given_p()) {
				// VALUE rb_obj_instance_eval(int argc, VALUE *argv, VALUE self)
				rb_obj_instance_eval(0, nullptr, self);
			}
			return self;
		}

		static VALUE show(VALUE self) {
			auto *wnd = GetNativeObject<RHFWindow>(self);
			wnd->Show();
			return self;
		}

		static VALUE hide(VALUE self) {
			auto *wnd = GetNativeObject<RHFWindow>(self);
			wnd->Hide();
			return self;
		}

		static VALUE native_handle(VALUE self) {
			auto *wnd = GetNativeObject<RHFWindow>(self);
			return INT2FIX(wnd->native_handle);
		}

		static VALUE set_title(VALUE self, VALUE title) {
			auto *wnd = GetNativeObject<RHFWindow>(self);
			std::wstring wtitle;
			U8ToU16(rb_string_value_cstr(&title), wtitle);
			wnd->SetTitle(wtitle);
			return self;
		}

		static VALUE resize(VALUE self, VALUE w, VALUE h) {
			auto *wnd = GetNativeObject<RHFWindow>(self);
			wnd->Resize(FIX2INT(w), FIX2INT(h));
			return self;
		}

        static VALUE width(VALUE self) {
            auto *wnd = GetNativeObject<RHFWindow>(self);
            return INT2FIX(wnd->width);
        }

        static VALUE height(VALUE self) {
            auto *wnd = GetNativeObject<RHFWindow>(self);
            return INT2FIX(wnd->height);
        }

        static VALUE set_fixed(VALUE self, VALUE f) {
            auto *wnd = GetNativeObject<RHFWindow>(self);
            wnd->SetFixed(f == Qtrue);
            return self;
        }

        static VALUE moveto(VALUE self, VALUE x, VALUE y) {
            auto *wnd = GetNativeObject<RHFWindow>(self);
            wnd->MoveTo(FIX2INT(x), FIX2INT(y));
            return self;
        }

        static VALUE get_position(VALUE self) {
            auto window = GetNativeObject<RHFWindow>(self);
            int x, y;
            window->GetPosition(x, y);
            return rb_ary_new3(2, INT2FIX(x), INT2FIX(y));
        }

		void Init() {
			klass = rb_define_class("HFWindow", rb_cObject);
			rb_define_alloc_func(klass, New);
			rb_define_method(klass, "initialize", (rubyfunc)initialize, 3);
			rb_define_method(klass, "show", (rubyfunc)show, 0);
			rb_define_method(klass, "hide", (rubyfunc)hide, 0);
			rb_define_method(klass, "native_handle", (rubyfunc)native_handle, 0);
			rb_define_method(klass, "set_title", (rubyfunc)set_title, 1);
			rb_define_method(klass, "resize", (rubyfunc)resize, 2);
            rb_define_method(klass, "width", (rubyfunc)width, 0);
            rb_define_method(klass, "height", (rubyfunc)height, 0);
		    rb_define_method(klass, "set_fixed", (rubyfunc)set_fixed, 1);
            rb_define_method(klass, "moveto", (rubyfunc)moveto, 2);
            rb_define_alias(klass, "move_to", "moveto");
            rb_define_method(klass, "get_position", (rubyfunc)get_position, 0);
		}
	}
}
