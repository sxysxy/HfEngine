#pragma once
#include <ThirdParties.h>

HFENGINE_NAMESPACE_BEGIN

class Canvas : public Utility::ReferredObject {
    int _width, _height;
    void CreateViews();
public:
    ComPtr<ID3D11Texture2D> native_texture2d;
    ComPtr<ID3D11Texture2D> native_depth_stencil_texture;
    ComPtr<ID3D11ShaderResourceView> native_shader_resource_view;
    ComPtr<ID3D11RenderTargetView> native_render_target_view;
    ComPtr<ID3D11DepthStencilView> native_depth_stencil_view;

    const int& width = _width, & height = _height;
    void Initialize() {};
    void Initialize(const std::wstring& filename);
    void Initialize(int w, int h, const void* init_data = nullptr);
    template<class T = void>
    void Initialize(int w, int h, const T* init_data = nullptr) {
        Initialize(w, h, (void*)init_data);
    }
    void CreateFromNativeTexture2D(ComPtr<ID3D11Texture2D> tex) {
        UnInitialize();
        native_texture2d = std::move(tex);
        D3D11_TEXTURE2D_DESC desc;
        native_texture2d->GetDesc(&desc);
        _width = desc.Width;
        _height = desc.Height;
        CreateViews();
    }

    void UnInitialize() {
        native_texture2d.ReleaseAndGetAddressOf();      //ComPtr & operator will call release...
        native_depth_stencil_texture.ReleaseAndGetAddressOf();
        native_shader_resource_view.ReleaseAndGetAddressOf();
        native_render_target_view.ReleaseAndGetAddressOf();
        native_depth_stencil_view.ReleaseAndGetAddressOf();
    }
    virtual void Release() {
        UnInitialize();
    }
};

extern thread_local RClass* ClassCanvas;

bool InjectCanvasExtension();

HFENGINE_NAMESPACE_END