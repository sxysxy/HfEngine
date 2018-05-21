#pragma once
#include "extension.h"
#include "DX.h"

class RenderPipeline;

struct MonitorInfo {
    int width, height;
    int refresh_frequency;
};

class D3DDevice : public Utility::ReferredObject {
public:
    ComPtr<ID3D11Device> native_device;
    ComPtr<IDXGIDevice> native_dxgi_device;
    ComPtr<IDXGIAdapter> native_dxgi_adapter;
    ComPtr<IDXGIFactory> native_dxgi_factory;
    ComPtr<ID3D11DeviceContext> native_immcontext;
    std::mutex immcontext_lock;

    void Initialize();
    void UnInitialize();

    virtual void Release() {
        UnInitialize();
    }

    void QueryAdapterInfo(DXGI_ADAPTER_DESC *desc);
    void QueryMonitorInfo(MonitorInfo *info);
    std::vector<std::wstring> EnumAdapters();

    void AcquireImmdiateContext(bool occupy = true);

    void VSDebugReport() {
#ifdef _DEBUG
         ComPtr<ID3D11Debug> debug;
         HRESULT hr = native_device->QueryInterface(__uuidof(ID3D11Debug), &debug);
         if (SUCCEEDED(hr)) {
             debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
         }
#else
        
#endif
    }
};

namespace Ext {
    namespace DX {
        namespace D3DDevice {
            extern VALUE klass;
            void Init();
        }
} }
