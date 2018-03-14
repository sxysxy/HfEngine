#include "stdafx.h"
#include "extension.h"
#include "HFWindow.h"
#include <regex>
#include <DX/D3DDevice.h>
#include <DX/SwapChain.h>
#include <DX/RenderPipeline.h>
#include <DX/D3DDeviceContext.h>
#include <DX/D3DTexture2D.h>
#include <DX/D3DBuffer.h>
#include <DX/Input.h>
#include <Graphics2D\G2DRenderer.h>
using namespace Utility;

int __cdecl cmain(wchar_t *path) {
    int argc; char **argv;
	ruby_sysinit(&argc, (char***)&argv);  //这个ruby_sysinit一定要有，哪怕不用命令行参数。
	{
		RUBY_INIT_STACK;
		ruby_init();
		Ext::ApplyExtensions();

	    auto buffer = ReferPtr<HFBuffer<wchar_t>>::New(MAX_PATH+10);
        GetModuleFileNameW(GetModuleHandle(0), buffer->ptr, MAX_PATH);
        int len = lstrlenW(buffer->ptr);
        int p;
        for (p = len - 1; ~p; p--) {
            if (buffer->ptr[p] == L'\\')break;
        }
        if (!path) {
            buffer->ptr[p + 1] = L'm';
            buffer->ptr[p + 2] = L'a';
            buffer->ptr[p + 3] = L'i';
            buffer->ptr[p + 4] = L'n';
            buffer->ptr[p + 5] = L'.';
            buffer->ptr[p + 6] = L'r';
            buffer->ptr[p + 7] = L'b';
            buffer->ptr[p + 8] = 0;
        }
        else {
            lstrcpyW(buffer->ptr, path);
            len = lstrlenW(buffer->ptr);
            for (p = len - 1; ~p; p--) {
                if (buffer->ptr[p] == L'\\')break;
            }
        }
        buffer->ptr[p] = 0;
        SetCurrentDirectory(buffer->ptr);
        buffer->ptr[p] = L'\\';

        std::string filename;
        Ext::U16ToU8(buffer->ptr, filename, CP_UTF8);
		VALUE script = rb_str_new_cstr(filename.c_str());
		int state = 0;
		rb_load_protect(script, 0, &state);
		if (state) {
            VALUE errorinfo = rb_errinfo();
            rb_funcall(rb_mKernel, rb_intern("show_console"), 0);

            VALUE backtrance = rb_funcall(rb_make_backtrace(), rb_intern("to_s"), 0);
            rb_funcall(rb_mKernel, rb_intern("puts"), 1, backtrance);
            rb_funcall(rb_mKernel, rb_intern("puts"), 1, errorinfo);
            rb_eval_string("STDOUT.flush");
            system("pause");
		}
		return state;
	}

}

void JustTest4();
wchar_t path_buffer[MAX_PATH+10];
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmd, int nShow) {
    MSVCRT::GetFunctions();
    CoInitialize(nullptr);
    Input::Initialize();

    //#define RUBY_ENTRY 
#ifdef RUBY_ENTRY
    if (GetFileAttributes(TEXT("main.rb")) == INVALID_FILE_ATTRIBUTES) {
        if (MessageBox(0, TEXT("main.rb not found, choose a script?."), TEXT("Tip"), MB_YESNO) == IDYES) {
            OPENFILENAMEW op;
            ZeroMemory(&op, sizeof op);
            op.lStructSize = sizeof(op);
            op.lpstrFilter = L"Ruby script files(.rb)\0*.rb\0All files(*.*)\0*.*\0\0";
            op.lpstrInitialDir = L"./";
            op.lpstrFile = path_buffer;
            op.nMaxFile = MAX_PATH;
            op.nFilterIndex = 0;
            op.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
            GetOpenFileNameW(&op);
            return cmain(path_buffer);
        }
        else
            return 0;
    }
    else {
        return cmain(nullptr);
    }
#else
    JustTest4();
#endif
}

void JustTest4() {
    auto window = ReferPtr<HFWindow>::New(L"emm...", 500, 500);
    window->SetFixed(true);
    window->Show();
    auto device = ReferPtr<D3DDevice>::New(D3D_DRIVER_TYPE_HARDWARE);
    auto swap_chain = ReferPtr<SwapChain>::New(device.Get(), window.Get());

    auto renderer = ReferPtr<G2D::Renderer>::New(device.Get(), window.Get());
    renderer->SetRenderTarget(&swap_chain->backbuffer);

    SleepFPSTimer timer;
    timer.Restart(60);
    MSG msg;
    while (true) {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) > 0) {
            if (msg.message == WM_QUIT)break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            renderer->FillRect({ 100, 100, 200, 200 }, { 1.0f, 0.0f, 1.0f, 1.0f });
            renderer->ExecuteRender();
            swap_chain->Present();
            timer.Await();
        }
    }
}

