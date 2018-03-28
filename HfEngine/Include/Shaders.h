#pragma once

#include "stdafx.h"
#include "D3DDevice.h"
#include "DX.h"
#include "Utility\referptr.h"

class Shader : public Utility::ReferredObject {
public:
    ComPtr<ID3D10Blob> byte_code;
    virtual void CreateFromHLSLFile(D3DDevice *device, const std::wstring &filename) = 0;
    virtual void CreateFromString(D3DDevice *device, const std::string &code) = 0;
    virtual void CreateFromBinary(D3DDevice *device, void *, int size) = 0;

    void UnInitialize() {
        byte_code.ReleaseAndGetAddressOf();
    }

    virtual void Release() {
        UnInitialize();
    }
    
    template<class T>
    static Utility::ReferPtr<T> LoadHLSLFile(D3DDevice *device, const std::wstring &filename) {
        auto ptr = ReferPtr<T>::New();
        ptr->CreateFromHLSLFile(device, filename);
        return ptr;
    }

    template<class T>
    static Utility::ReferPtr<T> LoadCodeString(D3DDevice *device, const std::string &str) {
        auto ptr = ReferPtr<T>::New();
        ptr->CreateFromString(device, str);
        return ptr;
    }
};

class VertexShader : public Shader {
public:
    ComPtr<ID3D11VertexShader> native_vshader;
    void Initialize() {}
    void CreateFromHLSLFile(D3DDevice *device, const std::wstring &filename);
    void CreateFromString(D3DDevice *device, const std::string &code);
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
    void CreateFromHLSLFile(D3DDevice *device, const std::wstring &filename);
    void CreateFromString(D3DDevice *device, const std::string &code);
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


//Sampler
class Sampler : public Utility::ReferredObject {
    D3D11_SAMPLER_DESC desc;
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

namespace Ext {
    namespace DX {
        namespace Shader {
            extern VALUE klass;
            extern VALUE klass_vshader;
            extern VALUE klass_pshader;
            extern VALUE klass_sampler;
            extern VALUE klass_eShaderCompileError;

            void Init();
        }
    }
}

