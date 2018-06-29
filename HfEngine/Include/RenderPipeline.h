#pragma once

#include "stdafx.h"
#include "DX.h"
#include "D3DBuffer.h"
#include "Shaders.h"
#include "Texture2D.h"
#include "SwapChain.h"
//#include "Utility\concurrentqueue.h"

class InputLayout : public Utility::ReferredObject {
public:
    ComPtr<ID3D11InputLayout> native_input_layout;

    void Initialize(D3DDevice *device, const std::string *idents,
        const DXGI_FORMAT *formats, int count, VertexShader *vs);
    void Initialize(D3DDevice *device, const std::initializer_list<std::string> &idents,
        const std::initializer_list<DXGI_FORMAT> &formats, VertexShader *vs) {
        int len1 = (int)(idents.end() - idents.begin());
        int len2 = (int)(formats.end() - formats.begin());
        if (len1 != len2)
            throw std::runtime_error("InputLayout: idents and formats should be in the same length");
        Initialize(device, idents.begin(), formats.begin(), len1, vs);
    }
    void UnInitialize() {
        native_input_layout.ReleaseAndGetAddressOf();
    }
    virtual void Release() {
        UnInitialize();
    }
};

class RenderPipeline : public Utility::ReferredObject {
public:
    Utility::ReferPtr<D3DDevice> device;
    ComPtr<ID3D11DeviceContext> native_context;

    //drawing renderer pipeline
    Utility::ReferPtr<VertexBuffer> vbuffer;

    //shaders
    Utility::ReferPtr<VertexShader> vshader;
    Utility::ReferPtr<PixelShader> pshader;
    Utility::ReferPtr<GeometryShader> gshader;

    //RS
    Utility::ReferPtr<Rasterizer> rasterizer;

    //OM data
    Utility::ReferPtr<Blender> blender;
    Utility::ReferPtr<RTT> rtt_target;
   
    void Initialize(D3DDevice *d) {
        device = d;
        device->native_device->CreateDeferredContext(0, &native_context);
    }
    void SetInputLayout(D3DDevice *device, const std::string *idents, const DXGI_FORMAT *formats, int count);
    void SetInputLayout(D3DDevice *device, const std::initializer_list<std::string> &idents,
        const std::initializer_list<DXGI_FORMAT> &formats) {
        int len1 = (int)(idents.end() - idents.begin());
        int len2 = (int)(formats.end() - formats.begin());
        if(len1 != len2)
            throw std::runtime_error("RenderPipeline::SetInputLayout: idents and formats should be in the same length");
        SetInputLayout(device, idents.begin(), formats.begin(), len1);
    }
    void SetInputLayout(InputLayout *ly) {
#ifdef _DEBUG
        if(!ly)throw std::invalid_argument("RenderPipeline::SetInputLayout: param can't be nullptr");
#endif
        native_context->IASetInputLayout(ly->native_input_layout.Get());
    }
    void SetVertexBuffer(VertexBuffer *vb);
    void SetIndexBuffer(IndexBuffer *ib);

    //VS
    void SetVertexShader(VertexShader *vs);
    void SetVSSampler(int slot, Sampler *sampler);
    void SetVSCBuffer(int slot, ConstantBuffer *cbuffer);
    void SetVSResource(int slot, Texture2D *tex);
    //PS
    void SetPixelShader(PixelShader *ps);
    void SetPSSampler(int slot, Sampler *sampler);
    void SetPSCBuffer(int slot, ConstantBuffer *cbuffer);
    void SetPSResource(int slot, Texture2D *tex);
    //GS
    void SetGeometryShader(GeometryShader *gs);
    void SetGSCBuffer(int slot, ConstantBuffer *cbuffer);
    void SetGSResource(int slot, Texture2D *tex);
    void SetGSSampler(int slot, Sampler *sampler);

    //RS 
    void SetViewport(const Utility::Rect &rect, float min_deep = 0.0f, float max_deep = 1.0f);
    void SetScissorRect(const Utility::Rect &rect);
    void SetRasterizer(Rasterizer *rs);

    //OM 
    void SetBlender(Blender *b);
    void SetTarget(RTT *rtt);

    void SetDepthStencilState(DepthStencilState *dss);

    inline void SetTopology(D3D11_PRIMITIVE_TOPOLOGY topo) {
        native_context->IASetPrimitiveTopology(topo);
    }

    inline void Draw(int start_pos, int count) {    
        native_context->Draw(count, start_pos);
    }
    inline void DrawIndex(int start_pos, int count) {
        native_context->DrawIndexed(count, start_pos, 0);
    }
    void ImmdiateRender();
    void ImmdiateCopy2D(Texture2D *dest, Texture2D *src,
        const Utility::Rect &dest_rect, const Utility::Rect &src_rect);
    void ImmdiateSavePNG(Texture2D *tex, const cstring &filename);
    void ImmdiateDumpPixels2D(Texture2D *tex, Utility::HFBuffer<Utility::Color8> *buffer);

    template<class T>
    void UpdateSubResource(D3DBuffer *bf, const T *data) {
        native_context->UpdateSubresource(bf->native_buffer.Get(), 0, 0, (void*)data, 0, 0);
    }
    void Clear(const Utility::Color &color, float depth = 1.0f) {
        float c[] = {color.r, color.g, color.b, color.a};
        native_context->ClearRenderTargetView(rtt_target->native_rtt_view.Get() , 
            c);
        native_context->ClearDepthStencilView(rtt_target->native_stencil_view.Get(), 
            D3D11_CLEAR_STENCIL | D3D11_CLEAR_DEPTH, depth, 0);
    }

    void UnInitialize() {
        if(native_context)native_context->ClearState();
        native_context.ReleaseAndGetAddressOf();
        device.Release();
        vshader.Release();
        pshader.Release();
        gshader.Release();
        rasterizer.Release();
        blender.Release();
        rtt_target.Release();
        vbuffer.Release();
    }
    virtual void Release() {
        UnInitialize();
    }
};

namespace Ext {
    namespace DX {
        namespace RenderPipeline {
            extern VALUE klass;
            extern VALUE klass_input_layout;
            void Init();
        }
    }
}

