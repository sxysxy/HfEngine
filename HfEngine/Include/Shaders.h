#pragma once

#include "stdafx.h"
#include "D3DDevice.h"
#include "DX.h"
#include "Utility\referptr.h"

class Shader : public Utility::ReferredObject {
public:
    ComPtr<ID3D10Blob> byte_code;
    virtual void CreateFromHLSLFile(D3DDevice *device, const std::wstring &filename, const std::string &entry = "main") = 0;
    virtual void CreateFromString(D3DDevice *device, const std::string &code, const std::string &entry = "main") = 0;
    virtual void CreateFromBinary(D3DDevice *device, void *, int size) = 0;

    void UnInitialize() {
        byte_code.ReleaseAndGetAddressOf();
    }

    virtual void Release() {
        UnInitialize();
    }
    
    template<class T>
    static Utility::ReferPtr<T> LoadHLSLFile(D3DDevice *device, const std::wstring &filename, const std::string &entry = "main") {
        auto ptr = ReferPtr<T>::New();
        ptr->CreateFromHLSLFile(device, filename, entry);
        return ptr;
    }

    template<class T>
    static Utility::ReferPtr<T> LoadCodeString(D3DDevice *device, const std::string &str, const std::string &entry = "main") {
        auto ptr = ReferPtr<T>::New();
        ptr->CreateFromString(device, str, entry);
        return ptr;
    }
};

class VertexShader : public Shader {
public:
    ComPtr<ID3D11VertexShader> native_vshader;
    void Initialize() {}
    void CreateFromHLSLFile(D3DDevice *device, const std::wstring &filename, const std::string &entry = "main");
    void CreateFromString(D3DDevice *device, const std::string &code, const std::string &entry = "main");
    void CreateFromBinary(D3DDevice *device, void *, int size);
    void UnInitialize() {
        Shader::UnInitialize();
        native_vshader.ReleaseAndGetAddressOf();
    }
    virtual void Release() {
        UnInitialize();
    }
};

class PixelShader : public Shader {
public:
    ComPtr<ID3D11PixelShader> native_pshader;
    void Initialize() {}
    void CreateFromHLSLFile(D3DDevice *device, const std::wstring &filename, const std::string &entry = "main");
    void CreateFromString(D3DDevice *device, const std::string &code, const std::string &entry = "main");
    void CreateFromBinary(D3DDevice *device, void *, int size);
    void UnInitialize() {
        Shader::UnInitialize();
        native_pshader.ReleaseAndGetAddressOf();
    }
    virtual void Release() {
        UnInitialize();
    }
};

class ShaderCompileError : public std::runtime_error {
public:
    template<class ...T>
    ShaderCompileError(const T &...arg):std::runtime_error(arg...) {}
};

template<class T>
class WithDescriptionStruct {
protected:
    T desc;
public:
    void DumpDescription(T *s) {
        memcpy(s, &desc, sizeof desc);
    }
    void LoadDescription(T *s) {
        memcpy(&desc, s, sizeof desc);
    }
};

//Sampler
class Sampler : public Utility::ReferredObject, public WithDescriptionStruct<D3D11_SAMPLER_DESC> {
public:
    ComPtr<ID3D11SamplerState> native_sampler;

    Sampler() { Initialize(); }
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
    void CreateState(D3DDevice *device) {
        device->native_device->CreateSamplerState(&desc, &native_sampler);
    }

    virtual void Release() { UnInitialize(); }
};

//Blender (for OM only)
class Blender : public Utility::ReferredObject, public WithDescriptionStruct<D3D11_BLEND_DESC>{
public:
    ComPtr<ID3D11BlendState> native_blender;
    Utility::Color blend_factor;

    void Initialize() {}
    void Enable(bool f) {
        desc.RenderTarget[0].BlendEnable = f;
    }
    void SetColorBlend(D3D11_BLEND src_blend, D3D11_BLEND dest_blend, D3D11_BLEND_OP op) {
        desc.RenderTarget[0].SrcBlend = src_blend;
        desc.RenderTarget[0].DestBlend = dest_blend;
        desc.RenderTarget[0].BlendOp = op;
    }
    void SetAlphaBlend(D3D11_BLEND src_blend, D3D11_BLEND dest_blend, D3D11_BLEND_OP op) {
        desc.RenderTarget[0].SrcBlendAlpha = src_blend;
        desc.RenderTarget[0].DestBlendAlpha = dest_blend;
        desc.RenderTarget[0].BlendOpAlpha = op;
    }
    void SetMask(D3D11_COLOR_WRITE_ENABLE mask) {
        desc.RenderTarget[0].RenderTargetWriteMask = mask;
    }
    void SetBlendFactor(const Utility::Color &c) {
        blend_factor = c;
    }
    
    void UseDefault() {
        Enable(false);
        SetColorBlend(D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD);
        SetAlphaBlend(D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD);
        SetMask(D3D11_COLOR_WRITE_ENABLE_ALL);
        SetBlendFactor({1.0, 1.0, 1.0, 1.0});
    }
    void CreateState(D3DDevice *device) {
        device->native_device->CreateBlendState(&desc, &native_blender);
    }

    void UnInitialize() {
        native_blender.ReleaseAndGetAddressOf();
    }
    void Release() {
        UnInitialize();
    }
};

namespace Ext {
    namespace DX {
        namespace Shader {
            extern VALUE klass;
            extern VALUE klass_vshader;
            extern VALUE klass_pshader;
            extern VALUE klass_sampler;
            extern VALUE klass_blender;
            extern VALUE klass_eShaderCompileError;

            void Init();
        }
    }
}

