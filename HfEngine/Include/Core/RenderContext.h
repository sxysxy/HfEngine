#pragma once
#include <ThirdParties.h>
#include <Core/GDevice.h>
#include <Core/Canvas.h>
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
    void CreateFromFile(const std::wstring& filename, const std::string &entry = "main");
    void CreateFromString(const std::string& code, const std::string &entry = "main");
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

class GBuffer : public Utility::ReferredObject {
    int _size;
public:
    const int& size = _size;
    ComPtr<ID3D11Buffer> native_buffer;
    GBuffer() {}
    void Initialize(UINT usage, UINT flag, size_t size, const void* init_data);
    void UnInitialize() {
        native_buffer.ReleaseAndGetAddressOf();
    }
    virtual void Release() {
        UnInitialize();
    }
};

class ConstantBuffer : public GBuffer {
public:
    ConstantBuffer() {}
    template<class T = void>
    ConstantBuffer(size_t sz, const T* init_data = nullptr) { Initialize(sz, init_data); }
    template<class T = void>
    void Initialize(size_t sz, const T* init_data = nullptr) {
        if (sz % 16)throw std::invalid_argument("ConstantBuffer::Initialize: size should can be devied by 16");
        GBuffer::Initialize(D3D11_USAGE_DEFAULT, D3D11_BIND_CONSTANT_BUFFER, sz, (void*)init_data);
    }
};

class VertexBuffer : public GBuffer {
    size_t _size_per_vertex;
public:
    const size_t& size_per_vertex = _size_per_vertex;
    VertexBuffer() {}
    template<class T = void>
    VertexBuffer(size_t sizeof_per_vertex, size_t numof_vertex, const T* init_data = nullptr) {
        Initialize(sizeof_per_vertex, numof_vertex, init_data);
    }
    template<class T = void>
    void Initialize(size_t sizeof_per_vertex, size_t numof_vertex, const T* init_data = nullptr) {
        _size_per_vertex = sizeof_per_vertex;
        GBuffer::Initialize(D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER,
            sizeof_per_vertex * numof_vertex, (void*)init_data);
    }
};

class IndexBuffer : public GBuffer {
public:
    IndexBuffer() {}
    IndexBuffer(size_t numof_indexes, const int32_t* init_data = nullptr) {
        Initialize(numof_indexes, init_data);
    }
    void Initialize(size_t numof_indexes, const int32_t* init_data = nullptr) {
        GBuffer::Initialize(D3D11_USAGE_DEFAULT, D3D11_BIND_INDEX_BUFFER,
            numof_indexes * sizeof(int32_t), (void*)init_data);
    }
};

template<class T>
class WithDescriptionStruct {
protected:
    T desc;
public:
    void DumpDescription(T* s) {
        memcpy(s, &desc, sizeof desc);
    }
    void LoadDescription(T* s) {
        memcpy(&desc, s, sizeof desc);
    }
};

