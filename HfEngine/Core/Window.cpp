#include <Core.h>

HFENGINE_NAMESPACE_BEGIN

#ifdef _WIN64
#define SetWindowLongV SetWindowLongPtr
#define GetWindowLongV GetWindowLongPtr
#else 
#define SetWindowLongV SetWindowLong
#define GetWindowLongV GetWindowLong
#endif

bool Window::_native_inited = false;

LRESULT CALLBACK Window::_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        auto HDC = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_CREATE:
    {
        CREATESTRUCT* pc = (CREATESTRUCT*)lParam;
        Window* w = (Window*)pc->lpCreateParams;
        SetWindowLongV(hWnd, GWLP_USERDATA, (LONG_PTR)pc->lpCreateParams);
        return 0;
    }
    case WM_DESTROY:
    case WM_CLOSE:
    {
        Window* w = (Window*)GetWindowLongV(hWnd, GWLP_USERDATA);
        w->OnClosed();
        return 0;
    }
    return 0;
    case WM_SIZE:
    {
        Window* w = (Window*)GetWindowLongV(hWnd, GWLP_USERDATA);
        w->OnResized();
        return 0;
    }
    case WM_SYSCOMMAND:
    case WM_LBUTTONUP:
    case WM_NCLBUTTONUP:
    case WM_NCMOUSEMOVE:
    case WM_MOUSEMOVE:
    case WM_NCRBUTTONDOWN:
    {
        Window* w = (Window*)GetWindowLongV(hWnd, GWLP_USERDATA);
        if (w->async_move)
            return _WndProcAsyncMove(hWnd, uMsg, wParam, lParam);
        else return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
        break;
    }
}

LRESULT CALLBACK Window::_WndProcAsyncMove(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg)
    {
    case WM_SYSCOMMAND:
    {
        Window* w = (Window*)GetWindowLongV(hWnd, GWLP_USERDATA);
        switch (wParam & 0xfff0) {
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
        Window* w = (Window*)GetWindowLongV(hWnd, GWLP_USERDATA);
        w->__ncl_button_down = false;
        ReleaseCapture();
        break;
    }
    case WM_NCMOUSEMOVE:
    case WM_MOUSEMOVE:
    {
        Window* w = (Window*)GetWindowLongV(hWnd, GWLP_USERDATA);
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
void Window::OnResized() {
    assert(_native_handle);

    RECT r;
    GetClientRect(_native_handle, &r);
    _width = r.right - r.left;
    _height = r.bottom - r.top;
}

void Window::OnClosed() {
    PostQuitMessage(0);
}

void Window::Create(const std::wstring& _title, int w, int h) {
    _width = w, _height = h;
    title = _title;
    HINSTANCE instance = GetModuleHandle(0);

    if (!_native_inited) {
        WNDCLASS wc;
        RtlZeroMemory(&wc, sizeof(wc));
        wc.hbrBackground = (HBRUSH)(GetStockObject(BLACK_BRUSH));
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
        wc.hInstance = instance;
        wc.lpfnWndProc = _WndProc;
        wc.lpszClassName = L"23333";
        wc.style = CS_OWNDC;
        RegisterClass(&wc);
        _native_inited = true;
    }

    RECT crect = { 0, 0, width, height };
    AdjustWindowRect(&crect, style, false);
    int cw = crect.right - crect.left;
    int ch = crect.bottom - crect.top;
    _native_handle = CreateWindowW(L"23333", title.c_str(), style,
        (GetSystemMetrics(SM_CXSCREEN) - cw) >> 1,
        (GetSystemMetrics(SM_CYSCREEN) - ch) >> 1,
        cw, ch,
        0, 0, instance, this);
}


//extension:
thread_local RClass* ClassWindow;

static mrb_value ClassWindow_new(mrb_state* mrb, mrb_value self) {
    mrb_value obj = mrb_obj_new(mrb, ClassWindow, 0, nullptr);
    obj.value.p = new Window();
    return obj;
}

static mrb_value ClassWindow_initalize(mrb_state* mrb, mrb_value self) {
    mrb_value title;
    int w, h;
    mrb_get_args(mrb, "sii", &title, &w, &h);
    const char* atitle = mrb_string_cstr(mrb, title);
    std::wstring wtitle;
    U8ToU16(atitle, wtitle);
    static_cast<Window*>(self.value.p)->Initialize(wtitle, w, h);
    return self;
}

bool InjectWindowExtension() {
    RubyVM* vm = currentRubyVM.get();
    RClass* HEG = mrb_define_module(vm->GetRuby(), "HEG");
    RClass* Object = vm->GetRuby()->object_class;
    ClassWindow = mrb_define_class_under(vm->GetRuby(), HEG, "Window", Object);
    mrb_define_class_method(vm->GetRuby(), ClassWindow, "new", ClassWindow_new, MRB_ARGS_NONE());
    mrb_define_method(vm->GetRuby(), ClassWindow, "initialize", ClassWindow_initalize, MRB_ARGS_REQ(3));

    return true;
}

HFENGINE_NAMESPACE_END