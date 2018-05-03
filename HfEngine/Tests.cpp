#include <Tests.h>
#include <stdafx.h>
#include <HFWindow.h>
#include <D3DDevice.h>
#include <Shaders.h>
#include <RenderPipeline.h>
#include <SwapChain.h>
#include <DirectXMath.h>
using namespace std;
using namespace Utility;
using namespace DirectX;

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
        auto device = ReferPtr<D3DDevice>::New();
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
    cbuffer wvpm : register(b0) {  \n \
        float4x4 wvp;               \n \
    }; \n \
    struct vs_output {             \n     \
        float4 pos : SV_POSITION;  \n     \
        float4 color : COLOR;      \n     \
    };                             \n     \
    vs_output main(float3 pos : POSITION, float4 color : COLOR) { \n  \
            vs_output opt;                                        \n  \
            opt.color = color;                                    \n  \
                \n \
            opt.pos = mul(float4(pos, 1.0f), wvp);                \n  \
                                                  \n  \
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
        auto device = Utility::ReferPtr<D3DDevice>::New();
        auto ps = Shader::LoadCodeString<PixelShader>(device.Get(), draw_rect_ps);
        auto vs = Shader::LoadCodeString<VertexShader>(device.Get(), draw_rect_vs);
        auto rp = Utility::ReferPtr<RenderPipeline>::New(device.Get());
        rp->SetVertexShader(vs.Get());
        rp->SetPixelShader(ps.Get());
        rp->SetInputLayout(device.Get(), { "POSITION", "COLOR" }, 
                                    { DXGI_FORMAT_R32G32B32_FLOAT,  DXGI_FORMAT_R32G32B32A32_FLOAT });
        struct vertex {
            XMFLOAT3 pos;
            XMFLOAT4 color;
        };
        vertex vecs[] = {
            { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT4(1.0, 1.0, 1.0, 1.0) },//white
            { XMFLOAT3(-0.5f, +0.5f, -0.5f), XMFLOAT4(0.0, 0.0, 0.0, 1.0) },//black
            { XMFLOAT3(+0.5f, +0.5f, -0.5f), XMFLOAT4(1.0, 0.0, 0.0, 1.0) },//red
            { XMFLOAT3(+0.5f, -0.5f, -0.5f), XMFLOAT4(1.0, 1.0, 0.0, 1.0) },//green
            { XMFLOAT3(-0.5f, -0.5f, +0.5f), XMFLOAT4(0.0, 0.0, 1.0, 1.0) },//blue
            { XMFLOAT3(-0.5f, +0.5f, +0.5f), XMFLOAT4(1.0, 1.0, 0.0, 1.0) },//yellow
            { XMFLOAT3(+0.5f, +0.5f, +0.5f), XMFLOAT4(0.0, 1.0, 1.0, 1.0) },//cyan
            { XMFLOAT3(+0.5f, -0.5f, +0.5f), XMFLOAT4(1.0, 0.0, 1.0, 1.0) }//magenta
        };
        auto vbuffer = Utility::ReferPtr<VertexBuffer>::New(device.Get(), sizeof vertex, 
                sizeof(vecs) / sizeof(vertex), vecs);
        rp->SetVertexBuffer(vbuffer.Get());
        int indexs[] = {      
            // front face
            0, 1, 2,
            0, 2, 3,
            // back face
            4, 6, 5,
            4, 7, 6,
            // left face
            4, 5, 1,
            4, 1, 0,
            // right face
            3, 2, 6,
            3, 6, 7,
            // top face
            1, 5, 6,
            1, 6, 2,
            // bottom face
            4, 0, 3,
            4, 3, 7
        };
        auto ibuffer = Utility::ReferPtr<IndexBuffer>::New(device.Get(), sizeof(indexs) / sizeof(int), indexs);
        rp->SetIndexBuffer(ibuffer.Get());
        auto cb = Utility::ReferPtr<ConstantBuffer>::New(device.Get(), sizeof XMMATRIX);
        rp->SetVSCBuffer(0, cb.Get());
        auto update = [&](int t) {
            XMVECTOR eyepos = XMVectorSet(0.0, 3.0, -8.0, 0.0);  //眼睛位置
            XMVECTOR target = XMVectorSet(0.0, 0.0, 0.0, 0.0);   //目标位置
            XMVECTOR up = XMVectorSet(0.0, 1.0, 0.0, 0.0);       //"上"方向向量

            XMMATRIX W = XMMatrixRotationY(t * XM_PI * 0.0125f);  //Local Space -> World Space 绕Y轴旋转
            XMMATRIX V = XMMatrixLookAtLH(eyepos, target, up);    //观察矩阵 View
            XMMATRIX P = XMMatrixPerspectiveFovLH(XM_PIDIV4, 1.0f * window->width / window->height, 1.0f, 1000.0f); 
                                                    //透视投影(场视角, 宽高比, 近平面, 远平面)
            XMMATRIX wvp = W*V*P;      
            auto wvpt = XMMatrixTranspose(wvp);  //hlsl特(bu)性(g)，需要转置一下
            rp->UpdateSubResource(cb.Get(), reinterpret_cast<float*>(&wvpt));
        };
        rp->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        auto swapchain = Utility::ReferPtr<SwapChain>::New(device.Get(), window.Get());
        rp->SetTarget(swapchain->GetRTT());
        rp->SetViewport({0, 0, window->width, window->height});

        int t = 0;
        MessageLoop(144, [&](){
            rp->Clear({0.0f, 0.0f, 0.0f, 0.0f});
            update(t++);
            rp->DrawIndex(0, sizeof(indexs) / sizeof(int));
            rp->ImmdiateRender();
            swapchain->Present();
        });
      
    }
}