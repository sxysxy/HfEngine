#include <Tests.h>
#include <stdafx.h>
#include <HFWindow.h>
#include <D3DDevice.h>
#include <Shaders.h>
using namespace std;
using namespace Utility;

namespace Tests {
    void TestHelloWorld() {
        MessageBox(nullptr, L"2333", L"666", 0);
    }

    void MessageLoop(int fps, const std::function<void(void)> &callback) {
        MSG msg;

        SleepFPSTimer timer(60);
       
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
}