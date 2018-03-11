#pragma once
#include <stdafx.h>
#include <DX/D3DDevice.h>
#include <DX/D3DDeviceContext.h>
#include <DX/D3DTexture2D.h>
#include <DX/D3DBuffer.h>
#include <shapes.h>
#include <HFWindow.h>

#include <array>

namespace G2D {

    using namespace Utility;
    class Renderer : public ReferredObject {
        ReferPtr<D3DDevice> device;
        ReferPtr<D3DDeviceImmdiateContext> context;
        ReferPtr<D3DTexture2D> render_target;
        ReferPtr<HFWindow> window;
        
        ReferPtr<D3DDeviceContext> draw_rect;
        ReferPtr<D3DVertexBuffer> draw_rect_vbuffer;
        ReferPtr<D3DDeviceContext> draw_texture;
        void InitPipelines();

        std::vector<ReferPtr<D3DDeviceContext>> contexts; //observing pointers to contexts
    public:
       
        Rect viewport;

        Renderer() {}
        template<class ...Arg>
        Renderer(const Arg &...arg) {
            Initialize(arg...);
        }
        void Initialize(D3DDevice *_device, HFWindow *wnd) {
            device = _device;
            context = _device->immcontext.Get();
            InitPipelines();
            SetViewport({0, 0, wnd->width, wnd->height});
        }
        
        ReferPtr<D3DDevice> GetDevice() {
            return device;
        }
        ReferPtr<D3DDeviceImmdiateContext> GetContext() {
            return context;
        }
        
        void SetViewport(int x, int y, int w, int h) {
            SetViewport({x, y, w, h});
        }
        void SetViewport(const Rect &rect) {;
            viewport = rect;
            for (auto pcontext : contexts) {
                pcontext->SetViewport(rect);
            }
        }
        void UseDefaultViewport() {
            SetViewport({0, 0, window->width, window->height});
        }
        void SetRenderTarget(D3DTexture2D *texture) {
            render_target = texture;
            for (auto pcontext : contexts) {
                pcontext->SetRenderTarget(texture);
            }
        }
        ReferPtr<D3DTexture2D> GetRenderTarget() {
            return render_target;
        }

        void ExecuteRender() {
            for (auto pcontext : contexts) {
                pcontext->FinishiCommandList();
                context->ExecuteCommandList(pcontext->native_command_list.Get());
            }
        }

        void Release() {
            UnInitialize();
        }
        void UnInitialize() {
            //...
        }

        void FillRectWith4Colors(const Rect &rect, const std::initializer_list<Color>& colors);
        void FillRect(const Rect &rect, const Color &color);
        void FillVerticalGradientRect(const Rect &rect, const Color &color1, const Color &color2);
        void FillHorizontalGradientRect(const Rect &rect, const Color &color1, const Color &color2);

    };

}