void JustTest2() {
    auto window = ReferPtr<HFWindow>::New(L"恋恋!", 300, 300);
    window->SetFixed(true);
    window->Show();
    auto device = ReferPtr<D3DDevice>::New(D3D_DRIVER_TYPE_HARDWARE);
    auto swap_chain = ReferPtr<SwapChain>::New(device.Get(), window.Get());
    auto context = ReferPtr<D3DDeviceContext>::New(device.Get());
    auto texture = ReferPtr<D3DTexture2D>::New(device.Get(), L"../CommonFiles/300px-Komeiji Koishi.jpg", false);
    auto pipeline = ReferPtr<RenderPipeline>::New();
    pipeline->vshader = VertexShader::LoadHLSLFile(device.Get(), L"texture_vs.shader");
    pipeline->pshader = PixelShader::LoadHLSLFile(device.Get(), L"texture_ps.shader");
    pipeline->SetInputLayout(device.Get(),
        std::initializer_list<std::string>({ "POSITION", "TEXCOORD" }).begin(),
        std::initializer_list<DXGI_FORMAT>({ DXGI_FORMAT_R32G32_FLOAT,  DXGI_FORMAT_R32G32_FLOAT }).begin(),
        2);
    context->BindPipeline(pipeline.Get());
    auto sampler = ReferPtr<D3DSampler>::New();
    sampler->UseDefault();
    sampler->CreateState(device.Get());
    context->BindShaderSampler(0, sampler.Get(), SHADERS_APPLYTO_PSHADER);
    context->BindShaderResource(0, texture.Get(), SHADERS_APPLYTO_PSHADER);
    struct Vertex { float pos[2], tex[2]; };
    Vertex vecs[] = {
        {{-1.0f, -1.0f}, {0.0f, 1.0f}},
        {{-1.0f, 1.0f},  {0.0f, 0.0f}},
        {{1.0f, -1.0f},  {1.0f, 1.0f}},
        {{1.0f, 1.0f},   {1.0f, 0.0f}}
    }; ////左下， 左上， 右下， 右上
    auto vbuffer = ReferPtr<D3DVertexBuffer>::New(device.Get(), sizeof vecs, reinterpret_cast<void *>(vecs));
    context->BindVertexBuffer(0, vbuffer.Get(), sizeof Vertex);
    context->SetRenderTarget(&swap_chain->backbuffer);
    context->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    context->SetViewport({0, 0, window->width, window->height});
    auto keyboard = ReferPtr<Input::Keyboard>::New(window->native_handle);
    auto rth = ReferPtr<RenderingThread>::New(device.Get(), swap_chain.Get(), 60);
    Utility::SleepFPSTimer timer;
    timer.Restart(60);
    MSG msg;
    while (true) {
        if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE) > 0){
            if(msg.message == WM_QUIT)break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            context->ClearRenderTarget(&swap_chain->backbuffer, 
                std::initializer_list<FLOAT>({0.0f, 0.0f, 0.0f, 0.0f}).begin());
            context->Draw(0, 4);
            context->FinishiCommandList();
            rth->PushCommandList(context.Get());
            timer.Await();
        }
    }
    rth->Terminate();
}

void JustTest3() {
    auto window = ReferPtr<HFWindow>::New(L"Deep Dark Fantasy", 600, 600);
    window->SetFixed(true);
    window->Show();
    auto device = ReferPtr<D3DDevice>::New(D3D_DRIVER_TYPE_HARDWARE);
    auto swap_chain = ReferPtr<SwapChain>::New(device.Get(), window.Get(),false, true);
    auto context = ReferPtr<D3DDeviceContext>::New(device.Get());
    
    auto rth = ReferPtr<RenderingThread>::New(device.Get(), swap_chain.Get(), 60);
    SleepFPSTimer timer;
    timer.Restart(60);
    MSG msg;
    while (true) {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            if(msg.message == WM_QUIT)break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        timer.Await();
    }
    rth->Terminate();
}
