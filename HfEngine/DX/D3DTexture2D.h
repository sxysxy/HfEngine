#pragma once

#include "DX.h"
#include <stdafx.h>
#include <extension.h>

class D3DTexture : public Utility::ReferredObject {

};

class D3DDevice;
class D3DTexture2D : public D3DTexture {
protected:
    int _width, _height;
    void CreateViews(ID3D11Device *d);
public:
    ComPtr<ID3D11Texture2D> native_texture2d;
    ComPtr<ID3D11Texture2D> native_stencil_buffer;
    
    ComPtr<ID3D11ShaderResourceView> native_shader_resource_view;
    ComPtr<ID3D11RenderTargetView> native_render_target_view;
    ComPtr<ID3D11DepthStencilView> native_depth_sencil_view;
    const int &width = _width, &height = _height;

    ~D3DTexture2D() {UnInitialize(); }
    D3DTexture2D(){}                                                    
    D3DTexture2D(D3DDevice *device, int w, int h, bool stenciled);                                //from params
    D3DTexture2D(D3DDevice *device, const cstring &filename, bool stenciled);                     //from file
    D3DTexture2D(ID3D11Texture2D *a_native_object, bool stenciled);                               //from a native object
    void Initialize(D3DDevice *device, int w, int h, bool stenciled);
    void Initialize(D3DDevice *device, const cstring &filename, bool stenciled);
    void Initialize(ID3D11Texture2D *a_native_object, bool stenciled);
    void UnInitialize() {
        native_texture2d.ReleaseAndGetAddressOf();
        native_stencil_buffer.ReleaseAndGetAddressOf();
        native_shader_resource_view.ReleaseAndGetAddressOf();
        native_render_target_view.ReleaseAndGetAddressOf();
        native_depth_sencil_view.ReleaseAndGetAddressOf();
    }
    virtual void Release() {
        UnInitialize();
    };


    void CreateStencil();
};

class LoadTextureFailed : public std::runtime_error {
public:
    template<class ...Arg>
    LoadTextureFailed(const Arg &...arg):std::runtime_error(arg...) {}
};

namespace Ext {
    namespace DX {
        namespace D3DTexture2D {
            extern VALUE klass;
            extern VALUE klass_D3DTexture; //abstract class...
            
            void Init();
        }
    }
}

