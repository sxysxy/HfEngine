#pragma once

#include <stdafx.h>
#include "D3DTexture2D.h"
#include "RenderPipeline.h"
#include <NativeThread.h>

class D3DBuffer;
class D3DConstantBuffer;
class D3DVertexBuffer;
class SwapChain;
enum SHADERS_WHICH_TO_APPLAY {
    SHADERS_APPLYTO_VSHADER = 1,
    SHADERS_APPLYTO_PSHADER = 2
};

class D3DDeviceContext : public Utility::ReferredObject{
protected:
    ComPtr<ID3D11Device> native_device;
public:
    ComPtr<ID3D11DeviceContext> native_context;
    ComPtr<ID3D11CommandList> native_command_list;
    
    D3DDeviceContext() {};
    D3DDeviceContext(D3DDevice *device);
    void Initialize(D3DDevice *device);
    void UnInitialize() {
        native_context.ReleaseAndGetAddressOf();
        native_command_list.ReleaseAndGetAddressOf();
    }
    virtual void Release() {
        UnInitialize();
    };
    void FinishiCommandList();

    //pipeline
    void BindPipeline(RenderPipeline *pipeline);
    void BindVertexShader(VertexShader *vs);
    void BindPixelShader(PixelShader *ps);

    //resources
    void BindShaderResource(int start_slot, int count, D3DTexture2D * const *texes, SHADERS_WHICH_TO_APPLAY which) {
        ID3D11ShaderResourceView *sr[16];
        for(int i = 0; i < count; i++)sr[i] = texes[i]->native_shader_resource_view.Get();
        if (as_integer(which) & as_integer(SHADERS_APPLYTO_VSHADER))
            native_context->VSSetShaderResources(start_slot, count, sr);
        if (as_integer(which) & as_integer(SHADERS_APPLYTO_PSHADER))
            native_context->PSSetShaderResources(start_slot, count, sr);
    }
    void BindShaderResource(int slot_pos, const D3DTexture2D *tex, SHADERS_WHICH_TO_APPLAY which) {
        if (as_integer(which) & as_integer(SHADERS_APPLYTO_VSHADER))
            native_context->VSSetShaderResources(slot_pos, 1, tex->native_shader_resource_view.GetAddressOf());
        if (as_integer(which) & as_integer(SHADERS_APPLYTO_PSHADER))
            native_context->PSSetShaderResources(slot_pos, 1, tex->native_shader_resource_view.GetAddressOf());
    }
    void ClearShaderResource(int slot, SHADERS_WHICH_TO_APPLAY which) {
        if(as_integer(which) & as_integer(SHADERS_APPLYTO_VSHADER))
            native_context->VSSetShaderResources(slot, 1, nullptr);
        if (as_integer(which) & as_integer(SHADERS_APPLYTO_PSHADER))
            native_context->PSSetShaderResources(slot, 1, nullptr);
    }
    void BindShaderSampler(int slot_pos, const D3DSampler *sampler, SHADERS_WHICH_TO_APPLAY which) {
        if(!sampler)throw std::invalid_argument("Nullptr sampler is given");
        if (as_integer(which) & as_integer(SHADERS_APPLYTO_VSHADER))
            native_context->VSSetSamplers(slot_pos, 1, sampler->native_sampler.GetAddressOf());
        if (as_integer(which) & as_integer(SHADERS_APPLYTO_PSHADER))
            native_context->PSSetSamplers(slot_pos, 1, sampler->native_sampler.GetAddressOf());
    }
    void BindShaderSampler(int start_slot, int count, const D3DSampler *const *ss, SHADERS_WHICH_TO_APPLAY which) {
        ID3D11SamplerState *s[16];
        for (int i = 0; i < count; i++) {
            s[i] = ss[i]->native_sampler.Get();
            if(!s[i])
                throw std::invalid_argument("Nullptr sampler is given");
        }
        if (as_integer(which) & as_integer(SHADERS_APPLYTO_VSHADER))
            native_context->VSSetSamplers(start_slot, count, s);
        if (as_integer(which) & as_integer(SHADERS_APPLYTO_PSHADER))
            native_context->PSSetSamplers(start_slot, count, s);
    }
    void BindShaderConstantBuffer(int start_slot, int count, D3DConstantBuffer *const *cs, SHADERS_WHICH_TO_APPLAY which);
    void BindShaderConstantBuffer(int slot_pos, D3DConstantBuffer *cb, SHADERS_WHICH_TO_APPLAY which);
    void BindVertexBuffer(int start_slot, int count, D3DVertexBuffer *const *vbs, UINT stride);
    void BindVertexBuffer(int slot_pos, const D3DVertexBuffer *vb, UINT stride);

