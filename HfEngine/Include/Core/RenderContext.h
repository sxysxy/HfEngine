#pragma once
#include <ThirdParties.h>
#include <Core/GDevice.h>
#include <ReferPtr.h>

HFENGINE_NAMESPACE_BEGIN

typedef enum {
    VERTEX_SHADER,
    GEOMETRY_SHADER,
    PIXEL_SHADER
}SHADER_TYPE;

static const char* SHADER_COMPILE_TOKEN[] = { "vs_4_0", "gs_4_0", "ps_4_0" };

class Shader : public Utility::ReferredObject {
public:
    SHADER_TYPE shader_type;
    ComPtr<ID3D11DeviceChild> native_shader;

    void Initialize(SHADER_TYPE type) { shader_type = type; }
    void CreateFromFile(const std::wstring& filename);
    void CreateFromString(const std::string& code);
    void CreateFromBinary(void*, int size);

    virtual void Release() {
        native_shader.ReleaseAndGetAddressOf();
    }
};

class VertexShader : public Shader {
public:
    void Initialize() { Shader::Initialize(VERTEX_SHADER); }
};

class PixelShader : public Shader {
public:
    void Initialize() { Shader::Initialize(PIXEL_SHADER); }
};

class GeometryShader : public Shader {
public:
    void Initialize() { Shader::Initialize(GEOMETRY_SHADER); }
};

class RenderContext : public Utility::ReferredObject {
public:
    ComPtr<ID3D11DeviceContext> native_context;
    void Initialize() {
        GDevice::GetInstance()->native_device->CreateDeferredContext(0, native_context.ReleaseAndGetAddressOf());
    }
    RenderContext() {
        Initialize();
    }
    virtual void Release() {
        native_context.ReleaseAndGetAddressOf();
    }
};

HFENGINE_NAMESPACE_END