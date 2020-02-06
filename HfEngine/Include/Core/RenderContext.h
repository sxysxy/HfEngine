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
    union {
        ID3D11VertexShader *native_vshader;
        ID3D11GeometryShader *native_gshader;
        ID3D11PixelShader *native_pshader;
    };
    ComPtr<ID3D11DeviceChild> native_shader;
    
    ComPtr<ID3D10Blob> compiled_byte_code;

    void Initialize(SHADER_TYPE type) { shader_type = type; }
    void CreateFromFile(const std::wstring& filename);
    void CreateFromString(const std::string& code);
    void CreateFromBinary(const void*, size_t size);

    virtual void Release() {
        native_shader.ReleaseAndGetAddressOf();
    }
};

class VertexShader : public Shader {
public: 
    VertexShader() { Initialize(); }
    void Initialize() { Shader::Initialize(VERTEX_SHADER); }
};

class PixelShader : public Shader {
public:
    PixelShader() { Initialize(); }
    void Initialize() { Shader::Initialize(PIXEL_SHADER); }
};

class GeometryShader : public Shader {
public:
    GeometryShader() { Initialize(); }
    void Initialize() { Shader::Initialize(GEOMETRY_SHADER); }
};

class RenderContext : public Utility::ReferredObject {
public:
    ComPtr<ID3D11DeviceContext> native_context;
    Utility::ReferPtr<Shader> vshader, gshader, pshader;
    
    void SetInputLayout(const std::string* idents, const DXGI_FORMAT* formats, int count);
    void SetInputLayout(const std::initializer_list<std::string>& idents,
        const std::initializer_list<DXGI_FORMAT>& formats) {
        int len1 = (int)(idents.end() - idents.begin());
        int len2 = (int)(formats.end() - formats.begin());
        if (len1 != len2)
            throw std::runtime_error("RenderContext::SetInputLayout: idents and formats should be in the same length");
        SetInputLayout(idents.begin(), formats.begin(), len1);
    }
    void SetShader(Shader* shader) {
        if (shader->shader_type == VERTEX_SHADER) {
            vshader = shader;
            native_context->VSSetShader(shader ? shader->native_vshader : nullptr, 0, 0);
        }
        else if (shader->shader_type == GEOMETRY_SHADER) {
            gshader = shader;
            native_context->GSSetShader(shader ? shader->native_gshader : nullptr, 0, 0);
        }
        else if (shader->shader_type == PIXEL_SHADER) {
            pshader = shader;
            native_context->PSSetShader(shader ? shader->native_pshader : nullptr, 0, 0);
        }
    }

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

extern thread_local RClass* ClassShader, * ClassVertexShader, * ClassGeometryShader, * ClassPixelShader;
extern thread_local RClass* ClassRenderContext;

bool InjectRenderContextExtension();

HFENGINE_NAMESPACE_END