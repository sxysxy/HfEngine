#pragma once

#include "stdafx.h"
#include "DX.h"
#include "D3DBuffer.h"
#include "Shaders.h"
#include "Texture2D.h"

class RenderPipeline : public Utility::ReferredObject {
public:
    Utility::ReferPtr<D3DDevice> device;
    ComPtr<ID3D11DeviceContext> native_context;

    //drawing renderer pipeline
    ComPtr<ID3D11InputLayout> native_input_layout;
    Utility::ReferPtr<VertexBuffer> vbuffer;

    //
    Utility::ReferPtr<VertexShader> vshader;
    Utility::ReferPtr<PixelShader> pshader;

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

    //RS 
    void SetViewport(const Utility::Rect &rect, float min_deep = 0.0f, float max_deep = 1.0f);

    //OM 
    void SetBlender(Blender *b);
    void SetTarget(RTT *rtt);

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
        native_context.ReleaseAndGetAddressOf();
        native_input_layout.ReleaseAndGetAddressOf();
        device.Release();
        vshader.Release();
        pshader.Release();
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

            void Init();
        }
    }
}

