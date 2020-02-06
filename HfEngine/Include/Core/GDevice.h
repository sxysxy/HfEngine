#pragma once
#include <ThirdParties.h>

HFENGINE_NAMESPACE_BEGIN

struct MonitorInfo {
    int width, height;
    int refresh_frequency;
};


//Single instance model, so do not extends ReferPtr
class GDevice {
public:
    ComPtr<ID3D11Device> native_device;
    ComPtr<IDXGIDevice> native_dxgi_device;
    ComPtr<IDXGIAdapter> native_dxgi_adapter;
    ComPtr<IDXGIFactory> native_dxgi_factory;
    ComPtr<ID3D11DeviceContext> native_immcontext;
    void Initialize();
    void Release() {
        native_device.ReleaseAndGetAddressOf();
        native_dxgi_adapter.ReleaseAndGetAddressOf();
        native_dxgi_factory.ReleaseAndGetAddressOf();
        native_immcontext.ReleaseAndGetAddressOf();
    }
    void QueryAdapterInfo(DXGI_ADAPTER_DESC* desc);

    void QueryMonitorInfo(MonitorInfo* info);
    std::vector<std::wstring> EnumAdapters();

    inline static void CreateInstance();
    inline static GDevice* GetInstance();
};
extern std::unique_ptr<GDevice> InstanceOfGDevice;

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