#include <Core.h>

HFENGINE_NAMESPACE_BEGIN

#ifdef _DEBUG
#define D3D_DEVICE_CREATE_FLAG D3D11_CREATE_DEVICE_DEBUG
#else
#define D3D_DEVICE_CREATE_FLAG 0
#endif

void GDevice::Initialize() {
    HRESULT hr;
    D3D_FEATURE_LEVEL flevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3
    };
    if (FAILED(hr = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, D3D_DEVICE_CREATE_FLAG,
        flevels, ARRAYSIZE(flevels), D3D11_SDK_VERSION, &native_device, 0, &native_immcontext))) {
        if (FAILED(hr = D3D11CreateDevice(0, D3D_DRIVER_TYPE_WARP, 0, D3D_DEVICE_CREATE_FLAG,
            flevels, ARRAYSIZE(flevels), D3D11_SDK_VERSION, &native_device, 0, &native_immcontext))) {
            //if all fail    
            THROW_ERROR_CODE(std::runtime_error, "Fail to create D3D11 Device, Error code:", hr);
        }
    }
    ID3D11Device* d = native_device.Get();
    d->QueryInterface(__uuidof(IDXGIDevice), &native_dxgi_device);
    native_dxgi_device->GetParent(__uuidof(IDXGIAdapter), &native_dxgi_adapter);
    native_dxgi_adapter->GetParent(__uuidof(IDXGIFactory), &native_dxgi_factory);
    //native_device->GetImmediateContext(&native_immcontext);
}

void GDevice::QueryMonitorInfo(MonitorInfo* info) {
    info->width = GetSystemMetrics(SM_CXSCREEN);
    info->height = GetSystemMetrics(SM_CYSCREEN);

    DEVMODE dvm;
    EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dvm);
    info->refresh_frequency = dvm.dmDisplayFrequency;
}

void GDevice::QueryAdapterInfo(DXGI_ADAPTER_DESC* d) {
    native_dxgi_adapter->GetDesc(d);
}

std::vector<std::wstring> GDevice::EnumAdapters() {
    UINT i = 0;
    IDXGIOutput* opt;
    std::vector<std::wstring> outputs;
    DXGI_OUTPUT_DESC d;
    while (native_dxgi_adapter->EnumOutputs(i, &opt) != DXGI_ERROR_NOT_FOUND) {
        opt->GetDesc(&d);
        outputs.push_back(d.DeviceName);
        i++;
    }
    return outputs;
}
#undef D3D_DEVICE_CREATE_FLAG

//extension:
thread_local RClass* ClassGDevice;
thread_local RClass* ClassNoDeviceError;
std::unique_ptr<GDevice> InstanceOfGDevice;
mrb_value ObjectInstanceOfGDevice;

mrb_data_type ClassGDeviceDataType = mrb_data_type{ "GDevice", [](mrb_state* mrb, void* ptr) {
} };

/*[DOCUMENT]
method: HEG::GDevice::instance -> device : GDevice
note: Get the signle instance of GDevice Class.
*/
mrb_value GDevice_instance(mrb_state* mrb, mrb_value self) {
    //return mrb_obj_value(mrb_data_object_alloc(mrb, ClassGDevice, GDevice::GetInstance(), &ClassGDeviceDataType));
    return ObjectInstanceOfGDevice;
}

/*[DOCUMENT]
method: HEG::GDevice::create -> device : GDevice
note: Create the single instance of GDevice Class.
*/
mrb_value GDevice_create(mrb_state* mrb, mrb_value self) {
    if(!InstanceOfGDevice) { 
        GDevice::CreateInstance();
        ObjectInstanceOfGDevice = mrb_obj_value(mrb_data_object_alloc(mrb, ClassGDevice, GDevice::GetInstance(), &ClassGDeviceDataType));
    }
    return ObjectInstanceOfGDevice;
}

/*
method: HEG::Device#monitor_info -> {:width => width, :height => height, :refresh_rate => refresh_rate} : Hash
note: Get some information of the monitor.
*/
mrb_value GDevice_monitor_info(mrb_state* mrb, mrb_value self) {
    MonitorInfo mi;
    mrb_value res = mrb_hash_new(mrb);
    if (!InstanceOfGDevice) {
        mrb_raise(mrb, ClassNoDeviceError, "No instance of GDvice");
        return res;
    }
    GDevice::GetInstance()->QueryMonitorInfo(&mi);
    mrb_hash_set(mrb, res, mrb_symbol_value(mrb_intern_cstr(mrb, "width")), mrb_fixnum_value(mi.width));
    mrb_hash_set(mrb, res, mrb_symbol_value(mrb_intern_cstr(mrb, "height")), mrb_fixnum_value(mi.height));
    mrb_hash_set(mrb, res, mrb_symbol_value(mrb_intern_cstr(mrb, "refresh_rate")), mrb_fixnum_value(mi.refresh_frequency));
    return res;
}

/*
method: HEG::Device#adapter_info -> {:description => description, :video_mem => video_mem, :sys_mem => sys_mem, :shared_mem => shared_mem} : Hash
note: Get some information of the adapter.(Adapter is usually refering the GPU)
*/
mrb_value GDevice_adapter_info(mrb_state* mrb, mrb_value self) {
    DXGI_ADAPTER_DESC ai;
    mrb_value res = mrb_hash_new(mrb);
    if (!InstanceOfGDevice) {
        mrb_raise(mrb, ClassNoDeviceError, "No instance of GDvice");
        return res;
    }
    GDevice::GetInstance()->QueryAdapterInfo(&ai);
    std::string desc;
    U16ToU8(ai.Description, desc);
    mrb_hash_set(mrb, res, mrb_symbol_value(mrb_intern_cstr(mrb, "description")), mrb_str_new_cstr(mrb, desc.c_str()));
    mrb_hash_set(mrb, res, mrb_symbol_value(mrb_intern_cstr(mrb, "sys_mem")), mrb_fixnum_value(ai.DedicatedSystemMemory));
    mrb_hash_set(mrb, res, mrb_symbol_value(mrb_intern_cstr(mrb, "video_mem")), mrb_fixnum_value(ai.DedicatedVideoMemory));
    mrb_hash_set(mrb, res, mrb_symbol_value(mrb_intern_cstr(mrb, "shared_mem")), mrb_fixnum_value(ai.SharedSystemMemory));
    return res;
}

bool InjectGDeviceExtension() {
    const RubyVM* vm = currentRubyVM;
    RClass* HEG = mrb_define_module(vm->GetRuby(), "HEG");
    RClass* Object = vm->GetRuby()->object_class;
    ClassGDevice = mrb_define_class_under(vm->GetRuby(), HEG, "GDevice", Object);
    ClassNoDeviceError = mrb_define_class_under(vm->GetRuby(), HEG, "NoDeviceError", vm->GetRuby()->eStandardError_class);
    mrb_define_class_method(vm->GetRuby(), ClassGDevice, "instance", GDevice_instance, MRB_ARGS_NONE());
    mrb_define_class_method(vm->GetRuby(), ClassGDevice, "create", GDevice_create, MRB_ARGS_NONE());
    mrb_define_method(vm->GetRuby(), ClassGDevice, "monitor_info", GDevice_monitor_info, MRB_ARGS_NONE());
    mrb_define_method(vm->GetRuby(), ClassGDevice, "adapter_info", GDevice_adapter_info, MRB_ARGS_NONE());
    return true;
}

HFENGINE_NAMESPACE_END