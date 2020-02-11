#pragma once
#include <ThirdParties.h>
#include <xy_async.h>

HFENGINE_NAMESPACE_BEGIN

struct MonitorInfo {
    int width, height;
    int refresh_frequency;
};
class GDevice;
extern std::unique_ptr<GDevice> InstanceOfGDevice;

class GDevice : public Utility::ReferredObject {
public:
    ComPtr<ID3D11Device> native_device;
    ComPtr<IDXGIDevice> native_dxgi_device;
    ComPtr<IDXGIAdapter> native_dxgi_adapter;
    ComPtr<IDXGIFactory> native_dxgi_factory;
    ComPtr<ID3D11DeviceContext> native_immcontext;
    SpinLock immcontext_lock;

    void Initialize();
    void Release() {
        native_immcontext.ReleaseAndGetAddressOf();
        native_dxgi_device.ReleaseAndGetAddressOf();
        native_dxgi_adapter.ReleaseAndGetAddressOf();
        native_dxgi_factory.ReleaseAndGetAddressOf();
#if _DEBUG
        ComPtr<ID3D11Debug> d3dDebug;
        HRESULT hr = native_device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(d3dDebug.GetAddressOf()));
        if (SUCCEEDED(hr)) {
            hr = d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
        }
        if (d3dDebug != nullptr)
            d3dDebug->Release();
#endif
        native_device.ReleaseAndGetAddressOf();
        InstanceOfGDevice.reset(nullptr);
    }
    void QueryAdapterInfo(DXGI_ADAPTER_DESC* desc);

    void QueryMonitorInfo(MonitorInfo* info);
    std::vector<std::wstring> EnumAdapters();

    inline static void CreateInstance();
    inline static GDevice* GetInstance();

    inline void Lock() { immcontext_lock.lock(); }
    inline void UnLock() { immcontext_lock.unlock(); }

};

void GDevice::CreateInstance() {
    if(!InstanceOfGDevice) {
        InstanceOfGDevice.reset(new GDevice());
        InstanceOfGDevice->Initialize();
    }
}
GDevice* GDevice::GetInstance() {
    return InstanceOfGDevice.get();
}

extern thread_local RClass* ClassGDevice;
extern thread_local RClass* ClassNoDeviceError;
bool InjectGDeviceExtension();

HFENGINE_NAMESPACE_END