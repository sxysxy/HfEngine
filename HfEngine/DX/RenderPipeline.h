#pragma once
#include <stdafx.h>
#include "DX.h"
#include "D3DTexture2D.h"

class D3DDevice;

class Shader : public Utility::ReferredObject {
protected:
public:
    //ComPtr<NativeShaderType> native_shader;
    ComPtr<ID3D10Blob> byte_code;

    virtual void CreateFromHLSLFile(D3DDevice *device, const cstring &filename) = 0;
    virtual void CreateFromBinaryFile(D3DDevice *device, const cstring &filename) = 0;
    virtual void CreateFromString(D3DDevice *device, const std::string &str) = 0;
    void UnInitialize() {
        byte_code.ReleaseAndGetAddressOf();
    }

    virtual void Release() {}
};

#define DEF_CREATOR(x) static Utility::ReferPtr<x> LoadHLSLFile(D3DDevice *device, const cstring &filename) {              \
        auto shader = Utility::ReferPtr<x>::New();                                                      \
        shader->CreateFromHLSLFile(device, filename);                                                   \
        return shader;                                                                                  \
    }                                                                                                   \
    static Utility::ReferPtr<x> LoadBinaryFile(D3DDevice *device, const cstring &filename) {            \
        auto shader = Utility::ReferPtr<x>::New();                                                      \
        shader->CreateFromBinaryFile(device, filename);                                                 \
        return shader;                                                                                  \
    }                                                                                                   \
    static Utility::ReferPtr<x> LoadCodeString(D3DDevice *device, const std::string &str) {             \
        auto shader = Utility::ReferPtr<x>::New();                                                      \
        shader->CreateFromString(device, str);                                                          \
        return shader;                                                                                  \
    }                                                                                                   \


class VertexShader : public Shader {
public:
    ComPtr<ID3D11VertexShader> native_shader;
    virtual void CreateFromHLSLFile(D3DDevice *device, const cstring &filename);
    virtual void CreateFromBinaryFile(D3DDevice *device, const cstring &filename);
    virtual void CreateFromString(D3DDevice *device, const std::string &str);
    void UnInitialize() {
        Shader::UnInitialize();
        native_shader.ReleaseAndGetAddressOf();
    }
    // DEF_CREATOR(VertexShader) //not good for debuging.
    static Utility::ReferPtr<VertexShader> LoadHLSLFile(D3DDevice *device, const cstring &filename) {
        auto shader = Utility::ReferPtr<VertexShader>::New();
        shader->CreateFromHLSLFile(device, filename);
        return shader;
    }
    static Utility::ReferPtr<VertexShader> LoadBinaryFile(D3DDevice *device, const cstring &filename) {
        auto shader = Utility::ReferPtr<VertexShader>::New();
        shader->CreateFromBinaryFile(device, filename);
        return shader;
    }
    static Utility::ReferPtr<VertexShader> LoadCodeString(D3DDevice *device, const std::string &str) {
        auto shader = Utility::ReferPtr<VertexShader>::New();
        shader->CreateFromString(device, str);
        return shader;
    }
};

class PixelShader : public Shader {
public:
    ComPtr<ID3D11PixelShader> native_shader;
    virtual void CreateFromHLSLFile(D3DDevice *device, const cstring &filename);
    virtual void CreateFromBinaryFile(D3DDevice *device, const cstring &filename);
    virtual void CreateFromString(D3DDevice *device, const std::string &str);
    void UnInitialize() {
        Shader::UnInitialize();
        native_shader.ReleaseAndGetAddressOf();
    }
   // DEF_CREATOR(PixelShader)  // not good for debuging.
    static Utility::ReferPtr<PixelShader> LoadHLSLFile(D3DDevice *device, const cstring &filename) {
        auto shader = Utility::ReferPtr<PixelShader>::New();
        shader->CreateFromHLSLFile(device, filename);
        return shader;
    }
    static Utility::ReferPtr<PixelShader> LoadBinaryFile(D3DDevice *device, const cstring &filename) {
        auto shader = Utility::ReferPtr<PixelShader>::New();
        shader->CreateFromBinaryFile(device, filename);
        return shader;
    }
    static Utility::ReferPtr<PixelShader> LoadCodeString(D3DDevice *device, const std::string &str) {
        auto shader = Utility::ReferPtr<PixelShader>::New();
        shader->CreateFromString(device, str);
        return shader;
    }
            
};