class Rasterizer : public Utility::ReferredObject, public WithDescriptionStruct<D3D11_RASTERIZER_DESC> {
public:
    ComPtr<ID3D11RasterizerState> native_rasterizer;
    inline void UseDefault() {
        desc.AntialiasedLineEnable = desc.MultisampleEnable = desc.ScissorEnable = desc.FrontCounterClockwise = false;
        desc.DepthClipEnable = true;
        desc.SlopeScaledDepthBias = desc.DepthBiasClamp = 0.0;
        desc.DepthBias = 0;
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_NONE;
    }
    inline void SetFillMode(D3D11_FILL_MODE m) {
        desc.FillMode = m;
    }
    inline void SetCullMode(D3D11_CULL_MODE m) {
        desc.CullMode = m;
    }
    inline void SetDepthBias(int bias, float clamp, float slope_scale) {
        desc.DepthBias = bias;
        desc.DepthBiasClamp = clamp;
        desc.SlopeScaledDepthBias = slope_scale;
    }
    inline void SetFrontCounter(bool clock_wise) {
        desc.FrontCounterClockwise = clock_wise;
    }
    inline void SetClip(bool depth_clip, bool scissor_clip) {
        desc.DepthClipEnable = depth_clip;
        desc.ScissorEnable = scissor_clip;
    }
    inline void SetMultiSample(bool multisample_enable) {
        desc.MultisampleEnable = multisample_enable;
    }
    inline void SetAntialiasedLine(bool enable) {
        desc.AntialiasedLineEnable = enable;
    }
    inline void SetScissorEnable(bool enable) {
        desc.ScissorEnable = enable;
    }
    void CreateState() {
        auto hr = GDevice::GetInstance()->native_device->CreateRasterizerState(&desc, &native_rasterizer);
        if (FAILED(hr)) 
            THROW_ERROR_CODE(std::runtime_error, "Fail to create rasterizer state", hr);
    }
    void UnInitialize() {
        native_rasterizer.ReleaseAndGetAddressOf();
    }
    virtual void Release() {
        UnInitialize();
    }
};
static const float DEFAULT_BLEND_FACTOR[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
//Blender (for OM)
class Blender : public Utility::ReferredObject, public WithDescriptionStruct<D3D11_BLEND_DESC> {
public:
    ComPtr<ID3D11BlendState> native_blender;
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
    void UseDefault() {
        Enable(false);
        SetColorBlend(D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD);
        SetAlphaBlend(D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD);
        SetMask(D3D11_COLOR_WRITE_ENABLE_ALL);
    }
    void CreateState() {
        auto hr = GDevice::GetInstance()->native_device->CreateBlendState(&desc, &native_blender);
        if (FAILED(hr))
            THROW_ERROR_CODE(std::runtime_error, "Fail to create blend state", hr);
    }

    void UnInitialize() {
        native_blender.ReleaseAndGetAddressOf();
    }
    virtual void Release() {
        UnInitialize();
    }
};

class RenderContext : public Utility::ReferredObject {
public:
    ComPtr<ID3D11DeviceContext> native_context;
    Utility::ReferPtr<Shader> vshader, gshader, pshader;

    Utility::ReferPtr<Rasterizer> rasterizer;

    Utility::ReferPtr<Canvas> render_target;

    void SetInputLayout(const std::string* idents, const DXGI_FORMAT* formats, int count);
    inline void SetInputLayout(const std::initializer_list<std::string>& idents,
        const std::initializer_list<DXGI_FORMAT>& formats) {
        int len1 = (int)(idents.end() - idents.begin());
        int len2 = (int)(formats.end() - formats.begin());
        if (len1 != len2)
            throw std::runtime_error("RenderContext::SetInputLayout: idents and formats should be in the same length");
        SetInputLayout(idents.begin(), formats.begin(), len1);
    }
    inline void SetShader(Shader* shader) {
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
    inline void SetShaderResource(SHADER_TYPE stage, int slot, Canvas* resource) {
        if (stage == VERTEX_SHADER) {
            native_context->VSGetShaderResources(slot, 1, resource->native_shader_resource_view.GetAddressOf());
        }
        else if (stage == GEOMETRY_SHADER) {
            native_context->GSGetShaderResources(slot, 1, resource->native_shader_resource_view.GetAddressOf());
        }
        else if (stage == PIXEL_SHADER) {
            native_context->PSGetShaderResources(slot, 1, resource->native_shader_resource_view.GetAddressOf());
        }
    }
    inline void SetConstantBuffer(SHADER_TYPE stage, int slot, ConstantBuffer* cb) {
        if (stage == VERTEX_SHADER) {
            native_context->VSSetConstantBuffers(slot, 1, cb->native_buffer.GetAddressOf());
        }
        else if (stage == GEOMETRY_SHADER) {
            native_context->GSSetConstantBuffers(slot, 1, cb->native_buffer.GetAddressOf());
        }
        else if (stage == PIXEL_SHADER) {
            native_context->PSSetConstantBuffers(slot, 1, cb->native_buffer.GetAddressOf());
        }
    }
    /*    inline void SetSampler(SHADER_TYPE stage, int slot, ConstantBuffer* cb) {
        if (stage == VERTEX_SHADER) {
            native_context->VSSetSamplers
        }
        else if (stage == GEOMETRY_SHADER) {

        }
        else if (stage == PIXEL_SHADER) {
        }
    }*/



    inline void SetVertexBuffer(VertexBuffer* vb) {
        UINT stride = (UINT)vb->size_per_vertex;
        UINT offset = 0;
        native_context->IASetVertexBuffers(0, 1, vb->native_buffer.GetAddressOf(), &stride, &offset);
    }
    inline void SetIndexBuffer(IndexBuffer* ib) {
        native_context->IASetIndexBuffer(ib->native_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    }
    inline void SetShaderConstantBuffer(SHADER_TYPE shader, int slot, ConstantBuffer* cb) {
        if (shader == VERTEX_SHADER) {
            native_context->VSSetConstantBuffers(slot, 1, cb->native_buffer.GetAddressOf());
        }
        else if (shader == GEOMETRY_SHADER) {
            native_context->GSSetConstantBuffers(slot, 1, cb->native_buffer.GetAddressOf());
        }
        else if (shader == PIXEL_SHADER) {
            native_context->PSSetConstantBuffers(slot, 1, cb->native_buffer.GetAddressOf());
        }
    }

    inline void Render() const {
        ID3D11CommandList* list = nullptr;
        native_context->FinishCommandList(true, &list);
        if (list) {
            //list->AddRef();
            auto device = GDevice::GetInstance();
            device->Lock();
            device->native_immcontext->ExecuteCommandList(list, false);
            device->UnLock();
            list->Release();
        }
    }

    inline void SetViewport(UINT topx, UINT topy, UINT width, UINT height) {
        
        D3D11_VIEWPORT vp;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = (FLOAT)topx;
        vp.TopLeftY = (FLOAT)topy;
        vp.Width = (FLOAT)width;
        vp.Height = (FLOAT)height;
        
        native_context->RSSetViewports(1, &vp);
    }

    inline void SetRasterizer(Rasterizer* rs) {
        if(rs)
            native_context->RSSetState(rs->native_rasterizer.Get());
    }
    inline void SetBlender(Blender* blender) {
        if(blender)
            native_context->OMSetBlendState(blender->native_blender.Get(), DEFAULT_BLEND_FACTOR, 0xffffffff);
    }

    void Initialize() {
        GDevice::GetInstance()->native_device->CreateDeferredContext(0, native_context.ReleaseAndGetAddressOf());
        Rasterizer rs;
        rs.UseDefault();
        rs.CreateState();
        SetRasterizer(&rs);
        Blender blender;
        blender.UseDefault();
        blender.CreateState();
        SetBlender(&blender);
    }

    inline void SetRenderTarget(Canvas* target) {
        render_target = target;
        native_context->OMSetRenderTargets(1, target->native_render_target_view.GetAddressOf(), target->native_depth_stencil_view.Get());
    }
    inline void ClearTarget() {
        ClearTarget(0.0f, 0.0f, 0.0f, 0.0f);
    }
    inline void ClearTarget(float r, float g, float b, float a) {
        float col[4];
        col[0] = r, col[1] = g, col[2] = b, col[3] = a;
        native_context->ClearRenderTargetView(render_target->native_render_target_view.Get(), col);
        native_context->ClearDepthStencilView(render_target->native_depth_stencil_view.Get(),
            D3D11_CLEAR_STENCIL | D3D11_CLEAR_DEPTH, 1.0f, 0);
    }

    inline void Draw(D3D11_PRIMITIVE_TOPOLOGY topo, UINT start_pos, UINT count) {
        native_context->IASetPrimitiveTopology(topo);
        native_context->Draw(count, start_pos);
    }
    inline void DrawIndex(D3D11_PRIMITIVE_TOPOLOGY topo, UINT start_pos, UINT count) {
        native_context->IASetPrimitiveTopology(topo);
        native_context->DrawIndexed(count, start_pos, 0);
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
extern thread_local RClass* ClassGBuffer, *ClassVertexBuffer, *ClassIndexBuffer, *ClassConstantBuffer;

bool InjectRenderContextExtension();

HFENGINE_NAMESPACE_END