    void SetViewport(const Utility::Rect &rect, float min_deep = 0.0f, float max_deep = 1.0f) {
        D3D11_VIEWPORT vp{(float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h, min_deep, max_deep};
        native_context->RSSetViewports(1, &vp);
    }
    void SetTopology(D3D11_PRIMITIVE_TOPOLOGY topology) {
        native_context->IASetPrimitiveTopology(topology);
    }
    void Draw(int start_pos, int count) {
        native_context->Draw(count, start_pos);
    }
    void SetRenderTarget(int count, D3DTexture2D *const *ts) {
        ID3D11RenderTargetView *rtvs[16];
        for(int i = 0; i < count; i++)rtvs[i] = ts[i]->native_render_target_view.Get();
        native_context->OMSetRenderTargets(count, rtvs, ts[0]->native_depth_sencil_view.Get());
    }
    void SetRenderTarget(D3DTexture2D *tex) {
        if(!tex)throw std::invalid_argument("SetRenderTarget : Nullptr is invalid arugment");
        native_context->OMSetRenderTargets(1, tex->native_render_target_view.GetAddressOf(), 
            tex->native_depth_sencil_view.Get());
    }
    void ClearRenderTarget(D3DTexture2D *tex, const FLOAT *color) {
        native_context->ClearRenderTargetView(tex->native_render_target_view.Get(), color);
        if(tex->native_depth_sencil_view)
            native_context->ClearDepthStencilView(tex->native_depth_sencil_view.Get(),
            D3D11_CLEAR_STENCIL | D3D11_CLEAR_DEPTH, 1.0f, 0);
    }
    void UpdateSubResource(D3DBuffer *buf, void *data);
    void ClearState();
};

class D3DDeviceImmdiateContext : public D3DDeviceContext {
    friend class D3DDevice;
    D3DDeviceImmdiateContext() {}
public:
    D3DDeviceImmdiateContext(D3DDevice *device);
    void ExecuteCommandList(D3DDeviceContext *ocontext);
    void ExecuteCommandList(ID3D11CommandList *command_list);
};

struct RenderingThreadParam {
    Utility::ReferPtr<SwapChain> swap_chain;
    Utility::ReferPtr<D3DDevice> device;
    int frame_rate;
    const RenderingThreadParam &operator=(const RenderingThreadParam &op);
};
struct ContextInteractData {
    int frame_rate;
    std::queue<ComPtr<ID3D11CommandList>> command_lists;
    bool exit_flag;
    ContextInteractData() {
        frame_rate = 0;
        exit_flag = 0;
    }
};
Utility::ReferPtr<Utility::NativeThread<ContextInteractData>> CreateRenderingThread(RenderingThreadParam *param);
class RenderingThread : public Utility::ReferredObject{
public:
    Utility::ReferPtr<Utility::NativeThread<ContextInteractData>> rendering_thread;
    RenderingThread(){}
    RenderingThread(D3DDevice *d, SwapChain *s, int frame_rate){Initialize(d, s, frame_rate); }
    void Initialize(D3DDevice *d, SwapChain *s, int frame_rate);
    void UnInitialize() {rendering_thread.Release();}
    virtual void Release() {UnInitialize(); }

    void SetFrameRate(int f) {
        auto d = rendering_thread->AccessBuffer(true);
        d->frame_rate = f;
        rendering_thread->AccessBuffer(false);
    }
    void Terminate() {
        auto d = rendering_thread->AccessBuffer(true);
        d->exit_flag = true;
        rendering_thread->AccessBuffer(false);
        rendering_thread->Join();
    }
    void PushCommandList(D3DDeviceContext *context) {
        auto d = rendering_thread->AccessBuffer(true);
        d->command_lists.push(context->native_command_list);
        rendering_thread->AccessBuffer(false);
    }
};

namespace Ext {
    namespace DX {
        namespace D3DDeviceContext {
            extern VALUE klass;
            extern VALUE klass_immcontext;

            void Init();
        }
    }
}

namespace Ext {
    namespace DX {
        namespace RenderingThread {
            extern VALUE klass;
            void Init();
        }
    }
}