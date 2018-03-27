#pragma once
#include "extension.h"
#include "DX.h"

class D3DDevice : public Utility::ReferredObject {
public:
    ComPtr<ID3D11Device> native_device;
    ComPtr<IDXGIDevice> native_dxgi_device;
    ComPtr<IDXGIAdapter> native_dxgi_adapter;
    ComPtr<IDXGIFactory> native_dxgi_factory;
    ComPtr<ID3D11DeviceContext> native_immcontext;

    D3DDevice(){}
    D3DDevice(D3D_DRIVER_TYPE t) {
        Initialize(t);
    }

    void Initialize(D3D_DRIVER_TYPE t);
    void UnInitialize();

    virtual void Release() {
        UnInitialize();
    }

    void QueryAdapterInfo(DXGI_ADAPTER_DESC *desc);
    std::vector<std::wstring> EnumAdapters();
};

namespace Ext {
    namespace DX {
        namespace D3DDevice {
            extern VALUE klass;
            void Init();
        }
} }