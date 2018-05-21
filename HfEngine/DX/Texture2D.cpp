#include "..\Include\Texture2D.h"

void Texture2D::CreateViews(D3DDevice *device) {
    D3D11_TEXTURE2D_DESC desc;
    native_texture2d->GetDesc(&desc);
    HRESULT hr = S_FALSE;
    if ((desc.BindFlags & D3D11_BIND_SHADER_RESOURCE) &&
        FAILED(hr = device->native_device->CreateShaderResourceView(reinterpret_cast<ID3D11Resource *>(native_texture2d.Get()),
            0, &native_shader_resource_view))) {
        MAKE_ERRMSG<std::runtime_error>("Failed to create shader resource view, Error code:", hr);
    }
    // only shader resource view
}

void Texture2D::Initialize(D3DDevice * device, const std::wstring & filename) {
    D3DX11_IMAGE_LOAD_INFO info;
    RtlZeroMemory(&info, sizeof info);
    info.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    info.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    info.MipFilter = D3DX11_FILTER_LINEAR;
    info.MipLevels = D3DX11_DEFAULT;
    HRESULT hr;
    if (FAILED(hr = D3DX11CreateTextureFromFileW(device->native_device.Get(), filename.c_str(), &info, nullptr,
        reinterpret_cast<ID3D11Resource **>(native_texture2d.GetAddressOf()), 0))) {
        MAKE_ERRMSG<LoadTextureFailed>("Failed to create D3D Texture2D from file, Error code:", hr);
    }
    D3D11_TEXTURE2D_DESC desc;
    native_texture2d->GetDesc(&desc);
    _width = desc.Width;
    _height = desc.Height;

    CreateViews(device);
}

void Texture2D::Initialize(D3DDevice * device, int w, int h, const void *init_data) {
    _width = w, _height = h;

    D3D11_TEXTURE2D_DESC td;
    RtlZeroMemory(&td, sizeof td);
    td.Width = width;
    td.Height = height;
    td.MipLevels = td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.SampleDesc.Quality = 0;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    HRESULT hr;
    
    D3D11_SUBRESOURCE_DATA data;
    if (init_data) {
        memset(&data, 0, sizeof data);
        data.pSysMem = init_data;
        data.SysMemPitch = w * 4;
    }
    if (FAILED(hr = device->native_device->CreateTexture2D(&td, init_data ? &data : nullptr, &native_texture2d))) {
        MAKE_ERRMSG<std::runtime_error>("Failed to create D3D Texture2D, Error code:", hr);
    }

    CreateViews(device);
}

void Texture2D::CreateFromNativeTexture2D(ID3D11Texture2D * tex) {
    native_texture2d = tex;
    native_shader_resource_view = nullptr;
}

void RTT::Initialize(Texture2D *tex) {
    texture = tex;
    D3D11_TEXTURE2D_DESC desc;
    texture->native_texture2d->GetDesc(&desc);
    ComPtr<ID3D11Device> native_device;
    texture->native_texture2d->GetDevice(&native_device);
    auto hr = native_device->CreateRenderTargetView((reinterpret_cast<ID3D11Resource *>(texture->native_texture2d.Get())), 
        nullptr, &native_rtt_view);
    if (FAILED(hr)) {
        MAKE_ERRMSG<std::runtime_error>("Fail to create a Render-To-Texture, Error code:", hr);
    };

    D3D11_TEXTURE2D_DESC td;
    RtlZeroMemory(&td, sizeof td);
    td.Width = desc.Width;
    td.Height = desc.Height;
    td.MipLevels = td.ArraySize = 1;
    td.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    td.SampleDesc.Count = 1;
    td.SampleDesc.Quality = 0;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
   
    if (FAILED(hr = native_device->CreateTexture2D(&td, nullptr, &native_stencil_buffer))) {
        MAKE_ERRMSG<std::runtime_error>("Failed to create stencil buffer, Error code:", hr);
    }
    if (FAILED(hr = native_device->CreateDepthStencilView(native_stencil_buffer.Get(), 0,
        &native_stencil_view))) {
        MAKE_ERRMSG<std::runtime_error>("Failed to create depth stencil view, Error code:", hr);
    }
}

