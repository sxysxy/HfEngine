#include "..\Include\D3DDevice.h"

#ifdef _DEBUG
#define D3D_DEVICE_CREATE_FLAG D3D11_CREATE_DEVICE_DEBUG
#else
#define D3D_DEVICE_CREATE_FLAG 0
#endif
#pragma comment(lib, "d3d11.lib")

void D3DDevice::Initialize(D3D_DRIVER_TYPE t) {
    if(t > 5)
        throw std::invalid_argument("Invalid D3DDevice Type");
    HRESULT hr;
    if (FAILED(hr = D3D11CreateDevice(0, t, 0, D3D_DEVICE_CREATE_FLAG,
        0, 0, D3D11_SDK_VERSION, &native_device, 0, &native_immcontext))) {
        MAKE_ERRMSG<std::runtime_error>("Fail to create D3D11 Device, Error code:", hr);
    }
    ID3D11Device *d = native_device.Get();
    d->QueryInterface(__uuidof(IDXGIDevice), &native_dxgi_device);
    native_dxgi_device->GetParent(__uuidof(IDXGIAdapter), &native_dxgi_adapter);
    native_dxgi_adapter->GetParent(__uuidof(IDXGIFactory), &native_dxgi_factory);
}

void D3DDevice::UnInitialize() {
    native_device.ReleaseAndGetAddressOf();
    native_dxgi_device.ReleaseAndGetAddressOf();
    native_dxgi_adapter.ReleaseAndGetAddressOf();
    native_dxgi_factory.ReleaseAndGetAddressOf();
    native_immcontext.ReleaseAndGetAddressOf();
}

void D3DDevice::QueryAdapterInfo(DXGI_ADAPTER_DESC *d) {
    native_dxgi_adapter->GetDesc(d);
}

void D3DDevice::QueryMonitorInfo(MonitorInfo *info) {
    info->width = GetSystemMetrics(SM_CXSCREEN);
    info->height = GetSystemMetrics(SM_CYSCREEN);
    
    DEVMODE dvm;
    EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dvm);
    info->refresh_frequency = dvm.dmDisplayFrequency;
}

std::vector<std::wstring> D3DDevice::EnumAdapters() {
    UINT i = 0;
    IDXGIOutput *opt;
    std::vector<std::wstring> outputs;
    DXGI_OUTPUT_DESC d;
    while (native_dxgi_adapter->EnumOutputs(i, &opt) != DXGI_ERROR_NOT_FOUND) {
        opt->GetDesc(&d);
        outputs.push_back(d.DeviceName);
        i++;
    }
    return outputs;
}

void D3DDevice::AcquireImmdiateContext(bool occupy) {
    if (occupy) {
        immcontext_lock.lock();
    }
    else immcontext_lock.unlock();
}



namespace Ext {
    namespace DX {
        namespace D3DDevice {
            VALUE klass;
            void Delete(::D3DDevice *d) {
                d->SubRefer();
            }

            VALUE New(VALUE klass) {
                auto d = new ::D3DDevice;
                d->AddRefer();
                return Data_Wrap_Struct(klass, nullptr, Delete, d);
            }

            static VALUE initialize(int argc, VALUE *argv, VALUE self) {
                if (argc > 1) {
                    rb_raise(rb_eArgError, "D3DDevice#intialize:Wrong number of arguments, expecting (0..1), but given %d", argc);
                }

                D3D_DRIVER_TYPE t;
                if (argc == 0)t = D3D_DRIVER_TYPE_HARDWARE;
                if (argc == 1)t = (D3D_DRIVER_TYPE)FIX2INT(argv[0]);
                auto d = GetNativeObject<::D3DDevice>(self);
                try {
                    d->Initialize(t);
                }
                catch (std::runtime_error err) {
                    rb_raise(rb_eRuntimeError, err.what());
                }
                catch (std::invalid_argument err) {
                    rb_raise(rb_eArgError, err.what());
                }
                return self;
            }

            static VALUE query_adapter_info(VALUE self) {
                auto sc = GetNativeObject<::D3DDevice>(self);
                DXGI_ADAPTER_DESC d;
                sc->QueryAdapterInfo(&d);
                VALUE info = rb_hash_new();
                std::string s;
                U16ToU8(d.Description, s, CP_UTF8);
                rb_hash_aset(info, ID2SYM(rb_intern("description")), rb_str_new(s.c_str(), (int)s.length()));
                rb_hash_aset(info, ID2SYM(rb_intern("dedicated_vide_memory")), LONG2NUM((long)(d.DedicatedVideoMemory / 1024 / 1024)));
                rb_hash_aset(info, ID2SYM(rb_intern("delicated_system_memory")), LONG2NUM((long)(d.DedicatedSystemMemory / 1024 / 1024)));
                rb_hash_aset(info, ID2SYM(rb_intern("shared_system_memory")), LONG2NUM((long)(d.SharedSystemMemory / 1024 / 1024)));
                return info;

            }
            
            static VALUE enum_adapters(VALUE self) {
                auto d = GetNativeObject<::D3DDevice>(self);
                auto os = d->EnumAdapters();
                auto arr = rb_ary_new();
                for (size_t i = 0; i < os.size(); i++) {
                    std::string s;
                    U16ToU8(os[i].c_str(), s);
                    rb_ary_push(arr, rb_str_new_cstr(s.c_str()));
                }
                return arr;
            }

            static VALUE query_monitor_info(VALUE self) {
                auto d = GetNativeObject<::D3DDevice>(self);
                MonitorInfo info;
                d->QueryMonitorInfo(&info);
                VALUE h = rb_hash_new();
                rb_hash_aset(h, RB_ID2SYM(rb_intern("width")), INT2FIX(info.width));
                rb_hash_aset(h, RB_ID2SYM(rb_intern("height")), INT2FIX(info.height));
                rb_hash_aset(h, RB_ID2SYM(rb_intern("refresh_frequency")), INT2FIX(info.refresh_frequency));
                return h;
            }
             
            static VALUE acquire_immcontext(VALUE self, VALUE occupy) {
                auto d = GetNativeObject<::D3DDevice>(self);
                d->AcquireImmdiateContext(occupy == Qtrue);
                return self;
            }

            void Init() {
                klass = rb_define_class_under(module, "D3DDevice", rb_cObject);
                rb_include_module(klass, module_release);
                rb_define_alloc_func(klass, New);
                rb_define_const(module, "HARDWARE_DEVICE", INT2FIX(D3D_DRIVER_TYPE_HARDWARE));
                rb_define_const(module, "SIMULATED_DEVICE", INT2FIX(D3D_DRIVER_TYPE_WARP));
                rb_define_method(klass, "initialize", (rubyfunc)initialize, -1);
                rb_define_method(klass, "query_adapter_info", (rubyfunc)query_adapter_info, 0);
                rb_define_method(klass, "query_monitor_info", (rubyfunc)query_monitor_info, 0);
                rb_define_method(klass, "enum_adapters", (rubyfunc)enum_adapters, 0);
                rb_define_method(klass, "acquire_immcontext", (rubyfunc)acquire_immcontext, 1);
            }
        }
    }
}