class D3DSampler : public Utility::ReferredObject {
    D3D11_SAMPLER_DESC desc;
public:
    ComPtr<ID3D11SamplerState> native_sampler;

    D3DSampler() {Initialize(); }
    void Initialize() {}
    void UnInitialize() {
        native_sampler.ReleaseAndGetAddressOf();
    }
    void SetUVWAddress(D3D11_TEXTURE_ADDRESS_MODE u, D3D11_TEXTURE_ADDRESS_MODE v, D3D11_TEXTURE_ADDRESS_MODE w,
                                        const Utility::Color &border_color) {
        /*
        {	D3D11_TEXTURE_ADDRESS_WRAP = 1,
            D3D11_TEXTURE_ADDRESS_MIRROR = 2,
            D3D11_TEXTURE_ADDRESS_CLAMP = 3,
            D3D11_TEXTURE_ADDRESS_BORDER = 4,
            D3D11_TEXTURE_ADDRESS_MIRROR_ONCE = 5
        }
        */
        desc.AddressU = u;
        desc.AddressV = v;
        desc.AddressW = w;
        memcpy(desc.BorderColor, &border_color, sizeof desc.BorderColor); //float x 4
    }
    void SetMip(float mip_min, float mip_max, float mip_bias) {
        desc.MinLOD = mip_min;
        desc.MaxLOD = mip_max;
        desc.MipLODBias = mip_bias;
    }
    void SetFilter(D3D11_FILTER filter, D3D11_COMPARISON_FUNC func) {
        desc.Filter = filter;
        desc.ComparisonFunc = func;
    }
    void SetMaxAnisotropy(UINT v) {
        desc.MaxAnisotropy = v;
    }
    void UseDefault() {
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
        desc.MipLODBias = 0.0f;
        desc.MaxAnisotropy = 0;
        desc.ComparisonFunc = static_cast<D3D11_COMPARISON_FUNC> (0);
        RtlZeroMemory(desc.BorderColor, sizeof(desc.BorderColor));
        desc.MinLOD = 0.0f;
        desc.MaxLOD = D3D11_FLOAT32_MAX;
    }
    void CreateState(D3DDevice *device);

    virtual void Release() {UnInitialize();}
};

class D3DBlendState : public Utility::ReferredObject {
public:
    ComPtr<ID3D11BlendState> native_blend_state;
    D3DBlendState() {}
    
    virtual void Release() {}
};

class RenderPipeline : public Utility::ReferredObject {
public:
    ComPtr<ID3D11InputLayout> native_input_layout;
    Utility::ReferPtr<VertexShader> vshader;
    Utility::ReferPtr<PixelShader> pshader;

    void UnInitialize(){
        native_input_layout.ReleaseAndGetAddressOf();
        vshader.Release();
        pshader.Release();
    }

    virtual void Release() {
        UnInitialize();
    };

    void SetVertexShader(VertexShader *vs) {vshader = vs; }
    void SetPixelShader(PixelShader *ps) {pshader = ps; }
    VertexShader *GetVertexShader() {return vshader.Get(); }
    PixelShader *GetPixelShader() {return pshader.Get(); }
    void SetInputLayout(D3DDevice *device, const std::string *idents, const DXGI_FORMAT *formats, int count);
};

class ShaderCompileError : public std::runtime_error {
public:
    template<class ...Arg>
    ShaderCompileError(const Arg &...arg) :std::runtime_error(arg...) {}
};

#undef DEF_CREATOR

namespace Ext {namespace DX {
    namespace Shader {
        extern VALUE klass;
        extern VALUE klass_vshader;
        extern VALUE klass_pshader;
        extern VALUE klass_sampler;
        extern VALUE klass_eShaderCompileError;

        void Init();
    }

    namespace RenderPipeline {
        extern VALUE klass;
        void Init();
    }
} }