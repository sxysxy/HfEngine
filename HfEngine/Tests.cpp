#include <Tests.h>
#include <stdafx.h>
#include <HFWindow.h>
#include <D3DDevice.h>
#include <Shaders.h>
#include <RenderPipeline.h>
#include <SwapChain.h>
using namespace std;
using namespace Utility;

namespace Tests {
    void TestHelloWorld() {
        MessageBox(nullptr, L"2333", L"666", 0);
    }

    void MessageLoop(int fps, const std::function<void(void)> &callback) {
        MSG msg;

        SleepFPSTimer timer(fps);
       
        while(true) {
            if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
                if(msg.message == WM_QUIT)break;
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            callback();
            timer.Await();
        }
    }

    void TestHFWindow() {
        auto window = ReferPtr<HFWindow>::New(L"Test", 500, 500);
        window->SetFixed(true);
        window->Show();
        MessageLoop(60, []() {});
    }

    void TestShaderBasic() {
        auto device = ReferPtr<D3DDevice>::New(D3D_DRIVER_TYPE_HARDWARE);
        auto vshader = Shader::LoadCodeString<VertexShader>(device.Get(),"\
         float4 main(float4 pos : POSITION) : SV_POSITION {      \n\
            return pos;                                          \n\
         }                                                       \n\
         ");
    }

    static const char *draw_rect_ps = " \
    float4 main(float4 pos : SV_POSITION, float4 color : COLOR) : SV_TARGET {\n \
        return color;                                                        \n \
    }                                                                        \n \
";

    static const char *draw_rect_vs = " \
    struct vs_output {             \n     \
        float4 pos : SV_POSITION;  \n     \
        float4 color : COLOR;      \n     \
    };                             \n     \
    vs_output main(float4 pos : POSITION, float4 color : COLOR) { \n  \
            vs_output opt;                                        \n  \
            opt.color = color;                                    \n  \
            opt.pos = pos;                                        \n  \
            return opt;                                           \n  \
    }                                                             \n  \
";

    static const char *draw_texture_vs = " \
    struct vs_output {            \n    \
        float4 pos : SV_POSITION; \n    \
        float2 tex : TEXCOORD;    \n    \
    };\
    cbuffer params : register(b0) { \n    \
        float sina, cosa;                 \
    }; \n                                 \
    vs_output main(float4 pos : POSITION, float2 tex : TEXCOORD) { \n \
        vs_output opt = (vs_output)0;                          \n \
        opt.pos = pos;            \n \
                                  \n \
        //float2 t = opt.pos.xy;    \n \
        //opt.pos.x = (cosa*t.x - sina*t.y); \n \
        //opt.pos.y = (sina*t.x + cosa*t.y); \n \
                                  \n \
        opt.tex = tex;          \n \
        return opt;             \n \
    }       \n";

    static const char *draw_texture_ps = " \
    Texture2D color_map : register(t0);   \n \
    SamplerState color_sampler : register(s0); \n \
    float4 main(float4 pos : SV_POSITION, float2 tex : TEXCOORD) : SV_TARGET { return color_map.Sample(color_sampler, tex); }";
    

    void TestSimpleRender() {
        auto window = Utility::ReferPtr<HFWindow>::New(L"Simple", 500, 500);
        window->Show();
        auto device = Utility::ReferPtr<D3DDevice>::New(D3D_DRIVER_TYPE_HARDWARE);
        auto ps = Shader::LoadCodeString<PixelShader>(device.Get(), draw_rect_ps);
        auto vs = Shader::LoadCodeString<VertexShader>(device.Get(), draw_rect_vs);
        auto rp = Utility::ReferPtr<RenderPipeline>::New(device.Get());
        rp->SetVertexShader(vs.Get());
        rp->SetPixelShader(ps.Get());
        rp->SetInputLayout(device.Get(), { "POSITION", "COLOR" }, 
                                    { DXGI_FORMAT_R32G32B32_FLOAT,  DXGI_FORMAT_R32G32B32A32_FLOAT });
        struct vertex {
            float pos[3], color[4];
        };
        vertex vecs[] = {
            { {-0.5, -0.5, 0.0},{0.0, 1.0, 0.0, 1.0} },
            { {-0.5, 0.5, 0.0}, {0.0, 0.0, 1.0, 1.0} },
            { {0.5, -0.5, 0.0}, {1.0, 0.0, 1.0, 1.0} },
            { {0.5, 0.5, 0.0},  {0.0, 1.0, 1.0, 1.0} },
        };
        auto vbuffer = Utility::ReferPtr<VertexBuffer>::New(device.Get(), sizeof vertex, 4, vecs);
        rp->SetVertexBuffer(vbuffer.Get());
        rp->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        rp->SetViewport({0, 0, window->width, window->height});
        auto swapchain =  Utility::ReferPtr<SwapChain>::New(device.Get(), window.Get());
        rp->SetTarget(swapchain->GetRTT());

        MessageLoop(60, [&](){
            rp->Clear({0.0f, 0.0f, 0.0f, 0.0f});
            rp->Draw(0, 4);
            rp->ImmdiateRender();
            swapchain->Present();
        });
    }
}