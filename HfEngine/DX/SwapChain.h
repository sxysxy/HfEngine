#pragma once

#include <stdafx.h>
#include <HFWindow.h>
#include <referptr.h>
#include "D3DDevice.h"
#include "D3DTexture2D.h"

class SwapChain : public Utility::ReferredObject {
    ComPtr<ID3D11Device> native_device;

public:
    ComPtr<IDXGISwapChain> native_swap_chain;
    bool stenciled;
    int vsync;
    D3DTexture2D backbuffer;

    static const int VSYNC_NO = 0;
    static const int VSYNC_1_BLANK = 1;
    static const int VSYNC_2_BLANK = 2;
    static const int VSYNC_3_BLANK = 3;
    static const int VSYNC_4_BLANK = 4;

    SwapChain() {
        stenciled = false;
    }

    SwapChain(D3DDevice *device, HFWindow *wnd, bool fullscreen = false, bool _stenciled = true) : SwapChain() {
        Initialize(device, wnd, fullscreen, _stenciled);
    }
    void Initialize(D3DDevice *device, HFWindow *wnd, bool fullscreen, bool _stenciled);

    void UnInitialize() {
        native_swap_chain.ReleaseAndGetAddressOf();
                    //do not need to release native_device. delete operation will do it automacailly.
                    //UnIntialize dose not entirely equal to Destructor. eg. You can Uninitalize an object and then call Initialize.
        
        backbuffer.UnInitialize(); //this should be called...
    }
    ~SwapChain() {
        UnInitialize();
    }
    virtual void Release() {
        UnInitialize();
    };

    void Present() {
        assert(native_swap_chain);

        native_swap_chain->Present(vsync, 0);
    }

    void SetFullScreen(bool fullscreen = true) {
        assert(native_swap_chain);

        native_swap_chain->SetFullscreenState(fullscreen, nullptr);
    }

    void SetVsyncLevel(int level) {
        vsync = max(0, min(level, 4));
    }

    void Resize(int w, int h);
};

namespace Ext {
    namespace DX {
        namespace SwapChain {
            extern VALUE klass;

            void Init();
        }
    }
}