void RTT::CreateFromNativeTexture2D(ID3D11Texture2D * t) {
    if (!t)
        throw std::invalid_argument("RTT::CreateFromNativeTexture2D Invalid Argument: NULL native_object");
    D3D11_TEXTURE2D_DESC desc;
    t->GetDesc(&desc);
    ComPtr<ID3D11Device> native_device;
    t->GetDevice(&native_device);
    auto hr = native_device->CreateRenderTargetView((reinterpret_cast<ID3D11Resource *>(t)),
        nullptr, &native_rtt_view);
    if (FAILED(hr)) {
        MAKE_ERRMSG<std::runtime_error>("Fail to create a Render-To-Texture, Error code:", hr);
    };

    D3D11_TEXTURE2D_DESC td;
    RtlZeroMemory(&td, sizeof td);
    td.Width = desc.Width;
    td.Height = desc.Height;
    td.MipLevels = td.ArraySize = 1;
    td.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    td.SampleDesc.Count = 1;
    td.SampleDesc.Quality = 0;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    if (FAILED(hr = native_device->CreateTexture2D(&td, nullptr, &native_stencil_buffer))) {
        MAKE_ERRMSG<std::runtime_error>("Failed to create stencil buffer, Error code:", hr);
    }
    if (FAILED(hr = native_device->CreateDepthStencilView(native_stencil_buffer.Get(), 0,
        &native_stencil_view))) {
        MAKE_ERRMSG<std::runtime_error>("Failed to create depth stencil view, Error code:", hr);
    }
}

namespace Ext { namespace DX{
    namespace Texture {
        VALUE klass_texture;
        VALUE klass_texture2d;
        VALUE klass_rtt;

        template<class T>
        static void Delete(T *s) {
            s->SubRefer();
        }

        static VALUE T2D_initialize(int argc, VALUE *argv, VALUE self) {
            if(argc < 2 || argc > 4)
                rb_raise(rb_eArgError, "Texutre2D::initialize : expected (2..4) args( (device, filename) or (device, width, height, [init_data])) but got %d", argc);
            auto tex = GetNativeObject<::Texture2D>(self);
            if ((!rb_obj_is_kind_of(argv[0], Ext::DX::D3DDevice::klass))) {
                rb_raise(rb_eArgError,
                    "Texture2D::initialize: First param should be a DX::D3DDevice");
            }
            auto device = GetNativeObject<::D3DDevice>(argv[0]);
            try {
                if (rb_obj_is_kind_of(argv[1], rb_cString)) {
                    std::wstring filename;
                    U8ToU16(rb_string_value_cstr(&argv[1]), filename, CP_UTF8);
                    tex->Initialize(device, filename);
                }
                else {
                    void *data = nullptr;
                    if (argc == 4) {
                        if (rb_obj_is_kind_of(argv[3], rb_cInteger)) {
                            data = (void*)FIX2PTR(argv[3]);
                        }
                        else if (rb_obj_is_kind_of(argv[3], rb_cString)) {
                            data = (void*)rb_string_value_ptr(&argv[3]);
                        }
                    }
                    tex->Initialize(device, FIX2INT(argv[1]), FIX2INT(argv[2]), data);
                }
            }
            catch (LoadTextureFailed le) {
                rb_raise(rb_eRuntimeError, le.what());
            }
            catch (std::runtime_error re) {
                rb_raise(rb_eRuntimeError, re.what());
            }
            return self;
        }
        static VALUE T2D_width(VALUE self) {
            return INT2FIX(GetNativeObject<::Texture2D>(self)->width);
        }
        static VALUE T2D_height(VALUE self) {
            return INT2FIX(GetNativeObject<::Texture2D>(self)->height);
        }

        static VALUE rtt_initialize(VALUE self, VALUE tex) {
            CheckArgs({ tex }, {klass_texture2d});
            auto rtt = GetNativeObject<::RTT>(self);
            auto t = GetNativeObject<::Texture2D>(tex);
            rtt->Initialize(t);
            rb_iv_set(self, "@texture", tex);
            return self;
        }
        static VALUE rtt_get_texture(VALUE self) {
            return rb_iv_get(self, "@texture");
        }
        void Init() {
            klass_texture = rb_define_class_under(module, "Texture", rb_cObject);
            rb_include_module(klass_texture, module_release);
            klass_texture2d = rb_define_class_under(module, "Texture2D", klass_texture);

            //Texture2D
            rb_define_alloc_func(klass_texture2d, [](VALUE k)->VALUE{
                auto t = new ::Texture2D;
                t->AddRefer();
                return Data_Wrap_Struct(k, nullptr, Delete<::Texture2D>, t);
            });
            rb_define_method(klass_texture2d, "initialize", (rubyfunc)T2D_initialize, -1);
            rb_define_method(klass_texture2d, "width", (rubyfunc)T2D_width, 0);
            rb_define_method(klass_texture2d, "height", (rubyfunc)T2D_height, 0);

            //RTT
            klass_rtt = rb_define_class_under(module, "RTT", rb_cObject);
            rb_include_module(klass_rtt, module_release);
            rb_define_alloc_func(klass_rtt, [](VALUE k)->VALUE {
                auto t = new ::RTT;
                t->AddRefer();
                return Data_Wrap_Struct(k, nullptr, Delete<::RTT>, t);
            });
            rb_define_method(klass_rtt, "initialize", (rubyfunc)rtt_initialize, 1);
            rb_define_method(klass_rtt, "texture", (rubyfunc)rtt_get_texture, 0);
        }
    }
}}