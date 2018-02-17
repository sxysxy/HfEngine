#pragma once

#include <stdafx.h>
#include "D3DDeviceContext.h"

class D3DDevice : public Utility::ReferredObject {
    
public:
    ComPtr<ID3D11Device> native_device;
    ComPtr<IDXGIDevice> native_dxgi_device;
    ComPtr<IDXGIAdapter> native_dxgi_adapter;
    ComPtr<IDXGIFactory> native_dxgi_factory;
    Utility::ReferPtr<D3DDeviceImmdiateContext> immcontext;

    D3DDevice(){}
    D3DDevice(D3D_DRIVER_TYPE t) {
        Initialize(t);
    }
    ~D3DDevice() {
        UnInitialize();
    }
    virtual void Release() {
        UnInitialize();
    };

    void Initialize(D3D_DRIVER_TYPE type);
    void UnInitialize() {
        native_device.ReleaseAndGetAddressOf();
        native_dxgi_device.ReleaseAndGetAddressOf();
        native_dxgi_factory.ReleaseAndGetAddressOf();
        immcontext.Release();
    }
    void QueryAdapterInfo(DXGI_ADAPTER_DESC *);
};

namespace Ext {
    namespace DX {
        namespace D3DDevice {
            extern VALUE klass;
            

            void Init();
        }
    }
}