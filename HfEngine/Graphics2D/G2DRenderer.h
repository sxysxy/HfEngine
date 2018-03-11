#pragma once
#include <stdafx.h>
#include <DX/D3DDevice.h>
#include <DX/D3DDeviceContext.h>
#include <DX/D3DTexture2D.h>

namespace G2D {

    using namespace Utility;
    class Renderer {
        ReferPtr<D3DDevice> device;
        ReferPtr<D3DDeviceImmdiateContext> context;
        ReferPtr<D3DTexture2D> render_target;

        //Execute Render 
        ReferPtr<D3DDeviceContext> draw_rect;
        ReferPtr<D3DDeviceContext> draw_texture;
        
        void InitPipelines();
    public:
        Renderer() {}
        template<class ...Arg>
        Renderer(const Arg &...arg) {
            Initailize(arg...);
        }
        void Initialize(D3DDevice *_device) {
            device = _device;
            context = _device->immcontext.Get();
            InitPipelines();
        }
        
        ReferPtr<D3DDevice> GetDevice() {
            return device;
        }
        ReferPtr<D3DDeviceImmdiateContext> GetContext() {
            return context;
        }
        
        void SetRenderTarget(D3DTexture2D *texture) {
            render_target = texture;
        }
        ReferPtr<D3DTexture2D> GetRenderTarget() {
            return render_target;
        }

        void ExecuteRender() {
            draw_rect->FinishiCommandList();
            context->ExecuteCommandList(draw_rect->native_command_list.Get());
            draw_texture->FinishiCommandList();
            context->ExecuteCommandList(draw_texture->native_command_list.Get());
        }

    };

}