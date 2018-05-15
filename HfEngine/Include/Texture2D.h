#pragma once
#include "stdafx.h"
#include "DX.h"
#include "D3DDevice.h"

class Texture : public Utility::ReferredObject{
};

class Texture2D : public Texture{
    int _width, _height;
    void CreateViews(D3DDevice *device);
public:
    ComPtr<ID3D11Texture2D> native_texture2d;
    ComPtr<ID3D11ShaderResourceView> native_shader_resource_view;

    const int& width = _width, &height = _height;
    void Initialize() {};
    void Initialize(D3DDevice *device, const std::wstring &filename); 
    void Initialize(D3DDevice *deivce, int w, int h);
    void CreateFromNativeTexture2D(ID3D11Texture2D *tex);

    void UnInitialize() {
        native_texture2d.ReleaseAndGetAddressOf(); //ComPtr & operator will call release...
        native_shader_resource_view.ReleaseAndGetAddressOf();
    }
    virtual void Release() {
        UnInitialize();
    }
};

class LoadTextureFailed : public std::runtime_error {
public:
    template<class ...Arg>
    LoadTextureFailed(const Arg &...arg) :std::runtime_error(arg...) {}
};

class RTT : public Utility::ReferredObject {
    Utility::ReferPtr<Texture2D> texture;
public:
    ComPtr<ID3D11RenderTargetView> native_rtt_view;
    ComPtr<ID3D11Texture2D> native_stencil_buffer;
    ComPtr<ID3D11DepthStencilView> native_stencil_view;

    void Initialize() {}
    void Initialize(Texture2D *tex);
    void CreateFromNativeTexture2D(ID3D11Texture2D *t); //only used in swap chain...
    void UnInitialize() {
        native_rtt_view.ReleaseAndGetAddressOf();
        native_stencil_buffer.ReleaseAndGetAddressOf();
        native_stencil_view.ReleaseAndGetAddressOf();
    }
    Texture2D *GetTexture2D() {
        return texture.Get();
    }
    Utility::Rect GetViewport() {
        return {0, 0, texture->width, texture->height};
    }
    virtual void Release() {
        UnInitialize();
    }
};

namespace Ext {
    namespace DX {
        namespace Texture {
            extern VALUE klass_texture;
            extern VALUE klass_texture2d;
            extern VALUE klass_rtt;
            void Init();
        }
    }
}