#include <Core/Window.h>
#include <Core/RubyVM.h>
#include <Core/GDevice.h>

HFENGINE_NAMESPACE_BEGIN

#ifdef _WIN64
#define SetWindowLongV SetWindowLongPtr
#define GetWindowLongV GetWindowLongPtr
#else 
#define SetWindowLongV SetWindowLong
#define GetWindowLongV GetWindowLong
#endif

bool Window::_native_inited = false;

std::unique_ptr<DirectX::Mouse> Window::mouse;

LRESULT CALLBACK Window::_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Window* w = (Window*)GetWindowLongV(hWnd, GWLP_USERDATA);
    if (w && w->destroyed) 
        return 0;
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
        SetWindowLongV(hWnd, GWLP_USERDATA, (LONG_PTR)pc->lpCreateParams);
        return 0;
    }
    case WM_SETFOCUS:
    {
        w->SetFocused(true);
        auto& mouse = DirectX::Mouse::Get();
        mouse.SetWindow(w->native_handle);
        return 0;
    }
    case WM_KILLFOCUS:
    {
        w->SetFocused(false);
        return 0;
    }
    case WM_INPUT:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_XBUTTONUP:
    case WM_XBUTTONDOWN:
    case WM_MOUSEWHEEL:
    case WM_MOUSEHOVER:
    case WM_MOUSEMOVE:
    case WM_NCLBUTTONUP:
    case WM_NCMOUSEMOVE:
    case WM_NCRBUTTONDOWN:
    case WM_SYSCOMMAND:
    {
        if (!w->IsFocused())
            return 0;
        auto& mouse = Window::mouse->Get();
        mouse.ProcessMessage(uMsg, wParam, lParam);
        {
            if (w->async_move)
                return _WndProcAsyncMove(hWnd, uMsg, wParam, lParam);
            else return 0;
        }
    }
    
    case WM_DESTROY:
    case WM_CLOSE:
    {
        w->OnClosed();
        return 0;
    }
    return 0;
    case WM_SIZE:
    {
        //if((wParam & SIZE_RESTORED) || (wParam & SIZE_MAXIMIZED) || (wParam & SIZE_MAXSHOW)) {
        
        {
            w->OnResized();
        }
        return 0;
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
    if (r.right - r.left == 0 || r.bottom - r.top == 0)
        return;
    _width = r.right - r.left;
    _height = r.bottom - r.top;

    if (!native_swap_chain)
        return;

    GDevice::GetInstance()->Lock();
    
    HRESULT hr = native_swap_chain->ResizeBuffers(1, _width, _height,
        DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    if (FAILED(hr)) {
        THROW_ERROR_CODE(std::runtime_error, "Fail to resize swapchain buffers, Error code:", hr);
    }

    ComPtr<ID3D11Texture2D> native_backbuffer;
    native_swap_chain->GetBuffer(0,
        __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(native_backbuffer.GetAddressOf()));

    if (!native_backbuffer) {
        THROW_ERROR_CODE(std::runtime_error, "Fail to get swapchain buffers, Error code:", hr);
    }
    back_canvas->CreateFromNativeTexture2D(std::move(native_backbuffer));

    GDevice::GetInstance()->UnLock();
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

    RECT crect = { 0, 0, _width, _height };
    AdjustWindowRect(&crect, style, false);
    int cw = crect.right - crect.left;
    int ch = crect.bottom - crect.top;
    _native_handle = CreateWindowW(L"23333", title.c_str(), style,
        (GetSystemMetrics(SM_CXSCREEN) - cw) >> 1,
        (GetSystemMetrics(SM_CYSCREEN) - ch) >> 1,
        cw, ch,
        0, 0, instance, this);
}

//special for ruby extension
static mrb_data_type ClassCanvasSpecialDataType = mrb_data_type{ "Canvas", [](mrb_state* mrb, void* ptr) {
} };
class Window2 : public Window {
public:
    mrb_value window_obj;
    mrb_value canvas_obj;

    Window2() : Window() {
        canvas_obj = mrb_obj_value(mrb_data_object_alloc(currentRubyVM->GetRuby(),
            ClassCanvas, back_canvas.Get(), &ClassCanvasSpecialDataType));
    }

    void OnResized() {
        Window::OnResized();
        try {
        mrb_funcall(currentRubyVM->GetRuby(), window_obj, "call_handler", 1,
            mrb_symbol_value(mrb_intern_cstr(currentRubyVM->GetRuby(), "resized")));
        }
        catch (...){

        }
    }

    void OnClosed() {
        try {
        mrb_funcall(currentRubyVM->GetRuby(), window_obj, "call_handler", 1, 
            mrb_symbol_value(mrb_intern_cstr(currentRubyVM->GetRuby(), "closed")));
        }
        catch (...) {

        }
    }
};

//extension:
thread_local RClass* ClassWindow;

mrb_data_type ClassWindowDataType = mrb_data_type{ "Window", [](mrb_state* mrb, void* ptr) {
    static_cast<Window*>(ptr)->SubRefer();
} };

/*[DOCUMENT]
method: HEG::Window::new(title : String, width : Fixnum, height : Fixnum) -> self
note: Create a titled window with specified size
*/
static mrb_value ClassWindow_new(mrb_state* mrb, mrb_value klass) {
    mrb_value title;
    mrb_int w, h;
    mrb_get_args(mrb, "Sii", &title, &w, &h);
    const char* atitle = RSTR_PTR(mrb_str_ptr(title));
    std::wstring wtitle;
    U8ToU16(atitle, wtitle);
    auto window = new Window2();
    window->Initialize(wtitle, (int)w, (int)h);
    window->AddRefer();
    mrb_value self = mrb_obj_value(mrb_data_object_alloc(mrb, ClassWindow, window, &ClassWindowDataType));
    window->window_obj = self;
    mrb_iv_set(mrb, self, mrb_intern_cstr(mrb, "@handlers"), mrb_hash_new(mrb));
    return self;
}

/*[DOCUMENT]
method: HEG::Window#show -> self
note: Make the window visible
*/
static mrb_value ClassWindow_show(mrb_state* mrb, mrb_value self) {
    GetNativeObject<HEG::Window>(self)->Show();
    return self;
}

/*[DOCUMENT]
method: HEG::Window#hide -> self
note: Make the window invisible
*/
static mrb_value ClassWindow_hide(mrb_state* mrb, mrb_value self) {
    GetNativeObject<HEG::Window>(self)->Hide();
    return self;
}

/*[DOCUMENT]
method: HEG::Window#fixed(fixed : true or false) -> self
note: Fix the size of window if fixed is true
*/
static mrb_value ClassWindow_fixed(mrb_state* mrb, mrb_value self) {
    mrb_bool fixed;
    mrb_get_args(mrb, "b", &fixed);
    GetNativeObject<HEG::Window>(self)->SetFixed(fixed);
    return self;
}

/*[DOCUMENT]
method: HEG::Window#call_handler(handler_name : Symbol) -> self or return value of called handler
note: Call the hanlder by its name. For example call_handler(:closed), call_handler(:resized).
If the handler exist, it will return its return value, or it will return self
*/
static mrb_value ClassWindow_call_handler(mrb_state* mrb, mrb_value self) {
    mrb_value key;
    mrb_get_args(mrb, "o", &key);
    mrb_value handlers = mrb_iv_get(mrb, self, mrb_intern_cstr(mrb, "@handlers"));
    mrb_value handler = mrb_hash_get(mrb, handlers, key);
    if (mrb_respond_to(mrb, handler, mrb_intern_cstr(mrb, "call"))) {
        return mrb_funcall(mrb, handler, "call", 0);
    }
    else return self;
}

/*[DOCUMENT]
method: HEG::Window#handle(hanlder_name : Symbol) {block} -> self
note: Set the hanlder of handler_name to the given block
*/
static mrb_value ClassWindow_handle(mrb_state* mrb, mrb_value self) {
    mrb_value key, block;
    mrb_get_args(mrb, "o&!", &key, &block);
    mrb_value handlers = mrb_iv_get(mrb, self, mrb_intern_cstr(mrb, "@handlers"));
    mrb_hash_set(mrb, handlers, key, block);
    return self;
}

/*[DOCUMENT]
method: HEG::Window#resize(width : Fixnum, height : Fixnum) -> self
note: Set new size of the window
*/
static mrb_value ClassWindow_resize(mrb_state* mrb, mrb_value self) {
    mrb_int w, h;
    mrb_get_args(mrb, "ii", &w, &h);
    GetNativeObject<HEG::Window>(self)->Resize((int)w, (int)h);
    return self;
}

/*[DOCUMENT]
method: HEG::Window#swap_buffers -> self
note: Swap the onscreen buffer and the offscreen buffer and present the image on the window.
*/
static mrb_value ClassWindow_swap_buffers(mrb_state* mrb, mrb_value self) {
    mrb_int argc;
    mrb_value* argv;
    mrb_get_args(mrb, "*!", &argv, &argc);
    mrb_int level = 0;
    if (argc > 0) {
        level = mrb_fixnum(argv[0]);
    }
    GetNativeObject<HEG::Window>(self)->SwapBuffers((int)level);
    return self;
}

/*[DOCUMENT]
method: HEG::Window#fullscreen(fullscreen : true or false) -> self
note: Set fullscreen or windowed.
*/
static mrb_value ClassWindow_fullsreen(mrb_state* mrb, mrb_value self) {
    mrb_bool f;
    mrb_get_args(mrb, "b", &f);
    GetNativeObject<HEG::Window>(self)->SetFullscreen(f);
    return self;
}

/*[DOCUMENT]
method: HEG::Window#entitle(title : String) -> self
note: Set the title of the window
*/
static mrb_value ClassWindow_entitle(mrb_state* mrb, mrb_value self) {
    mrb_value t;
    mrb_get_args(mrb, "S", &t);
    char* titlea = RSTRING_PTR(t);
    std::wstring titlew;
    U8ToU16(titlea, titlew);
    GetNativeObject<HEG::Window>(self)->SetTitle(titlew);
    return self;
}

/*[DOCUMENT]
method: HEG::Window#width -> w : Fixnum
note: Get the width of client area in the window.
*/
mrb_value ClassWindow_width(mrb_state* state, mrb_value self) {
    return mrb_fixnum_value(GetNativeObject<Window>(self)->width);
}

/*[DOCUMENT]
method: HEG::Window#height -> h : Fixnum
note: Get the height of client area in the window.
*/
mrb_value ClassWindow_height(mrb_state* state, mrb_value self) {
    return mrb_fixnum_value(GetNativeObject<Window>(self)->height);
}

/*[DOCUMENT]
method: HEG::Window#canvas -> canvas : Canvas
note: Get the canvas of window.
*/
mrb_value ClassWindow_canvas(mrb_state* state, mrb_value self) {
    return GetNativeObject<Window2>(self)->canvas_obj;
}

bool InjectWindowExtension() {
    const RubyVM* vm = currentRubyVM;
    RClass* HEG = mrb_define_module(vm->GetRuby(), "HEG");
    RClass* Object = vm->GetRuby()->object_class;
    ClassWindow = mrb_define_class_under(vm->GetRuby(), HEG, "Window", Object);
    mrb_define_class_method(vm->GetRuby(), ClassWindow, "new", ClassWindow_new, MRB_ARGS_REQ(3));
    mrb_define_method(vm->GetRuby(), ClassWindow, "show", ClassWindow_show, MRB_ARGS_NONE());
    mrb_define_method(vm->GetRuby(), ClassWindow, "hide", ClassWindow_hide, MRB_ARGS_NONE());
    mrb_define_method(vm->GetRuby(), ClassWindow, "fixed", ClassWindow_fixed, MRB_ARGS_REQ(1));
    mrb_define_method(vm->GetRuby(), ClassWindow, "call_handler", ClassWindow_call_handler, MRB_ARGS_REQ(1));
    mrb_define_method(vm->GetRuby(), ClassWindow, "handle", ClassWindow_handle, MRB_ARGS_REQ(1));
    mrb_define_method(vm->GetRuby(), ClassWindow, "resize", ClassWindow_resize, MRB_ARGS_REQ(2));
    mrb_define_method(vm->GetRuby(), ClassWindow, "swap_buffers", ClassWindow_swap_buffers, MRB_ARGS_ANY());
    mrb_define_method(vm->GetRuby(), ClassWindow, "fullscreen", ClassWindow_fullsreen, MRB_ARGS_REQ(1));
    mrb_define_method(vm->GetRuby(), ClassWindow, "entitle", ClassWindow_entitle, MRB_ARGS_REQ(1));
    mrb_define_method(vm->GetRuby(), ClassWindow, "width", ClassWindow_width, MRB_ARGS_NONE());
    mrb_define_method(vm->GetRuby(), ClassWindow, "height", ClassWindow_height, MRB_ARGS_NONE());
    mrb_define_method(vm->GetRuby(), ClassWindow, "canvas", ClassWindow_canvas, MRB_ARGS_NONE());
    return true;
}

HFENGINE_NAMESPACE_END