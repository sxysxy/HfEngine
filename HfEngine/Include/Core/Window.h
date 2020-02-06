#pragma once
#include <ThirdParties.h>
#include <Core/GDevice.h>

HFENGINE_NAMESPACE_BEGIN

const UINT WM_EXITLOOP = (WM_USER + 2333);
const UINT WM_PROCESS_FLAG = (WM_USER + 2332);

class Window : public Utility::ReferredObject {
    std::wstring title;
    HWND _native_handle;
    int _width, _height;

    static const UINT wstyle = WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_THICKFRAME);
    static bool _native_inited;

    ComPtr<IDXGISwapChain> native_swap_chain;
public:
    static LRESULT CALLBACK _WndProcAsyncMove(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK _WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
    //pretend to be private.
    bool __ncl_button_down;
    RECT __ncl_down_rect;
    POINT __ncl_down_pos;
    //为了避免拖拽窗口导致消息处理过程阻塞。。。

    const HWND& native_handle = _native_handle;
    const int& width = _width, & height = _height;
    UINT style;
    bool async_move;


    Window() {
        _native_handle = 0;
        _width = _height = 0;
        style = wstyle;
        __ncl_button_down = false;
        async_move = false;
    }
    ~Window() {
        Uninitialize();
    }

    //this Initalize is for ruby extension, for RB_NEWOBJ_OF don't call the constructor function.
    template<typename ... _Arg>
    void Initialize(const _Arg& ..._arg) {
        _native_handle = 0;
        _width = _height = 0;
        style = wstyle;
        __ncl_button_down = false;
        Create(_arg ...);

        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof sd);
        sd.BufferCount = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.Windowed = 1;
        sd.OutputWindow = _native_handle;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        sd.BufferDesc.Width = _width;
        sd.BufferDesc.Height = _height;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        auto& native_device = GDevice::GetInstance()->native_device;

        HRESULT hr = S_FALSE;
        if (FAILED(hr = GDevice::GetInstance()->native_dxgi_factory->CreateSwapChain(native_device.Get(),
            &sd, &native_swap_chain)) || !native_swap_chain) { 
               //THROW_ERROR_CODE(std::runtime_error, "Fail to Create SwapChain, Error code:", hr);
            throw std::runtime_error(std::string("Fail to Create SwapChain, Error Code: ") + std::to_string(hr));
        }
        //Usually cause a _com_error because of DXGI_STATUS_OCCLUDED.
        //See https://msdn.microsoft.com/en-us/library/windows/desktop/cc308061(v=vs.85).aspx

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

    void Create(const std::wstring& _title, int w, int h);

    const std::wstring& GetTitle() const {
        return title;
    }

    void GetPosition(int* x, int* y) {
        assert(native_handle);
        RECT r;
        GetWindowRect(native_handle, &r);
        if (x)*x = r.left; if (y)*y = r.top;
    }
    void GetPosition(int& x, int& y) {
        assert(native_handle);
        RECT r;
        GetWindowRect(native_handle, &r);
        x = r.left, y = r.top;
    }


    void SetTitle(const std::wstring& t) {
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
    inline void SetAsyncMove(bool b) {
        async_move = b;
    }

    inline void SwapBuffers(int sync_level = 0) {
        native_swap_chain->Present(sync_level, 0);
    }
    inline void SetFullscreen(bool fullscreen) {
        native_swap_chain->SetFullscreenState(fullscreen, nullptr);
    }

    virtual void OnResized();
    virtual void OnClosed();
};

extern thread_local RClass* ClassWindow;
bool InjectWindowExtension();

HFENGINE_NAMESPACE_END