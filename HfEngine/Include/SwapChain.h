#pragma once


#include <stdafx.h>
#include <HFWindow.h>
#include "D3DDevice.h"
#include "Texture2D.h"

class SwapChain : public Utility::ReferredObject {
    RTT backrtt;
public:
    ComPtr<IDXGISwapChain> native_swap_chain;

    SwapChain() {}

    SwapChain(D3DDevice *device, HFWindow *wnd, bool fullscreen = false) : SwapChain() {
        Initialize(device, wnd, fullscreen);
    }
    void Initialize(D3DDevice *device, HFWindow *wnd, bool fullscreen = false);

    void UnInitialize() {
        native_swap_chain.ReleaseAndGetAddressOf();
        //do not need to release native_device. delete operation will do it automacailly.
        //UnIntialize dose not entirely equal to Destructor. eg. You can Uninitalize an object and then call Initialize.

    }
    ~SwapChain() {
        UnInitialize();
    }
    virtual void Release() {
        UnInitialize();
    };

    void Present() {
        assert(native_swap_chain);
        native_swap_chain->Present(0, 0);
    }
    void SetFullScreen(bool fullscreen = true) {
        assert(native_swap_chain);
        native_swap_chain->SetFullscreenState(fullscreen, nullptr);
    }
    inline RTT *GetRTT() {
        return &backrtt;
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
